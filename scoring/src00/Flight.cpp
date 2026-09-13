#include "Global.h"
#include "Airfields.h"


CFlight **FlightA = NULL;
int nFlights = 0;



CFlight::CFlight(int First, int Last, BOOL bGapL)
{ nFlight        = nFlights - 1;
  Start          = First;
  bGap           = bGapL;
  npt            = Last - First + 1;
  ptA            = &Trace.ptA[Start];
  nBaseLine      = 0;
  dtBaseLine     = 0.;
  AltBaseLine    = 0.;
  Release        = 0;
  ReleaseTrigger = RT_DVZ;
  bRelease       = FALSE;
  AirfTO         = -1;
  EngRuns        = 0;
  EngTest        = -1;
  EngTimeTot     = 0.;
  EngTimeMax     = 0.;
  EngClimbTot    = 0.;
  EngClimbMax    = 0.;
  EngDistTot     = 0.;
  EngDistMax     = 0.;
  EngA           = NULL;
  SB             = NULL;
  nSB            = nSBR  = 0;
  StartSoar      = 0;
  MaxAlt         = 0.;
  iAltGainMax    = -1;

  Init();

int i0 = 50;
int i2 = 100;
}



CFlight::~CFlight(void)
{ if ( SB )
  { for ( int i = 0;   i < nSB;   i ++ )
    { if ( SB[i] ) delete SB[i];
    }
    free ( (void**)&SB );
  }
  free ( (void**)&EngA );
}




void CFlight::GetBaseLine(void)
{ int    i;
  double a;
  double t0 = max ( Trace.ptA[0].Time, ptA[0].Time - 3. / 60. );   // Max duration of the base line = 3'
  double a0 = ptA[0].Altitude;
  double aA = 0.;

  if ( Start > 0)
  { for ( i = Start - 1;  Trace.ptA[i].Time > t0;  i -- )
    { if ( Trace.dTime ( i, i+1, 60. ) > 6. )  break;
      a = Trace.ptA[i].Altitude;
      if ( fabs ( a - a0 ) > 30. )  break;
      aA += a;
    }
    i ++;     // this is the last point used in the base line
  }
  else i = 0;
  nBaseLine   = Start - i;
  if ( nBaseLine )
  { aA /= nBaseLine;
    AltBaseLine = aA;
    dtBaseLine  = Trace.dTime ( i, Start, 60. );
  }
}





void CFlight::AdjustLdg(void)
{ while ( npt > 2 )
  { if ( Speed ( npt-2, npt-1 ) > 40. )  break;
    npt --;
  }
}



// Init
// - Get basic flight statistics
// - Check the altitude sensor, mark bad altitude fixes
void CFlight::Init(void)
{ FlightTime     = dTime ( 0, npt-1 );
  FixInterval    = 3600.0 * FlightTime / npt;    // seconds
  SamplesPerHour = (double)npt / FlightTime ;

  // Mark bad altitude fixes.
  CheckAlt();            // Based upon filtered altitude
  CountAltBad();

  AirfTO = Airfields.Find ( &ptA[0].pt, AirfTO_Radius );
}





BOOL CFlight::CheckAltGpsDif(void)
{ // Calculate filtered altitude between i0 and i2
  // returns TRUE if additional bad fixzs were found
  int    i;
  int    n = 0;
  double d;

  // Calculate average difference GPS altitude - pneumatic altitude
  double a = 0.;
  for ( i = 1;  i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD ) )  continue;
    a +=  ptA[i].AltiGPS - ptA[i].Altitude;
    n ++;   // Count the valid fixes
  }
  a /= n;

  // Calculate standard deviation of a
  double s = 0.;
  for ( i = 0;  i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD ) )  continue;
    d = ptA[i].AltiGPS - ptA[i].Altitude - a;
    d *= d;
    s += d;
  }
  s /= n;
  s = sqrt ( s );

  // Mark all bad fixes, count the new ones
  // s will receive the acceptable deviation from a
  s *= 3.;
  s = max ( s, 300. );
  for ( i = 0,  n = 0;  i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD     ) )  continue;
    if ( GetFlag ( i, FLT_ALT_GPS_BAD ) )  continue;
    d = ptA[i].AltiGPS - ptA[i].Altitude - a;
    if ( d < -s   ||
         d > +s
       )
    { SetFlag ( i, FLT_ALT_BAD, TRUE );
      n ++;
    }
  }

  return n != 0;
}






void CFlight::CheckAltGps(void)
{ // Check altitude validity by comparing to previous (filtered) alt
  int    i;
  int    i0  = 0;;
  int    n   = 0;
  double tau = 60.;
  double aA  = ptA[0].AltiGPS;
  double dt, da;

  for ( i = 1;   i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_GPS_BAD ) )   continue;
    dt = dTime ( i0, i, 3600. );   // Time since previous fix, seconds
    da = fabs ( aA - ptA[i].AltiGPS );
    if ( da > 300 + dt * 7. )
    { // Bad altitude
      SetFlag ( i, FLT_ALT_GPS_BAD, TRUE );
      n ++;
      continue;
    }
    if ( dt >= tau )  aA  = ptA[i].AltiGPS;
    else              aA += ( ptA[i].AltiGPS - aA ) * dt / tau;
    i0 = i;
  }
}





void CFlight::CheckAlt(void)
{ // Check altitude validity by comparing to previous (filtered) alt
  int    i;
  int    i0  = 0;;
  int    n   = 0;
  double tau = 60.;
  double aA  = ptA[0].Altitude;
  double dt, da;
  double dtM = 0.;

  for ( i = 1;   i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD ) )   continue;
    dt  = dTime ( i0, i, 3600. );   // Time since previous fix, seconds
    dtM = max ( dtM, dt );
    da  = fabs ( aA - ptA[i].Altitude );
    if ( da > 300 + dt * 8. )
    { // Bad altitude
      SetFlag ( i, FLT_ALT_BAD, TRUE );
      n ++;
      continue;
    }
    if ( dt >= tau )  aA  = ptA[i].Altitude;
    else              aA += ( ptA[i].Altitude - aA ) * dt / tau;
    i0 = i;
  }

  if ( dtM > 90. )
  { CheckAltGps();               // Based upon filtered GPS altitude
    while ( CheckAltGpsDif() );  // Based upon GPS alt. Loop until no additional fixzs are found
  }
}





void CFlight::AddEngine(sEngine *E)
{ realloc_s ( (void**)&EngA, sizeof(sEngine), &EngRuns );
  EngA[EngRuns-1] = *E;
  double a = dTime ( E->On, E->Off );
  EngTimeTot += a;
  EngTimeTot  = max ( EngTimeTot, a );
  a = dAlt ( E->On, E->Off );
  EngClimbTot += a;
  EngClimbTot  = max ( EngClimbTot, a );
  a = OrthoDist ( E->On, E->Off );
  EngDistTot += a;
  EngDistTot  = max ( EngClimbTot, a );
  if ( debugEngine )
  { printf ( "run %d %s - %s, %4.0f\", %4.1fmps\n", EngRuns, stHour ( E->On ), stHour ( E->Off ), dTime ( E->On, E->Off, 3600. ), Vz ( E->On, E->Off) );
  }
}




void CFlight::SetSoarBlocks(void)
{ int First = Release;
  StartSoar = -1;

  for ( int i = 0;   i < EngRuns;  i ++ )
  { if ( EngA[i].On <= Release )
    { if ( EngA[i].Off <= Release )  continue;
      First = EngA[i].Off;
    }
    if ( Engine.SkipTestRun  ||   i )  // If a test run is required, don't add the soar block before the test run
    { AddSoarBlock ( First, EngA[i].On - 1 );
      if ( StartSoar < 0 )  StartSoar = First;
    }
    First = EngA[i].Off;
  }
  AddSoarBlock ( First, npt - 1 );
  if ( StartSoar < 0 )  StartSoar = Release;
  if ( !Engine.SkipTestRun   &&   EngRuns )  EngTest = 0;
  else EngTest = -1;

  if ( debugFlt  ||  debugEngine )
  { CFlight *pF;
    CSoarBlock *pSB;
    for ( int i = 0;   i < nFlights;   i ++ )
    { pF  = FlightA[i];
    printf ( "\nFlight %d: %d soaring blocks\n", i+1, pF->nSB );
      for ( int j = 0;   j < pF->nSB;   j ++ )
      { pSB = pF->SB[j];
        printf ( "  Soarblock %d  %s - %s\n", j+1, pSB->stHour ( 0 ), pSB->stHour ( pSB->npt-1 ) );
      }
    }
  }
}





void CFlight::AddSoarBlock(int First, int Last )
{ // Don't add very short blocks
  // Also performs validity checking of First end Last
  if ( ( Last - First ) < 50 )  return;

  realloc_s ( (void**)&SB, sizeof(CSoarBlock), &nSB );
  SB[nSB-1] = new CSoarBlock;
  if ( !SB[nSB-1] )
  { char st[256];
    sprintf ( st, "Memory allocation error SoarBlock object %d\n", nSB - 1 );
    report_error ( EC_MEMORY, st );
  }

  CSoarBlock *p = SB[nSB-1];
  p->pOwner = this;
  p->nrSeq  = nSB - 1;
  p->Start  = First;
  p->ptA    = &ptA[First];
  p->npt    = Last - First + 1;
  p->CountAltBad();
/*
  if ( debugEngine  ||  debugFlt )
  { printf ( "Flight %d Soar block %2d %s -> %s\n", nFlight + 1, nSB, p->stHour ( 0 ), p->stHour ( p->npt-1 ) );
  }
*/
}






void CFlight::CalcAltGain(void)
{ if ( !nSB )  return;

  CSoarBlock *pB;
  double AGM = -999.;
  for ( int nb = 0;   nb < nSB;   nb ++ )
  { pB = SB[nb];
    pB->CalcAltGain();
    if ( pB->AltGain > AGM )
    { AGM = pB->AltGain;
      iAltGainMax = nb;
    }
    if ( pB->MaxAlt > MaxAlt )  MaxAlt = pB->MaxAlt;
  }
}

