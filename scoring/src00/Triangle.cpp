#include "Global.h"
#include "Triangle.h"


sTriangle TriM  = { 0 };
sTriangle TriBF = { 0 };

int nTriangle;   // Debugging
int nLoop;       // Debugging

void CalcTriangle(void)
{ double Max = 0.;

  if ( debugTRI )
  { printf ( "\n" );
    printf ( "***********************************************\n" );
    printf ( "TRIANGLE EVALUATION \n" );
    printf ( "\n" );
    Flush();
  }

  CTriangle Triangle;

  Triangle.Evaluate ( sbMaxFF );
  if ( triBF )
  { Triangle.EvaluateBF ( sbMaxFF );
  }

  if ( debugTRI )
  { printf ( "\n" );
    printf ( "END TRIANGLE EVALUATION\n" );
    printf ( "***********************************************\n" );
    printf ( "\n" );
    Flush();
  }
}




CTriangle::CTriangle(void)
{ Knicks   = NULL;
  nKnicksR = 0;
  nKnicks  = 0;
  memset ( &tri, 0, sizeof(sTriangle) );
  psb = NULL;
}



CTriangle::~CTriangle(void)
{ if ( debugTRI )
  { TriDebug.Knicks  = Knicks;
    TriDebug.nKnicks = nKnicks;
    Knicks = 0;
  }
  else free ( (void**)&Knicks );
}



void CTriangle::Evaluate(CSoarBlock *psbL)
{
//  int    i1, i2, i3;
  int    k1, k2, k3;
  int    n  = 0;

  if ( !psbL )  return;
  if ( psbL->FF.Dist < 60. )  return;

  psb = psbL;
  if ( !FindClosed() ) return;

  memset ( &tri,  0, sizeof(sTriangle) );
  memset ( &TriM, 0, sizeof(sTriangle) );
  memset ( tM,    0, sizeof(tM) );
  ntM = NELEMS ( tM );

  MakeKnicks();

  for ( k1 = 0;   k1 < nKnicks - 2;   k1 ++ )
  { tri.Index[0] = Knicks[k1];
    k2 = k1 + 1;
    for ( k2 = k1 + 1;   k2 < nKnicks - 1;   k2 ++ )
    { tri.Index[1] = Knicks[k2];
      for ( k3 = k2 + 1;   k3 < nKnicks;   k3 ++ )
      { tri.Index[2] = Knicks[k3];
n ++;
        Calc();
        SetMax();
//        Match = FindMatch();
//        if ( tri.TotDist > TriM.TotDist )  TriM = tri;
      }
    }
  }
  if ( debugTRI )
  { printf ( "Number of iterations: %d\n", n );
    Flush();
  }

  double dta = psb->ptA[i2].Time - psb->ptA[i0].Time;
  double shs = npt / dta;                                    // Soarblock's average samples / hour
  int   step = max ( 1, IROUND ( shs * 5. / 3600. ) );

  for ( k1 = 0;   k1 < ntM;   k1 ++ )
  { nTriangle = k1 + 1;
    if ( debugTRI )
    { printf ( "-------------------------------------\n" );
      printf ( "Triangle #%d : %7.3f %7.3f %7.3f %8.3f\n", k1+1, tM[k1].Legs[0],  tM[k1].Legs[1],  tM[k1].Legs[2], tM[k1].TotDist );
      printf ( "              %7.3f %7.3f %7.3f\n",              tM[k1].f[0],     tM[k1].f[1],     tM[k1].f[2] );
      printf ( "              %7d %7d %7d\n",                    tM[k1].Index[0], tM[k1].Index[1], tM[k1].Index[2] );
      Flush();
    }
    Optimise ( &tM[k1], step * 40, step );
    if ( step > 1 )
      Optimise ( &tM[k1], max ( step * 6, 20 ), 1 );
    if ( tM[k1].TotDist > TriM.TotDist ) TriM = tM[k1];
  }
  if ( debugTRI )
  { printf ( "-------------------------------------\n" );
    printf ( "Max triangle: %7.3f %7.3f %7.3f %8.3f\n", TriM.Legs[0],  TriM.Legs[1],  TriM.Legs[2], TriM.TotDist );
    printf ( "              %7.3f %7.3f %7.3f\n",       TriM.f[0],     TriM.f[1],     TriM.f[2] );
    printf ( "              %7d %7d %7d\n",             TriM.Index[0], TriM.Index[1], TriM.Index[2] );
    Flush();
  }
/*
//TODO
  Optimise ( &TriM, step * 50, step );
  Optimise ( &TriM, max ( step * 5, 20 ), 1 );
*/
}





BOOL CTriangle::FindClosed(void)
{ if ( triCloseD < epsilon )
  { i0  = 0;
    i2  = psb->npt - 1;
    npt = 12 - 10 + 1;
    return TRUE;
  }
  int i0L, i2L;
  int nf;
  double d, df = psb->pOwner->SamplesPerHour / 300;    // Fixes per km at 200 kmh
  npt = 0;
  if ( debugTRI )
  { printf ( "Finding closed loop, max dist %0.1f\n", triCloseD );
  }

  for ( i0L = 0;   i0L < psb->npt;   i0L ++ )
  { i2L = psb->npt - 1;
    if ( i2L - i0L < npt ) break;    // Done: no better solution possible. 
    while ( i2L > i0L )
    { d = OrthoDist ( &psb->ptA[i0L].pt, &psb->ptA[i2L].pt );
      if ( d <= triCloseD )
      { nf = i2L - i0L;
        if ( nf > npt )
        { npt = nf;
          i0  = i0L;
          i2  = i2L;
          if ( debugTRI )
          { double d0 = OrthoDist ( &psb->ptA[i0L].pt, &psb->ptA[i2L+1].pt );
             printf ( "  >> %5d %5d %5d   %0.6f   %0.6f\n", i0, i2, npt, d, d0 );
          }
        }
        i2L  = i0L;
      }
      else
      { i2L -= max ( IROUND ( ( d - triCloseD ) * df ), 1 );
      }
    }
  }
  if ( npt > psb->pOwner->SamplesPerHour / 2 )
  { if ( debugTRI )
    { printf ( "Closed loop between fix %d and %d, total %d fixes\n", i0, i2, npt );
    }
    return TRUE;
  }
  
  report_error ( TRI_NOT_CLOSED, NULL );
  if ( debugTRI )
  { printf ( "Triangle calculation not possible: no closed segment found\n" );
  }
  return FALSE;
}




void CTriangle::SetMax(void)
{ int i, t;
  int n;
  int nm = 99999;
  int im;
  int d;
  if ( tri.TotDist < 50. )  return;
sTriangle tt[3];
int nn[3];

  sTriangle *pt = tM;
  for ( t = 0;   t < ntM;   t ++,  pt ++ )
  { n = 0;
    for ( i = 0;   i < 3;   i ++ )
    { d = abs ( pt->Index[i] - tri.Index[i] );
      if ( d > psb->npt / 2 )   d = psb->npt - d;
tt[t].Index[i] = d;
      n += d;
    }
nn[t] = n;
    if ( n < nm )
    { nm = n;
      im = t;
    }
  }
int im0 = im;
double dm = 0.;
  if ( nm > npt / 40 )         // else: tri is an evolution of this triangle
  { // No matching triangle: find the smallest
    dm = 1.E10; 
    for ( t = 0,  pt = tM;   t < ntM;   t ++,  pt ++ )
    { if ( pt->TotDist < dm )
      { dm = pt->TotDist;
        im = t;
      }
    }
  }
  if ( tri.TotDist > tM[im].TotDist )
  {
/*
if (debugTRI )
{ for ( d = 0;  d < 3;   d ++ )
  { printf ( "/" );
    for ( t = 0;   t < 3;   t ++ )  printf ( "%6d", tt[d].Index[t] );
    printf ( " -%5d", nn[d] );
  }
  printf ( " -" );
  printf ( "%5d/%5d/%6.1f\n", im0, im, dm );
}
*/
    tM[im] = tri;
  }
}





void CTriangle::EvaluateBF(CSoarBlock *psbL)
{ // Brute force
  if ( !psbL )  return;
  if ( psbL->FF.Dist < 60. )  return;
  psb = psbL;

  if ( !FindClosed() ) return;

  if ( debugTRI )
  { printf ( ">>> BRUTE FORCE\n" );
    printf ( "Number of fixes : %d\n",   npt  );
    printf ( "Target step size: %d\"\n", triBFstep );
    printf ( "Max nr of steps : %d\n",   triBFmns  );
    Flush();
  }

  // calculate the step size
  double dta = psb->ptA[i2].Time - psb->ptA[i0].Time;
  double shs = npt / dta;                                    // Soarblock's average samples / hour
  int   step = IROUND ( shs * triBFstep / 3600. );
  int     ns = npt / step;                                   // Number of steps

  if ( triBFmns   &&   triBFmns < ns )
  { step = ( npt + triBFmns * 4 / 5 ) / triBFmns;
  }

  double dts = step * 3600. / shs;
  if ( dts > 45.55 )
  { if ( debugTRI )
    { printf ( "Step size       : %0.0f = above limit of 45\", aborted\n", dts );
      Flush();
    }
    return;
  }

  DWORD tc = GetTickCountOwn();

  // Generate the fix table
  int  n  = ( npt / step + 1000 ) / 512 * 512;
  int nTR = 0;
  int nT  = 0;
  int *T  = 0;
  realloc_s ( (void **)&T, sizeof(int), &nTR, n, 0 );

  dts /= 3600.;     // Convert seconds to decimal hours
  double t2;
  n = i0;
  do
  { T[nT] = n;
    nT ++;
    t2 = psb->ptA[n].Time + dts;
    // Find the fix at dts after the previous fix
    while ( n <= i2   &&   psb->ptA[n].Time < t2 )  n ++;
  }
  while ( n <= i2 );
  // Make sure the last point is included as well
  if ( T[nT-1] != i2 )
  { T[nT] = i2;
    nT ++;
  }

  if ( debugTRI )
  { printf ( "Actual step size: %0.0f\"\n", dts * 3600. );
    printf ( "Actual nr. steps: %d\n", nT );
    Flush();
  }

  int k1, k2, k3;
  n = 0;
  memset ( &tri,  0, sizeof(sTriangle) );
  for ( k1 = 0;   k1 < nT - 2;   k1 ++ )
  { tri.Index[0] = T[k1];
    k2 = k1 + 1;
    for ( k2 = k1 + 1;   k2 < nT - 1;   k2 ++ )
    { tri.Index[1] = T[k2];
      for ( k3 = k2 + 1;   k3 < nT;   k3 ++ )
      { tri.Index[2] = T[k3];
n ++;
        Calc();
        if ( tri.TotDist > TriBF.TotDist )  TriBF = tri;
      }
    }
  }

  if ( debugTRI )
  { printf ( "Calc after step 1: %7.3f %7.3f %7.3f %8.3f\n", TriBF.Legs[0], TriBF.Legs[1], TriBF.Legs[2], TriBF.TotDist );
    printf ( "                   %7.3f %7.3f %7.3f\n",       TriBF.f[0], TriBF.f[1], TriBF.f[2] );
    printf ( "                   %7d %7d %7d\n",             TriBF.Index[0], TriBF.Index[1], TriBF.Index[2] );
    DWORD DT = GetTickCountOwn() - tc;
    printf ( "Time: %d sec\n", DT / 1000 );
    Flush();
  }

  n = min ( 200, step * 5 );
  Optimise ( &TriBF, n, 1 );
}





void CTriangle::Optimise(sTriangle *pM, int r, int step)
{ int n = 0;
  double d;
  if ( debugTRI )
  { printf ( "Optimisation over range %d, step size %d\n", r, step );
    Flush();
  }
  do
  { d   = pM->TotDist;
    tri = *pM;
    nLoop = n + 1;
    Optimise1 ( pM, r, step );

    n ++;
    if ( debugTRI )
    { printf ( "      Loop %d: %7.3f %7.3f %7.3f %8.3f\n", n, pM->Legs[0],  pM->Legs[1],   pM->Legs[2], pM->TotDist );
      printf ( "              %7.3f %7.3f %7.3f\n",           pM->f[0],     pM->f[1],     pM->f[2] );
      printf ( "              %7d %7d %7d\n",                 pM->Index[0], pM->Index[1], pM->Index[2] );
      Flush();
    }
  }
  while ( pM->TotDist > d );
}




void CTriangle::Optimise1(sTriangle *pM, int r, int step)
{ // Optimise by moving all turnpoints
  BOOL bCont = TRUE;
  int n1 = 0;
  int n2 = 0;
  int n3 = 0;
  int j0, j1, j2;
  int p0 = tri.Index[0];
  int p1 = tri.Index[1];
  int p2 = tri.Index[2];
if ( nTriangle == 2  &&  nLoop == 3 )
nLoop = nLoop;

  tri.Index[0] -= r;
  if ( tri.Index[0] < i0 )  tri.Index[0] += npt;
  for ( j0 = -r;   j0 <= r;   j0 += step,  tri.Index[0] += step )
  { if ( tri.Index[0] > i2 )  tri.Index[0] -= npt;

    tri.Index[1] = p1 - r;
    if ( tri.Index[1] < i0 )  tri.Index[1] += npt;
    for ( j1 = -r;   j1 <= r;   j1 += step,  tri.Index[1] += step )
    { if ( tri.Index[1] > i2 )  tri.Index[1] -= npt;

      tri.Index[2] = p2 - r;
      if ( tri.Index[2] < i0 )  tri.Index[2] += npt;  //  psb->npt;
      for ( j2 = -r;   j2 <= r;   j2 += step,  tri.Index[2] += step )
      { if ( tri.Index[2] > i2 )  tri.Index[2] -= npt;
        Calc();
        if ( tri.TotDist > pM->TotDist )
          *pM = tri;
      }
    }
  }
}



/*
void CTriangle::Optimise0(sTriangle *ptri, int d)
{ // Optimise by moving all turnpoints
  // Optimise by moving all turnpoints
  BOOL bCont = TRUE;
  int n1 = 0;
  int n2 = 0;
  int n3 = 0;
  int i1, i2;

  sTriangle trim;
  tri = trim = *ptri;
//  trim.TotDist = 0.;
  while ( bCont )
  { bCont = FALSE;
n1 ++;
    for ( i1 = 0;   i1 < 3;   i1 ++ )
    { tri.Index[i1] -= d;
      if ( tri.Index[i1] < 0 )  tri.Index[i1] += psb->npt;
n2 ++;
//      trim = tri;
      for ( i2 = -d;  i2 < d;  i2 ++,  tri.Index[i1] ++ )
      { if ( tri.Index[i1] >= psb->npt )  tri.Index[i1] -= psb->npt;
n3 ++;
        Calc();
        if ( tri.TotDist > trim.TotDist )
          trim = tri;
        if ( tri.TotDist > ptri->TotDist )
        { *ptri = tri;
          bCont = TRUE;
        }
      }
      tri = trim;
    }
      tri = *ptri;
  }
  if ( debugTRI )
  { printf ( "Calc after step 2: %7.3f %7.3f %7.3f %8.3f\n", ptri->Legs[0], ptri->Legs[1], ptri->Legs[2], ptri->TotDist );
    printf ( "                   %7.3f %7.3f %7.3f\n",       ptri->f[0], ptri->f[1], ptri->f[2] );
    printf ( "                   %7d %7d %7d\n",             ptri->Index[0], ptri->Index[1], ptri->Index[2] );
  }

}
*/




void CTriangle::Calc(void)
{ BOOL bValid = TRUE;
  int i;
  sCoord p0 = psb->ptA[tri.Index[0]].pt;
  sCoord p1 = psb->ptA[tri.Index[1]].pt;
  sCoord p2 = psb->ptA[tri.Index[2]].pt;

  tri.Legs[0] = OrthoDist ( &p0, &p1 );
  tri.Legs[1] = OrthoDist ( &p1, &p2 );
  tri.Legs[2] = OrthoDist ( &p2, &p0 );

  tri.TotDist = 0.;
  for ( i = 0;  i < 3;   i ++ )  tri.TotDist += tri.Legs[i];

  double fMin, fMax;
  if ( tri.TotDist < 750. )
  { fMin = 0.28;
    fMax = 1.;
  }
  else
  { fMin = 0.25;
    fMax = 0.45;
  }
  for ( i = 0;  i < 3;   i ++ )
  { tri.f[i] = tri.Legs[i] / tri.TotDist;
    if ( tri.f[i] < fMin   ||
         tri.f[i] > fMax
       )
    { bValid = FALSE;
      break;
    }
  }
  if ( !bValid )  tri.TotDist = 0.;
}




void CTriangle::MakeKnicks(void)
{ int    j, jj, j1, j2, jM;
  double d, dd, dM;
  sCoord *p1, *p2;
  
  sTable T1 = { 0 };
  sTable T2 = { 0 };
  tS = &T1;
  tD = &T2;

  // Step 1: copy the psb knicks table to tS and tD
  // reserve adequate memory
  j = ( psb->nKnicks + 512 ) / 256 * 256;
  realloc_s ( (void **)&tS->A, sizeof(sTable), &tS->nAR, j, 0 );
//  realloc_s ( (void **)&T2.A, sizeof(sTable), &T2.nAR, j, 0 );

  tS->A[0] = i0;    tS->nA = 1;
  for ( j = 0;   j < psb->nKnicks;   j ++ )
  { if ( psb->Knicks[j] >= i2 )  break;
    if ( psb->Knicks[j] <= i0 )  continue;
    tS->A[tS->nA] = psb->Knicks[j];
    tS->nA ++;
  }
  tS->A[tS->nA] = i2;    tS->nA ++;

  if ( debugTRI )
  { printf ( "MakeKnicks: Initial nKnicks: %d\n", tS->nA );
    Flush();
  }

  // Step 2: insert points at maximum distance gain between knicks at distance > dtgt km
  double dtgt = 10.;
  j = 0;
  while ( j < tS->nA - 1 )
  { j1 = tS->A[j];
    j2 = tS->A[j+1];
    if ( j2 - j1 < 2 )
    { j ++;
      continue;
    }
    p1 = &psb->ptA[j1].pt;
    p2 = &psb->ptA[j2].pt;
    d  = LoxoDist ( p1, p2 );
    if ( d > dtgt )
    { // Find the point in the segment that yields the biggest ditsnce gain
      dM = 0;
      for ( jj = j1+1;   jj < j2;   jj ++ )
      { dd = LoxoDist ( p1, &psb->ptA[jj].pt ) + LoxoDist ( p2, &psb->ptA[jj].pt );
        if ( dd > dM )
        { dM = dd;
          jM = jj;
        }
      }
      if ( dM / d > 1.02 )
      { // Insert the point in tS
        // Don't increment i: i1 -> iM may be > dtgt
        tInsert ( j + 1, jM );
      }
      else j ++;
    }
    else j ++;
  }
  if ( debugTRI )
  { printf ( "MakeKnicks: After step 1: %d\n", tS->nA );
    Flush();
 }

  // Create a point at about every 5 km
//  dtgt = max ( 2.5, sbMaxFF->FF.Dist / 250. );
  dtgt = 5.;
  double dt2 = dtgt * 1.2;  // Don't break-up legs shorter than this
  j = 0;
  while ( j < tS->nA - 1 )
  { j1 = tS->A[j];
    j2 = tS->A[j+1];
    if ( j2 - j1 < 2 )
    { j ++;
      continue;
    }
    p1 = &psb->ptA[j1].pt;
    p2 = &psb->ptA[j2].pt;
    d  = LoxoDist ( p1, p2 );
    if ( d > dt2 )
    { // Insert a point halfway and start again
      tInsert ( j + 1, ( j1 + j2 ) / 2 );
    }
    else j ++;
  }
  if ( debugTRI )
  { printf ( "MakeKnicks: After step 2: %d\n", tS->nA );
    Flush();
  }

  // Copy tS to Knicks
  Knicks   = tS->A;
  nKnicksR = tS->nAR;
  nKnicks  = tS->nA;

  memset ( tS,  0, sizeof(sTable) );        // This will avoid freeing the Knicks
  free ( (void**)&T1.A );
  free ( (void**)&T2.A );
}






void CTriangle::tInsert(int iT, int isb)
{ // Reserve memory in tD if required
  int n;

  if ( tD->nAR < tS->nA + 1 )
  { n = ( tS->nA + 200 ) / 128 * 128 - tD->nAR;
    realloc_s ( (void **)&tD->A, sizeof(sTable), &tD->nAR, n, 0 );
  }

  // Copy
  if ( iT ) memcpy ( tD->A, tS->A, sizeof(int) * iT );
  tD->A[iT] = isb;
  n = tS->nA - iT;
  if ( n ) memcpy ( &tD->A[iT+1], &tS->A[iT], sizeof(int) * n );
  tD->nA = tS->nA + 1;

  // Swap the tables
  sTable *t = tS;
  tS = tD;
  tD = t;
}



CTriDebug TriDebug;

CTriDebug::CTriDebug(void)
{ Knicks  = 0;
  nKnicks = 0;
}


CTriDebug::~CTriDebug(void)
{ free ( (void **)&Knicks );
}
