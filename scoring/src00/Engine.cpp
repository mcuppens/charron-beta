#include "Global.h"

CEngine Engine;

extern BOOL bNoENL;         // Allow file without ENL registration

void AnalyseEngine(void)
{ if ( !nFlights ) return;
  if ( !Engine.Analyse() )
  { // Create a single soaring block from Release to end in all flights
    CFlight *pF;
    for ( int i = 0;   i < nFlights;   i ++ )
    { pF = FlightA[i];
      pF->AddSoarBlock ( pF->Release, pF->npt - 1 );
    }
  }
}




CEngine::CEngine(void)
{ Installed     = FALSE;
  InstAuto      = FALSE;
  SkipTestRun   = TRUE;
  Threshold_Min = 0.;
  Threshold_Max = 0;
  Threshold_Man = 0.;
  Detection     = ED_NONE;

  MinENL        = 0.;
  MaxENL        = 0.;
  Average       = 0.;
  Median        = 0.;
  Sigma         = 0.;
  Threshold     = 0.;
}

CEngine::~CEngine(void)
{
}


// Check engine runs & get soaring blocks
// returns FALSE if soaring blocks were not detected within this function
// => detect frome caller
BOOL CEngine::Analyse(void)
{
  if ( !Installed     )   return FALSE;
  if ( !FLT_Automatic )   return FALSE;   // StartSoar set via the command line
  if ( Detection == ED_NONE )
  { // No engine data found in IGC file
    if ( bNoENL )
    { // No ENL data required as per command line, Non critical error
      report_error ( EC_NO_ENL_OVRD, NULL );
      Installed  = FALSE;
      return FALSE;
    }
    else // Critical error
    { report_error ( EC_NO_ENL, NULL );
    }
  }

  Filter();
  Statistics();
  GetThreshold();
  SetFlag();               // Set & cleanup the engine running flag for each fix in the trace
  SetFlights();
  if ( debugEngine )  fflush ( stdout );
  return TRUE;
}





void CEngine::Filter(void)
{ // Dampen the ENL / MOP over the whole trace
  // Always include both ENL & MOP: max of both
  int i;
  double tauInc = 15. / 3600.;
  double tauDec = 45. / 3600.;
  double tau, dt;
  double a = max ( Trace.ptA[0].ENL, Trace.ptA[0].MOP );
  double ai;
  if ( Detection == ED_MOP )      { tauInc /= 3.;  tauDec /= 3.; }
  if ( Detection == ED_ENL_MOP )  { tauInc /= 2.;  tauDec /= 2.; }
  for ( i = 1;   i < Trace.npt;  i ++ )
  { // Forward run
    ai = max ( Trace.ptA[i].ENL, Trace.ptA[i].MOP );
    tau = ai >= a  ?  tauInc  :  tauDec;
    dt  = Trace.dTime ( i-1, i );
    if ( dt > tau ) a  = ai;
    else            a += ( ai - a ) * dt / tau; 
    Trace.ptA[i].fENL = a;
  }
  a = Trace.ptA[Trace.npt-1].fENL;
  for ( i = Trace.npt-2;  i >= 0;   i -- )
  { // Backward run
    tau = Trace.ptA[i].fENL >= a  ?  tauInc  :  tauDec;
    dt  = Trace.dTime ( i, i+1 );
    if ( dt > tau ) a  = Trace.ptA[i].fENL;
    else            a += ( Trace.ptA[i].fENL - a ) * dt / tau; 
    Trace.ptA[i].fENL = a;
  }
}




void CEngine::Statistics(void)
{ // For all flights combined, get average, median and std deviation
  // Calculate only while in flight
  int nf, i;
  int np = 0;
  int ENM;     // Max ( ENL, MOP );
  double Sum = 0.;
  MinENL = 9999.;
  MaxENL = -9999.;
  CFlight *pF;
  sFlightPoint *pt;

  for ( nf = 0;   nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    pt = pF->ptA;
    for ( i = 0;   i < pF->npt;   i ++,  np ++,  pt ++ )
    { ENM = max ( pt->ENL, pt->MOP );
      Sum   += ENM;
      Sigma += fabs ( (double)ENM );
      MinENL = min ( MinENL, ENM );
      MaxENL = max ( MaxENL, ENM );
    }
  }
  Average = Sum / np;
  Median  = ( MinENL + MaxENL ) / 2.;

  for ( nf = 0;   nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    pt = pF->ptA;
    for ( i = 0;   i < pF->npt;   i ++,  np ++,  pt ++ )
    { ENM = max ( pt->ENL, pt->MOP );
      Sigma += fabs ( ENM - Average );
    }
  }
  Sigma /= np;
}




void CEngine::GetThreshold(void)
{ //Determine the ENL threshold
  if ( Threshold_Man )
  { Threshold = Threshold_Man;
  }
  else switch ( Detection )
  { case ED_NONE:  break;  // This case has been intercepted above
    case ED_ENL:
    case ED_MOP:
    case ED_ENL_MOP:
    case ED_RPM:
      Threshold = min ( ( Average + 6. * Sigma ), ( Average + ( MaxENL - Average ) / 2. ) );
      break;
    case ED_EON:
      Threshold = 500.;
      break;
  }

  if ( debugEngine )
  { printf ( "\n#### ENL engine detection ####\n" );
    printf ( "\nAutomatic ENL threshold: %0.0f\n", Threshold );
  }

  if ( Threshold < Threshold_Min )
  { Threshold = Threshold_Min;
    if ( debugEngine )   printf ( "\nThreshold set to lower limit %0.0f\n", Threshold_Min );
  }
  if ( Threshold > Threshold_Max )
  { Threshold = Threshold_Max;
    if ( debugEngine )   printf ( "\nThreshold set to higher limit %0.0f\n", Threshold_Max );
  }
  if ( debugEngine ) 
  { printf ( "\n#### ENL engine detection ####\n");
    printf ( "Average = %f \nMedian  = %f \nMaximum = %f \nSigma   = %f\n",
              Average, Median, MaxENL, Sigma );
    printf ( "==> Threshold = %f\n\n", Threshold );
  }
}




void CEngine::SetFlag(void)
{ // Set the engine running flag over the whole trace
  int    i;
  int    iOff = -1;
  int    ENM;
  BOOL   bRunning = FALSE;
  sFlightPoint *pt = Trace.ptA;

  if ( debugEngine ) printf ( ">>> Set all engine flags\n" );
  for ( i = 0;   i < Trace.npt;  i ++,  pt ++ )
  { ENM = max ( pt->ENL, pt->MOP );
    if ( ENM > Threshold )
    { if ( !bRunning )
      { bRunning = TRUE;
      }
      Trace.SetFlag ( i, FLT_ENG_ON, 1 );
    }
    else
    { if ( bRunning )
      { bRunning = FALSE;
        if ( debugEngine )
        { double dt = Trace.dTime ( iOff+1, i-1, 3600. );
          double v  = Trace.Speed ( iOff+1, i-1 );
          double vz = Trace.Vz ( iOff+1, i-1 );
          printf ( "Engine running block %5d %s -> %5d %s, %4.0f\" %3.0fkmh %0.1fmps\n", iOff+1, Trace.stHour(iOff+1), i-1, Trace.stHour(i-1), dt, v, vz );
        }
      }
      Trace.SetFlag ( i, FLT_ENG_ON, 0 );
      iOff = i;
    }
  }

  // Cleanup the engine running flags
  RemoveShort ( 12. );                  // Remove all runs less than 12"
  RemovePerformance ( 170., -1.2 );
  if ( debugEngine )  Debug();
  Consolidate ( 45., 20. );             // Consolidate gaps
  RemoveShort ( EngineTimeThreshold );  // Remove all runs less than EngineTimeThreshold
}




void CEngine::RemoveShort(double tt)
{ // Remove all runs shorter tha tt seconds
  // Take time from first engine on -> first engine off
  if ( debugEngine ) printf ( "\n>>> Remove runs less than %0.0f\"\n", tt );
  int  i, j;
  int  iOff = -1;    // Will receive previous last engine off time
  BOOL bRunning = FALSE;
  sFlightPoint *pt = Trace.ptA;
  for ( i = 0;   i < Trace.npt;  i ++,  pt ++ )
  { if ( pt->Flag & FLT_ENG_ON )
    { bRunning = TRUE;
    }
    else
    { iOff ++;
      if ( bRunning  &&  
           ( i - iOff < 2   ||
             Trace.dTime ( iOff, i, 3600. ) < tt
           )
         )
      { 
        // Remove the running flag for this block
        if ( debugEngine )
          printf ( "Removed short run %4.0f\"  %5d %s -> %5d %s\n", Trace.dTime ( iOff, i, 3600. ), iOff+1, Trace.stHour(iOff), i-1, Trace.stHour(i-1) );
        for ( j = iOff;   j < i;  j ++ )  Trace.SetFlag ( j, FLT_ENG_ON, 0 );
      }
      bRunning = FALSE;
      iOff = i;
    }
  }
}




void CEngine::RemovePerformance(double vLim, double vzLim)
{ if ( debugEngine ) printf ( "\n>>> Remove runs with bad performance\n" );
  int  i, j;
  int  iOff = -1;    // Will receive previous last engine off time
  BOOL bRunning = FALSE;
  double v, vz;
  sFlightPoint *pt = Trace.ptA;
  for ( i = 0;   i < Trace.npt;  i ++,  pt ++ )
  { if ( pt->Flag & FLT_ENG_ON )
    { bRunning = TRUE;
    }
    else
    { if ( bRunning )
      { // Check the perforamnce of this run
        iOff ++;                  // Now points to the first fix with engine flag set
        v  = Trace.Speed ( iOff, i-1 );
        vz = Trace.Vz ( iOff, i-1 );
        if ( vz < vzLim   ||
             v  > vLim
           )
        { if ( debugEngine )
            printf ( "Removed low perfo run %s -> %s, %3.0fkmh %0.1fmps\n", Trace.stHour(iOff), Trace.stHour(i-1), v, vz );
          for ( j = iOff;   j < i;  j ++ )  Trace.SetFlag ( j, FLT_ENG_ON, 0 );
        }
      }
      bRunning = FALSE;
      iOff = i;
    }
  }
}




void CEngine::Consolidate(double tt1, double tt2)
{ if ( debugEngine ) printf ( "\n>>> Consolidate running gaps less than %0.0f\"\n", tt1 );
  int    i, j, i0, i2;
  int    iOn  = 0;     // Last Index with engine running
  int    iOn0, iOff2;
  double dt, v, vz;
  BOOL bRunning = FALSE;
  sFlightPoint *pt = Trace.ptA;

  for ( i = 0;   i < Trace.npt;  i ++,  pt ++ )
  { if ( pt->Flag & FLT_ENG_ON )
    { if ( !bRunning )
      { bRunning = TRUE;
        if ( iOn )
        { dt = Trace.dTime ( iOn, i-1, 3600 );
          if ( dt < tt1 )
          { i2 = iOn;
            i0 = i;
            // Increment i2 until the average ENL drops below threshold
            while ( Trace.ptA[i2].fENL > Threshold  &&  i2 < i0 )  i2 ++;
            // Decrement i0 until the average ENL drops below threshold
            while ( Trace.ptA[i0].fENL > Threshold  &&  i0 > i2 )  i0 --;
            dt = Trace.dTime ( i2, i0, 3600. );
            if ( debugEngine )
            { printf ( "Engine gap iLast %5d %s\n", iOn, Trace.stHour(iOn) );
              printf ( "           i2    %5d %s\n", i2,  Trace.stHour(i2) );
              printf ( "           i0    %5d %s\n", i0,  Trace.stHour(i0) );
              printf ( "           i     %5d %s\n", i,   Trace.stHour(i)  );
              printf ( "           dt1   %5.1f\n",  Trace.dTime ( iOn, i-1, 3600 ) );
              printf ( "           dt2   %5.1f\n",  dt );
            }
            if ( dt < tt2 )
            { // Find the starting time of the first block
              iOn0 = iOn;
              while ( iOn  &&   Trace.GetFlag ( iOn0, FLT_ENG_ON ) )  iOn0 --;
              iOn0 ++;
              // Find the end time of the next block
              iOff2 = i;
              while ( iOff2 < Trace.npt-1  &&   Trace.GetFlag ( iOff2, FLT_ENG_ON ) )  iOff2 ++;
              iOff2 --;
              v  = Trace.Speed ( iOn0, iOff2 );
              vz = Trace.Vz    ( iOn0, iOff2 );
              if ( debugEngine )
              { printf ( "           v     %5.0f\n", v );
                printf ( "           vz    %5.1f\n", vz );
              }
              if ( v < 150.   &&   vz > -1.2 )
              { // Consider time iLast->i as engine running
                for ( j = iOn;   j < i;  j ++ )  Trace.SetFlag ( j, FLT_ENG_ON, 1 );
                if ( debugEngine )  printf ( "           Consolidated\n" );
              }
              else if ( debugEngine )  printf ( "           NOT Consolidated\n" );
            }
            else if ( debugEngine )  printf ( "           NOT Consolidated\n" );
          }
        }
      }
      iOn = i;     // The last fix with engine flag set
    }
    else if ( bRunning )  bRunning = FALSE;
  }
}





void CEngine::SetFlights(void)
{ // Set the  engine array for all flights, using the cleaned engine flag
  int  nf, i;
  BOOL bRunning;
  CFlight *pF = FlightA[0];
  sFlightPoint *pt;
  sEngine E = { 0, 0 };

  for ( nf = 0;   nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    pt = pF->ptA;
    bRunning = FALSE;
    if ( debugEngine )
    { printf ( "\n>>> Setting engine array flight %d %s - %s\n", nf + 1, pF->stHour(0), pF->stHour(pF->npt-1) );
    }
    for ( i = 0;   i < pF->npt;   i ++,  pt ++ )
    { if ( pt->Flag & FLT_ENG_ON )
      { if ( !bRunning )
        { bRunning = TRUE;
          E.On     = i;
        }
      }
      else if ( bRunning )
      { bRunning = FALSE;
        E.Off    = i - 1;
        pF->AddEngine ( &E );
      }
    }
    if ( bRunning )
    { E.Off = pF->npt - 1;
      pF->AddEngine ( &E );
    }
    if ( debugEngine )
    { printf ( "%d runs, %s, %0.0fm climb, %0.1fmps\n", pF->EngRuns, stHour ( pF->EngTimeTot ), pF->EngClimbTot, pF->EngClimbTot / pF->EngTimeTot / 3600. );
    }
    pF->SetSoarBlocks();
  }
}



void CEngine::Debug(void)
{ // Lists all remining engine running blocks
  BOOL   bRunning = FALSE;
  int    iOff = -1;    // Will receive previous last engine off time
  double dt, v, vz;
  sFlightPoint *pt = Trace.ptA;
  printf ( "\n>>> Remaining engine running blocks\n" );
  for ( int i = 0;   i < Trace.npt;  i ++,  pt ++ )
  { if ( pt->Flag & FLT_ENG_ON )
    { bRunning = TRUE;
    }
    else
    { if ( bRunning )
      { // Check the perforamnce of this run
        iOff ++;                  // Now points to the first fix with engine flag set
        dt = Trace.dTime ( iOff, i-1, 3600. );
        v  = Trace.Speed ( iOff, i-1 );
        vz = Trace.Vz ( iOff, i-1 );
        printf ( "Run %s -> %s, %3.0f\" %3.0fkmh %0.1fmps\n", Trace.stHour(iOff), Trace.stHour(i-1), dt, v, vz );
      }
      bRunning = FALSE;
      iOff = i;
    }
  }
}
