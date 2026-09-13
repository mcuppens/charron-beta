#include "Global.h"

CTrace Trace;



CTrace::CTrace(void)
{ d24         = 0.;
  bInterupted = FALSE;
  memset ( Pilot,      0, sizeof(Pilot) );
  memset ( GliderType, 0, sizeof(GliderType) );
  memset ( GliderReg,  0, sizeof(GliderReg) );
  memset ( CompClass,  0, sizeof(CompClass) );
  memset ( MOPsensor,  0, sizeof(MOPsensor) );
  Year = Month = Day = 0;
  bMOP = FALSE;
}



CTrace::~CTrace(void)
{ if ( FlightA )
  { for ( int i = 0;   i < nFlights;   i ++ )
    { if ( FlightA[i] ) delete FlightA[i];
    }
    free ( (void**)&FlightA );
  }
  free ( (void**)&ptA );
}



// Init
// - Remove position jumps
// - Split the trace in flights
void CTrace::Init(void)
{ int i;
  double V1, V2;
  // Delete position jumps
  for ( i = 1;   i < (npt-1);   i++ )
  { V1 = Speed ( i - 1 );
    V2 = Speed ( i );

    if ( ( V1 > 500.0 )  &&  ( V2 > 500.0 )  ||  // Jump of a single point
         ( V1 > 700.0 )                          // Sudden offset of several points
       )
    { if (debug)  printf("*********  False GPS fix found at time %s -> deleted\n", stHour(i) );
      DeleteFix ( i );
      i--;
    }
  }

  // Cancel flight if too few fixes
  if ( npt < 100 )   report_error ( EC_SHORT_FLIGHT, NULL );

  if ( FLT_Automatic )
  { GetFlights();
  }
  else
  { // Create a single flight object with the given start and end times
    int Start = FixAtTime ( tStartSoar, -1 );
    AddFlight ( Start, FixAtTime ( tEndSoar, +1 ), FALSE );
    FlightA[nFlights-1]->Release = Start;
  }
}



// Find all flights in the trace
// A flight are separated by either:
// - a portion of non-moving fixes,
// - a gap between fixes of 10 minutes or more
// Flights of less than 50 fixes are discarded from within AddFlight
void CTrace::GetFlights(void)
{ int    i;
  int    Airborne = 0;
  int    nv = 0;
  int    nz = 0;
  int    iTOv = 0;
  int    iTOz = 0;
  int    iTO  = 0;
  int    iLdg;
  int    bGap = 0;    // Set true of the previous flight was discontinued due to a gap in the trace
  double dt;
  double v;
  double vz, vzInt = 0.;
  double tauVz = 30. / 3600.;
  // debugging variables
  char   sti[64];

  for ( i = 0;   i < npt - 1;   i ++ )
  { if ( debugFlt )  sprintf ( sti, "%5d  %s %6d  ", i, stHour(i), ptA[i].Altitude );   
    dt = dTime ( i, i+1 );
    v  = Speed ( i );
    vz = Vz ( i );
    vzInt = ( dt <= tauVz )  ?  ( ( tauVz-dt ) * vzInt + dt * vz ) / tauVz  :  vz;
    if ( Airborne )
    { // Check for long gaps in the trace
      if ( dTime ( i, i+1, 60. ) > 10. )
      { // A gap of more than 10 minutes in the log file: close this flight block
        if ( debugFlt )   printf ( "%s Gap of %0.1f minutes -> %d %s\n", sti, dTime ( i, i+1, 60. ), i+1, stHour( i+1 ) );
        AddFlight ( iTO, i, bGap );
        bGap     = TRUE;
        Airborne = FALSE;
        nv = nz = 0;
        iTOv = iTOz = i + 1;
        continue;
      }
      // Detect landing
      if ( v  > 12.0 )
      { if ( nv )
        { nv --;
          if ( debugFlt )  printf ( "%s speed %0.1f\n", sti, v );
        }
        else
        { if ( debugFlt   &&   iLdg != i )  printf ( "%s LDG reset, high speed\n", sti );
          iLdg = i+1;
        }
        continue;
      }
      // Speed lower than 15
      if ( debugFlt   &&    !nv )   printf ( "%s landing trigger speed %0.1f\n", sti, v );
      nv ++;   nv = min ( nv, 3 );
      if ( dTime ( iLdg, i, 3600. ) < 120.0 )  continue;

      // Low speed during >= 120"
      // Check vario & altitude
      if ( ( ptA[i].Altitude > 2500 )    ||  // Suppose no landing ablove this altitude...
           ( vzInt > 0.3 )               ||  // Still some Vz...
           ( Vz ( i, iLdg ) > 0.2 )
         )
      { if ( iLdg == i - 1 )  // TODO check
        { if ( debugFlt ) printf ( "%s landing rejected alt %d, vz %0.1f, vzInt %0.1f\n", sti, ptA[i].Altitude, vz, vzInt );
          iLdg ++;              // Move the landing point along
        }
        continue;
      }
      // Arriving here means landing occured
      // The landing time found here is not quite exact.
      // The landing time is corrected in the CFlight constructor
      if ( debugFlt )
      { printf ( "%s Landing detect %d %s\n", sti, iLdg, stHour ( iLdg ) );
        printf ( "vzInt %0.1f,  dAlt %0.1f\n", vzInt, dAlt ( i, iLdg ) );
      }
      AddFlight ( iTO, iLdg, bGap );
      bGap     = FALSE;
      Airborne = FALSE;
      nv =  nz = 0;
      iTOv = iTOz = iLdg + 1;
    }
    else   // not Airborne
    { // Detect take-off  *******************************
      if ( v > 20. )
      { // Trigger immediatly if speed above 20 kmh
        if ( debugFlt   &&    !nv )
        { printf ( "%s Take-off trigger speed %0.1f\n", sti, v );
        }
        nv ++;    nv = min ( nv, 3 );   // countback for case speed drops low again
        /* Disabled: too sensitive for spiralling
        if ( ( dt = dTime ( iTOv, i, 3600. ) ) >= 25. )
        { // Avoid detection based on drifting GPS speed: check on average speed
          int i0 = iTOv;
          while ( ( dt = dTime ( i0, i, 6600. ) ) > 25. )
          { // Limit i0 to avoid filtering out due a turn-back
            i0 ++;
          }
          vv = Speed ( i0, i );
          if ( vv < 20. )
          { // Speed may be caused by GPS drift.  Reset TO detection based on speed
            if ( debugFlt )  printf ( "%s TOv reset, average speed %0.1f\n", sti, vv );
            iTOv = i;  nv = 0;
          }
        }
        */
      }
      else
      { if ( nv )
        { nv --;   // Low speeed
          if ( debugFlt )  printf ( "%s speed %0.1f\n", sti, v );
        }
        else
        { // Low speed during more than the set limit: reset TOv detection
          if ( debugFlt   &&   iTOv < i - 1 )  printf ( "%s TOv reset, low speed\n", sti );
          iTOv = i;
        }
      }

      if ( ( vzInt > 2. )   &&   ( v > 5. ) )
      { // Trigger immediately if climbing
        nz ++;   nz = min ( nz, 3 );
        if ( debugFlt   &&    iTOz == i - 1 )
        { printf ( "%s Take-off trigger vz\n", sti );
        }
      }
      else
      { if ( nz )
        { nz --;
          if ( debugFlt )  printf ( "%s vz %0.1f, vzint %0.1f\n", sti, vz, vzInt );
        }
        else
        { // Not climbing during several fixes
          if ( debugFlt &&   iTOz < i - 1 )  printf ( "%s TOvz reset, vz %0.1f, vzint %0.1f\n", sti, vz, vzInt );
          iTOz = i;
        }
      }

      if ( ( dt = dTime ( iTOv, i+1, 3600. ) ) > 90. )
      { // Store take-off point based on speed
        Airborne = TRUE;
        iTO  = iTOv;
        i    = iTO - 1;
        iLdg = iTO;
        nv   = 0;
        if ( debugFlt )  printf ( "%s TO detect %d %s SPEED\n", sti, iTO, stHour(iTO) );
      }
      else if ( dTime ( iTOz, i+1, 3600. ) > 30. )
      { // Store take-off point based on vz
        Airborne = TRUE;
        iTO = min ( iTOv, iTOz );
        i    = iTO - 1;
        iLdg = iTO;
        nv   = 0;
        if ( debugFlt )  printf ( "%s TO detect %d %s VZ\n", sti, iTO, stHour(iTO) );
      }      
    }
  }

  if ( Airborne )
  { // Arrived at the end of the trace and no landing detected
    if ( debugFlt )  printf ( "Airborne upon end of flight\n" );
    AddFlight ( iTO, i, bGap );
  }
  if ( !nFlights )  report_error ( EC_FR_NO_FLIGHT, NULL );
}




void CTrace::AddFlight(int Start, int End, BOOL bGap)
{ if ( ( End - Start ) < 50 )
  { if ( debugFlt )
    { printf ( "*********  Short flight skipped: from %s -> %s\n", stHour(Start), stHour(End) );
    }
    return;
  }

  End = min ( End, npt - 1 );
  realloc_s ( (void**)&FlightA, sizeof(*FlightA), &nFlights );
  CFlight *pF = FlightA[nFlights-1] = new CFlight ( Start, End, bGap );
  if ( !pF )
  { char st[256];
    sprintf ( st, "Memory allocation error flight object %d\n", nFlights - 1 );
    report_error ( EC_MEMORY, st );
  }
  if ( debugFlt )
  { printf ( ">>  FLIGHT #%d, TO at %s  LDG %s", nFlights, stHour(Start), stHour(End) ); 
  }
  pF->GetBaseLine();
  pF->AdjustLdg();
  if ( debugFlt )   printf ( " adjusted %s\n", pF->stHour ( pF->npt-1 ) ); 
}





void CTrace::DeleteFix(int n)
{ if ( n >= npt )  return;
  npt --;
  for ( int i = n;   i < npt;   i ++ )
    memcpy ( &ptA[i], &ptA[i+1], sizeof(sFlightPoint) );
}




void CTrace::AddFix(sFlightPoint *pt)
{ // Don't append bad fixes
  if ( !pt->f3D       )   return;
  if ( pt->Time < 0.0 )   return;
  if ( fabs ( pt->Time - Time0 ) < epsilon )   return;
  if ( IsZeroFix ( &pt->pt ) )   return;

  // Detect passing midnight
  if ( !npt )  Time0 = pt->Time;
  if ( pt->Time < Time0 )
  { if ( Time0 > 24. - 10. / 60. )
    { // Previous fix was close to midnight, current one has passed midnight
      // Increment d24 for day correction
      d24 += 24.;
    }
  }
  Time0 = pt->Time;  // Prepare detection of passing midnight upon next fix
  pt->Time += d24;

  if ( npt >= nptR )
  { // Grow the array
    realloc_s ( (void**)&ptA, sizeof(sFlightPoint), &nptR, 256 );
  }
  memcpy ( &ptA[npt], pt, sizeof(sFlightPoint) );
  npt ++;
}
