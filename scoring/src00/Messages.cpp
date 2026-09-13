#include "Global.h"


int      nCritErrors = 0;      // Number of critical errors
int      nErrors     = 0;      //           non-critical errors
sMessage *CritErrors = 0;      // List of critical errors
sMessage *Errors     = NULL;   //         non-critical errors


struct sError
{ int  code;
  int  critical;         // FALSE: evaluation continues, YES: evaluation stops
  int  extra;            // Switch to indicate that extra info is available
  char text[MAXLEN] ;
  char textNL[MAXLEN] ;
} Error[NERRORS] = 
{
  EC_NIL              , NO,  0, "No errors",                         "Geen fouten",

  EC_UNDEFINED        , YES, 0, "Undefined fatal error",             "Onbepaalde fatale fout",
  EC_NO_ARGUMENTS     , YES, 0, "No program input given",            "Geen programma input gegeven",
  EC_UNDEF_ARGUMENT   , YES, 1, "Invalid program option: ",                     "Ongeldige programma optie: ",
  EC_ARG_OUT_RANGE    , YES, 1, "Argument out of range",                        "Parameter buiten limiet",
  EC_MEMORY           , YES, 1, "Memory allocation error in: ",                 "Geheugen allocatie fout: ",
  EC_ERRORCODE        , YES, 1, "Error code undefined: ",                       "Fout code niet gedefinieerd: ",
  EC_MUTEX_TO         , NO,  0, "Mutex timeout",                                "Mutex timeout",
                                                                    
  EC_CL_NO_FILE       , YES, 0, "IGC file name not specified",                  "Geen IGC file naam gegeven",
  EC_CL_FN_SHORT      , YES, 0, "IGC file name too short",                      "IGC file naam te kort",
  EC_CL_FN_LONG       , YES, 0, "IGC file name too long",                       "IGC file naam te lang",
  EC_CL_PN_LONG       , YES, 0, "SCOREDIR + IGC file name too long",            "SCOREDIR + IGC file naam te lang",
  EC_CL_PL_AREA       , NO,  0, "Plot area definition not complete",            "Plot-gebied definitie onvolledig",
  EC_CL_PL_AREA_ERROR , NO,  0, "Invalid plot area definition",                 "Plot-gebied definitie ongeldig",
                                                                    
  EC_IGC_NOT_OPEN     , YES, 0, "Cannot open IGC file",                         "Kan IGC file niet openen",
  EC_IGC_INVALID      , NO,  0, "IGC file security invalid",                    "IGC file security niet OK", // Not used
  EC_IGC_TOO_LONG     , YES, 1, "IGC file data line too long: ",                "Lijn in IGC file te lang: ",
  EC_AIRF_NOT_OPEN    , YES, 0, "Cannot open airfield file",                    "Kan vliegveld bestand niet openen", 
  EC_AIRF_F_EMTY      , YES, 0, "Airfield file empty",                          "Vliegveld bestand leeg", 
  EC_AIRF_F_ERROR     , YES, 0, "Error reading airfield file",                  "Fout lezen vliegveld bestand", 
                                                                    
  EC_LOGGER           , NO,  1, "Logger type undefined: ",                      "Logger type niet bekend: ",
  EC_GPS_DATUM        , YES, 0, "GPS datum differs from WGS84",                 "GPS datum verschilt van WGS84",
  EC_SHORT_FLIGHT     , YES, 0, "Too few fixes in IGC file",                    "Te weinig punten in IGC file",
  EC_RELEASE_POINT    , YES, 0, "No release point found",                       "Geen ontkoppelpunt gevonden",
  EC_SAMPLE_TOOLONG   , NO,  1, "Logger interval exceeds limit of ",            "Logger interval groter dan limiet van ",
  EC_NO_ENL           , YES, 0, "No ENL signal in IGC file",                    "Geen ENL signaal in IGC file",
  EC_NO_VALID3DFIX    , YES, 0, "No valid 3D GPS fixes",                        "Geen enkele geldige 3D GPS fix",
  EC_NO_ENL_OVRD      , NO,  0, "No ENL signal, scoring forced",                "Geen ENL signaal, scoring geforceerd",
  EC_GPS_DATUM_NSTD   , NO,  0, "GPS datum format not standard",                "GPS datum format niet standaard",
  EC_IGC_FILE_ERROR   , YES, 0, "Invalid character or line length in IGC file", "Ongeldig karakter of lijn lengte in IGC file",

  EC_PILOT_NAME       , YES, 0, "Pilot name not declared",                      "Naam piloot niet gedeclareerd",
  EC_GLIDER_TYPE      , YES, 0, "Glider type not declared",                     "Zweefvliegtuig type niet gedeclareerd",
  EC_GLIDER_REGN      , YES, 0, "Glider registration not declared",             "Immatriculatie niet gedeclareerd",
  EC_DECLAR_INCMPL    , NO,  0, "Declaration incomplete (checked)",             "Declaratie onvolledig (gecheckt)",
  EC_DECLAR_LINE      , NO,  0, "Found declaration line in invalid place",      "Declaratie lijn op ongeldige plaats",

  EC_ENG_NO_TESTRUN   , YES, 0, "No engine test run made",                      "Geen motor testloop gemaakt",
  EC_ENG_ENL_PARAMS   , YES, 1, "Invalid ENL parameters",                       "Ongeldige ENL parameters",
  EC_ENG_NO_MOP       , YES, 0, "No MOP detection",                             "Geen MOP detectie",

  EC_FR_TOO_SHORT     , YES, 0, "Too short flight, less than 20 fixes",         "Te korte vlucht, minder dan 20 fixes",
  EC_FR_GPS_SIGNAL    , YES, 0, "No GPS signal after take-off",                 "Geen GPS signaal na take-off",
  EC_FR_NO_FLIGHT     , YES, 0, "No flight found",                              "Geen vlucht gevonden",
  EC_FR_NO_SOAR_B     ,  NO, 0, "No soaring block found",                       "Geen soaring blok gevonden",
  EC_FR_SOAR_SHORT    ,  NO, 0, "Soaring block insufficient length",            "Soaring blok te kort",

  EC_OUT_DATAFILE     , YES, 0, "Cannot open data output file",                 "Kan data output file niet openen",
  EC_OUT_TEXTFILE     , YES, 0, "Cannot open text output file",                 "Kan tekst output file niet openen",
  EC_OUT_SCOREDIR     , YES, 0, "SCOREDIR variable not defined",                "SCOREDIR variabele niet gedefinieerd",
  EC_OUT_BAROFILE     , YES, 0, "Cannot open barogram file",                    "Kan barogram output file niet openen",
  EC_OUT_PLOTFILE     ,  NO, 1, "Error in plot file ",                          "Fout in plot file ",
  EC_OUT_PLOTPLOT     ,  NO, 1, "Error drawing the plot ",                      "Fout plotten file ",

  TRI_NOT_CLOSED      ,  NO, 0, "Triangle: no closed segment found",            "Driehoek: geen gesloten segment gevonden",
                                                                    
  TM_NO_TASK          ,  NO, 0, "No task declaration",                          "Geen declaratie van opgegeven proef",
  TM_OK               ,  NO, 0, "Task declaration valid",                       "Geldige opgegeven proef declaratie",
  TM_BEERCAN_USED     ,  NO, 0, "Beercan sectors used",                         "Beercan fotosectoren gebruikt.",
  TM_INVALID          ,  NO, 0, "Task declaration invalid",                     "Ongeldige opgegeven proef declaratie",
  TM_NTURNPT_ALLOWED  ,  NO, 0, "Too many turnpoints",                          "Meer keerpunten in declaratie dan toegelaten",
  TM_NTURNPT_INVALID  ,  NO, 0, "No or too many turnpoints",                    "Nul of te veel keerpunten volgens regels",
  TM_TURNPT_TOO_CLOSE ,  NO, 0, "Distance between turnpoints too small",        "Onderlinge afstand tussen keerpunten kleiner dan minimum",
  TM_CORRECTED_START  ,  NO, 0, "Short leg: corrected start point",             "Te kort been: start punt gecorrigeerd",
  TM_CORRECTED_FINISH ,  NO, 0, "Short leg: corrected finish point",            "Te kort been: finish punt gecorrigeerd",
  TM_TASK_TOO_SMALL   ,  NO, 0, "Task distance insufficient",                   "Afstand opgegeven proef kleiner dan minimum",
  TM_TASK_DIST_ERR    ,  NO, 0, "Task distance erratic",                        "Totale afstand opgave buiten limieten",
  TM_TASK_INCONSIST   ,  NO, 0, "Task declaration inconsistent",                "Waarschuwing: opgegeven proef declaratie inconsistent",
  TM_DECL_AFTER_START ,  NO, 0, "Declaration after start of flight",            "Declaratie na begin van vlucht",
  TM_NO_GID           ,  NO, 0, "No glider identification in logger",           "Geen vliegtuig registratie record in de logger header",
};


/****************************************************************************
 * report_error : checks given error code, adds message and checks action
 ****************************************************************************/
void report_error(int code, const char *extra_info, int bExit/*=TRUE*/)
{ int i, found=NO;
  int nMsg;
  char message[MAXLEN];
  sMessage *Msg;

  // Find error in list of errors
  for (i=0; i<NERRORS; i++)
    if (Error[i].code == code)
    { found=YES;
      break;
    }
  
  // Add error message to list of critical or non-critical errors
  if (found)
  {
    if (Error[i].critical) {  Msg = CritErrors;  nMsg = nCritErrors; }
    else                   {  Msg = Errors;      nMsg = nErrors; }

    realloc_s ( (void**)&Msg, sizeof(sMessage), &nMsg );

    Msg[nMsg-1].code = Error[i].code;
   #ifdef DUTCH
    strcpy(message, Error[i].textNL);
   #else
    strcpy(message, Error[i].text);
   #endif
    if ( Error[i].extra   &&   ( extra_info != NULL ) )
    { strcat ( message, "  " );
      strcat ( message, extra_info );
    }
    strcpy(Msg[nMsg-1].text, message);
    
    if (Error[i].critical) {  CritErrors = Msg;  nCritErrors++; }
    else                   {  Errors     = Msg;  nErrors++; }    
    
    if ( debugAny )
    { printf ( "error code %d, %s ", code, Msg[nMsg-1].text );
      if ( extra_info ) printf ( extra_info );
      printf ( "\n" );
    }

    // If the error is critical, exit program (else continue)
    if ( bExit   &&   Error[i].critical ) exit_error ( Error[i].code, Msg[nMsg-1].text );
  }
  else
  { // Error due to error code itself not being found
    sprintf(message,"Undefined error code: %d ", code);
    if ( debugAny )
    { printf ( message );
      if ( extra_info ) printf ( extra_info );
      printf ( "\n" );
    }
    exit_error ( EC_ERRORCODE, message );
  }
}



 
/****************************************************************************
 * exit_error : Error message handling.
 ****************************************************************************/
void exit_error(int code, const char *message)
{
  extern int  debug;
  extern char file_data[];
  FILE  *F;

  // Open output file
  if ( *file_data )
  { F = fopen(file_data,"w");
    if ( !F ) 
    { printf("\n");
      printf("****************************\n");
      printf("Fatal error: Could not open output file\n\n");
      exit(EC_UNDEFINED);
    }

    // Write error message to data file
    fprintf ( F, "%s\n", Version );   // New
    fprintf ( F, "%s\n", IGC_name );  // New
    fprintf ( F, "%d  %s\n", code, message );
    fclose ( (void**)&F );
    if (debug)
    { printf("\n") ;
      printf("*****************************\n");
      printf("Error %d : \n", code) ;
      printf("%s\n\n", message) ;
    }
  }
  else
  { printf("\n") ;
    printf("*****************************\n");
    printf("Error %d : \n", code) ;
    printf("%s\n\n", message) ;
  }
#ifdef _DEBUG
  if ( debugAny )
  { printf ("press any key..." );
    _getch();
  }
#endif

  exit ( code ) ;
}



/****************************************************************************
 * list_errors: writes the list of defined error codes & messages to stdout
 ****************************************************************************/
void list_errors(char *fn)
{ struct sComment { int i; char st[80]; }
  Comment[] =
  {   0, "# General errors\n",
    100, "# Command line parameter errors\n",
    110, "# File IO errors\n",
    120, "# Logger errors\n",
    130, "# Declaration errors\n",
    140, "# Engine errors\n",
    150, "# Flight recording errors\n",
    190, "# Output errors\n",
    300, "# Task messages\n",
    999, ""
  };

  int i;
  int ic = 0;
  char stc[12];

  FILE *F = stdout;
  if ( fn   &&   *fn  )
  { fn ++;    // skip the kolon
    F = fopen ( fn, "wt" );
    if ( !F )
    { F = stdout;
      printf ( "Error opening error listing file %s\n", fn );
    }
  }

  fputs ( VersionSt, F );
  fputs ( "\n#Code\tDescription\tCritical\n", F );

  for ( i = 0;   i < NERRORS;   i ++ )
  { if ( Error[i].code == Comment[ic].i )
    { fputs ( Comment[ic].st, F );
      ic ++;
    }
    if ( Error[i].critical )  strcpy ( stc, "critical" );
    else *stc = 0;

    fprintf ( F, "%3d\t%s\t%s\n", Error[i].code, Error[i].textNL, stc );
  }

  if ( F != stdout )  fclose ( (void**)&F );
  else
  {
   #ifdef WIN32
    printf ( "Press any key ..." );
    _getch();
   #endif
  }
  exit ( 1 );
}





