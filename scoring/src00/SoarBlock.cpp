#include "Global.h"
#include "SoarBlock.h"
#include "FreeFlight.h"


CSoarBlock *sbMaxT   = NULL;   // This pointer will receive the soaring block with the longest time
CSoarBlock *sbMaxAG  = NULL;   // This pointer will receive the soaring block with the biggest altitude gain
CSoarBlock *sbMaxFF  = NULL;   // The soaring block containing the maximum soaring distance;
CSoarBlock *sbMaxDT  = NULL;   // The soaring block containing the best declared task performance
CSoarBlock *sbMaxTRI = NULL;   // The soaring block containing the best triangle performance


CSoarBlock::CSoarBlock(void)
{ pOwner  = NULL;
  nrSeq   = 0;
  MaxAlt  = 0.;
  iLow    = iHigh = -1;
  AltGain = 0.;
  memset ( &FF,  0, sizeof(FF) );
  memset ( &FT,  0, sizeof(FT) );
  memset ( &DT,  0, sizeof(DT) );
  memset ( &TRI, 0, sizeof(TRI) );
  memset ( &BRU, 0, sizeof(BRU) );
  Knicks  = NULL;
  nKnicks = 0;
}



CSoarBlock::~CSoarBlock(void)
{ free ( (void**) &Knicks );
}



void CSoarBlock::CalcAltGain(void)
{ sFlightPoint *pt = ptA;
  int    iL   = 0;
  double aLow = 99999.;
  double a;

  for ( int i = 0;   i < npt;   i ++,  pt ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD ) )   continue;
    if ( pt->Altitude > MaxAlt )  MaxAlt = pt->Altitude;
    if ( pt->Altitude < aLow )
    { aLow = pt->Altitude;
      iL = i;
      if ( debugAlt )
      { printf ( "  Low point %6d,  %s, %6d m\n", i, stHour(i), ptA[i].Altitude );
      }
    }
    else
    { a = pt->Altitude - aLow;
      if ( a > AltGain )
      { AltGain = a;
        iLow    = iL;
        iHigh   = i;
        if ( debugAlt )
        { printf ( "+ Alt gain  %6d,  %s, %6.0f m\n", i, stHour(i), AltGain );
        }
      }
    }
  }
}




void CSoarBlock::CalcFF(void)
{ // Calculates various details after optimalisation by CFreeFlight
  int i, i1, i2;
  sFlightPoint *pt;
  FF.Dist = 0;
  if ( !FF.npt )  return;

  for ( i = 0;   i < ( FF.npt - 1 );  i ++ )
  { i1 = FF.TptA[i].i;
    i2 = FF.TptA[i+1].i;
    FF.TptA[i].ti   = ptA[i1].Time;
    FF.TptA[i].Dist = wgs84 ( i1, i2 );
    FF.TptA[i].Trk  = OrthoTrk ( i1, i2 );
    FF.TptA[i].tLeg = dTime ( i1, i2 );
    FF.TptA[i].Spd  = FF.TptA[i].Dist / FF.TptA[i].tLeg;
    FF.Dist += FF.TptA[i].Dist;
  }
  FF.TptA[i].ti = ptA[FF.TptA[i].i].Time;
  FF.Time  = dTime ( FF.TptA[0].i, FF.TptA[i].i );
  FF.Speed = FF.Dist / FF.Time;
  FF.StartCrossAlt  = ptA[FF.TptA[0].i].Altitude;
  FF.FinishCrossAlt = ptA[FF.TptA[i].i].Altitude;
  // Get the start altitude
  FF.StartAlt = ptA[0].Altitude;
  for ( i = i,  pt = ptA;   i <= FF.TptA[0].i;   i ++,  pt ++ )
  { FF.StartAlt = min ( FF.StartAlt, pt->Altitude );
  }
  // Get the finish altitude
  i = FF.TptA[FF.npt-1].i;
  pt = &ptA[i];
  FF.FinishAlt = pt->Altitude;
  for ( ;   i < FF.TptA[0].i;   i ++,  pt ++ )
  { FF.FinishAlt = max ( FF.FinishAlt, pt->Altitude );
  }
  FF.HeightLoss = FF.StartAlt - FF.FinishAlt;

  FF.ScoreDist = FF.Dist;
  if ( FF.HeightLoss > FreeHeightLoss ) 
    FF.ScoreDist -= ( FF.HeightLoss - FreeHeightLoss ) * LDnominal / 1000.;
  if ( FF.ScoreDist < 0. )  FF.ScoreDist = 0.;
}




void FindLongestSB(void)
{ if ( !nFlights )  return;

  int      nf, nb;
  double   dt, M  = 0.;
  CFlight *pF;
  CSoarBlock *pB;
  for ( nf = 0;  nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    if ( !pF->bRelease )  continue;
    for ( nb = 0;   nb < pF->nSB;   nb ++ )
    { pB = pF->SB[nb];
      dt = pB->dTime ( 0, pB->npt-1 );
      if ( dt > M )
      { M = dt;
        sbMaxT = pB;
      }
    }
  }

  if ( debug   &&   sbMaxT )
  { printf ( "**************************\n" );
    printf ( "LONGEST SOAR BLOCK:\n" );
    printf ( "  Flight %d, soaring block %d\n", sbMaxT->pOwner->nFlight, sbMaxT->nrSeq );
    printf ( "  Start %4d at %s\n", sbMaxT->Start, sbMaxT->stHour ( 0 ) );
    printf ( "  End   %4d at %s\n", sbMaxT->npt-1, sbMaxT->stHour ( sbMaxT->npt-1 ) );
    printf ( "\n" );
    fflush ( stdout );
  }

  if ( debugAlt   &&   sbMaxT   &&   sbMaxT->nAltBad )
  { char st[64];
    sprintf ( st, "Found %d (%0.1f pct) bad altitude fixes in longest soar block\n", sbMaxT->nAltBad, sbMaxT->pAltBad );
    printf ( "%s", st );
    AddDebugMsg ( st );
    fflush ( stdout );
  }
}




void FindLongestSoarBlock(void)
{
  FindLongestSB();

  if ( !sbMaxT )
  { report_error ( EC_FR_NO_SOAR_B, NULL );
  }
  else if ( sbMaxT->npt < 50 )  // TODO
  { report_error ( EC_FR_SOAR_SHORT, NULL );
  }
}




void CalcAltGain(void)
{ if ( !nFlights )  return;

  int      nf;
  double AGM = -999.;
  CFlight *pF;
  for ( nf = 0;   nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    pF->CalcAltGain();
    if ( ( pF->iAltGainMax >= 0 )  &&
         ( pF->SB[pF->iAltGainMax]->AltGain  > AGM )
       )
    { sbMaxAG = pF->SB[pF->iAltGainMax];
      AGM = sbMaxAG->AltGain;
    }
  }

  if ( debug  &&  sbMaxAG )
  { printf ( "**************************\n" );
    printf ( "MAX ALTITUDE GAIN:\n" );
    printf ( "  Flight %d, soaring block %d, Gain %0.0f\n", sbMaxAG->pOwner->nFlight, sbMaxAG->nrSeq, sbMaxAG->AltGain );
    printf ( "  Low %5d, %5d m  at %s\n", sbMaxAG->iLow,  sbMaxAG->ptA[sbMaxAG->iLow ].Altitude, sbMaxAG->stHour ( sbMaxAG->iLow ) );
    printf ( "  Low %5d, %5d m  at %s\n", sbMaxAG->iHigh, sbMaxAG->ptA[sbMaxAG->iHigh].Altitude, sbMaxAG->stHour ( sbMaxAG->iHigh ) );
    printf ( "\n" );
    fflush ( stdout );
  }
}



