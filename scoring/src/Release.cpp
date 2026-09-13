#include "Global.h"
#include "Release.h"
#include "Airfields.h"


double tReleaseManual = 0.;


void FindRelease(void)
{ // Find release of all flights
  if ( FlightA == NULL )  return;
  if ( !FLT_Automatic )
  { // Flt is set via command line.  In this case ther is only one flight.
    // Set Release = start of flight
    FlightA[0]->Release  = 0;
    FlightA[0]->bRelease = TRUE;
    return;
  }

  int f;
  CRelease R;
  BOOL bRelease = FALSE;

  for ( f = 0;   f < nFlights;   f ++ )
  { R.Calc ( FlightA[f] );
    bRelease |= FlightA[f]->bRelease;
  }
  if ( !bRelease )
  { report_error ( EC_RELEASE_POINT, NULL );
  }
  if ( debugRelease )  fflush ( stdout );
}





CRelease::CRelease(void)
{ Trigger[RT_ALT].dtLim = 40;      Trigger[RT_ALT].nLim = 3;   // Altitude drop
  Trigger[RT_GS ].dtLim = 45;      Trigger[RT_GS ].nLim = 3;   // Ground speed
  Trigger[RT_ROT].dtLim = 25;      Trigger[RT_ROT].nLim = 3;   // Rate of turn
  Trigger[RT_DVZ].dtLim = 20;      Trigger[RT_DVZ].nLim = 3;   // Delta Vz
  Trigger[RT_VZI].dtLim = 40;      Trigger[RT_VZI].nLim = 4;   // Integrated Vz
}
 


CRelease::~CRelease(void)
{
}



void CRelease::Calc(CFlight *pf)
{ pF  = pf;
  ptA = pF->ptA;
  npt = pF->npt;
  
  Init();
  GetFixAccuracy();

  if ( debugRelease )
  { printf ( "***** Detetcting release Flight %d\n", pF->nFlight + 1 );
    printf ( "Take-off %s alt %d - Landing %s\n", stHour(0), ptA[0].Altitude, stHour(pF->npt-1) );
    printf ( "low accuracy %d, dtMin %0.0f\n", LowAccuracy, dtMin );
    printf ( "Minimum altitude: %0.0f\n", MinAlt );
    printf ( "First fix speed: %0.1f\n", Speed ( 0, 1 ) );
  }

  while ( i < npt - 1 )
  { if ( !GetFix() )  break;
    GetFixData();
    if ( ptA[i].Altitude > MinAlt )
    { CheckVz_D();
      CheckVz_I();
      CheckAD();
      CheckROT();
      CheckGS();
      if ( CheckRelease() ) break;
    }
    CalcVzI_D();
    CalcVzI_I();
    i0 = i;
    i ++;
  }

  if ( debugRelease  &&  !pF->Release )
  { printf ( ">>>>> Release at point not found \n" );
  }
}




void CRelease::Init(void)
{ // Initialise variables
  i0 = 0;
  i  = 1;
  i1 = 1;
  message[0] = 0;
  ErrInterv  = FALSE;
  dtTooLong  = 0.;
  // vz algo params
  vzI_D = vzI_I = Vz ( 0 );
  // Alt drop algo params
  hmin       = 99999.;
  hmax       = -9999.;
  ihmax      = 0;
  hdrop      = 0.;
  // ROT algo params
  iStraight  = 0;
  iTurning   = 0;
  tr0        = OrthoTrk ( 0, 1 );
  // GS algo params
  trMin      = 360.;  // Low ground speed - minimum track
  trMax      = 0.;    // Low ground speed - maximum track
  nLowspeed  = 0;
  Trigger[RT_DVZ].iRelease = 0;
  Trigger[RT_VZI].iRelease = 0;
  Trigger[RT_ALT].iRelease = 0;
  Trigger[RT_ROT].iRelease = 0;
  Trigger[RT_GS ].iRelease = 0;

  if ( Speed ( 0, 1 ) > 60.   &&
       pF->dtBaseLine < 1.    &&
       pF->nBaseLine  < 6
     )
  { // This could be an in-flight resume 
    MinAlt = 0.;
  }
  else MinAlt = pF->AltBaseLine + 200.;
}




void CRelease::GetFixAccuracy(void)
{ // Determine GPS fix accuracy
  // LowAccuracy: 1 if position is known to 1/100 arc minute
  int    nFixAcy = min ( 100, npt - 2 );
  int    nLowAcy = 0;
  double Lat, Lon;
  double LatM1, LonM1, LatM2, LonM2;
  double dLat, dLon;

  for ( int i = 0;   i < nFixAcy;   i++ )
  { Lat   = ptA[i].pt.Lat * c180divPi * 6000.;
    Lon   = ptA[i].pt.Lon * c180divPi * 6000.;
    LatM1 = floor ( Lat * 10. + 0.01 ) - floor ( Lat + 0.01 ) * 10.;
    LonM1 = floor ( Lon * 10. + 0.01 ) - floor ( Lon + 0.01 ) * 10.;

    Lat   = ptA[i+1].pt.Lat * c180divPi * 6000.;
    Lon   = ptA[i+1].pt.Lon * c180divPi * 6000.;
    LatM2 = floor ( Lat * 10. + 0.01 ) - floor ( Lat + 0.01 ) * 10.;
    LonM2 = floor ( Lon * 10. + 0.01 ) - floor ( Lon + 0.01 ) * 10.;

    dLat = fabs ( LatM2 - LatM1 );
    dLon = fabs ( LonM2 - LonM1 );
 
    if ( ( dLat == 0. )  &&  ( dLon == 0. ) )  nLowAcy ++;
  }
  
  if ( ( 10 * nLowAcy ) / nFixAcy > 2 )
  { LowAccuracy = TRUE;
    dtMin = 5.;
  }
  else
  { LowAccuracy = FALSE;
    dtMin = 3.;
  }
  
  if ( debugRelease && LowAccuracy )
    printf("**** FIX ACCURACY LOW: increasing intervals to 5 s minimum ****\n");
}



BOOL CRelease::GetFix(void)
{ // get the next fix indexes for calulation
  dt = dTime ( i0, i, 3600. );

  if ( dt > TmaxInterval )
  { dtTooLong += ( dt - TmaxInterval );
    if ( ( dtTooLong > TInterval_Trsh )   &&   !ErrInterv )
    { sprintf ( message,"%.0f s", TmaxInterval);
      report_error ( EC_SAMPLE_TOOLONG, message );
      ErrInterv = 1;
    }
  }

//  if ( LowAccuracy )
  { while ( dt <= dtMin )
    { i ++;
      if ( i >= npt )  return FALSE;
      dt = dTime ( i0, i, 3600. );
      tr0 = OrthoTrk ( i0, i );
    }
    
    i1 = i+1;
    while ( dTime ( i, i1, 3600. ) <= dtMin )
    { i1 ++;
      if ( i1 >= npt )  return FALSE;
    }
  }
  if ( debugRelease )
  { sprintf ( sti, "%4d %4d %4d %s ", i0, i, i1, stHour(i) );
  }
  return TRUE;
}




void CRelease::GetFixData(void)
{ hmin = min ( hmin, ptA[i].Altitude );
  // get ground speed, ROT, Vz, dAlt
  v  = Speed ( i0, i );
  
  tr = OrthoTrk ( i, i1 );
  a  = fabs ( tr - tr0 );
  if ( a > 180. ) a = 360.0 - a;
  ROT  = a / dt;                         // rate of turn (degree/s)

  Bank = atan ( v / 3.6 * ROT * piDiv180 / 9.81 ) / piDiv180;
if ( ptA[i].Altitude > 1929 )
i = i;
  
  vz  = Vz ( i0, i );                  // instantaneous rate of climb (m/s)
  tr0 = tr;
  // Integrated vz must be calculated later !
}





void CRelease::CalcVzI_D(void)
{ // Calculate integrated Vz, first determine time constant
  double Tau;
  if ( vz > vzI_D ) Tau = 35.;
  else              Tau = 80.;
  if ( dt <= Tau )  vzI_D += ( vz - vzI_D ) * dt / Tau;
  else              vzI_D = vz;
}




void CRelease::CalcVzI_I(void)
{ // Calculate integrated Vz, first determine time constant
  double Tau = 40.;
  if ( dt <= Tau )  vzI_I += ( vz - vzI_I ) * dt / Tau;
  else              vzI_I = vz;
}





void CRelease::CheckVz_D(void)
{ // Check on delta rate of climb
  if ( //vzI_D > 3   &&              // Will not work well for weak tugs: 
       vz < ( vzI_D - 2.5 )
     )
  { if ( !Trigger[RT_DVZ].iRelease )
    { Trigger[RT_DVZ].iRelease = i;
      if ( debugRelease )  printf ( "%s Trigger dVz %d %0.1f  %0.1f\n", sti, Trigger[RT_DVZ].iRelease, vz, vzI_D );
    }
  }
  else if ( vz > ( vzI_D - 2. ) )
  { if ( debugRelease  &&  Trigger[RT_DVZ].iRelease )  printf ( "%s Reset dVz %0.1f  %0.1f\n", sti, vz, vzI_D );
    Trigger[RT_DVZ].iRelease = 0;
  }
}





void CRelease::CheckVz_I(void)
{ if ( vzI_I < 0.3 )
  { if ( !Trigger[RT_VZI].iRelease )
    { Trigger[RT_VZI].iRelease = i0;
      if ( debugRelease )  printf ( "%s Trigger vzI_D %d %0.1f  %0.1f\n", sti, Trigger[RT_VZI].iRelease, vz, vzI_I );
    }
  }
  else if ( vzI_I > 1. )
  { if ( debugRelease  &&  Trigger[RT_VZI].iRelease )  printf ( "%s Reset vzI_D %0.1f  %0.1f\n", sti, vz, vzI_I );
    Trigger[RT_VZI].iRelease = 0;
  }
}







void CRelease::CheckAD(void)
{
  if ( ptA[i].Altitude >= hmax )
  { if ( debugRelease   &&   hdrop )  printf ( "%s Reset alt drop  %d  %0.0f\n", sti, ptA[i].Altitude, hmax );
    hmax  = ptA[i].Altitude;
    hdrop = 0.;
    ihmax = i;
  }
  else
  { hdrop  = hmax - ptA[i].Altitude;     // altitude drop since maximum hmax
    if ( debugRelease )  printf ( "%s hdrop %0.0f\n", sti, hdrop );
  }

  if ( ( ptA[i].Altitude > hmin + 20 )   &&
       ( hdrop > hdrop_Trsh )            &&
       !Trigger[RT_ALT].iRelease
     )
  { Trigger[RT_ALT].iRelease = ihmax + 1;
    if ( debugRelease )  printf ( "%s Trigger %d  hdrop %0.0f\n", sti, Trigger[RT_ALT].iRelease, hdrop );
  }
}





void CRelease::CheckROT(void)
{ // Check on rate of turn
    
  // Start of turn: turn rate >= turnrate_Trsh_Hi
  // Stop of turn:  turn rate <  turnrate_Trsh_Lo
  //                during more than Tstraight_Trsh
  if ( iTurning )
  { if ( ( ROT < turnrate_Trsh_Lo ) )
    { if ( debugRelease )  printf ( "%s Straight, ROT < Low\n", sti );
      iTurning   = 0;
      iStraight  = 0;
      Trigger[RT_ROT].iRelease = 0;
      return;
    }
    if ( !iStraight )  iStraight = i0;
    if ( dTime ( iStraight, i ) > Tstraight_Trsh )
    { if ( debugRelease )  printf ( "%s Straight, time between low & high\n", sti );
      iTurning   = 0;
      Trigger[RT_ROT].iRelease = 0;
      return;
    }
//    else iStraight = 0;

    // Released when time turning exceeds time threshold
    if ( dTime ( iTurning, i, 3600. ) > Tturning_Trsh )
      if ( !Trigger[RT_ROT].iRelease )
      { Trigger[RT_ROT].iRelease = iTurning;
        if ( debugRelease )  printf ( "%s Trigger ROT  %d\n", sti, Trigger[RT_ROT].iRelease );
      }
  }
  else if ( ROT > turnrate_Trsh_Hi )
  { iTurning   = i0;
    iStraight  = 0;
    Trigger[RT_ROT].iRelease = 0;
    if ( debugRelease )  printf ( "%s ROT Start %d %0.1f\n", sti, i0, ROT );
  }
}




void CRelease::CheckGS(void)
{ // Check on low ground speed on 2 tracks differing >=90°
  if ( dAlt ( 0, i ) < 250 )  return;
  if ( v < 80. )
  { double dtr;
    if ( nLowspeed > 3 )
    { dtr = trMin - tr;
      if ( dtr < 0.)    dtr = -dtr;
      if ( dtr > 180.)  dtr = 360. - dtr;
      if ( ( dtr >= 90. )  &&  !Trigger[RT_GS].iRelease )
        Trigger[RT_GS].iRelease = i0;
  
      dtr = trMax - tr;
      if ( dtr < 0. )    dtr = -dtr;
      if ( dtr > 180. )  dtr = 360. - dtr;
      if ( ( dtr >= 90. )  &&  !Trigger[RT_GS].iRelease )
      { Trigger[RT_GS].iRelease = i0;
        if ( debugRelease )  printf ( "%s Trigger GS %0.0f° %0.0f° %d kmh\n", sti, trMin, trMax, Trigger[RT_GS].iRelease );
      }
    }
    else
    { if ( debugRelease )  printf ( "%s start GS  %0.0f\n", sti, v );
    }
     
    if ( tr < trMin )  trMin = tr;
    if ( tr > trMax )  trMax = tr;
  
    nLowspeed++;
  }
  else
  { Trigger[RT_GS].iRelease = 0;
  }
}






BOOL CRelease::CheckRelease(void)
{ // Is any of the release algorithms triggererd?
  int j;
  int nt = 0;
  int it = -1;
  sTrigger T = { 9999., 9999 };
  pF->Release = npt;

  for ( j = 0;   j < 5;   j ++ )
  { if ( Trigger[j].iRelease )
    { if ( Trigger[j].iRelease < pF->Release )
      { pF->Release = Trigger[j].iRelease;
        it = j;
      }
      T.dtLim = min ( T.dtLim, Trigger[j].dtLim );
      T.nLim  = min ( T.nLim,  Trigger[j].nLim );
      nt ++;
    }
  }
  if ( !nt ) return FALSE;   // Nothing triggered

  // Return Release if time since earliest detection exceeds the maximum detection delay time
  int    di = i - pF->Release + nt / 2;                   // fictively increase di & dt ico multiple triggers
  double dt = dTime ( pF->Release, i, 3600. ) + nt * 10.;
  if ( dt >= T.dtLim   &&
       di >= T.nLim
     )
  { pF->bRelease = TRUE;
    pF->ReleaseTrigger = (eReleaseType)it;

    if ( debugRelease )
    { printf ( "%s Release detected %d %s Alt %d, Type ", sti, pF->Release, stHour(pF->Release), ptA[pF->Release].Altitude );
      switch ( pF->ReleaseTrigger )
      { case RT_ALT:  printf ( "Altitude drop\n" ); break;
        case RT_GS :  printf ( "Ground speed\n" );  break;
        case RT_ROT:  printf ( "Rate of turn\n" );  break;
        case RT_DVZ:  printf ( "Delta Vz\n" );      break;
        case RT_VZI:  printf ( "Vz Integrtd\n" );   break;
      }
    }
    return TRUE;    // Release found
  }
  return FALSE;
}