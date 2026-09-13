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
    printf ( "  Number of points: %d\n", sbMaxT->npt );
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




void LimitFixes(void)
{ int i, f, fF, tp, nf, nb;
  int nd  = 0;
  int ndt = 0;
  int n0;
//return;
  UINT TC0 = GetTickCountOwn();
  double intvl0, intvl2, tt;
  BOOL bSector;
  CFlight *pF;
  CSoarBlock *pB;
  CSoarBlock *pB2;
  sFlightPoint *pFP;
  sFlightPoint *pFP2;
  CTask::sPoint *pt;
nFixMax = 20000;
  for ( nf = 0;  nf < nFlights;   nf ++ )
  { pF = FlightA[nf];
    if ( !pF->bRelease )  continue;
    for ( nb = 0;   nb < pF->nSB;   nb ++ )
    { pB = pF->SB[nb];
      if ( pB->npt > nFixMax )
      { nd = 0;
        n0 = pB->npt;
        intvl0 = ( pB->ptA[pB->npt - 1].Time - pB->ptA[0].Time ) / pB->npt;
        intvl2 = ( pB->ptA[pB->npt - 1].Time - pB->ptA[0].Time ) / nFixMax * 1.03;  // Factor: default adjustment for fixes near sectors
        tt = pB->ptA[1].Time;
        for ( f = 1,  pFP = &pB->ptA[1];   f < pB->npt - 1;   f ++,  pFP ++ )
        { if ( pFP->Time > tt )
          { tt += intvl2;
            continue;                            // Keep this fix 
          }
          pt = &Task.TPT[1];                     // Exclude take-off
          bSector = FALSE;
          for ( tp = 0;   tp < Task.nTPT - 1;   tp ++,  pt ++ )
          { if ( LoxoDist ( &pt->pt, &pFP->pt ) < 3. )
            { bSector = TRUE;
              break;
            }
          }
          if ( bSector ) continue;               // Point in vicinity of any task sector: keep
          // Arriving here: delete this fix
          fF = pB->Start + f;
          pFP2 = &pF->ptA[fF];
          for ( i = fF;   i < pF->npt;   i ++,  pFP2 ++ )
          { *pFP2 = pFP2[1];
          }
          pB->npt --;
//          pF->DeleteFix ( pB, f );
          f   --;                                  // The original point was deleted -> reset to same point
          pFP --;
          nd  ++;
        }
      }
      if ( nd )
      { ndt += nd;
        pB2 = *pF->SB;
        for ( i = 0;   i < pF->nSB;   i ++,  pB2 ++ )
        { if ( pB2->Start > pB->Start )  pB2->Start -= nd;
        }

        for ( int i = 0;   i < pF->EngRuns;   i ++ )
        { if ( pF->EngA[i].On  > pB->Start )  pF->EngA[i].On  -= nd;
          if ( pF->EngA[i].Off > pB->Start )  pF->EngA[i].Off -= nd;
        }

        if ( debugAny )
        { printf ( "Flight %d, soaring block %d: Fixes deleted\n", nf +1,  nb + 1 );
          printf ( "   Original n fixes: %d\n", n0 );
          printf ( "   new n fixes     : %d\n", pB->npt );
          printf ( "   n fixes deleted : %d\n", nd );
        }
      }
    }
  }
  if ( ndt )  report_error ( EC_FR_FIX_RECUCED, NULL );
  if ( debugAny )
     printf ( "   Time spent: %d ms\n", GetTickCountOwn() - TC0 );
}
/*
void CFlight::DeleteFix(CSoarBlock *pSBP, int n)
{ int i;
  int f = pSBP->Start + n;
  CSoarBlock *pSB = *SB;
  if ( n < 0   ||   n >= npt ) return;
  for ( i = f;   i < npt;   i ++ )
  { ptA[i] = ptA[i+1];
  }
  pSBP->npt --;

  for ( i = 0;   i < nSB;   i ++,  pSB ++ )
  { if ( pSB->Start > f )  pSB->Start --;
  }

  for ( int i = 0;   i < EngRuns;  i ++ )
  { if ( EngA[i].On  > f )  EngA[i].On  --;
    if ( EngA[i].Off > f )  EngA[i].Off --;
  }
}

*/

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



