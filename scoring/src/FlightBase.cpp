#include "Global.h"


CFlightBase::CFlightBase(void)
{ ptA     = NULL;
  npt     = nptR  = 0;
  Start   = 0;
  nAltBad = 0;
  pAltBad = 0.;
}


CFlightBase::~CFlightBase(void)
{
}




void CFlightBase::SetFlag(int i, DWORD Flag, BOOL bSet)
{ ptA[i].Flag |= Flag;
  if ( !bSet )   ptA[i].Flag ^= Flag;
}



void CFlightBase::CountAltBad(void)
{ nAltBad = 0;
  for ( int i = 0;  i < npt;   i ++ )
  { if ( GetFlag ( i, FLT_ALT_BAD ) )  nAltBad ++;
  }
  pAltBad = 100. * nAltBad / npt;
}



// Look for the fix at a given time (decimal hour)
// If there is no exact match for the fix:
//   if d >  0: find the first next fix
//   if d <= 0: find the first previous fix
int CFlightBase::FixAtTime(double time, int d)
{ int n = 1;
  while ( 1 )
  { n *= 2;
    if ( n >= npt )  break;
    if ( ptA[n].Time > time )  break;
  }
  int i2 = n;
  while ( n )
  { n /= 2;
    if ( n == 0 ) break;
    if ( n >= npt )
    { i2 -= n;
      continue;
    }
    if ( i2 >= npt   ||   ptA[i2].Time > time )
    { i2 -= n;
      continue;
    }
    else if ( ptA[i2].Time < time )
    { i2 += n;
      continue;
    }
    else n = 0;
  }
  
  if ( ptA[i2].Time == time )  return i2;
  if ( ptA[i2].Time >  time )
  { if ( d > 0 )  return i2;
    else          return max ( 0, i2-1 );
  }
  else if ( d < 0 )  return i2;
  else               return min ( npt-1, i2+1 );

  return npt - 1;   // time was not found
}




// Find the fix at dTime after fix i0
// If there is no exact match for the fix:
//   if d >  0: find the first next fix
//   if d <= 0: find the first previous fix
int CFlightBase::FixAtdTime(int i0, double dTime, int d)
{ double t2 = ptA[i0].Time + dTime;
  int n = 1;
  int i2;
  while ( 1 )
  { n *= 2;
    i2 = i0 + n;
    if ( i2 >= npt )  break;
    if ( ptA[i2].Time > t2 )  break;
  }

  while ( n )
  { n /= 2;
    if ( n == 0 ) break;
    if ( i2 >= npt )
    { i2 -= n;
      continue;
    }
    if ( ptA[i2].Time > t2 )
    { i2 -= n;
      continue;
    }
    else if ( ptA[i2].Time < t2 )
    { i2 += n;
      continue;
    }
    else n = 0;
  }
  
  if ( ptA[i2].Time == t2 )  return i2;
  if ( ptA[i2].Time >  t2 )
  { if ( d > 0 )  return i2;
    else          return max ( 0, i2-1 );
  }
  else if ( d < 0 )  return i2;
  else               return min ( npt-1, i2+1 );

  return npt - 1;   // time was not found
}





// Find the closest fix at at least Dist from fix i0
// Dist can be negative: look before i0
// returns -1 if no fix found
int CFlightBase::FixAtDist(int i0, double Dist)
{ int i2 = i0;
  int s  = 1;
  if ( Dist < 0. )
  { Dist = -Dist;
    s    = -1;
  }
  while ( 1 )
  { i2 += s;
    if ( i2 <  0   )  return -1;
    if ( i2 >= npt )  return -1;
    if ( OrthoDist ( i0, i2 ) >= Dist ) return i2;
  }
}





double CFlightBase::OrthoDist(int p1, int p2)
{ return ::OrthoDist ( &ptA[p1].pt, &ptA[p2].pt );
}


double CFlightBase::OrthoTrk(int p1, int p2)
{ return ::OrthoTrk ( &ptA[p1].pt, &ptA[p2].pt );
}



double CFlightBase::wgs84(int p1, int p2)
{ return ::wgs84 ( &ptA[p1].pt, &ptA[p2].pt );
}


double CFlightBase::dTime(int i1, int i2, double u/*=1.*/)
{ return ( ptA[i2].Time - ptA[i1].Time ) * u;
}


double CFlightBase::dAlt(int i1, int i2)
{ return ptA[i2].Altitude - ptA[i1].Altitude;
}



double CFlightBase::Speed(int i)
{ // From fix i to next fix
  return Speed ( i, i+1 );
}


double CFlightBase::Speed(int i1, int i2)
{ // From fix i1 to fix i2
  if ( ( i1 < 0    )  ||  ( i2 < 0    ) )  return 0.;
  if ( ( i1 >= npt )  ||  ( i2 >= npt ) )  return 0.;
  return fabs ( OrthoDist ( i1, i2 ) / ( ptA[i2].Time - ptA[i1].Time ) );
}



double CFlightBase::Vz(int i)
{ // From fix i to next fix
  return Vz ( i, i+1 );
}


double CFlightBase::Vz(int i1, int i2)
{ // From fix i1 to fix i2
  if ( ( i1 < 0    )  ||  ( i2 < 0    ) )  return 0.;
  if ( ( i1 >= npt )  ||  ( i2 >= npt ) )  return 0.;
  return ( ptA[i2].Altitude - ptA[i1].Altitude ) / ( ptA[i2].Time - ptA[i1].Time ) / 3600.;
}



char  *CFlightBase::stHour(int i)
{ double t = 0.;
  if ( ( i >= 0 )   &&   i < npt )   t = ptA[i].Time;
  return ::stHour ( t );
}
