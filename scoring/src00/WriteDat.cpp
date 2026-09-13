#include "Global.h"
#include "WriteText.h"
#include "Airfields.h"
#include "Triangle.h"

void BlankLines(FILE *F, int n );


void WriteDat(void)
{ int i;
  CFlight *pF = NULL;
  FILE  *F;

  if ( sbMaxT )        pF = sbMaxT->pOwner;
  else if ( FlightA )  pF = FlightA[0];
  if ( !pF )  report_error ( EC_FR_NO_FLIGHT, NULL );

  F = fopen(file_data, "wt" );
  if ( !F )  (EC_OUT_DATAFILE, NULL); 

  fprintf ( F, "%s\n", Version );   // New
  fprintf ( F, "%s\n", IGC_name );   // New
  // Error messages
  fprintf ( F, "%d  No critical errors\n", EC_NIL );
  fprintf ( F, "%d\n", nErrors );
  for ( i = 0;  i < nErrors;  i++ )
    fprintf ( F, "%3d  %s\n", Errors[i].code, Errors[i].text );
  BlankLines ( F, 1 );       // Obsolete: task messages
  
  // Declarations in logger
  fprintf ( F, "%02d-%02d-%d\n", Trace.Day, Trace.Month, Trace.Year);
  fprintf ( F, "%s %s\n", pLogger->Code, LoggerId );
  fprintf ( F, "%s\n", Trace.Pilot);
  fprintf ( F, "%s\n", Trace.GliderType);
  fprintf ( F, "%s\n", Trace.GliderReg);
  fprintf ( F, "%s\n", Trace.CompClass);

  // Take-off and landing time
  fprintf ( F, "%s\n", pF->stHour(0) );
  fprintf ( F, "%s\n", pF->stHour ( pF->npt-1 ) );
  
  // Release time & altitude
  fprintf ( F, "%s\n", pF->stHour ( pF->Release ) );       // new
  fprintf ( F, "%d\n", pF->ptA[pF->Release].Altitude );
  
  // Flight time, maximum soar time and altitude gain
  fprintf ( F, "%s\n", stHour ( pF->dTime ( 0 , pF->npt-1 ) ) );       // Flight time
  if ( sbMaxT )  fprintf ( F, "%s\n", stHour ( sbMaxT->dTime ( 0, sbMaxT->npt - 1 ) ) );   // Soar time
  else BlankLines ( F, 1 );
  if ( sbMaxAG )   fprintf ( F, "%0.0f\n", sbMaxAG->AltGain );   // altitude gain
  else BlankLines ( F, 1 );
  
  // Declared task distance, & number of poinyts
  fprintf ( F, "%0.1f\n", Task.TotDist );
  fprintf ( F, "%d\n",    Task.nTPT );
  // Results: altitude loss, score distance, average speed
  if ( sbMaxDT )
  { sTaskAnalysis *pTA = &sbMaxDT->DT;
    fprintf ( F, "%d   \n", pTA->Finished );
    fprintf ( F, "%0.1f\n", pTA->TotDist );
    fprintf ( F, "%0.0f\n", pTA->StartAlt );
    fprintf ( F, "%0.0f\n", pTA->Leg[1].Alt );
    fprintf ( F, "%0.0f\n", pTA->HeightLoss );
    fprintf ( F, "%0.1f\n", pTA->ScoreDist );
    fprintf ( F, "%0.1f\n", pTA->AvgSpeed );
  }
  else BlankLines ( F, 7 );

  // Free flight distance, altitude loss, score distance, average speed
  if ( sbMaxFF )
  { fprintf ( F, "%0.1f\n", sbMaxFF->FF.Dist );
    fprintf ( F, "%d\n",    sbMaxFF->FF.npt );
    fprintf ( F, "%0.0f\n", sbMaxFF->FF.HeightLoss );
    fprintf ( F, "%0.1f\n", sbMaxFF->FF.ScoreDist );
    fprintf ( F, "%0.1f\n", sbMaxFF->FF.Speed );
     // Best speed track
    if ( SpeedComp )  fprintf ( F, "%0.1f\n", sbMaxFF->FT.Speed );
    else              fprintf ( F, "0.0\n");
  }
  else BlankLines ( F, 6 );
  
  // Take-off airfield data: BNL value / ICAO code / name
  // BNL = 1 if airfield in BeNeLux, 0 otherwise
  sAirfield *pA = &Airfields.List[pF->AirfTO];
  fprintf ( F, "%d %s %s\n", pA->BNL, pA->icao, pA->name );
  // Take-off point coordinates
  fprintf ( F, "%s\n", stPos ( pF, 0, TRUE ) );
  // Landing point coordinates
  fprintf ( F, "%s\n", stPos ( pF, pF->npt - 1, TRUE ) );
  // Distance take-off - Landing point
  fprintf ( F, "%0.2f\n", pF->OrthoDist ( 0, pF->npt-1 ) );

  // Charronde
  if ( sbMaxFF )
    fprintf ( F, "%0.1f\n", sbMaxFF->BRU.Angle );
  else BlankLines ( F, 1 );

  // Triangle
  if ( TriM.TotDist >= 60. )
  { for ( i = 0;   i < 3;   i ++ )
    { fprintf ( F, "%0.1f\n", TriM.Legs[i] );
    }
    fprintf ( F, "%0.1f\n", TriM.TotDist );
fprintf ( F, "%0.1f\n", TriBF.TotDist );
  }
  else BlankLines ( F, 4 );
BlankLines ( F, 1 );   // TriBF

  // Debug messages
  for ( i = 0;   i < nDebugMsg;   i ++ )
  { fprintf ( F, GetDebugMsg(i) );
  }

  fclose ( (void**)&F );
}


void BlankLines(FILE *F, int n )
{ for ( int i = 0;  i < n;  i ++ )  fprintf ( F, "0\n" );
}
