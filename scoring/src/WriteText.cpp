#include "Global.h"
#include "WriteText.h"
#include "Airfields.h"
#include "Runs.h"


// Define the stringtable for every language to be used in tables below
// Add a defines block for every language
// The DUTCH block can be used as a template
#ifdef DUTCH
# define  SCORING_VERSION  "Scoring versie: %s\n\n"
# define  LOGGER_DATA      "LOGGER GEGEVENS\n"
# define  FLIGHT_DATE      "Vlucht datum:   %02d/%02d/%d\n"
# define  PILOT            "Piloot:         %s\n"
# define  REGISTRATION     "Zweefvliegtuig: %s\n"
# define  COMP_CLASS       "Toestel type:   %s    Klasse: %s\n"
# define  LOGGER           "Logger type:    %s  s/n: %s\n"
# define  FILE_NAME        "Logger file:    %s\n"
# define  ENG_DETECTION    "Motor detectie: "
# define  ST_ED_NONE       "Geen"
# define  ST_ED_ENL        "ENL\n"
# define  ST_ED_MOP        "MOP\n:"
# define  ST_ED_ENL_MOP    "ENL + MOP\n"
# define  ST_ED_RPM        "RPM\n"
# define  ST_ED_EON        "ON-OFF\n"
# define  MOP_SENSOR       "MOP sensor:     %s\n"
# define  ENL_THRESHOLD    "ENL Threshold:  %0.0f\n"
# define  FLT_DATA_GEN     "VLUCHTGEGEVENS ALGEMEEN\n"
# define  AIRF_TO          "Vliegveld van vertrek\n"
# define  BENELUX_YES      "    %s  %s        BeNeLux: JA\n"
# define  BENELUX_NO       "    %s  %s        BeNeLux: NEE\n"
# define  PT_START         "Vertrekpunt \n"
# define  PT_POS           "    pos.: %s\n"
# define  TIME             "    tijd: %s UTC\n"
# define  ALTI             "    alti: %d m\n"
# define  RELEASE          "Ontkoppelpunt\n"
# define  RELEASE_POS      "    pos.: %s = %3.0f° %.1f km van vertrek\n"
# define  LANDING          "Landingspunt \n"
# define  LDG_POS          "    pos.: %s = %3.0f° %.1f km van vertrek\n"
# define  TO_TO_LDG        "Afstand take-off tot landing: %0.1f\n"
# define  FLT_TIME         "Totale vluchtduur: %s\n"
# define  ENGINE           "Motorgebruik\n"
# define  ENG_NOT_DET      "    aantal keren: NIET GEDETECTEERD\n"
# define  ENG_MANUAL       "    Motordetectie verstoord. Begin- en eindtijd zweefvlucht ingegeven.\n"
# define  ENG_RUNS         "    aantal keren: %d\n"
# define  ENG_TEST         "    motor testloop:   %.1f min @ %s UTC\n"
# define  ENG_TIME_TOT     "    totale motortijd: %.1f min\n"
# define  ANAL_MAN         "\nManuele analyse tussen:\n"
# define  ANAL_MAN_1       "    tijd: %s UTC, en\n"
# define  ANAL_MAN_2       "    tijd: %s UTC\n"
# define  DISABLED         "    UItgeschakeld:\n"
# define  DIS_RELEASE      "      Detectie ontkoppelen\n"
# define  DIS_ENG          "      Detectie motorgebruik\n"
# define  DIS_LDG          "      Detectie landing\n\n"
  // Free flight data
# define  FREE_FLIGHT      "VRIJE VLUCHT ANALYSE\n"
# define  FF_TPT_OPT       "Keerpunten vrije vlucht optimalisatie: %d\n"
# define  FF_START         "    Start :  "
# define  FF_FINISH        "    Finish:  "
# define  FF_PT            "    punt %d:  "
# define  FF_LEGS          "Benen vrije vlucht optimalisatie\n"
# define  FF_LEG_N         "    been %d: %6.1f km %3.0f° @ %5.1f km/h\n"
# define  FF_DIST          "Afgelegde afstand:                   %0.1f km\n"
# define  FF_TIME          "Duur:                                %s\n"
# define  FF_SPEED         "Gemiddelde snelheid:                 %0.1f km/h\n"
# define  FF_TO_TO_START   "Afst. start - vliegveld van vertrek: %0.1f km\n"
# define  FF_TO_TO_FINISH  "Afst finish - vliegveld van vertrek: %0.1f km\n"
# define  FF_START_ALT     "Starthoogte:                         %0.1f m\n"
# define  FF_FINISH_ALT    "Finishhoogte:                        %0.1f m\n"
# define  FF_HEIGHT_LOSS   "Hoogteverlies vrije vlucht:          %0.0f m\n"
# define  FF_SCORE         "Totale score afstand:                %.1f km\n"
  // Speed track data
# define  SPEEDTRACK       "BESTE SNELHEIDSTRAJECT ANALYSE\n"
# define  ST_TOT_TIME      "Duur van snelheidscompetitie: %.0f uur \n"
# define  ST_START         "Startpunt\n"
# define  ST_POS           "    pos.:    %s = %3.0f° %.1f km van vertrek\n"
# define  ST_TIME          "    tijd:    %s UTC\n"
# define  ST_ALT           "    alti:    %d m\n"
# define  ST_FINISH        "Finishpunt\n"
# define  ST_MAX_DIST      "Grootste afstand in %.0f uur: %.1f km\n"
# define  ST_MAX_SPD       "Beste snelheid in %.0f uur:   %5.1f km/h\n"
# define  ST_HEIGHT_LOSS   "Hoogteverlies:             %d m\n"
  // Declared task data
  // Task declaration
# define  DECL_TASK        "OPGEGEVEN PROEF ANALYSE\n"
# define  DT_NONE          "Geen proef opgegeven\n"
# define  DT_MIN_LEG_DIST  "Minimaal toegelaten lengte van de benen: %0.0f km\n"
# define  DT_TOT_DIST      "Opgegeven proef: %0.1f km  "
# define  DT_VALID         "GELDIG\n"
# define  DT_INVALID       "NIET GELDIG\n"
# define  DT_DEPARTURE     "    vertrek: %s   %s\n"
# define  DT_START         "    start:   %s   %s\n"
# define  DT_TPT           "    punt %d:  %s   %s  %6.1f km %3.0f°\n"
# define  DT_FINISH        "    finish:  %s   %s  %6.1f km %3.0f°\n"
# define  DT_LDG           "    landing: %s   %s\n"
  // Task performance
# define  DT_RESULT        "Keerpunten gerond en afgelegde afstand\n"
# define  DTR_START        "    start:   %s   @ %s UTC, %5.0f m\n"
# define  DTR_TPT          "    punt %d:  %s   @ %s UTC, %5.0f m : %6.1f km  %5.1f km/h\n"
# define  DTR_FAILED_PT    "    punt %d:  %s   niet gerond\n"
# define  DTR_BREAK_PT     "    afbreekpunt: %s  @ %s UTC, %0.0f m : %0.1f km %0.1f km/h\n"
# define  DTR_FINISH       "    finish:  %s   @ %s UTC, %5.0f m : %6.1f km  %5.1f km/h\n"
# define  DTR_FNSH_FAIL    "    finish:   %s   niet gerond\n"
# define  DTR_START_FAIL   "    start: NIET GEROND\n"
# define  DTR_SUCCESS      "Proef GELUKT\n"
# define  DTR_FAILED       "Proef NIET GELUKT\n"
# define  DTR_BEERCAN_USED "Afstanden gepenaliseerd omwille van gebruik van beercan\n"
# define  DTR_TOT_DIST     "Afgelegde afstand:        %.1f km\n"
# define  DTR_TOT_TIME     "Duur van de proef:        %s\n"
# define  DTR_V            "Gemiddelde snelheid:      %.1f km/h\n"
# define  DTR_START_ALT    "Start hoogte:             %.0f m\n"
# define  DTR_STRT_ALT_ACT "Hoogte passeren start:    %.0f m\n"
# define  DTR_FIN_ALT_ACT  "Hoogte passeren finish:   %.0f m\n"
# define  DTR_BP_ALT_ACT   "Hoogte aan break-punt:    %.0f m\n"
# define  DTR_HEIGHT_LOSS  "Hoogteverlies over proef: %.0f m\n"
# define  DTR_SCORE        "Totale score afstand:     %.1f km\n"
// Triangle data
# define  TRIANGLE         "DRIEHOEK ANALYSE\n"
# define  TRI_LEG          "Been %d: %6.1f km\n"
# define  TRI_DIST         "Totaal: %6.1f km\n"
// Runs analysis
# define  RUNS             "RUNS ANALYSE\n"
# define  RUNS_NAME        "    Stretch %s\n"
# define  RUNS_LAPS        "       Aantal maal:         %d\n"
# define  RUNS_TIME        "       Totale tijd:         %s\n"
# define  RUNS_SPEED       "       Gemiddelde snelheid: %0.1f\n"

// Flight duration data
# define  FLIGHT_TIME      "DUURVLUCHT ANALYSE\n"
# define  FT_MAX           "Blok langste zweefvlucht\n"
# define  FT_START         "    begin: %s UTC\n"
# define  FT_END           "    einde: %s UTC\n"
# define  FT_TIME          "    duur:  %s\n"
# define  FT_NO_SOARING    "    Geen voldoend lange blok gevonden"
  // Altitude gain data
# define  ALT_GAIN         "HOOGTEWINST ANALYSE\n"
# define  AG_MAX           "Blok grootste hoogtewinst\n"
# define  AG_LOW           "    laag punt:   %5d m @ %s UTC\n"
# define  AG_HIGH          "    hoog punt:   %5d m @ %s UTC\n"
# define  AG_GAIN          "    hoogtewinst: %5.0f m\n"
  // Charronde data
# define  BRUSSEL          "CHARRONDE ANALYSE\n"
# define  BRU_T1           "    Eerste punt: %s  bearing %0.1f°\n"
# define  BRU_T2           "    Tweede punt: %s  bearing %0.1f°\n"
# define  BRU_ANGLE        "    Totale hoek: %0.1f°\n"
  // Messages 
# define  MESSAGES         "MELDINGEN EVALUATIEPROGRAMMA\n"
# define  RUN_TIME         "Rekentijd %d\n"
# define  DEBUG_MSG        "DEBUG BERICHTEN\n"
#endif


unsigned int TC0 = GetTickCountOwn();
void WriteMsg(FILE *F);       // Write al  messages except task messages
void WriteTskMsg(FILE *F);    // Write task messages only

void WriteText(void)
{ 
  int    i, len, np;
  char   st[MAXLEN];
  double t, d, h;
  FILE  *F;
  CFlight *pF = 0;
  
  if ( sbMaxT )        pF = sbMaxT->pOwner;
  else if ( FlightA )  pF = FlightA[0];
  if ( !pF )  report_error ( EC_FR_NO_FLIGHT, NULL );
  sFlightPoint *ptf = pF->ptA;

  F = fopen ( file_text,"w" );
  if ( !F ) 
    report_error(EC_OUT_TEXTFILE, NULL);

  // Logger data
  fprintf ( F, SCORING_VERSION, Version);
  fprintf ( F, LOGGER_DATA );
  fprintf ( F, FLIGHT_DATE,    Trace.Day, Trace.Month, Trace.Year);
  fprintf ( F, PILOT,          Trace.Pilot);
  fprintf ( F, REGISTRATION,   Trace.GliderReg);
  fprintf ( F, COMP_CLASS,     Trace.GliderType, Trace.CompClass);
  fprintf ( F, LOGGER,         pLogger->Name, LoggerId);
  fprintf ( F, FILE_NAME,      IGC_name );
  strcpy ( st, ENG_DETECTION );
  switch ( Engine.Detection )
  {
    case ED_NONE:    strcat ( st, ST_ED_NONE );    break;
    case ED_ENL:     strcat ( st, ST_ED_ENL );     break;
    case ED_MOP:     strcat ( st, ST_ED_MOP );     break;
    case ED_ENL_MOP: strcat ( st, ST_ED_ENL_MOP ); break;
    case ED_RPM:     strcat ( st, ST_ED_RPM );     break;
    case ED_EON:     strcat ( st, ST_ED_EON );     break;
  }
  fprintf ( F,"%s", st );
  switch ( Engine.Detection )
  { case ED_MOP:
    case ED_ENL_MOP:
      fprintf ( F, MOP_SENSOR, Trace.MOPsensor );
  }
  switch ( Engine.Detection )
  { case ED_ENL:
    case ED_MOP:
    case ED_ENL_MOP:
      fprintf ( F, ENL_THRESHOLD, Engine.Threshold );
  }
  fprintf ( F, "\n" );

  // Samnvatting
  fprintf ( F, "SAMENVATTING\n" );
  fprintf ( F, "Vluchttijd     : %s\n",        stHour ( pF->dTime ( 0, pF->npt-1 ) ) );
  fprintf ( F, "Duurvlucht     : %s\n",        stHour ( sbMaxT->dTime ( 0, sbMaxT->npt-1 ) ) );
  if ( sbMaxFF )
  {
  fprintf ( F, "Vrije vlucht   : %6.1f km\n",  sbMaxFF->FF.ScoreDist );
  fprintf ( F, "Fasttrack      : %6.1f kmh\n", sbMaxFF->FT.Speed );
  }
  if ( sbMaxDT   &&   sbMaxDT->DT.ScoreDist > 100. )
  fprintf ( F, "Opgegeven proef: %6.1f km\n",  sbMaxDT->DT.ScoreDist );
  if ( TriM.TotDist > 100. )
  fprintf ( F, "Driehoek       : %6.1f km\n",  TriM.TotDist );
  if ( sbMaxFF    &&   sbMaxFF->BRU.Angle >= 150. )
  fprintf ( F, "Charronde      : %6.1f°\n",    sbMaxFF->BRU.Angle );
  if ( sbMaxAG    &&   sbMaxAG->AltGain >= 1000. )
  fprintf ( F, "Hoogtewinst    : %6.1f m\n",  sbMaxAG->AltGain );
  fprintf ( F, "\n" );

  // General flight data
  fprintf ( F, FLT_DATA_GEN );
  // Take-off point
  sAirfield *pA = &Airfields.List[pF->AirfTO];
  fprintf ( F, AIRF_TO );
  if ( pA->BNL )  fprintf ( F, BENELUX_YES, pA->icao, pA->name );
  else            fprintf ( F, BENELUX_NO,  pA->icao, pA->name );
  fprintf ( F, PT_START );
  fprintf ( F, PT_POS, stPos ( pF, 0 ) );
  fprintf ( F, TIME, pF->stHour(0) );
  fprintf ( F, ALTI, ptf[0].Altitude );

  if ( FLT_Automatic )
  { // Release point
    np = pF->StartSoar;
    fprintf ( F, RELEASE );
    fprintf ( F, RELEASE_POS, stPos ( pF, np ), pF->OrthoTrk ( 0, np ), pF->OrthoDist ( 0, np ) );
    fprintf ( F, TIME,        pF->stHour ( np ) );
    fprintf ( F, ALTI,        ptf[np].Altitude );
    // Landing point
    np = pF->npt-1;
    fprintf ( F, LANDING );
    fprintf ( F, LDG_POS, stPos ( pF, np ), pF->OrthoTrk(0, np), pF->OrthoDist(0, np) );
    fprintf ( F, TIME, pF->stHour(np) );
    fprintf ( F, ALTI, ptf[np].Altitude );
    fprintf ( F, TO_TO_LDG, pF->OrthoDist ( 0, np ) );
    // Total flight time
    fprintf ( F, FLT_TIME, stHour ( pF->dTime ( 0, np ) ) );

    if ( Engine.Installed )
    { fprintf ( F, ENGINE );
      if      ( Engine.Detection == ED_NONE )  fprintf ( F, ENG_NOT_DET );
      else if ( !FLT_Automatic )               fprintf ( F, ENG_MANUAL );
      else                                     fprintf ( F, ENG_RUNS, pF->EngRuns );
      if ( pF->EngRuns > 0)
      { if ( pF->EngTest >= 0 )
        { np = pF->EngA[pF->EngTest].On;
          t = 60. * ( pF->ptA[pF->EngA[pF->EngTest].Off].Time - pF->ptA[np].Time );
          fprintf ( F, ENG_TEST, t, pF->stHour(np) );
        }
        fprintf ( F, ENG_TIME_TOT, pF->EngTimeTot * 60. );
      }
    }
    fprintf ( F, "\n" );
  }
  else
  { fprintf ( F, ANAL_MAN );
    fprintf ( F, ANAL_MAN_1, pF->stHour ( pF->StartSoar ) );
    fprintf ( F, ANAL_MAN_2, pF->stHour ( pF->npt-1 ) );
    fprintf ( F, DISABLED );
    fprintf ( F, DIS_RELEASE );
    fprintf ( F, DIS_ENG );
    fprintf ( F, DIS_LDG );
  }

  // Free flight data
  if ( sbMaxFF )
  { fprintf ( F, FREE_FLIGHT );
    fprintf ( F, FF_TPT_OPT, sbMaxFF->FF.npt );
    for ( i = 0;   i < sbMaxFF->FF.npt;   i ++ )
    { np = sbMaxFF->FF.TptA[i].i;
      if ( i == 0 )                          fprintf ( F, FF_START );
      else if ( i == sbMaxFF->FF.npt - 1 )   fprintf ( F, FF_FINISH );
      else                                   fprintf ( F, FF_PT, i );
      fprintf ( F, "%s  @ %s UTC, %5d m\n", stPos ( sbMaxFF, np ), sbMaxFF->stHour(np), sbMaxFF->ptA[np].Altitude );
    }
    fprintf ( F, FF_LEGS );
    for ( i = 0;   i < (sbMaxFF->FF.npt-1);   i ++ )
      fprintf ( F, FF_LEG_N, 
                     i + 1, sbMaxFF->FF.TptA[i].Dist, sbMaxFF->FF.TptA[i].Trk, sbMaxFF->FF.TptA[i].Spd );

    fprintf ( F, FF_DIST, sbMaxFF->FF.Dist );
    fprintf ( F, FF_TIME, stHour ( sbMaxFF->FF.Time ) );
    fprintf ( F, FF_SPEED, sbMaxFF->FF.Speed );
    np = sbMaxFF->FF.TptA[0].i;
    fprintf ( F, FF_TO_TO_START, sbMaxFF->pOwner->wgs84 ( 0, sbMaxFF->Start +np ) );
    np = sbMaxFF->FF.TptA[sbMaxFF->FF.npt-1].i;
    fprintf ( F, FF_TO_TO_FINISH, sbMaxFF->pOwner->wgs84 ( 0, sbMaxFF->Start + np ) );
    fprintf ( F, FF_START_ALT,   sbMaxFF->FF.StartAlt );
    fprintf ( F, FF_FINISH_ALT,  sbMaxFF->FF.FinishAlt );
    fprintf ( F, FF_HEIGHT_LOSS, sbMaxFF->FF.HeightLoss );
    fprintf ( F, FF_SCORE,       sbMaxFF->FF.ScoreDist );
    fprintf ( F, "\n" );

    // Speed track data
    if ( SpeedComp )
    { fprintf ( F, SPEEDTRACK );
      fprintf ( F, ST_TOT_TIME, FastTrackTime );

      np = sbMaxFF->FT.i0;
      h = sbMaxFF->pOwner->OrthoTrk ( 0, np + sbMaxFF->Start );
      d = sbMaxFF->pOwner->wgs84    ( 0, np + sbMaxFF->Start );
      fprintf ( F, ST_START );
      fprintf ( F, ST_POS,  stPos ( sbMaxFF, np ), h, d );
      fprintf ( F, ST_TIME, stHour ( sbMaxFF->FT.t0 ) );
      fprintf ( F, ST_ALT,  sbMaxFF->FT.Alt0 );

      np = sbMaxFF->FT.i2;
      h = sbMaxFF->pOwner->OrthoTrk ( 0, np + sbMaxFF->Start );
      d = sbMaxFF->pOwner->wgs84    ( 0, np + sbMaxFF->Start );
      fprintf ( F, ST_FINISH );
      fprintf ( F, ST_POS,  stPos ( sbMaxFF, np ), h, d );
      fprintf ( F, ST_TIME, stHour ( sbMaxFF->FT.t2 ) );
      fprintf ( F, ST_ALT,  sbMaxFF->FT.Alt2 );

      fprintf ( F, ST_MAX_DIST,    FastTrackTime, sbMaxFF->FT.Dist );
      fprintf ( F, ST_MAX_SPD,     FastTrackTime, sbMaxFF->FT.Speed );
      fprintf ( F, ST_HEIGHT_LOSS, sbMaxFF->FT.HeightLoss );
      fprintf(F,"\n");
    }
  }

  // Declared task data
  // Task declaration  
  fprintf ( F, DECL_TASK );
  if ( !Task.Declared )
    fprintf ( F, DT_NONE );
  else
  { fprintf ( F, DT_MIN_LEG_DIST, dtMinLegDist );
    fprintf ( F, DT_TOT_DIST, Task.TotDist );
    if ( Task.Valid ) fprintf ( F, DT_VALID );
    else              fprintf ( F, DT_INVALID );
    WriteTskMsg ( F );
    len = 0;
    for ( i = 0;   i <= Task.nPoints+1;  i++ )
      len = max ( len, (int)strlen ( Task.TPT[i].Name ) );
    for ( i = 0;   i <= Task.nPoints+1;   i++ )
      AppendSpaces ( Task.TPT[i].Name, len );
      
    fprintf ( F, DT_DEPARTURE, Task.TPT[0].Name, stPosTask(0) );
    fprintf ( F, DT_START, Task.TPT[1].Name, stPosTask(1) );
    for ( i = 2;   i < Task.nPoints;   i ++ )
    { fprintf ( F, DT_TPT, i-1, Task.TPT[i].Name, stPosTask(i), Task.TPT[i-1].Dist, Task.TPT[i-1].Track );
    }
    np = Task.nPoints;
    fprintf ( F, DT_FINISH, Task.TPT[np].Name, stPosTask(np), Task.TPT[np-1].Dist, Task.TPT[np-1].Track );
    np = Task.nPoints+1;
    fprintf ( F, DT_LDG, Task.TPT[np].Name, stPosTask(np) );
  }

  // Task performance
  if ( sbMaxDT )
  { if ( Task.Valid )
    { sTaskAnalysis *pA = &sbMaxDT->DT;
      fprintf ( F, DT_RESULT );
      if ( pA->Leg[1].Index > 0 )
      { // Start point
        fprintf ( F, DTR_START, Task.TPT[1].Name, stHour(pA->Leg[1].Time), pA->Leg[1].Alt );
        
        // Turn points
        for ( i = 2;   i < Task.nPoints;   i++ )
        { if ( pA->Leg[i].Index > 0 )
          { fprintf ( F, DTR_TPT, i-1, Task.TPT[i].Name, stHour(pA->Leg[i].Time), pA->Leg[i].Alt, pA->Leg[i-1].DMG, pA->Leg[i-1].Speed );
          }
          else
          { fprintf ( F, DTR_FAILED_PT, i-1, Task.TPT[i].Name );
            fprintf ( F, DTR_BREAK_PT, stPos(&pA->BreakPointPos), stHour(pA->BreakPointTime), pA->BreakPointAlt, pA->Leg[i-1].DMG, pA->Leg[i-1].Speed );
            break;
          }
        }
        
        // Finish point
        int n = Task.nPoints;
        if ( pA->Leg[n-1].Index > 0 )
        { if ( pA->Leg[n].Index > 0 )
            fprintf ( F, DTR_FINISH, Task.TPT[n].Name, stHour(pA->Leg[n].Time), pA->Leg[n].Alt, pA->Leg[n-1].DMG, pA->Leg[n-1].Speed );
          else
          { fprintf ( F, DTR_FNSH_FAIL, Task.TPT[n].Name );
            fprintf ( F, DTR_BREAK_PT, stPos(&pA->BreakPointPos), stHour(pA->BreakPointTime), pA->BreakPointAlt, pA->Leg[n-1].DMG, pA->Leg[n-1].Speed );
          }
        }
      }
      else fprintf ( F, DTR_START_FAIL );

      // Total distance done
      if ( pA->Finished )    fprintf ( F, DTR_SUCCESS );
      else                   fprintf ( F, DTR_FAILED );
      if ( pA->BeercanUsed ) fprintf ( F, DTR_BEERCAN_USED );
      fprintf ( F, DTR_TOT_DIST,     pA->TotDist);
      fprintf ( F, DTR_TOT_TIME,     stHour ( pA->TotTime ) );
      fprintf ( F, DTR_V,            pA->AvgSpeed );
      fprintf ( F, DTR_START_ALT,    pA->StartAlt );
      fprintf ( F, DTR_STRT_ALT_ACT, pA->Leg[1].Alt );
      if ( pA->Finished )    fprintf ( F, DTR_FIN_ALT_ACT, pA->FinishAlt );
      else                   fprintf ( F, DTR_BP_ALT_ACT,  pA->FinishAlt );
      fprintf ( F, DTR_HEIGHT_LOSS,   pA->HeightLoss );
      fprintf ( F, DTR_SCORE,         pA->ScoreDist );
      
    } // End if TaskValid
  }
  fprintf(F,"\n");

  // Triangle data
  if ( TriM.TotDist >= 60. )
  { fprintf ( F, TRIANGLE );
    for ( i = 0;   i < 3;   i ++ )
    { fprintf ( F, TRI_LEG, i, TriM.Legs[i] );
    }
    fprintf ( F, TRI_DIST, TriM.TotDist );
    fprintf ( F, "\n" );
  }

  if ( Runs.nStretches > 0 )
  { fprintf ( F, RUNS );
    for ( i = 0;   i < Runs.nStretches;   i ++ )
    { fprintf ( F, RUNS_NAME,  Runs.Stretches[1].Name );
      fprintf ( F, RUNS_LAPS,  Runs.Stretches[1].nLaps );
      fprintf ( F, RUNS_TIME,  stHour ( Runs.Stretches[1].tTotal ) );
      fprintf ( F, RUNS_SPEED, Runs.Stretches[1].aSpeed );
    }
    fprintf ( F, "\n" );
  }

  // Flight duration data
  fprintf ( F, FLIGHT_TIME );
  fprintf ( F, FT_MAX );
  if ( sbMaxT )
  { np = sbMaxT->npt-1;
    fprintf ( F, FT_START, sbMaxT->stHour(0) );
    fprintf ( F, FT_END,   sbMaxT->stHour(np) );
    fprintf ( F, FT_TIME,  stHour ( sbMaxT->dTime ( 0, np ) ) );
  }
  else fprintf ( F, FT_NO_SOARING );
  fprintf ( F, "\n" );

  // Altitude gain data
  if ( sbMaxAG )
  { fprintf ( F, ALT_GAIN );
    fprintf ( F, AG_MAX );
    fprintf ( F, AG_LOW,  sbMaxAG->ptA[sbMaxAG->iLow].Altitude, sbMaxAG->stHour ( sbMaxAG->iLow ) );
    fprintf ( F, AG_HIGH, sbMaxAG->ptA[sbMaxAG->iHigh].Altitude, sbMaxAG->stHour ( sbMaxAG->iHigh ) );
    fprintf ( F, AG_GAIN, sbMaxAG->AltGain );
    fprintf ( F, "\n" );
  }

  if ( sbMaxFF   &&   fabs ( sbMaxFF->BRU.Angle ) > 15. )
  { fprintf ( F, BRUSSEL );
    fprintf ( F, BRU_T1,    sbMaxFF->stHour ( sbMaxFF->BRU.i1 ), sbMaxFF->BRU.t1 );
    fprintf ( F, BRU_T2,    sbMaxFF->stHour ( sbMaxFF->BRU.i2 ), sbMaxFF->BRU.t2 );
    fprintf ( F, BRU_ANGLE, sbMaxFF->BRU.Angle );
    fprintf ( F, "\n" );
  }

  // Messages
  fprintf ( F, MESSAGES );
  WriteMsg ( F );
  fprintf ( F, RUN_TIME, GetTickCountOwn() - TC0 );  

  // Debug messages
  if ( nDebugMsg )
  { fprintf ( F, "\n" );
    fprintf ( F, DEBUG_MSG );
    for ( i = 0;   i < nDebugMsg;   i ++ )
    { fprintf ( F, GetDebugMsg(i) );
    }
  }
  fprintf ( F, "\n" );

  fclose ( (void**)&F );
}




void WriteMsg(FILE *F)
{ // Write al  messages except task messages
  for ( int i = 0;  i < nErrors;  i++ )
  { if ( Errors[i].code >= TM_FIRST   &&    Errors[i].code <= TM_LAST )  continue;
    fprintf ( F, "%s\n", Errors[i].text );
  }
}



void WriteTskMsg(FILE *F)
{ // Write task messages only
  for ( int i = 0;  i < nErrors;  i++ )
  { if ( Errors[i].code < TM_FIRST )   continue;
    if ( Errors[i].code > TM_LAST  )   continue;
    fprintf ( F, "%s\n", Errors[i].text );
  }
}
  

// stPos: converts decimal (lat,lon) position to DDD°MM.mmm' strings
// bSplit: if TRUE: splits lat & lo,g in two strings
char *stPos(CFlightBase *p, int i, BOOL bSplit /*= FALSE*/)
{ return stPos ( &p->ptA[i].pt, bSplit );
}


char *stPosTask(int i, BOOL bSplit /*= FALSE*/)
{ return stPos ( &Task.TPT[i].pt, bSplit );
}



char *stPos(sCoord *p, BOOL bSplit /*= FALSE*/)
{ static char st[48];
  double lat = p->Lat;
  double lon = p->Lon;
  int    latD, lonD;
  double latM, lonM;
  char   signEW = 'E';
  char   signNS = 'N';
  
  latD = (int)(lat*c180divPi);  latM = 60.0*(lat*c180divPi - (double)latD);
  lonD = (int)(lon*c180divPi);  lonM = 60.0*(lon*c180divPi - (double)lonD);
  if (lat < 0.0)
  {
    signNS = 'S';
    latD  *= -1;
    latM  *= -1;
  }
  if (lon < 0.0)
  {
    signEW = 'W';
    lonD  *= -1;
    lonM  *= -1;
  }

  sprintf ( st, "%2d°%06.3f\'%c %3d°%06.3f\'%c", latD, latM, signNS, lonD, lonM, signEW );
  if ( bSplit )
  { st[11] = '\n';
  }

  return st;
}
