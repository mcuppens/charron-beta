#include "Global.h"


int SpeedComp = TRUE;
double FastTrackTime  = 2.;     // Duration for fastest track calculation (decimal hour)


void CalcFastTrack(void)
{ if ( !sbMaxFF )  return;
  CFastTrack FT ( sbMaxFF );
  FT.Calc();

  if ( debug )
  { int i0 = sbMaxFF->FT.i0;
    int i2 = sbMaxFF->FT.i2;
    printf ( "**************************\n" );
    printf ( "FAST TRACK:\n" );
    printf ( "  Flight %d, soaring block %d, %d points\n", sbMaxFF->pOwner->nFlight, sbMaxFF->nrSeq, i2 - i0 + 1 );
    printf ( "  From %5d at %s\n", i0, sbMaxFF->stHour ( i0 ) );
    printf ( "  To   %5d at %s\n", i2, sbMaxFF->stHour ( i2 ) );
    printf ("\n" );
    fflush ( stdout );
  }
}



CFastTrack::CFastTrack(CSoarBlock *psb)
{ pSB = psb;
  if ( psb )
  { ptA  = psb->ptA;
    npt  = psb->npt;
    memcpy ( &FF, &psb->FF, sizeof(FF) );
  }
  else 
  { ptA = NULL,
    npt = 0;
    memset ( &FF, 0, sizeof(FF) );
  }
  memset ( &FT,      0, sizeof(FT) );
  memset ( &pSB->FT, 0, sizeof(FT) );

  if ( debug )
  { int n  = FF.npt;
    int i0 = FF.TptA[0].i;
    int i2 = FF.TptA[n-1].i;
    printf ( "**************************\n" );
    printf ( "FAST TRACK, START CALC:\n" );
    printf ( "  Flight %d, soaring block %d, Dist %0.0f,  %d points\n", psb->pOwner->nFlight, psb->nrSeq, FF.Dist, pSB->npt );
    printf ( "  From %5d at %s\n", i0, pSB->stHour ( i0 ) );
    printf ( "  To   %5d at %s\n", i2, pSB->stHour ( i2 ) );
    printf ("\n" );
    fflush ( stdout );
  }
}



CFastTrack::~CFastTrack(void)
{
}




void CFastTrack::Calc(void)
{ double t2;
  if ( pSB->npt < 2 )  return;
  FT.i0 = 0;
  ffi2  = FT.i0 + pSB->npt - 1;
  FT.i2 = FixAtdTime ( FT.i0, FastTrackTime, 1 );
  FT.i2 = min ( FT.i2, ffi2 );

  while ( 1 )
  { t2 = ptA[FT.i0].Time + FastTrackTime;
    while ( FT.i2 < ffi2   &&   ptA[FT.i2].Time < t2 )  FT.i2 ++;
    FT.Time = dTime ( FT.i0, FT.i2 );
    if ( FT.Time < ( FastTrackTime / 2. ) )   break;   // No (more) calculation in this case
/*
if ( FT.i0 > 466 )
FT.i0 = FT.i0;
*/
    FT.Dist  = GetDist();
    FT.Speed = FT.Dist / max ( FT.Time, FastTrackTime );
    if ( FT.Speed > pSB->FT.Speed )
      memcpy ( &pSB->FT, &FT, sizeof(FT) );
    FT.i0 ++;
  }

  // Fill the remainder of the FT struct
  if ( pSB->FT.i2 )
  { pSB->FT.t0   = ptA[pSB->FT.i0].Time;
    pSB->FT.t2   = ptA[pSB->FT.i2].Time;
    pSB->FT.Alt0 = ptA[pSB->FT.i0].Altitude;
    pSB->FT.Alt2 = ptA[pSB->FT.i2].Altitude;
    pSB->FT.HeightLoss = pSB->FT.Alt0 - pSB->FT.Alt2;
  }
}






double CFastTrack::GetDist(void)
{ // Calculate the distance
  double d = 0.;
  int    iff = 0;
  int    ipt;

  // Find the FF point after FT.i0
  while ( FF.TptA[iff].i <= FT.i0 )  iff ++;
  ipt = FF.TptA[iff].i;           // Just for easier reading

  if ( FT.i2 <= ipt )
  { // The fast track is located in a single leg
    return wgs84 ( FT.i0, FT.i2 );
  }

  d = wgs84 ( FT.i0, ipt );       // From i0 to the first turnpoint thereafter
  while ( 1 )
  { iff ++;
    if ( iff >= FF.npt ) break;
    ipt = FF.TptA[iff].i;         // Just for easier reading
    if ( FT.i2 <= ipt ) break;    // i2 is located before the next ff turnpoint
    d += FF.TptA[iff-1].Dist;     // This leg is fully located with the time window
  }
  d += wgs84 ( FF.TptA[iff-1].i - 1, FT.i2 );
  return d;
}

