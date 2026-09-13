/*********************************************************************
 ** CHARRON ONLINE SOARING CONTEST SCORING
 ** Copyright(c) 2003-2024 Diest Aero Club
 **            www.dac.be
 **
 ** version 0.0 -- 31/07/2024
 **
 ** Developed by :
 **   Luc Beerts (beerts.l@skynet.be) 
 **
 *********************************************************************/
#include "Global.h"
#include "Airfields.h"
#include "Baro.h"
#include "Brussel.h"
#include "PlotFile.h"
#include "IGC.h"
#include "PlotFile.h"
#include "Release.h"
#include "Runs.h"
#include "FreeFlight.h"
#include "PlotFlight.h"
#include "Triangle.h"
#include "WriteDat.h"
#include "WriteText.h"

// output\469G8NL1.IGC -etmin300 -noenl -ddt -c 49.6 -nruns -nd -df
// output\469G8NL1.IGC -etmin400 -etmax500  -ddt  -noenl -c 53.7 -plot:BNL:312  -nruns 
// output\469G8NL1.IGC -etmin400 -etmax500  -noenl -c 53.7 -plot:BNL:312  -nruns  -nograph -d  -df
// output\468V75M1.igc -etmin300 -noenl -dtri -c 49.6 -nruns  -nograph

BOOL ParseCommandLine(int argc, char* argv[]);
FILE *fCON = stdout;
extern unsigned int TC0;

int main(int argc, char* argv[])
{ 
  #if defined WIN32
  atexit(ExitWin32);
   #ifdef _DEBUG
     debugAny = TRUE;
   #endif
  #endif
  ParseCommandLine ( argc, argv );
  Airfields.Read();
  readfile_IGC();
  Trace.Init();
//  PlotList.Init();
  FindRelease();
  AnalyseEngine();
//  LimitFixes();
  FindLongestSoarBlock();
  CalcAltGain();
  if ( ScoreFF   ) CalcFreeFlight();
  if ( SpeedComp ) CalcFastTrack();
  if ( ScoreDT   ) CalcTask();

  if ( ScoreTRI  ) CalcTriangle();
  if ( ScoreBru  ) Charronde();
  if ( ScoreRuns ) CalcRuns();

  if ( !noGraph )
  { PlotFlight();
    PlotBaro();
    ExportFlight();
  }
  WriteDat();        // After graphs: able to report mutexing problems
  WriteText();
#ifdef WIN32
#ifdef _DEBUG
  freopen ( "con", "w", stdout );
  printf ( "Done\n" );
  printf ( "Rekentijd: %d\n", GetTickCountOwn() - TC0 );  
  if ( debugAny )
  { printf ("press any key..." );
    _getch();
  }
#endif
#endif
  return 0;
}




BOOL ParseCommandLine(int argc, char* argv[])
{ int  i, n, hr, min, sec;
  int  LenIGC = 0;
  int  LenDIR = 0;
//  int  cError = 0;
  char string[MAXLEN];
//  char stError[MAXLEN] = { 0 };
  char *pc;

  if ( argc <= 1 )  
    report_error ( EC_NO_ARGUMENTS, NULL );

  pc = strrchr ( argv[0], PathDelimiter );
  if ( pc == NULL )  pc = argv[0];
  else               pc ++;
  sprintf ( VersionSt, "%s v. %s", pc, Version );

  for ( i=1;  i<argc;  i++ )
  { strcpy ( string, argv[i] );
    
    if ( !strncmp ( string, "-#", 2 ) )
    { // commented out
    }
    else if ( !strcmp ( string, "-c" ) )
    { i++;
      strcpy ( string,argv[i] );
      n = sscanf ( string, "%le", &LDnominal );
      if ( n != 1           ||
           LDnominal < 10   ||
           LDnominal > 80
         )
      { report_error ( EC_ARG_OUT_RANGE, "-c L/D", FALSE );
      }
    } 
    else if ( !strncmp ( string, "-fixm:", 6 ) )
    { if ( string[6] )
      { nFixMax = max ( atoi ( &string[6] ), 5000 );
      }
    }
    else if ( !strncmp ( string, "-dtime", 6 ) )    // Add execution time of XX seconds
    { if ( string[6] )
      { debugTime = atoi ( &string[6] );
        if ( debugTime <   0 ) debugTime = 0;
        if ( debugTime > 180 ) debugTime = 180;
      }
    }
    else if ( !strncmp ( string, "-dttot", 6 ) )    // Add execution time to a total of XX seconds
    { if ( string[6] )
      { debugtTot = atoi ( &string[6] );
        if ( debugtTot <   0 ) debugtTot = 0;
        if ( debugtTot > 180 ) debugtTot = 180;
      }
    }
    else if ( !strcmp ( string, "-d"       ) )   debug            = YES;
    else if ( !strcmp ( string, "-df"      ) )   debugFile        = YES;
    else if ( !strcmp ( string, "-digc"    ) )   debugIGC         = YES;
    else if ( !strcmp ( string, "-dflt"    ) )   debugFlt         = YES;
    else if ( !strcmp ( string, "-drel"    ) )   debugRelease     = YES;
    else if ( !strcmp ( string, "-deng"    ) )   debugEngine      = YES;
    else if ( !strcmp ( string, "-dalt"    ) )   debugAlt         = YES;
    else if ( !strcmp ( string, "-dff"     ) )   debugFF          = YES;
    else if ( !strcmp ( string, "-ddt"     ) )   debugDT          = YES;
    else if ( !strcmp ( string, "-dtri"    ) )   debugTRI         = YES;
    else if ( !strcmp ( string, "-dbru"    ) )   debugBru         = YES;
    else if ( !strcmp ( string, "-druns"   ) )   debugRuns        = YES;
    else if ( !strcmp ( string, "-ddl"     ) )   debugDislin      = YES;
    else if ( !strcmp ( string, "-dplot"   ) )   debugPlot        = YES;
    else if ( !strcmp ( string, "-nograph" ) )   noGraph          = YES;
    else if ( !strcmp ( string, "-e"       ) )   Engine.Installed = YES;
    else if ( !strcmp ( string, "-ea"      ) )   Engine.InstAuto  = YES;
    else if ( !strcmp ( string, "-moponly" ) )   bMOPonly         = YES;
    else if ( !strcmp ( string, "-nomop"   ) )   bNoMOP           = YES;
    else if ( !strncmp ( string, "-noenl", 6 ) ) bNoENL           = YES;
    else if ( !strncmp ( string, "-etmin", 6 ) )       // Minimum engine threshold
    { if ( string[6] )
      { Engine.Installed = YES;
        Engine.Threshold_Min = (double)atoi ( &string[6] );
        if ( Engine.Threshold_Min < 100. ) Engine.Threshold_Min = 100.;
        if ( Engine.Threshold_Min > 999. ) Engine.Threshold_Min = 999.;
      }
    } 
    else if ( !strncmp ( string, "-etmax", 6 ) )       // Maximum engine threshold
    { if ( string[6] )
      { Engine.Installed = TRUE;
        Engine.Threshold_Max = (double)atoi ( &string[6] );
        if ( Engine.Threshold_Max < 100. ) Engine.Threshold_Max = 100.;
        if ( Engine.Threshold_Max > 999. ) Engine.Threshold_Max = 999.;
      }
    } 
    else if ( !strncmp ( string, "-et", 3 ) )       // Use this engine threshold
    { if ( string[3] )
      { Engine.Installed = TRUE;
        Engine.Threshold_Man = (double)atoi ( &string[3] );
        if ( Engine.Threshold_Man <   0. ) Engine.Threshold_Man =   0.;
        if ( Engine.Threshold_Man > 999. ) Engine.Threshold_Man = 999.;
      }
    }
    else if ( !strncmp ( string, "-ffmld", 6 ) )       // Free flight task minimum leg distance
    { ffMinLegDist = atof ( &string[6] );
      if ( ffMinLegDist <  0   ||
           ffMinLegDist > 50
         )
      { sprintf ( string, ": -ffmld [0-50]: %0.1f", ffMinLegDist );
        report_error ( EC_ARG_OUT_RANGE, string, FALSE ); 
      }
    }
    else if ( !strncmp ( string, "-dtmld", 6 ) )       // Declared task minimum leg distance
    { dtMinLegDist = atof ( &string[6] );
      if ( dtMinLegDist <  0   ||
           dtMinLegDist > 50
         )
      { sprintf ( string, ": -dtmld [0-50]: %0.1f", dtMinLegDist );
        report_error ( EC_ARG_OUT_RANGE, string, FALSE ); 
      }
    }
    else if ( !strncmp ( string, "-tricd", 6 ) )
    { triCloseD = atof ( &string[6] );
    }
    else if ( !strncmp ( string, "-tribfss", 8 ) )
    { triBF = TRUE;
      triBFstep = max ( atoi ( &string[8] ), 1 );
    }
    else if ( !strncmp ( string, "-tribfmns", 8 ) )
    { triBF = TRUE;
      triBFmns = max ( atoi ( &string[9] ), 1 );
    }
    else if ( !strcmp ( string, "-tribf"    ) )   triBF        = TRUE;
    else if ( !strcmp ( string, "-gid"      ) )   no_GID       = YES;
    else if ( !strcmp ( string, "-nff"      ) )   ScoreFF      = FALSE;
    else if ( !strcmp ( string, "-nft"      ) )   SpeedComp    = FALSE;
    else if ( !strcmp ( string, "-ndt"      ) )   ScoreDT      = FALSE;
    else if ( !strcmp ( string, "-nbru"     ) )   ScoreBru     = FALSE;
    else if ( !strcmp ( string, "-ntri"     ) )   ScoreTRI     = FALSE;
    else if ( !strcmp ( string, "-nruns"    ) )   ScoreRuns    = FALSE;
    else if ( !strcmp ( string, "-nd"       ) )   CheckDeclar  = NO;
    else if ( !strcmp ( string, "-ndtime"   ) )   CheckDecTime = NO;
    else if ( !strcmp ( string, "+nt"       ) )   Engine.SkipTestRun = FALSE;
    else if ( !strncmp ( string, "-ec", 3   ) )   list_errors ( &string[3] );
    else if ( !strncmp ( string, "-rm", 3   ) )
    { if ( string[3] )
      { n = sscanf ( &string[3],"%02d:%02d:%02d", &hr, &min, &sec );
        if ( n != 3    ||
             hr  < 0   ||   hr  > 23   ||
             min < 0   ||   min > 59   ||
             sec < 0   ||   sec > 59
           )
        { report_error ( EC_ARG_OUT_RANGE, "-rm Release time", FALSE );
        }
        tReleaseManual = (double)hr + 1/60.*(double)min + 1/3600.*(double)sec;
        if ( tReleaseManual > 0 )  releaseManual = YES;
      }
    }
    else if ( !strncmp ( string, "-nknicks", 8 ) )
    { nKnicksBase = atoi ( &string[8] );
      if ( nKnicksBase < 14   ||
           nKnicksBase > 30
         )
      { sprintf ( string, ": -nknicks [14-30]: %d", nKnicksBase );
        report_error ( EC_ARG_OUT_RANGE, string, FALSE ); 
      }
    }
    else if ( !strcmp ( string, "-st" ) )     // Evaluate soaring block between the given times
    { Engine.InstAuto = FALSE;
      FLT_Automatic = NO;
      i++;
      strcpy ( string,argv[i] );
      n = sscanf ( string,"%02d:%02d:%02d", &hr, &min, &sec );
      if ( n != 3    ||
           hr  < 0   ||   hr  > 23   ||
           min < 0   ||   min > 59   ||
           sec < 0   ||   sec > 59
         )
      { report_error ( EC_ARG_OUT_RANGE, "-st Start time", FALSE );
      }
      tStartSoar = (double)hr + 1/60.*(double)min + 1/3600.*(double)sec;
      i++;
      strcpy ( string,argv[i] );
      n = sscanf ( string,"%02d:%02d:%02d", &hr, &min, &sec );
      if ( n != 3    ||
           hr  < 0   ||   hr  > 23   ||
           min < 0   ||   min > 59   ||
           sec < 0   ||   sec > 59
         )
      { report_error ( EC_ARG_OUT_RANGE, "-st End time", FALSE );
      }
      tEndSoar = (double)hr + 1/60.*(double)min + 1/3600.*(double)sec;
    } 
    else if ( !strcmp ( string, "-pause" ) )  bPause = TRUE;
    else if ( !strncmp ( string, "-plot", 5 ) )
    { char *cRgn = string + 6;
      char *cNme = &cRgn[strcspn ( cRgn, " :" )];
      if ( !cNme )
      { report_error ( EC_ARG_OUT_RANGE, "-plot Name", FALSE );
      }
      *cNme = 0;
      cNme ++;
      PlotList.Add ( cNme, cRgn );
    }
    else if ( !strncmp ( string, "-latmax", 7 ) )
    { LatMaxPlot = (float)atof ( &string[7] );
    }
    else if ( !strncmp ( string, "-latmin", 7 ) )
    { LatMinPlot = (float)atof ( &string[7] );
    }
    else if ( !strncmp ( string, "-lonmax", 7 ) )
    { LonMaxPlot = (float)atof ( &string[7] );
    }
    else if ( !strncmp ( string, "-lonmin", 7 ) )
    { LonMinPlot = (float)atof ( &string[7] );
    }
    else if ( *string == '-' )
    { // Any other command (except file name) is unknown: abort
      report_error ( EC_UNDEF_ARGUMENT, string, FALSE );
    }
    else
    { strcpy ( file_igc, argv[i] );
      LenIGC = (int)strlen ( file_igc );
      if ( LenIGC < 4 )
      { sprintf ( string, "Undefined argument: %s", argv[i] );
        report_error ( EC_UNDEF_ARGUMENT, string, FALSE ); 
      }
      else if ( strcmp ( &file_igc[LenIGC-4],".igc" )   &&
                strcmp ( &file_igc[LenIGC-4],".IGC" )  )
      { sprintf ( string, "Undefined argument: %s", argv[i] );
        report_error ( EC_UNDEF_ARGUMENT, argv[i], FALSE );
      }
    }
  }

  /*****************************/
  /* Read environment variable */   
  /*****************************/

 #ifndef WIN32
  if ( getenv("SCOREDIR") != NULL )
    strcpy ( score_dir, getenv("SCOREDIR") );
  else
  { report_error(EC_OUT_SCOREDIR, NULL);
/*strcpy ( score_dir, "/root/luc" );*/
  }
 #else
//  strcpy ( score_dir, "ScoreDir" );
 #endif
  LenDIR = (int)strlen ( score_dir );
  if ( !LenIGC    ) report_error ( EC_CL_NO_FILE,  NULL );
  if ( LenIGC < 5 ) report_error ( EC_CL_FN_SHORT, NULL );
  if ( LenDIR + LenIGC + 2 > MAXLEN ) report_error ( EC_CL_PN_LONG, NULL ); // 2: backslash + trailing zero

 #ifndef WIN32
  strcpy ( file_airf, score_dir );
  strcat ( file_airf, "/airfields_icao.txt" );
  strcpy ( file_stretches, score_dir );
  strcat ( file_stretches, "/stretches.txt" );
  strcpy ( dir_country, score_dir );
  strcat ( dir_country, "/Borders/" );
 #else
  strcpy ( file_airf, "Files\\airfields_icao.txt" );
  strcat ( file_stretches, "Files\\stretches.txt" );
  strcpy ( dir_country, "Files\\Borders\\" );
 #endif

  strncpy ( base_name, file_igc, LenIGC-4 );
  sprintf ( file_data, "%s.dat", base_name );
  sprintf ( file_text, "%s.txt", base_name );
  sprintf ( file_plot, "%s.png", base_name );
  sprintf ( file_baro, "%sB.png", base_name );

  /****************************************/
  /* Remove existing files with this name */
  /****************************************/
  remove ( file_data );
  remove ( file_text );
  remove ( file_plot );
  remove ( file_baro );

  if ( debug )
  { debugIGC     = TRUE;
    debugFlt     = TRUE;
//    debugRelease = YES;
    debugEngine  = YES;
    debugAlt     = YES;
    debugFF      = YES;
    debugDT      = YES;
    debugTRI     = YES;
    debugBru     = YES;
    debugRuns    = YES;
    debugDislin  = YES;
    debugPlot    = YES;
  }
  if ( debugDislin )  debugPlot = YES;
  debugAny = debugIGC || debugFlt || debugRelease || debugEngine || debugAlt || debugFF || debugDT || debugTRI || debugBru || debugRuns || debugDislin || debugPlot;

// TODO  remove
//TRUE for debugging the task declaration after flight issue
if ( debugFile )  debugAny = TRUE;

  if ( debugAny )
  { if ( debugFile )
    { char fn[MAXLEN];
      sprintf ( fn, "%s.out", base_name );
      OutF = freopen ( fn, "w", stdout );
    }
    printf ( "0,  %s\n", VersionSt );
    for ( i=1;  i<argc;  i++ )
    { printf ( "%d,  %s\n", i, argv[i] );
    }
    printf ( "\n" );
  }

  if ( nCritErrors )
  { // Only report the first error
    exit_error ( CritErrors[0].code, CritErrors[0].text );
  }

  if ( !Engine.Threshold_Man  &&  Engine.Threshold_Min < epsilon )  Engine.Threshold_Min = 400;
  if ( !Engine.Threshold_Man  &&  Engine.Threshold_Max < epsilon )  Engine.Threshold_Max = 500;
  if ( Engine.Threshold_Max < Engine.Threshold_Min )
  { char st[256];
    sprintf ( st, "Invalid ENL parameters: min %0.0f, max %0.0f", Engine.Threshold_Min, Engine.Threshold_Max );
    report_error ( EC_ENG_ENL_PARAMS, st );
  }

  return TRUE;
}
