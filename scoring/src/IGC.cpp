#include <cctype>
#include "Global.h"
#include "IGC.h"
#include "WriteText.h"

BOOL CheckDeclar = TRUE;
BOOL bNoENL      = FALSE;       // Allow file without ENL registration
BOOL bNoMOP      = FALSE;       // Do not use MOP
BOOL bMOPonly    = FALSE;       // Analyse only flights with MOP detection
BOOL no_GID      = FALSE;       // Allow file without Glider ID

char alphanumeric(int number);
void decimal_to_base36(int decimal, int base36[], int n);


struct sExtension
{ int   First ;
  int   Last ;
};

sExtension ENL = { 0 };
sExtension MOP = { 0 };
sExtension RPM = { 0 };
sExtension EON = { 0 };

int readfile_IGC(void)
{ // Initially set the IGC_name to the file name without path  
  // The IGC file name will be reconstructed from the logger header in CIGC::Check
  char *c = strrchr ( file_igc, PathDelimiter );
  if ( c != NULL )    c ++;
  else                c = file_igc;
  strcpy ( IGC_name, c );

  CIGC IGC;
  //IGC.CheckFile();
  return IGC.Read();
}



CIGC::CIGC(void)
{ F      = NULL;
  dFR_ID = 0;
  dPLT    = dGID = dGTY = dCCL = 0;
}


CIGC::~CIGC(void)
{ fclose ( (void**)&F );
}



void CIGC::CheckFile(void)
{
//  int   nLine = 0;
//  char Line[MAXLEN];
  int i;
  int n = 0;
  int b = 0;
  int nLine = 0;
  int nError = 0;
  int bBrecord = 0;
  char msg[MAXLEN];
  char CharSet[MAXLEN];
  char c;
  int bExit = debugIGC  ?  0  :  1;
  int bCritical = 0;

  for ( c = 'A';  c <= 'Z';   c ++ )
  { CharSet[n] = c;
    n ++;
  }
  for ( c = '0';  c <= '9';   c ++ )
  { CharSet[n] = c;
    n ++;
  }
  CharSet[n] = ' ';    n ++;
  CharSet[n] = '.';    n ++;
  CharSet[n] = ',';    n ++;
  CharSet[n] = ':';    n ++;
  CharSet[n] = ';';    n ++;
  CharSet[n] = '(';    n ++;
  CharSet[n] = ')';    n ++;
  CharSet[n] = '\n';   n ++;
  CharSet[n] = '\r';   n ++;
  CharSet[n] = '_';    n ++;
  CharSet[n] = '-';    n ++;
  CharSet[n] = '+';    n ++;
  CharSet[n] = '/';    n ++;
  CharSet[n] = '\'';   n ++;
  CharSet[n] = '\"';   n ++;

  FILE *F = fopen ( file_igc, "rt" );
  if ( F <= 0 )  report_error ( EC_IGC_NOT_OPEN, NULL ) ;

  Line[sizeof(Line)-1] = 0;
  while ( fgets ( Line, MAXLEN, F ) != NULL )
  { nLine++ ;
//if ( debugDT ) printf ( Line );

    if ( strlen ( Line ) >= MAXLEN-1 )
    { sprintf ( msg, "Line %d: too long", nLine );
      bCritical = TRUE;
      report_error ( EC_IGC_FILE_ERROR, msg, 0 ) ;
    } 
          
    // Make all uppercase and trim lines
    str_upper ( Line );
    str_trim_right ( Line );
    
    if ( *Line == 'C' )  continue;
    if ( *Line == 'H' )  continue;
    if ( *Line == 'L' )  continue;
    if ( *Line == 'G' )  continue;
    if ( *Line == 'A' &&  nLine != 1 )
    { sprintf ( msg, "Line %d: invalid A record", nLine );
      report_error ( EC_IGC_FILE_ERROR, msg, 0 ) ;
    }
    if ( nLine == 1  &&  *Line != 'A' )
    { sprintf ( msg, "First line not an A record" );
      bCritical = TRUE;
      report_error ( EC_IGC_FILE_ERROR, msg, 0 ) ;
    }
    if ( *Line == 'B' )  bBrecord = 1;
    if ( bBrecord )
    { if ( *Line == 'H' || 
           *Line == 'I' || 
           *Line == 'J' )
       { sprintf ( msg, "Line %d: Found %c record after first B record", nLine, Line[0] );
         report_error ( EC_IGC_FILE_ERROR, msg, 0 );
       }
      if ( *Line == 'C' )
      { sprintf ( msg, "Line %d: found C record after first B record", nLine );
        report_error ( EC_DECLAR_LINE, msg );
       }
    }
    
    if ( ( i = (int)strspn ( Line, CharSet ) ) >= 0   &&  Line[i] )
    { sprintf ( msg, "Line %d Invalid char found: %c (ASC %d)", nLine, Line[i], (unsigned char)Line[i] );
      bCritical = TRUE;
      report_error ( EC_IGC_FILE_ERROR, msg, 0 );
    }
  }

  if ( bCritical )
  { if ( debugIGC )  printf ( "Execution aborted" );
    report_error ( EC_IGC_FILE_ERROR, NULL );
  }

  if ( F )  fclose ( (void**)&F );
}





int CIGC::Read(void)
{ int   nLine = 0;
  int   j, k;
  int   C1 = 0;        // Set TRUE after first C record was read
  int   log_hrs, log_min, log_sec, log_altprs, log_altGPS ;
  int   hrs_prev=0, offset24h=0;
  int   log_latdeg, log_latmin, log_latsde ; 
  int   log_londeg, log_lonmin, log_lonsde ;
  int   log_accur,  log_datum;
  char  message[MAXLEN+1], st[MAXLEN+1], st2[MAXLEN+2];
  sFlightPoint  pF;
  CTask::sPoint pT;

  F = fopen ( file_igc, "rt" ) ;
  if ( F <= 0 )  report_error ( EC_IGC_NOT_OPEN, NULL ) ;
  Line[sizeof(Line)-1] = 0;
  while ( fgets ( Line, MAXLEN, F ) != NULL )
  { nLine++ ;
//if ( debugDT ) printf ( Line );

    if ( Line[MAXLEN] != 0 )
    { sprintf ( message, "line %d", nLine );
      report_error ( EC_IGC_TOO_LONG, message );
    } 
          
    // Make all uppercase and trim lines
    str_upper ( Line );
    str_trim_right ( Line );

    /**************************/
    /* Decode all IGC records */
    /**************************/
    
    switch( Line[0] )
    { case 'A':                         //***** Logger ID record
        strcpy_sec(st,  Line, 1,3);
        strcpy_sec(st2, Line, 4,5);
        k = 0 ;
        while ( k < NLOGGERS )
        { if ( !strcmp ( st, LoggerList[k].Code ) )
          { pLogger = &LoggerList[k];
            dFR_ID = 1;
            GetSN ( st, st2 );
            break;
          }
          k++;
        }
        if ( !dFR_ID )
        { strncpy ( LoggerX.Code, st, 3 );
          LoggerX.Code[3] = 0;
          pLogger = &LoggerX;     // Will be set later again. Here: avoid errors in GetSN
          GetSN ( st, st2 );
          // Set the error in Check(): this will ensure registering the error if no A record was present
        }
        break ;

      case 'B' :                        // Logger fix record
      { // Read basic fix data
        strcpy_sec(st, Line, 1, 2); log_hrs = atoi(st);
        strcpy_sec(st, Line, 3, 2); log_min = atoi(st);
        strcpy_sec(st, Line, 5, 2); log_sec = atoi(st);
        strcpy_sec(st, Line, 7, 2); log_latdeg = atoi(st);
        strcpy_sec(st, Line, 9, 5); log_latmin = atoi(st); log_latsde = Line[14];
        strcpy_sec(st, Line,15, 3); log_londeg = atoi(st);
        strcpy_sec(st, Line,18, 5); log_lonmin = atoi(st); log_lonsde = Line[23]; log_accur = Line[24];
        strcpy_sec(st, Line,25, 5); log_altprs = atoi(st);
        strcpy_sec(st, Line,30, 5); log_altGPS = atoi(st);

        // Check record validity
        if ( log_hrs    < 0   ||    log_hrs    >    24    ||
             log_min    < 0   ||    log_min    >    60    ||
             log_sec    < 0   ||    log_sec    >    60    ||
             log_latdeg < 0   ||    log_latdeg >    89    ||
             log_latmin < 0   ||    log_latmin > 60000    ||
             log_londeg < 0   ||    log_londeg >   180    ||
             log_lonmin < 0   ||    log_lonmin > 60000
           )
        { break;     // Invalid value: discard fix 
        }
   
        // Add 24h time offset when passing midnight
        if ( ( log_hrs == 0 )  &&  ( hrs_prev == 23 ) )  offset24h += 24;
        hrs_prev = log_hrs;
   
        // Convert fix data 
        if      (log_accur == 'A')   pF.f3D = YES; 
        else if (log_accur == 'V')
        { pF.f3D = NO;  break; }  // Do not record non 3D fixes
        else            break;    // Invalid record  !! check with ammendements !!

        pF.Time  = (double)(offset24h+log_hrs) + ( (double)log_min + (double)log_sec/60.0 )/60.0 ;
        if ( pF.Time < 0. )   break;
        pF.pt.Lat  = (double)log_latdeg + (double)log_latmin/(1000.0 * 60.0) ;   // in degrees
        pF.pt.Lon  = (double)log_londeg + (double)log_lonmin/(1000.0 * 60.0) ;   // in degrees
        pF.pt.Lat *= piDiv180 ;
        pF.pt.Lon *= piDiv180 ;
        if      ( log_latsde == 'S' ) pF.pt.Lat *= -1.0 ;
        else if ( log_latsde != 'N' ) break;             // Invalid value, discard fix
        if      ( log_lonsde == 'W' ) pF.pt.Lon *= -1.0 ;
        else if ( log_lonsde != 'E' ) break;             // Invalid value, discard fix
        pF.Altitude = log_altprs ;
        pF.AltiGPS  = log_altGPS ;
   
        // Read engine detection data (optional extension)
        pF.ENL  = 0 ; 
        pF.MOP  = 0 ; 
        pF.Flag = 0 ;
        if ( MOP.First )
        { strcpy_sec ( st, Line, MOP.First, MOP.Last-MOP.First+1 );  
          pF.MOP = atoi(st) ;
          if ( pF.MOP < 0   ||   pF.MOP > 999 ) break;    // Invalid value, discard fix
        }
        if ( ENL.First )
        { strcpy_sec ( st, Line, ENL.First, ENL.Last-ENL.First+1 );  
          pF.ENL = atoi(st) ;
          if ( pF.ENL < 0   ||   pF.ENL > 999 ) break;    // Invalid value, discard fix
        }
        if ( Engine.Detection  == ED_RPM )
        { strcpy_sec(st, Line, RPM.First, RPM.Last-RPM.First+1) ;  
          pF.ENL = atoi(st) ;
        }
        if ( Engine.Detection == ED_EON )
        { strcpy_sec(st, Line, EON.First, EON.Last-EON.First+1) ;
          if (st[0] == 'N') pF.ENL = 0;
          else              pF.ENL = 999;
        }   

        Trace.AddFix ( &pF );
        break ;
      }

      case 'C' :                        // Task records
      { if ( debugDT )
        { printf ( "Line %5d,  Reading C record %s\n", nLine, Line );
        }
        if ( C1 )
        { // Read next task point data
          strcpy_sec(st, Line, 1, 2) ; log_latdeg = atoi(st) ;
          strcpy_sec(st, Line, 3, 5) ; log_latmin = atoi(st) ; log_latsde = Line[8] ;
          strcpy_sec(st, Line, 9, 3) ; log_londeg = atoi(st) ;
          strcpy_sec(st, Line,12, 5) ; log_lonmin = atoi(st) ; log_lonsde = Line[17] ;

          // Convert task point coordinates
          pT.pt.Lat  = (double)log_latdeg + (double)log_latmin/(1000.0 * 60.0) ;   // in degrees
          pT.pt.Lon  = (double)log_londeg + (double)log_lonmin/(1000.0 * 60.0) ;   // in degrees
          pT.pt.Lat *= piDiv180 ;
          pT.pt.Lon *= piDiv180 ;
          if (log_latsde == 'S') pT.pt.Lat *= -1.0 ;
          if (log_lonsde == 'W') pT.pt.Lon *= -1.0 ;
          sprintf ( st, "%s", &Line[18] );
          st[LREC-1] = 0;
          strcpy ( pT.Name, st );

          if ( !Task.Append ( &pT ) )  break;
          if ( debugDT )
          { char st[256];
            sprintf ( st, "%s            ", Task.TPT[Task.nTPT-1].Name );
            st[12] = 0;
            printf ( "Point %d: %s %s\n", Task.nTPT, st, stPosTask(Task.nTPT-1) );
          }
        }
        else
        { // Read task declaration info
          strcpy_sec(Task.DeclareDate, Line, 1, 6);
          strcpy_sec(Task.DeclareTime, Line, 7, 6);
          strcpy_sec(st, Line,23, 2); sscanf(st,"%d", &Task.nPoints);
          C1 = TRUE;
          Task.nPoints += 2;     // Task points = waypoints + start & finish points
                                 // Note: LX20 has bug such that nWaypoint is 1 lower
          if ( debugDT )
          { printf ( "C record[23] %s\n", &Line[23] );
            printf ( "Task C record nPoints: %d\n", Task.nPoints );
          }
        }
        break ;
      }

      case 'H' :                        // Header records
      { strcpy_sec(st, Line, 1, 4) ;

        if ( !strcmp(st, "FDTM") )
        { char *c = Line;
          while ( *c   &&   ( *c >= 'A'  &&  *c <= 'Z' ) )  c++;
          if ( c - Line == 5 )
          { // Old format: read datum from code
            log_datum = atoi ( c );
          }
          else
          { // New format
            while ( *c   &&   ( *c < 'A'  ||  *c > 'Z' ) )  c++;
            if      ( !strncmp ( c, "WGS84",   5 ) )  log_datum = 100;
            else if ( !strncmp ( c, "WGS1984", 7 ) )  log_datum = 100;
          }
          if ( log_datum == 100 )  break;
          report_error(EC_GPS_DATUM, NULL);
          break ;
        }
        if ( !strcmp(st, "FDTE") )
        { char *c = Line;
          while ( *c   &&   ( *c < '0'  ||  *c > '9' ) )  c++;
          strcpy_sec ( st, c, 0, 2); sscanf(st,"%d", &Trace.Day );
          strcpy_sec ( st, c, 2, 2); sscanf(st,"%d", &Trace.Month );
          strcpy_sec ( st, c, 4, 2); sscanf(st,"%d", &Trace.Year );
          if ( Trace.Year < 90 )  Trace.Year += 2000 ;
          else                    Trace.Year += 1900 ;
          break ;
        }

        if ( !strcmp(st, "FPLT") )   if ( read_Hrecord ( Trace.Pilot ) )      dPLT = 1;
        if ( !strcmp(st, "FGTY") )   if ( read_Hrecord ( Trace.GliderType ) ) dGTY = 1;
        if ( !strcmp(st, "FGID") )   if ( read_Hrecord ( Trace.GliderReg ) )  dGID = 1;
        if ( !strcmp(st, "FCCL") )   if ( read_Hrecord ( Trace.CompClass ) )  dCCL = 1;
        if ( !strcmp(st, "FMOP") )   read_Hrecord ( Trace.MOPsensor );
        break ;
      }    
      
      case 'I' :                        // Fix extension definition
      { strcpy_sec(st, Line, 1, 2) ;
        nExtension = atoi(st) ;
        for ( k = 0;   k < nExtension;   k++ )
        { j  = 3+7*k ;
          strcpy_sec(st, Line, j,   2);  ExtFirst = atoi(st) ;
          strcpy_sec(st, Line, j+2, 2);  ExtLast  = atoi(st) ;
          strcpy_sec(st, Line, j+4, 3); 

          // Determine engine registration method. Priority:
          // 1 MOP / ENL.  Value = max ENL / MOP
          // 3 Others: RPM, ON/OFF
          if ( !strcmp(st,"EON") )
          { Engine.Detection = ED_EON ;
            EON.First = ExtFirst-1 ;
            EON.Last  = ExtLast -1 ;
          }
          if ( !strcmp(st,"RPM") )
          { Engine.Detection = ED_RPM ;
            RPM.First = ExtFirst-1 ;
            RPM.Last  = ExtLast -1 ;
          }
          if ( !strcmp ( st, "ENL" ) )
          { if ( Engine.Detection == ED_MOP )  Engine.Detection = ED_ENL_MOP;
            else                               Engine.Detection = ED_ENL;
            ENL.First = ExtFirst-1 ;
            ENL.Last  = ExtLast -1 ;
          }
          if ( !strcmp ( st, "MOP" )    &&   !bNoMOP )
          { if ( Engine.Detection == ED_ENL )  Engine.Detection = ED_ENL_MOP;
            else                               Engine.Detection = ED_MOP;
            MOP.First  = ExtFirst-1 ;
            MOP.Last   = ExtLast -1 ;
            Trace.bMOP = TRUE;
          }
        }

        break ;
      }
      
      case 'J' :                        /***** Definition of K records *****/
        break ;
   
      case 'K' :                        /***** Extra data, less frequent than B *****/
        break ;
      
    }  // End switch
  }  // End while

  fclose ( (void**)&F );

  Check();

  return 1;
}



void CIGC::GetSN(char *st, char *st2)
{ int len;
  char  ewa_model;
  int   igc_sn;
  int   base36[] = {0,0,0};

  // Store flight recorder serial number.
  // Convert old 5 digit format to base 36
  if (  ((st2[3] >= '0') && (st2[3] <= '9'))
     && ((st2[4] >= '0') && (st2[4] <= '9')) )
  { if ( ( pLogger->Type == LT_EW )  &&  (st2[0] > '9'))  // Special EWA s/n format
    { ewa_model = st2[0];
      st2[0]    = '0';
      switch (ewa_model)
      { case 'A':  igc_sn =        atoi(st2);  break;    // Model A s/n encoding uncertain!
        case 'B':  igc_sn = 2000 + atoi(st2);  break;
        case 'C':  igc_sn = 4000 + atoi(st2);  break;    // Model C s/n encoding uncertain!
        case 'D':  igc_sn = 9000 + atoi(st2);  break;
      }
      decimal_to_base36(igc_sn, base36, 3);
      sprintf ( LoggerId, "%c%c%c", alphanumeric(base36[2]), 
                                    alphanumeric(base36[1]),
                                    alphanumeric(base36[0]) );
    }
    else                 // Normal 5 digit s/n format
    { igc_sn = atoi(st2);
      decimal_to_base36(igc_sn, base36, 3);
      sprintf ( LoggerId, "%c%c%c", alphanumeric(base36[2]), 
                                    alphanumeric(base36[1]),
                                    alphanumeric(base36[0]) );
    }
  }
  else  strcpy_sec ( LoggerId, st2, 0, 3 );
  
  if ( pLogger->Type == LT_CAMBRIDGE )   // Only for Cambridge
  { len = (int)strlen(st2);
    if (len>=3) strcpy_sec ( LoggerId, st2, len-3, 3 );
  }
  
  str_lower ( LoggerId );
}




// Perform basic checks on the file
void CIGC::Check(void)
{ if ( Engine.InstAuto   &&   !ENL.First )   Engine.InstAuto = FALSE;
  if ( !dFR_ID )
  { pLogger = &LoggerX;
    report_error ( EC_LOGGER, pLogger->Code );
  }
  if ( !MOP.First   &&  bMOPonly )  report_error ( EC_ENG_NO_MOP,  NULL) ;

  // Reconstruct original IGC short file name
  // Construct the igc name out of the date and logger data
/*
  *IGC_name = 0;
  if ( pLogger->Letter > ' ' )
  { // Make short file name
    char st[MAXLEN];
    sprintf ( st, "%d%c%c", Trace.Year % 10, alphanumeric(Trace.Month), alphanumeric(Trace.Day));
    str_lower ( st );
    sprintf ( IGC_name, "%s%c%s1.igc", st, pLogger->Letter, LoggerId );
  }
  int n = strlen ( IGC_name );
  if ( !n   ||   n >= 12 )
*/
  { // Make long file name
    sprintf   ( IGC_name, "%4d-%02d-%02d-%s-%s-01.igc", Trace.Year, Trace.Month, Trace.Day, pLogger->Code, LoggerId );
    str_upper ( IGC_name );
  }

  // Check if all necessary flight information is defined,
  // except for EW loggers.
  // EW logger: replace pilot name by serial number
  if ( pLogger->Type != LT_EW )
  { if ( CheckDeclar )
    { if ( !dPLT ) report_error ( EC_PILOT_NAME,  NULL) ;
      if ( !dGTY ) report_error ( EC_GLIDER_TYPE, NULL) ;
      if ( !dGID )
      { if ( no_GID )  report_error ( TM_NO_GID, NULL ) ;
        else           report_error ( EC_GLIDER_REGN, NULL) ;
      }
    }
    else if ( (!dPLT) || (!dGTY) || (!dGID) ) 
        report_error ( EC_DECLAR_INCMPL,  NULL );
  }
  else
  { sprintf ( Trace.Pilot,     "FR: %s s/n %s", pLogger->Code, LoggerId );
    sprintf ( Trace.GliderType,"zzz");
    sprintf ( Trace.GliderReg, "zzz");
  }
}



/****************************************************************************
 * read_Hrecord : reads text from H record, starting at the ":"
 ****************************************************************************/
int CIGC::read_Hrecord(char *data)
{ char *sp, *sp2 ;
  
  if ( sp = strchr ( Line, ':' ) )
  { sp++ ; 
    if ( !strlen(sp) ) return 0 ;
    
    // Replace all underscores by spaces, trim and convert to uppercase
    do 
      if ( sp2 = strchr(sp,'_') ) *sp2 = 32 ;
    while ( sp2 );
    str_trim_left(sp); 
    str_upper(sp) ;
    
    sprintf ( data, "%s", sp );
    return 1 ;
  }
  return 0 ;
}





/****************************************************************************
 * alphanumeric : converts integer to alphanumeric character : 0-9, A-Z
 ****************************************************************************/
char alphanumeric(int number)
{
  char alpha;
  
  if (number > 9)
    alpha = 'A' + (number - 10);
  else if (number >= 0)
    alpha = '0' + number;
  else 
    alpha = '0';
  
  return alpha;
}






/****************************************************************************
 * decimal_to_base36 : converts decimal integer to base-36 integer
 ****************************************************************************/
void decimal_to_base36(int decimal, int base36[], int n)
{ int i, p;
  for ( i = n-1;    i>= 0;    i-- )
  { p = (int)pow ( (float)36, i );
    base36[i] = decimal/p;
    decimal  -= base36[i]*p;
  }
}
