#include "Global.h"


double   ffMinLegDist = 20.;
int      nKnicksBase  = 18;  // 18;
sffTable ffTable[5]   = { 0 };           // Setup in InitffTable()


void CalcFreeFlight(void)
{ // Calc the free flight distance of all soaring blocks of
  // the flight containing the longest soaring block
  if ( !sbMaxT )  return;
  if ( sbMaxT->npt < 100 )  return;

  int i, j;
  CFlight *pF = sbMaxT->pOwner;
  if ( !pF )  return;
  double      Max = 0.;        // The maximum FF distance 
  CSoarBlock *pSB = pF->SB[0];
  sbMaxFF = NULL;
  InitffTable();

  if ( debugFF )
  { printf ( "FREE FLIGHT EVALUATION\n" );
  }
  for ( i = 0;   i < pF->nSB;   i ++ )
  { pSB = pF->SB[i];
    if ( pSB->npt < 100    ||   pSB->dTime ( 0, pSB->npt - 1, 60. ) < 20. ) continue;
    if ( debugFF )
    { printf ( "FLIGHT %d, SOARBLOCK %d\n", pF->nFlight + 1, i + 1 );
    }
    CFreeFlight FF ( pSB );
    FF.Calc();

    // Store the result in the soar block
    pSB->FF.npt = FF.A.nA;
    for ( j = 0;   j < pSB->FF.npt;   j ++ )  pSB->FF.TptA[j].i = FF.A.A[j].i;
    pSB->CalcFF();    // Calculate all details (heightloss, ...)

    // Get the maximum FF soar block
    if ( pSB->FF.Dist > Max )
    { Max = pSB->FF.Dist;
      sbMaxFF = pSB;
    }
  }

  if ( debugFF  &&  sbMaxFF )
  { int n  = sbMaxFF->FF.npt;
    int i0 = sbMaxFF->FF.TptA[0].i;
    int i2 = sbMaxFF->FF.TptA[n-1].i;
    printf ( "**************************\n" );
    printf ( "MAX FREE FLIGHT:\n" );
    printf ( "  Flight %d, soaring block %d, Dist %0.1f,  %d points\n", sbMaxFF->pOwner->nFlight, sbMaxFF->nrSeq, sbMaxFF->FF.Dist, sbMaxFF->npt );
    printf ( "  From %5d at %s\n", i0, sbMaxFF->stHour ( i0 ) );
    printf ( "  To   %5d at %s\n", i2, sbMaxFF->stHour ( i2 ) );
    printf ("\n" );
    fflush ( stdout );
  }
}





void InitffTable(void)
{ // Must be done after reading command line. Do in CalcFreeFlight
  // Last entry Min must be 0 !!!!
  if ( ffMinLegDist < epsilon )
  { ffTable[0].Min = 200.;       ffTable[0].npt = ffMaxTpt;
    ffTable[1].Min = 150.;       ffTable[1].npt = 6;
    ffTable[2].Min = 100.;       ffTable[2].npt = 5;
    ffTable[3].Min =  50.;       ffTable[3].npt = 3;
    ffTable[4].Min =   0.;       ffTable[4].npt = 0;
//ffTable[1].Min =   0.;       ffTable[1].npt = ffMaxTpt-1;   // Debugging -> allows comparison with ffMinLegDist != 0
  }
  else
  { // Only two steps
    ffTable[0].Min = 200.;       ffTable[0].npt = ffMaxTpt;
    ffTable[1].Min =   0.;       ffTable[1].npt = ffMaxTpt - 1;
  }
}





CFreeFlight::CFreeFlight(CSoarBlock *psb)
{ pSB   = psb;
  ptA   = pSB->ptA;
  npt   = pSB->npt;
  ptL   = NULL;
  nptLR = 0;
  DT    = NULL;
  nDT   = 0;
}



CFreeFlight::~CFreeFlight(void)
{ free ( (void **)&ptL );
  free ( (void **)&DT );
}





void CFreeFlight::Calc(void)
{
StartTiming();
  MakeLoxo();
EndTiming ( "MakeLoxo" );
  MakeKnicks();
EndTiming ( "MakeKnicks" );

  int nTbl = 0;
  while ( 1 )
  { if ( !ffTable[nTbl].npt )
    { // Distance too small to be scored
      A.Init ( ptL, ptA, npt, 0, 0 );
      break;
    }

    FindA ( ffTable[nTbl].npt );
EndTiming ( "FindA" );
    Optimize();
EndTiming ( "Optimize" );

    if ( A.Dist >= ffTable[nTbl].Min ) break;

    // if the distance is way below, skipp as many loops as possible
    while ( A.Dist < ffTable[nTbl].Min )
    { // To avoid corruption, the last distance in the table must be 0
      nTbl ++;
    }
  }
}





void CFreeFlight::MakeLoxo(void)
{ int i;
  sLoxo *ptl;
  sFlightPoint *pta = ptA;
  if ( npt > nptLR )
    realloc_s ( (void**)&ptL, sizeof(sLoxo), &nptLR, npt - nptLR );

  sCoord Max = ptA->pt;
  sCoord Min = ptA->pt;

  for ( i = 1;   i < npt;   i ++,  pta ++ )
  { Max.Lat = max ( Max.Lat, pta->pt.Lat );
    Min.Lat = min ( Min.Lat, pta->pt.Lat );
    Max.Lon = max ( Max.Lon, pta->pt.Lon );
    Min.Lon = min ( Min.Lon, pta->pt.Lon );
  }
  Max.Lat += Min.Lat;    Max.Lat /= 2.;
  Max.Lon += Min.Lon;    Max.Lon /= 2.;
  double fy  = rEarth;
  double fx  = fy * cos ( Max.Lat );

  ptl = ptL;
  pta = ptA;
  for ( i = 0;   i < npt;   i ++,  pta ++,  ptl ++ )
  { ptl->y = ( pta->pt.Lat - Max.Lat ) * fy;
    ptl->x = ( pta->pt.Lon - Max.Lon ) * fx;
  }
}




void CFreeFlight::MakeKnicks(void)
{ // Generate the knick points list (K)
  int i, j = 0;
  double d, dm;
  double td = 0.;
  double FD = 200.;                    // Sensitivity of adding knicks
  double DM =   1.;                    // Sensitivity of adding knicks
  int    nm = nKnicksBase * 3 / 4;     // The minimum number of knicks

  K.Init ( ptL, ptA, npt, nKnicksBase * 3, nKnicksBase );
  K.Insert ( 0 );                      // Add the first point
  K.Insert ( npt - 1 );                // Add the last point
  // Add points until nKnicksBase is reached or K.Dist doesn't increase sufficiently
  while ( 1 )
  { d = K.Dist;
//    td += d;
    K.ntgt = max ( nKnicksBase, K.nA + 2 );    K.SplitOptimal();
    K.ntgt = max ( nKnicksBase, K.nA + 1 );    K.InsertMaxPtLoxo();
    if ( K.nA >= nm )
    { if ( K.nA >= nKnicksBase )  break;
      d  = K.Dist - d;
      dm = max ( K.Dist / FD, DM );
      if ( d < dm ) break;
    }
  }

  // Refine the Knicks.  Ortho distances from now on
  // In many cases this us counter-productive, but in more cases, it helps...
  for ( i = 1;   i < K.nA - 1;   i ++ )
  { K.Delete ( i );
    K.InsertMaxPt ( K.A[i-1].i, K.A[i].i );
    td += K.A[i-1].Dist;
  }

  if ( /*td < 500  &&*/  ffMinLegDist > 0. )
  { // Calculate k : knick points at - & + min dist from every knick point
    CffArray k;
    k.Init ( ptL, ptA, npt, nKnicksBase * 2, nKnicksBase * 2 );
    for ( i = 0;   i < K.nA;   i ++ )
    { j = FixAtDist ( K.A[i].i, -ffMinLegDist /** 1.5*/ );
      if ( j > 0 )  k.Insert ( j );
      j = FixAtDist ( K.A[i].i, ffMinLegDist  /** 1.5*/ );
      if ( j > 0 )  k.Insert ( j );
    }

    // Insert k into K
    for ( i = 0;   i < k.nA;   i ++ )
    { K.ntgt ++;
      K.Insert ( k.A[i].i );
    }
    K.ntgt = K.nA;
  }
/*
  fmld = max ( ffMinLegDist * 0.8, td / 36. );

  if ( fmld > 15. )
  { // Calculate k : knick points at - & + min dist from every knick point
    CffArray k;
    k.Init ( ptL, ptA, npt, nKnicksBase * 2, nKnicksBase * 2 );
    for ( i = 0;   i < K.nA;   i ++ )
    { if ( i > 0 )
      { j = -1;
        if ( K.A[i-1].Dist > fmld * 2. )
        { j = FixAtDist ( K.A[i].i, -fmld );
        }
        else if ( K.A[i].Dist > fmld * 1.0 )
        { j = FixAtDist ( K.A[i].i, -K.A[i].Dist / 2. );
        }
        if ( j > 0 )  k.Insert ( j );
      }
      if ( i < K.nA - 1 )
      { j = -1;
        if ( K.A[i].Dist > fmld * 2.0 )
        { j = FixAtDist ( K.A[i].i, fmld );
        }
        else if ( i < K.nA - 1   &&  K.A[i].Dist > fmld * 1.0 )
        { j = FixAtDist ( K.A[i].i, -K.A[i].Dist / 2. );
        }
        if ( j > 0 )  k.Insert ( j );
      }
    }

    // Insert k into K
    for ( i = 0;   i < k.nA;   i ++ )
    { K.ntgt ++;
      K.Insert ( k.A[i].i );
    }
    K.ntgt = K.nA;
  }
*/

  // Split long legs
  K.SplitLegs ( max ( ffMinLegDist, td / 30. ), 60 ); 

  // Construct the distance table
  if ( nDT < K.nA * K.nA )
    realloc_s ( (void**)&DT, sizeof(double), &nDT, K.nA * K.nA - nDT, TRUE );
  for ( i = 0;      i < K.nA - 1;   i ++ )
  { for ( j = i+1;  j < K.nA;       j ++ )
    { DT[dtIndex(i,j)] = OrthoDist ( K.A[i].i, K.A[j].i );
    }
  }

  if ( debugFF )
  { printf ( "Constructed knicks table\n" );
  printf ( " Base nKnicks %d, total nKnicks: %d\n%7d", nKnicksBase, K.nA, 0 );
    for ( i = 0;   i < K.nA;   i ++ )
    { printf ( "%7d", K.A[i].i );
    }
    printf ( "\n" );
    for ( i = 0;   i < K.nA;   i ++ )
    { printf ( "%7d", K.A[i].i );
      for ( j = 0;   j < K.nA;   j ++ )
      { printf ( "%7.1f", dtDist ( i, j ) );
      }
      printf ( "\n" );
    }
    printf ( "\n" );
  }
  // Copy the knicks to the Soarblock knicks table -> For use in Triangle
  realloc_s ( (void**)&pSB->Knicks, sizeof(int), &pSB->nKnicks, K.nA );
  for ( i = 0;   i < K.nA;   i ++ )
  { pSB->Knicks[i] = K.A[i].i;
  }
}






void CFreeFlight::FindA(int ntgt)
{ // Find the max free flight distance out of K over ntgt turnpoints
  if ( debugFF )
  { printf ( "Free flight evaluation over %d points ************\n", ntgt );
    printf ( "Max distance over %d knicks:\n", K.nA );
  }
  A.Init ( ptL, ptA, npt, ntgt+2, ntgt );
  A.dMin = ffMinLegDist;
  memset ( kOpt, 0, sizeof(kOpt) );
  memcpy ( kOM, kOpt, sizeof(kOM) );
  if ( A.dMin > 0. )  kRecurseBase2();
  else                kRecurseBase1();
  A.CalcDist ( -1 );
  if ( debugFF )  DebugA();
}







// Get the maximum dist along the knicks.  no ffMinLegDist
// First step, as it is different from the other steps
void CFreeFlight::kRecurseBase1(void)
{ int kM = K.nA - A.ntgt + 1;
  A.nA = A.ntgt;
  for ( kOpt[0].i = 0;   kOpt[0].i < kM;    kOpt[0].i ++ )
  { kRecurseB1 ( 1 );
  }
}





// Get the maximum dist along the knicks.  no ffMinLegDist
inline void CFreeFlight::kRecurseB1(int k)
{ int kM = K.nA - A.ntgt + k + 1;
  for ( kOpt[k].i = kOpt[k-1].i + 1 ;   kOpt[k].i < kM;    kOpt[k].i ++ )
  { kOptDist   ( k - 1 );
    if ( k >= A.ntgt )  continue;
    if ( k == A.ntgt - 1 )
    {
//if ( debugFF ) DebugkOpt();
      if ( kOpt[k].Sum > A.Dist )
      { for ( int i = 0;   i < A.nA;   i ++ )    A.A[i].i = K.A[kOpt[i].i].i;
        A.Dist = kOpt[k].Sum;
        if ( debugFF )
        { memcpy ( kOM, kOpt, sizeof(skOpt) * A.nA );
          if ( debugFF ) DebugkOpt();
        }
      }
      continue;
    }
    kRecurseB1 ( k + 1 );
  }
}





/// Get the maximum dist along the knicks.  ffMinLegDist != 0
// First step, as it is different from the other steps
void CFreeFlight::kRecurseBase2(void)
{ int kM = K.nA - 1;
  for ( kOpt[0].i = 0;   kOpt[0].i < kM;    kOpt[0].i ++ )
  { kRecurseB2 ( 1 );
  }
}





// Get the maximum dist along the knicks.  ffMinLegDist != 0
inline void CFreeFlight::kRecurseB2(int k)
{ for ( kOpt[k].i = kOpt[k-1].i + 1 ;   kOpt[k].i < K.nA;    kOpt[k].i ++ )
  { kOptDist ( k - 1 );
//if ( debugFF ) DebugkOpt();
    if ( kOpt[k-1].Dist < ffMinLegDist )   continue;
    if ( kOpt[k].Sum > A.Dist )
    { A.nA = k + 1;
      for ( int i = 0;   i < A.nA;   i ++ )    A.A[i].i = K.A[kOpt[i].i].i;
      A.Dist = kOpt[k].Sum;
      if ( debugFF )
      { memcpy ( kOM, kOpt, sizeof(skOpt) * A.nA );
        if ( debugFF ) DebugkOpt();
      }
    }
    if ( k >= A.ntgt ) continue;
    kRecurseB2 ( k + 1 );
  }
}





inline void CFreeFlight::kOptDist(int i)
{ // returns distance from kOpt[i]->kOpt[i+1]
  if ( i < 0   ||   i >= A.ntgt - 1 )  return;
  kOpt[i].Dist = dtDist ( kOpt[i].i, kOpt[i+1].i );
  if ( i < A.ntgt )  kOpt[i+1].Sum = kOpt[i].Sum + kOpt[i].Dist;
}





void CFreeFlight::Optimize(void)
{ // Optimize
  int    i, i0, i2;
  int    n  = 0;
  int    nA = A.nA;
  double dM = 0.;

  // Refine A
  if ( debugFF )  printf ( "Optimization, step 1\n" );
  while ( fabs ( dM - A.Dist ) > 1.E-6 )
  { n ++;
    if ( debugFF )  printf ( ">>  LOOP %d\n", n );
    dM = A.Dist;
    for ( i = 0;   i < A.nA;   i ++ )
    { A.Delete ( i );
      i0 =  i  ?   A.A[i-1].i  :  0;
      i2 =  i == A.nA  ?  npt  :  A.A[i].i;
      A.InsertMaxPt ( i0, i2 );
      if ( debugFF )
      { printf ( "Iteration %2d\n", i+1 );
        DebugA();
      }
    }
  }
  if ( debugFF )  printf ( "Optimized dist: %6.1f, nLoops: %d\n", A.Dist, n );

  // Calculate WGS84 distances
  A.Dist = 0.;
  for ( int i = 0;   i < A.nA - 1;   i ++ )
  { A.A[i].Dist = wgs84 ( A.A[i].i, A.A[i+1].i );
    A.Dist += A.A[i].Dist;
  }
  if ( debugFF )
  { printf ( "WGS84 dist results: %0.1f\n", A.Dist );
    DebugA();
  }
}





void CFreeFlight::DebugA(void)
{ for ( int i = 0;   i < A.nA;   i ++ )
  { printf ( "%5d  %s  %6.1f\n", A.A[i].i, stHour ( A.A[i].i ), A.A[i].Dist );
  }
  printf ( "Total distance %0.1f\n", A.Dist );
}





void CFreeFlight::DebugkOpt(void)
{ for ( int j = 0;   j < A.ntgt;   j ++ )
  { printf ( "%2d  ", kOpt[j].i );
  }
  printf ( " %6.1f  %6.1f   %d  ", kOpt[A.nA-1].Sum, A.Dist, A.nA );
//  for ( int j = 0;   j < A.nA;   j ++ )  printf ( "%2d  ", kOM[j].i );
  for ( int j = 0;   j < A.nA-1;   j ++ )  printf ( "%6.1f  ", kOM[j].Dist );
  printf ( "\n" );
}
