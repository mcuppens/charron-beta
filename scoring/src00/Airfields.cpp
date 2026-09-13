#include "Global.h"
#include "Airfields.h"

CAirfields Airfields;


CAirfields::CAirfields(void)
{ List  = NULL;
  nList = 0;
}



CAirfields::~CAirfields(void)
{ free ( (void**)&List );
}



void CAirfields::Read(void)
{ char Line[MAXLEN] = { 0 };

  FILE *F = fopen ( file_airf, "r" );
  if ( !F )  report_error ( EC_AIRF_NOT_OPEN, NULL );

  // Determine the number of airfilelds in the file
  // The first line contains the header, but that will be replaced by the unknown airfield code
  nList = 0;
  while ( 1 )
  { if ( !fgets ( Line, MAXLEN, F ) )  break;
    if ( strlen ( Line ) < 20 )  continue;
    nList ++;
  }
  if ( !nList )  report_error ( EC_AIRF_F_EMTY, NULL );
/*
  // Read number of airfields and allocate storage
  fscanf ( F, "%d\n", &nList );
  if ( !nList )  report_error ( EC_AIRF_F_EMTY, NULL );
*/
  List = (sAirfield *) calloc ( nList, sizeof(sAirfield) );
  if ( !List )
  { fclose ( (void**)&F );
    exit_error ( EC_MEMORY, "Allocation error in CAirfields::Read" );
  }

  // Set the first airfield to "unknown"
  strcpy ( List[0].icao, "ZZZZ" );
  strcpy ( List[0].name, "-Onbekend-" );
  List[0].pt.Lat = 0.F;
  List[0].pt.Lon = 0.F;
  List[0].level  = 0;
  List[0].BNL    = 0;

  rewind ( F );
  fgets ( Line, MAXLEN, F );  // Skip the header

  // Read all airfields
  for ( int i = 1;  i < nList;   i ++ )
  {
    if ( !ReadAirfield ( F, &List[i] ) )
    { fclose ( (void**)&F );
      sprintf ( Line, "Line %d", i );
      report_error ( EC_AIRF_F_ERROR, Line );
    }   
  }
  
  fclose ( (void**)&F );
}




int CAirfields::ReadAirfield(FILE *F, sAirfield *afd)
{ char st[MAXLEN];
  char st2[MAXLEN];
  char Line[MAXLEN] = { 0 };
  char *c = Line;
  double D, M;

  fgets ( Line, MAXLEN, F );
  if ( Line[MAXLEN-1] )
  { fclose ( (void**)&F );
    report_error ( EC_AIRF_F_ERROR, NULL );
  }
  str_trim_right ( Line );

  // Read airfield name & ICAO code
  if ( !ReadField ( &c, afd->icao,  4 ) )  return 0;
  if ( !ReadField ( &c, afd->name, 39 ) )  return 0;

  // Read latitude
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 2 );  D = atoi(st2);
  strcpy_sec ( st2, st, 2, 5 );  M = atoi(st2);
  afd->pt.Lat = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[7] == 'S' ) afd->pt.Lat *= (float)-1.;
  afd->pt.Lat *= piDiv180;

  // Read longitude
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 3 );  D = atoi(st2);
  strcpy_sec ( st2, st, 3, 5 );  M = atoi(st2);
  afd->pt.Lon = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[8] == 'W' ) afd->pt.Lon *= (float)-1.;
  afd->pt.Lon *= piDiv180;

  // Read airfield level
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  afd->level = atoi(st);
    
  // Read Benelux flag
  // 24/01/2015: did set Arras 1
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  afd->BNL = atoi(st);

  return 1;
}





void CAirfields::ResetSel(void)
{ for ( int i = 0;  i < nList;   i ++ )  List[i].sel = 0;
}





int CAirfields::Find(sCoord *pt, double r)
{ // Find the airfield closest to position pt, invalid (0) if beyond radius r
  double d;
  double dm = 9999.;
  int    n = 0;

  for ( int i = 0;  i < nList;   i ++ )
  { d = OrthoDist ( pt, &List[i].pt );
    if ( d < dm )
    { dm = d;
      n = i;
    }
  }
  if ( dm > r )  n = 0;
  return n;
}

