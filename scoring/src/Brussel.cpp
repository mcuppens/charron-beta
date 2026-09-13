#include "Global.h"
#include "Brussel.h"
#include "Airfields.h"

sCoord CBrussel::ptCtr = { 0 };


void Charronde(void)
{ CSoarBlock *pSB = sbMaxFF;
  if ( !pSB ) pSB = sbMaxT;
  if ( !pSB ) return;

  if ( debugBru )
  { printf ( "\n" );
    printf ( "***********************************************\n" );
    printf ( "CHARONDE EVALUATION \n" );
    printf ( "\n" );
    Flush();
  }

  sAirfield *pA = &Airfields.List[pSB->pOwner->AirfTO];
  if ( !pA->BNL )  return;     // Only files taking off from benelux are checked

  // Center of Brussels CTR: AIP: 505405N 0042904E (dms)
  CBrussel::ptCtr.Lat = 50. + ( 54. + 5. / 60. ) / 60.;
  CBrussel::ptCtr.Lon =  4. + ( 29. + 4. / 60. ) / 60.;

  // other positions for debugging
/*
  // Kleine Brogel 51 10 150N  005 28 300E
  CBrussel::ptCtr.Lat = 51. + ( 10. + 150. / 1000. ) / 60.;
  CBrussel::ptCtr.Lon =  5. + ( 28. + 300. / 1000. ) / 60.;
/*
  // Leopoldsburg 51 07 250N  005 18 450E
  CBrussel::ptCtr.Lat = 51. + (  7. + 250. / 1000. ) / 60.;
  CBrussel::ptCtr.Lon =  5. + ( 18. + 450. / 1000. ) / 60.;
/*
  // Keiheuvel 51 10 750N  005 13 250E
  CBrussel::ptCtr.Lat = 51. + ( 10. + 750. / 1000. ) / 60.;
  CBrussel::ptCtr.Lon =  5. + ( 13. + 250. / 1000. ) / 60.;
*/

  CBrussel::ptCtr.Lat *= piDiv180;
  CBrussel::ptCtr.Lon *= piDiv180;

  CBrussel B ( pSB );
  B.Calc();

  if ( debugBru )
  { printf ( "\n" );
    printf ( "END CHARONDE EVALUATION\n" );
    printf ( "***********************************************\n" );
    printf ( "\n" );
    Flush();
  }
}




CBrussel::CBrussel(CSoarBlock *psb)
{ pSB = psb;
}




CBrussel::~CBrussel(void)
{
}



void CBrussel::Calc(void)
{ if ( !pSB )  return;
  
  double t,  t0;
  double tm, tM;        // Minimum & Maximum track
  int    im, iM;        // Index in pSB->ptA of tm & tM
  int    npt = pSB->npt;
  sFlightPoint *pt = pSB->ptA;

  t0 = tm = tM = ::OrthoTrk ( &ptCtr, &pt->pt );
  im = iM = 0;
  pt ++;

  for ( int i = 1;   i < npt;   i ++,  pt ++ )
  { t = ::OrthoTrk ( &ptCtr, &pt->pt );
    while ( t - t0 >  180. )   t -= 360.;   // from NE to NW quadrant:
    while ( t - t0 < -180. )   t += 360.;   // from NW to NE quadrant:
    if ( t < tm )  { tm = t;    im = i;  }
    if ( t > tM )  { tM = t;    iM = i;  }
    t0 = t;
  }

  // Fill the pSB BRU struct
  sBrussel *B = &pSB->BRU;
  if ( iM >= im )
  { B->t1 = tm;     B->i1 = im;
    B->t2 = tM;     B->i2 = iM;
  }
  else
  { B->t1 = tM;     B->i1 = iM;
    B->t2 = tm;     B->i2 = im;
  }
  B->Angle = B->t2 - B->t1;
  // Cast t1 & t2 to range 0 -> 360
  while ( B->t1 <   0. - 0.05 )  B->t1 += 360.;    // Take into account roundings: formatting %0.1f
  while ( B->t1 > 360. + 0.05 )  B->t1 -= 360.;
  while ( B->t2 <   0. - 0.05 )  B->t2 += 360.;
  while ( B->t2 > 360. + 0.05 )  B->t2 -= 360.;
}


