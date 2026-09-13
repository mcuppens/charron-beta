#include "Global.h"
#include "ffArray.h"

CffArray::CffArray(void)
{ A    = NULL;
  ptL  = NULL;
  nA   = nR = Seq = 0;
  Dist = 0.;
  dMin = 0.;
  Loxo = 0.;
}

CffArray::~CffArray(void)
{ free ( (void **) &A );
}




void CffArray::Init(sLoxo *pl, sFlightPoint *pta, int npta, int nr, int t)
{ ptL  = pl;
  ptA  = pta;
  npt  = npta;
  ntgt = t;
  if ( nr > nR )  realloc_s ( (void **)&A, sizeof(sffA), &nR, nr );
  memset ( A, 0, sizeof(sffA) * nR );
  Dist = 0.;
  Loxo = 0.;
}






double CffArray::LoxoDist(int i1, int i2)
{ // Calculate loxo dist between points in ptL
  double dx = ptL[i2].x - ptL[i1].x;
  dx *= dx;
  double d = ptL[i2].y - ptL[i1].y;
  d *= d;
  d += dx;
  return sqrt ( d );
}






int CffArray::InsertMaxPt(int i0 /*= -1*/, int i2 /*= -1*/)
{ // Insert the point that yields the maximum distance between points i0 to but not including i2
  // Use orthodromic calculions,
  // Take dMin into account
  int    niA = -1;      // Index of the point to be inserted in TPT
  double dL = 0;        // Actual distance of the current leg
  double d1, d2;
  double dM  = 0.;      // Maximum distance increment due to the insertion of the point
  int    idM = 0;       // Index in ptA of the point corresponding to the max dist
  int    niM = 0;       // Index in TPT where the point should be inserted (actual niTPT + 1)
  if ( i0 == -1 ) i0 = 0;
  if ( i2 == -1 ) i2 = npt;
  
  for ( int i = i0;  i < i2;   i ++ )
  { while ( ( niA < nA - 1 )  &&  ( i >= A[niA+1].i ) )
    { // i equals the next point in TPT.  Increment niA
      niA ++;
      if ( niA < ( nA - 1 )  )  dL = A[niA].Dist;
      else                      dL = 0.;
      continue;         // No use to calculate this point
    }
    if ( niA >= 0 )
    { d1 = OrthoDist ( A[niA].i, i );
      if ( d1 < dMin ) continue;
    }
    else d1 = 0.;
    if ( niA < ( nA - 1 ) )
    { d2 = OrthoDist ( i, A[niA+1].i );
      if ( d2 < dMin ) continue;
    }
    else d2 = 0.;
//double d10 = d1;
    d1 += d2;           // Calculate the distance gain resulting from the insertion of the point here
    d1 -= dL;
    if ( d1 > dM )
    { dM = d1;
      idM = i;
      niM = niA;
    }
  }
  if ( dM > 0. )  Insert ( niM + 1, idM );
  return idM;
}





int CffArray::InsertMaxPtLoxo(int i0 /*= -1*/, int i2 /*= -1*/)
{ // Insert the point that yields the maximum distance between points i0 to but not including i2
  // Use loxodromic calculions,
  // Doesn't take dMin into account
  int    niA = -1;      // Index of the point to be inserted in TPT
  double dL = 0;        // Actual distance of the current leg
  double d1, d2;
  double dM  = 0.;      // Maximum distance increment due to the insertion of the point
  int    idM = 0;       // Index in ptA of the point corresponding to the max dist
  int    niM = 0;       // Index in TPT where the point should be inserted (actual niTPT + 1)
  if ( i0 == -1 ) i0 = 0;
  if ( i2 == -1 ) i2 = npt;
  
  for ( int i = i0;  i < i2;   i ++ )
  { while ( ( niA < nA - 1 )  &&  ( i >= A[niA+1].i ) )
    { // i equals the next point in TPT.  Increment niA
      niA ++;
      if ( niA < ( nA - 1 )  )  dL = A[niA].Loxo;   // Don't use Dist: don't mix ortho / loxo
      else                      dL = 0.;
      continue;         // No use to calculate this point
    }
    if ( niA >= 0 )          d1 = LoxoDist ( A[niA].i, i );
    else                     d1 = 0.;
    if ( niA < ( nA - 1 ) )  d2 = LoxoDist ( i, A[niA+1].i );
    else                     d2 = 0.;
    d1 += d2;           // Calculate the distance gain resulting from the insertion of the point here
    d1 -= dL;
    if ( d1 > dM )
    { dM = d1;
      idM = i;
      niM = niA;
    }
  }
  if ( dM > 0. )  Insert ( niM + 1, idM );
  return idM;
}





void CffArray::CalcDist(int Index)
{ // Calculates the distance of legs Index - 1 -> Index -> Index + 1
  // If Index = -1: Calculates all legs
  // Always re-calculates total distance
  if ( Index >= 0 )
  { if ( Index > 0 )
    { A[Index-1].Dist = OrthoDist ( A[Index-1].i, A[Index].i );
      A[Index-1].Loxo = LoxoDist  ( A[Index-1].i, A[Index].i );
    }
    if ( Index < nA - 1 )
    { A[Index].Dist = OrthoDist ( A[Index].i, A[Index+1].i );
      A[Index].Loxo = LoxoDist  ( A[Index].i, A[Index+1].i );
    }
  }
  else 
  { for ( int i = 0;   i < nA-1;   i ++ )
    { A[i].Dist = OrthoDist ( A[i].i, A[i+1].i );
      A[i].Loxo = LoxoDist  ( A[i].i, A[i+1].i );
    }
  }
  Dist = 0.;
  Loxo = 0.;
  for ( int i = 0;   i < nA - 1;   i ++ )
  { Dist += A[i].Dist;
    Loxo += A[i].Loxo;
  }
}





BOOL CffArray::SplitOptimal(void)
{ // Insert two waypoints in any leg of the flight yielding the biggest distance increment
  sSplit sM = { 0 };
  double D0 = Loxo;
  int    i;
  int    nA0 = 0;
  sffA  *A0 = NULL;
  realloc_s ( (void**)&A0, sizeof(sffA), &nA0, nA );

  memcpy ( A0, A, sizeof(sffA) * nA );
  memset ( &Split, 0, sizeof(Split) );
  while ( nA > ntgt - 2 )  DeleteLeast();
  for ( i = 0;   i < nA-1;   i ++ )
  { SplitLeg ( i );
    if ( Split.Gain > sM.Gain + 1.E-6 )  sM = Split;
  }
  if ( Loxo + Split.Gain > D0 + 1.E-6 )
  { // There is a gain by splitting
    Insert ( Split.s1 );
    Insert ( Split.s2 );
    CalcDist ( - 1 );
    free ( (void**)&A0 );
    return TRUE;
  }
  else // No Gain
  { nA = nA0;
    memcpy ( A, A0, sizeof(*A0) * nA );
    Dist = D0;
    free ( (void**)&A0 );
    return FALSE;
  }
}









void CffArray::SplitLeg(int i)
{ // Insert two waypoints the leg i
  int    p0 = A[i  ].i;
  int    p2 = A[i+1].i;
  int    i1, i2, i1M, i2M;
  double d1M = -1.;
  double d2M = 0.;
  double dM  = 0.;
  double d;

  for ( i1 = p0+1;   i1 < p2;   i1 += 15 )
  { d = LoxoDist ( p0, i1 );
    if ( d > d1M )
    { d1M = d;
      i1M = i1;
    }
    else
    { // Insert the max dist point between i1 and p2
      d2M = 0;
      i2M = i1M;
      for ( i2 = i1M;   i2 < p2;   i2 += 15 )
      { d = LoxoDist ( i1M, i2 ) + LoxoDist ( i2, p2 );
        if ( d > d2M )
        { d2M = d;
          i2M = i2;
        }
      }
      d = d1M + d2M - A[i].Loxo;
      if ( d > Split.Gain )
      { Split.Gain = d;
        Split.i    = i;
        Split.s1   = i1M;
        Split.s2   = i2M;
      }
    }
  }
}









void CffArray::Insert(int Index, int p)
{ // Insert point p at index
  if ( Index >= ntgt )  return;      // Beyond boundary
  if ( nA >= nR )
  { realloc_s ( (void**)&A, sizeof(sffA), &nR, 10 );
  }
  if ( Index < nA )
  { // Point must be inserted
    // Move all points one down
    for ( int i = nA;   i > Index;   i -- )
    { if ( i == ntgt )   continue;   // This point will be dropped
      A[i]  = A[i-1];
    }
  }
  if ( nA < ntgt )  nA ++;
  A[Index].i   = p;
  A[Index].Seq = ++ Seq;
  CalcDist ( Index );
}





int CffArray::Insert(int p)
{ // Insert point p, finds out where to insert
  // returns the index of the point
  for ( int i = 0;   i < nA;   i ++ )
  { if ( p == A[i].i )   return i;    // Point is already included
    if ( p < A[i].i )
    { // Insert the point before i
      Insert ( i, p );
      return i;
    }
  }
  // The point is after the last point
  Insert ( nA, p );
  return ( nA );
}




void CffArray::SplitLegs(double dM, int nkM)
{ // Spit long legs
  // Split legs > 100 km in any case,
  // Split legs > dM up to nkM number of knixks is reached
  int    i, j;
  double d;
  while ( 1 )
  { d = 0;              // Find the longest leg
    for ( i = 0;   i < nA - 1;  i ++ )
    { if ( A[i].Dist > d )
      { d = A[i].Dist;
        j = i;
      }
    }
    if ( d > 100.    ||
         ( d > dM   &&   nA < nkM )
       )
    { ntgt = max ( ntgt, nA + 1 );    // Increase ntgt if required
      // Split at the half-time point
      int nA0 = nA;
      Insert ( FixAtdTime ( A[j].i, ( ptA[A[j+1].i].Time - ptA[A[j].i].Time ) / 2., 1 ) );
      if ( nA == nA0 )  break;
    }
    else break;   // No long leg or limit number of knicks reached
  }
}





void CffArray::Delete(int Index)
{ // Delete the point at index
  if ( Index >= nA )   return;
  for ( int i = Index;   i < nA - 1;   i ++ )
  { A[i] = A[i+1];
  }
  nA --;
  CalcDist ( Index );
}






void CffArray::DeleteOldest(void)
{ // Delete the oldest point
  int s = A[0].Seq;
  int n = 0;
  if ( nA <= 1 )
  { nA = 0;
    return;
  }
  for ( int i = 1;   i < nA;   i ++ )
  { if ( A[i].Seq < s )
    { s = A[i].Seq;
      n = i;
    }
  }
  Delete ( n );
}




void CffArray::DeleteLeast(void)
{ // Delete the least relevant turnpoint
  double d;
  double dm = 9999.;
  int    im = 0;
  for ( int i = 1;   i < nA-1;   i ++ )
  { d = A[i-1].Dist + A[i].Dist - OrthoDist ( A[i-1].i, A[i+1].i );
    if ( d < dm )
    { dm = d;
      im = i;
    }
  }
  Delete ( im );
  CalcDist ( -1 );
}



