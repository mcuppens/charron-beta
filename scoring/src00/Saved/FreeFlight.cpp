#include "Global.h"



void CalcFreeFlight(void)
{ // Calc the free flight distance of all soaring blocks of
  // the flight containing the longest soaring block
  if ( !sbMaxT )  return;
  CFlight *pF = sbMaxT->pOwner;
  if ( !pF )  return;
  double      Max = 0.;        // The maximum FF distance 
  CSoarBlock *pSB = pF->SB[0];
  sbMaxFF = NULL;

  for ( int i = 0;   i < pF->nSB;   i ++,  pSB ++ )
  { CFreeFlight FF ( pSB->ptA, pSB->npt );
    FF.Calc();

    // Store the result in the soar block
    memcpy ( pSB->FF.TptA, FF.TPT, sizeof(pSB->FF.TptA) );
    pSB->FF.npt  = FF.nTPT;
    pSB->FF.Dist = FF.Dist;
    pSB->CalcFF();

    // Get the maximum FF soar block
    if ( pSB->FF.Dist > Max )
    { Max = pSB->FF.Dist;
      sbMaxFF = pSB;
    }
  }
}



CFreeFlight::CFreeFlight(sFlightPoint *pta, int n)
{ ptA  = pta;
  npt  = n;
  nTPT = 0;
  Dist = 0.;
  nTPT = 0;
  nSeq = 0;
  memset ( TPT,  0, sizeof(TPT) );
  memset ( SeqA, 0, sizeof(SeqA) );
}



CFreeFlight::~CFreeFlight(void)
{
}



void CFreeFlight::Calc(void)
{ ntgt = ffMaxTpt;
  Insert ( 0 );                               // Seed the TPT array with first and last point of the soaring block
  Insert ( npt - 1 );
  while ( nTPT < ffMaxTpt )  InsertMaxPt();   // Preliminary turnpoints
  Optimize();
  if ( Dist < ffExtraPtDist )
  { // Recalculate with one less turnpoint
    ntgt --;
    if ( nTPT > ntgt )
    { // Just delete any point
      nTPT --;
      CalcDist ( -1 );
    }
    // Optimise again
    Optimize();
  }
}


void CFreeFlight::Optimize(void)
{ // Optimize
  double dM = 0.;
  int    nM = 0;
  while ( nM < ntgt + 1 )
  { dM = Dist;
    DeleteOldest();
    InsertMaxPt();
    if ( Dist <= dM )
    { nM ++;
    }
    else nM = 0;
  }
}



void CFreeFlight::InsertMaxPt(void)
{ // Insert the point that yields the maximum distance
  int    niTPT = -1;    // Index of the point to be inserted in TPT
  double dL = 0;        // Actual distance of the current leg
  double d1, d2;
  double dM  = 0.;      // Maximum distance increment due to the insertion of the point
  int    idM = 0;       // Index in ptA of the point corresponding to the max dist
  int    niM = 0;       // Index in TPT where the point should be inserted (actual niTPT + 1)
  
  for ( int i = 0;  i < npt;   i ++ )
  {
if ( i == npt - 3 )
i = i;
    if ( ( niTPT < nTPT - 1 )  &&  ( i == TPT[niTPT+1].i ) )
    { // i equals the next point in TPT.  Increment niTPT
      niTPT ++;
      if ( niTPT < ( nTPT - 1 )  )  dL = TPT[niTPT].Dist;
      else                          dL = 0.;
      continue;         // No use to calculate this point
    }
    if ( niTPT >= 0 )            d1 = OrthoDist ( TPT[niTPT].i, i );
    else                         d1 = 0.;
    if ( niTPT < ( nTPT - 1 ) )  d2 = OrthoDist ( i, TPT[niTPT+1].i );
    else                         d2 = 0.;
    d1 += d2;           // Calculate the distance gain resulting from the insertion of the point here
    d1 -= dL;
    if ( d1 > dM )
    { dM = d1;
      idM = i;
      niM = niTPT;
    }
  }
  if ( dM > 0. )  Insert ( niM + 1, idM );
}




void CFreeFlight::Insert(int Index, int p)
{ // Insert point p at index
  if ( Index >= ntgt )  return;      // Beyond boundary
  if ( Index < nTPT )
  { // Point must be inserted
    // Move all points one down
    for ( int i = nTPT;   i > Index;   i -- )
    { if ( i == ntgt )   continue;   // This point will be dropped
      TPT[i]  = TPT[i-1];
      SeqA[i] = SeqA[i-1];
    }
  }
  if ( nTPT < ntgt )  nTPT ++;
  TPT[Index].i = p;
  SeqA[Index]  = ++nSeq;
  CalcDist ( Index );
}



void CFreeFlight::Insert(int p)
{ // Insert point p, finds out where to insert
  for ( int i = 0;   i < nTPT;   i ++ )
  { if ( p == TPT[i].i )   return;    // Point is already included
    if ( p < TPT[i].i )
    { // Insert the point before i
      Insert ( i, p );
      return;
    }
  }
  // The point is after the last point
  Insert ( nTPT, p );
}



void CFreeFlight::Delete(int Index)
{ // Delete the point at index
  if ( Index >= nTPT )   return;
  for ( int i = Index;   i < nTPT - 1;   i ++ )
  { TPT[i] = TPT[i+1];
    SeqA[i] = SeqA[i+1];
  }
  nTPT --;
  CalcDist ( Index );
}



void CFreeFlight::DeleteOldest(void)
{ // Delete the oldest point
  int s = SeqA[0];
  int n = 0;
  if ( nTPT <= 1 )
  { nTPT = 0;
    return;
  }
  for ( int i = 1;   i < nTPT;   i ++ )
  { if ( SeqA[i] < s )
    { s = SeqA[i];
      n = i;
    }
  }
  Delete ( n );
}




void CFreeFlight::CalcDist(int Index)
{ // Calculates the distance of legs Index - 1 -> Index -> Index + 1
  // If Index = -1: Calculates all legs
  // Always re-calculates total distance
  if ( Index >= 0 )
  { if ( Index > 0 )        TPT[Index-1].Dist = OrthoDist ( TPT[Index-1].i, TPT[Index].i );
    if ( Index < nTPT - 1 ) TPT[Index  ].Dist = OrthoDist ( TPT[Index].i, TPT[Index+1].i );
  }
  Dist = 0.;
  for ( int i = 0;   i < nTPT - 1;   i ++ )
  { Dist += TPT[i].Dist;
  }
}
