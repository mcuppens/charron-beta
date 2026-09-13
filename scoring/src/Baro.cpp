#include "Global.h"
#include "Baro.h"


void PlotBaro(void)
{ CBaro B;
  B.Draw();
}


#include "gdfonts.h"
#include "gdfontl.h"
#include "gdfontg.h"

CBaro::CBaro(void)
{ // Perform general initialisations
  Scale = 1;
 #ifdef _DEBUG
  Scale = 3;
 #endif
  switch ( Scale )
  { case 1:  Font = gdFontSmall;   break;
    case 2:  Font = gdFontLarge;   break;
    default: Font = gdFontGiant;
  }
  xoffset  = 0;
  xmax     = 850 * Scale;
  yoffset  = 15  * Scale;
  ymax     = 300 * Scale;
  xLegend  = xmax;
  RangeENL = 1200.;

  im      = gdImageCreate ( xmax, ymax+25 );
  //                                     red  grn  blu
  white     = gdImageColorAllocate ( im, 255, 255, 255 );
  Red       = gdImageColorAllocate ( im, 255,   0,   0 );
  green     = gdImageColorAllocate ( im,   0, 255,   0 );
  blue      = gdImageColorAllocate ( im,   0,   0, 255 );
  black     = gdImageColorAllocate ( im,   0,   0,   0 );
//  SkyBlue   = gdImageColorAllocate ( im, 145, 200, 255 );
//  ltSkyBlue = gdImageColorAllocate ( im, 155, 220, 255 );
  SkyBlue   = gdImageColorAllocate ( im, 200, 225, 255 );
  ltSkyBlue = gdImageColorAllocate ( im, 220, 240, 255 );
  BlueSpd   = gdImageColorAllocate ( im, 120, 070, 255 );
  pink      = gdImageColorAllocate ( im, 200, 100, 200 );
  cyan      = gdImageColorAllocate ( im, 100, 230, 255 );
  Orange    = gdImageColorAllocate ( im, 255, 153,   0 );
  yellow    = gdImageColorAllocate ( im, 200, 200,   0 );  // ENL
  ltyellow  = gdImageColorAllocate ( im, 230, 230,   0 );  // MOP
  magenta   = gdImageColorAllocate ( im, 255,   0, 255 );
  greendk   = gdImageColorAllocate ( im,  50, 180,   0 );
  grey      = gdImageColorAllocate ( im,  70,  70,  70 );
  greylt    = gdImageColorAllocate ( im, 140, 140, 140 );
  maroon    = gdImageColorAllocate ( im, 200,  50,  50 );

  styleDashed[0] = greylt;
  styleDashed[1] = greylt;
  styleDashed[2] = greylt;
  styleDashed[3] = gdTransparent;
  styleDashed[4] = gdTransparent;
  styleDashed[5] = gdTransparent;
}

CBaro::~CBaro(void)
{
  gdImageDestroy(im);
}



void CBaro::Draw(void)
{ 
  if ( debugPlot )
  { printf ( "\n>>>>Drawing baro\n" );
    if ( OutF )  fflush ( OutF );
  }
  if ( !Init() )
  { printf ( "Error initialising Baro\n" );
    return;
  }
  DrawFlights  ( ltSkyBlue );
  DrawSoaring  ( SkyBlue );
  DrawEngine   ( Orange );
  DrawMOP      ( ltyellow );
  DrawENL      ( yellow );
  DrawMOPline  ( ltyellow );
  DrawAltGrid  ( grey );
  DrawTimeGrid ( black );
  DrawfENL     ( Orange );
  DrawENLThreshold ( Orange );
  DrawGPS      ( greylt );
  DrawAlt      ( pink );
  DrawSpeed    ( BlueSpd );
  DrawAltGain  ( magenta );
  DrawFF       ( Red );
  DrawDT       ( green );
  DrawRelease  ( Red );
  WriteTitle   ( black );

  FILE *F = fopen ( file_baro, "w+b" ); 
  if ( !F )  report_error ( EC_OUT_BAROFILE, NULL );
  gdImagePngOwn ( im, F );    // On windows gdImagePng crashes
  fclose ( (void**)&F );
}






void CBaro::gdImagePngOwn(gdImagePtr im, FILE *F)
{ // First read the image into a buffer, then write the buffer to the file
  int S;
  char *B = (char *) gdImagePngPtr ( im, &S );
  if ( !B )
  { report_error ( EC_MEMORY, "CBaro::gdImagePngOwn" );
    return;
  }
  if ( fwrite ( B, S, 1, F ) != 1 )
  { report_error ( EC_OUT_BAROFILE, NULL );
    return;
  }
  gdFree ( B );  
}




BOOL CBaro::Init(void)
{ // Determine altitude maximum and minimum
  alti_max = 1000.0; 
  alti_min =    0.0;
  if ( !Trace.npt )  return FALSE;

  sFlightPoint *pt = Trace.ptA;
  
  // Set altitude and time scales and zeros
  time_first = pt[0].Time;
  time_last  = pt[Trace.npt-1].Time;  

  for ( int i = 0;   i < Trace.npt;   i++,  pt ++ )
  { alti_max = max ( alti_max, pt->Altitude );
    alti_max = max ( alti_max, pt->AltiGPS );
    alti_min = min ( alti_min, pt->Altitude );
    alti_min = min ( alti_min, pt->AltiGPS );
  }
  alti_min = max ( alti_min, -400 ); 
  alti_max = min ( alti_max, 12000 ); 
 
  alti_max = 500. * ceil ( ( alti_max + 50. ) / 500. );

  time_scale = ( xmax - xoffset ) / ( time_last - time_first );
  alti_scale = ( ymax - yoffset ) / ( alti_max  - alti_min );

  bENL = TRUE;
//  if ( !Engine.Installed )            bENL = FALSE;
//  if ( !FLT_Automatic )               bENL = FALSE;   // StartSoar set via the command line
  if ( Engine.Detection == ED_NONE )  bENL = FALSE;
  ScaleENL = ( ymax - yoffset ) / RangeENL;   // Fixed scale 0 -> RangeENL

  bMOP = Trace.bMOP;
  if ( !Engine.Installed ) bMOP = FALSE;

  // Remove baro file if it exists already
  FILE *F = fopen ( file_baro, "rb" );
  if ( F )
  { fclose ( (void**)&F );
    remove ( file_baro );
  }
  return TRUE;
}




void CBaro::DrawFlights(int Color)
{ CFlight *pF;

  if ( debugPlot )
  { printf ( "    CBaro::DrawFlights\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 0;   i < nFlights;   i ++ )
  { pF = FlightA[i];
    gdImageFilledRectangle ( im, X ( pF, 0 ), yoffset, X ( pF, pF->npt-1 ), ymax, Color );
  }
}




void CBaro::DrawSoaring(int Color)
{ CSoarBlock *pSB;
  CFlight *pF;

  if ( debugPlot )
  { printf ( "    CBaro::DrawSoaring\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 0;   i < nFlights;   i ++ )
  { pF = FlightA[i];
    for ( int j = 0;   j < pF->nSB;   j ++ )
    { pSB = pF->SB[j];
      gdImageFilledRectangle ( im, X ( pSB, 0 ), yoffset, X ( pSB, pSB->npt-1 ), ymax, Color );
    }
  }
}




void CBaro::DrawEngine(int Color)
{ if ( !bENL ) return;
  int  i0, x0;
  int dy = 40;
  BOOL b = FALSE;

  if ( debugPlot )
  { printf ( "    CBaro::DrawEngine\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 0;   i < Trace.npt;   i ++ )
  { if ( !b )
    { if ( Trace.GetFlag ( i, FLT_ENG_ON ) )
      { i0 = i;
        x0 = X ( &Trace, i );
        b = TRUE;
      }
    }
    else
    { if ( !Trace.GetFlag ( i, FLT_ENG_ON ) )
      { b = FALSE;
        if ( Trace.dTime ( i0, i, 3600. ) >= EngineTimeThreshold )
          gdImageFilledRectangle ( im, x0, ymax, X ( &Trace, i ), ymax + dy, Color );
      }
    }
  }
  if ( b ) gdImageFilledRectangle ( im, x0, ymax, X ( &Trace, Trace.npt-1 ), ymax + dy, Color );
  WriteLegend ( "Engine", Color );
}




void CBaro::DrawENL(int Color)
{ if ( !bENL ) return;
  int    x1 = X ( &Trace, 0 );
  int    x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawENL\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yENL ( Trace.ptA[i].ENL );
    gdImageFilledRectangle ( im, x1, y, x, ymax, Color );
    x1 = x;
  }
  WriteLegend ( "ENL", Color );
}




void CBaro::DrawMOP(int Color)
{ if ( !bMOP ) return;
  int    x1 = X ( &Trace, 0 );
  int    x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawMOP\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yENL ( Trace.ptA[i].MOP );
    gdImageFilledRectangle ( im, x1, y, x, ymax, Color );
    x1 = x;
  }
  WriteLegend ( "MOP", Color );
}




void CBaro::DrawMOPline(int Color)
{ if ( !bMOP   ||   !bENL  ) return;
  int    x0 = X ( &Trace, 0 );
  int    y0 = yENL ( Trace.ptA[0].MOP );
  int    x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawMOPline\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yENL ( Trace.ptA[i].MOP );
    gdImageLine ( im, x0, y0, x, y, Color );
    x0 = x;
    y0 = y;
  }
}




void CBaro::DrawfENL(int Color)
{ if ( !bENL ) return;
  int    x0 = X ( &Trace, 0 );
  int    y0 = yENL ( Trace.ptA[0].fENL );
  int    x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawfENL\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yENL ( Trace.ptA[i].fENL );
    gdImageLine ( im, x0, y0, x, y, Color );
    x0 = x;
    y0 = y;
  }
  WriteLegend ( "ENL", Color );
}




void CBaro::DrawENLThreshold(int Color)
{ if ( !bENL ) return;
  int    y = yENL ( Engine.Threshold );

  if ( debugPlot )
  { printf ( "    CBaro::DrawENLThreshold\n" );
    if ( OutF )  fflush ( OutF );
  }

  gdImageLine ( im, 0, y, xmax, y, Color );
}




void CBaro::DrawRelease(int Color)
{ CFlight *pF;
  int x, y, yb;
  int d = 20;

  if ( debugPlot )
  { printf ( "    CBaro::DrawRelease\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 0;   i < nFlights;   i ++ )
  { pF = FlightA[i];
    x = X ( pF, pF->Release );
    y = yAlt ( pF, pF->Release );
    yb = y;
    gdImageLine ( im, x-d, y, x+d, y, Color );
    if ( pF->dtBaseLine >= 1.   &&
         pF->nBaseLine  >= 6
       )
    { yb = Y ( pF->AltBaseLine );
      gdImageLine ( im, x-d, yb, x+d, yb, Color );
    }
    gdImageLine ( im, x, y-d, x, yb+d, Color );
  }
  WriteLegend ( "Release", Color );
}



void CBaro::DrawGPS(int Color)
{ int x0 = X ( &Trace, 0 );
  int y0 = yGPS ( &Trace, 0 );
  int x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawGPS\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yGPS ( &Trace, i );
    gdImageLine ( im, x0, y0, x, y, Color );
    x0 = x;
    y0 = y;
  }
  WriteLegend ( "GPS", Color );
}



void CBaro::DrawAlt(int Color)
{ int x0 = X ( &Trace, 0 );
  int y0 = yAlt ( &Trace, 0 );
  int x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawAlt\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;   i < Trace.npt;   i ++ )
  { x = X ( &Trace, i );
    y = yAlt ( &Trace, i );
    gdImageLine ( im, x0, y0, x, y, Color );
    x0 = x;
    y0 = y;
  }
  WriteLegend ( "BARO", Color );
}






void CBaro::DrawSpeed(int Color)
{ if ( !sbMaxFF )  return;
  if ( !sbMaxFF->FT.i2 )    return;
  int x0 = X ( sbMaxFF, sbMaxFF->FT.i0 );
  int y0 = yAlt ( sbMaxFF, sbMaxFF->FT.i0 );
  int x, y;

  if ( debugPlot )
  { printf ( "    CBaro::DrawSpeed\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = sbMaxFF->FT.i0+1;   i < sbMaxFF->FT.i2;   i ++ )
  { x = X ( sbMaxFF, i );
    y = yAlt ( sbMaxFF, i );
    gdImageLine ( im, x0, y0, x, y, Color );
    x0 = x;
    y0 = y;
  }
  WriteLegend ( "Speed", Color );
}






void CBaro::DrawAltGrid(int Color)
{ char st[16];
  int    i, y;
  int    n;      // Hard line spacing
  double a, da;

  if ( debugPlot )
  { printf ( "    CBaro::DrawAltGrid\n" );
    if ( OutF )  fflush ( OutF );
  }

  gdImageSetStyle ( im, styleDashed, 6 );
  if ( alti_max < 1200 )
  { da = 100.;
    n  = 5;     // -> hard line every 500 m
  }
  else if ( alti_max < 2700 )
  { da = 250;
    n = 2;      // -> hard line every 500 m
  }
  else
  { da = 500;
    n = 2;      // -> hard line every 1000 m
  }
  for ( a = 0.,  i = 0;   a < alti_max + 500.;   a += da,  i ++ )
  { y = Y ( a );
    if ( i % n )
    { // soft line
      gdImageLine ( im, xoffset, y, xmax, y, gdStyled );
    }
    else
    { // Hard line
      gdImageLine ( im, xoffset, y, xmax, y, Color);
      if ( i )
      { // draw label
        sprintf ( st, "%.0f", a );
	      gdImageString ( im, Font, 3, y+0, (unsigned char *)st, Color );
      }
    }
  }
}




void CBaro::DrawTimeGrid(int Color)
{ gdImageLine ( im, xoffset, ymax, xmax, ymax, Color );
  char st[16];
  int    i, x, dx;
  int    n;      // Hard line spacing
  double t, t0, t2, dt;

  if ( debugPlot )
  { printf ( "    CBaro::DrawTimeGrid\n" );
    if ( OutF )  fflush ( OutF );
  }

  t = Trace.dTime ( 0, Trace.npt - 1, 60. ) / Scale;

  int    dash[7] =
  { gdTransparent, gdTransparent,
    gdTransparent, gdTransparent, gdTransparent,
    Color, Color,
  };
  gdImageSetStyle ( im, dash, 6 );
  if ( Scale != 1 )
  { t /= Scale;
    t *= 0.75;
  }
  if ( t < 30. )
  { dt = 1.;
    n  = 10;     // -> hard line every 10'
  }
  else if ( t < 90. )
  { dt = 5.;
    n  = 3;     // -> hard line every 15'
  }
  else if ( t < 160. )
  { dt = 10.;
    n = 3;      // -> hard line every 30'
  }
  else if ( t < 220. )
  { dt = 20.;
    n = 3;      // -> hard line every hr
  }
  else
  { dt = 30.;
    n = 2;      // -> hard line every hr
  }
  t2 = dt * n;
  t0 = floor ( Trace.ptA[0].Time * 60. / t2 ) * t2 / 60.;
  t2 = Trace.ptA[Trace.npt-1].Time;
  dt /= 60.;
  for ( t = t0,  i = 0;   t <= t2;   t += dt,  i ++ )
  { x = X ( t );
    if ( i % n )
    { // soft line
      gdImageSetStyle ( im, dash, 6 );
      gdImageLine ( im, x, ymax, x, yoffset, gdStyled );
      gdImageLine ( im, x, ymax, x, ymax+7, Color );
    }
    else
    { // Hard line
      gdImageSetStyle ( im, dash, 7 );
      gdImageLine ( im, x, ymax, x, yoffset, gdStyled );
      gdImageLine ( im, x, ymax, x, ymax + 11, Color);
      if ( i )
      { // draw label
        sprintf ( st, "%s", stHour(t) );
        dx = (int)strlen(st) * Font->w / 2;
        gdImageString ( im, Font, x-dx, ymax+12, (unsigned char *)st, Color );
      }
    }
  }
}




void CBaro::WriteTitle(int Color)
{ char st[256];

  if ( debugPlot )
  { printf ( "    CBaro::WriteTitle\n" );
    if ( OutF )  fflush ( OutF );
  }

  sprintf ( st, "%02d-%02d-%02d  ::  %s  ::  %s (%s)",
                      Trace.Day, Trace.Month, Trace.Year, Trace.Pilot, Trace.GliderType, Trace.GliderReg) ;
  gdImageString ( im, Font, 10,0, (unsigned char *)st, Color );
}



void CBaro::DrawAltGain(int Color)
{ if ( !sbMaxAG )  return;
  int x  = X ( sbMaxAG, sbMaxAG->iLow );
  int y  = yAlt ( sbMaxAG, sbMaxAG->iLow );
  int dx = 20;

  if ( debugPlot )
  { printf ( "    CBaro::DrawAltGain\n" );
    if ( OutF )  fflush ( OutF );
  }

  gdImageLine ( im, x - dx, y, x + dx, y, Color );

  x = X ( sbMaxAG, sbMaxAG->iHigh );
  y = yAlt ( sbMaxAG, sbMaxAG->iHigh );
  gdImageLine ( im, x - dx, y, x + dx, y, Color );
  WriteLegend ( "HW", Color );
}




void CBaro::DrawFF(int Color)
{ if ( !sbMaxFF )  return;
  int    dash[8] =
  { Color, Color, Color, Color,
    gdTransparent, gdTransparent, gdTransparent, gdTransparent,
/*
    Color, Color, Color, Color,
    Color, Color, Color, Color,
    Color, Color, Color, Color,
    gdTransparent, gdTransparent, gdTransparent, gdTransparent,
    gdTransparent, gdTransparent, gdTransparent, gdTransparent,
    gdTransparent, gdTransparent, gdTransparent, gdTransparent,
*/
  };

  if ( debugPlot )
  { printf ( "    CBaro::DrawFF\n" );
    if ( OutF )  fflush ( OutF );
  }

  gdImageSetStyle ( im, dash, 8 );  // Only 8 gives a correct result for tchick lines
  int i, x;
  int y1 = yoffset;
  int y2 = ymax;
  for ( i = 0;  i < sbMaxFF->FF.npt;  i ++ )
  { x = X ( sbMaxFF, sbMaxFF->FF.TptA[i].i );
//    y = yAlt ( sbMaxFF, sbMaxFF->FF.TptA[i].i );
    gdImageLine ( im, x-1, y1, x-1, y2, gdStyled );
    gdImageLine ( im, x,   y1, x,   y2, gdStyled );
    gdImageLine ( im, x+1, y1, x+1, y2, gdStyled );
  }
  WriteLegend ( "VV", Color );
}




void CBaro::DrawDT(int Color)
{ if ( !sbMaxDT )  return;
  int    dash[8] =
  { Color, Color, Color, Color,
    gdTransparent, gdTransparent, gdTransparent, gdTransparent,
  };

  if ( debugPlot )
  { printf ( "    CBaro::DrawDT\n" );
    if ( OutF )  fflush ( OutF );
  }

  gdImageSetStyle ( im, dash, 8 );
//  gdImageSetThickness ( im, 1 );
  int i, x;
  int y1 = yoffset;
  int y2 = ymax;
  for ( i = 1;  i < Task.nTPT-1;  i ++ )
  { x = X ( sbMaxDT, sbMaxDT->DT.Leg[i].Index );
    gdImageLine ( im, x-1, y1, x-1, y2, gdStyled );
    gdImageLine ( im, x,   y1, x,   y2, gdStyled );
    gdImageLine ( im, x+1, y1, x+1, y2, gdStyled );
  }
  WriteLegend ( "OP", Color );
}


void CBaro::WriteLegend(const char *st, int Color)
{ int dx = (int)strlen(st) * Font->w;

  if ( debugPlot )
  { printf ( "    CBaro::WriteLegend\n" );
    if ( OutF )  fflush ( OutF );
  }

  xLegend  -= (int)strlen(st) * Font->w + 18;
  gdImageString ( im, Font, xLegend, 0, (unsigned char *)st, Color );
}



int CBaro::X(CFlightBase *pB, int i)
{ return IROUND ( ( pB->ptA[i].Time - time_first ) * time_scale );
}


int CBaro::X(double t)
{ return IROUND ( ( t - time_first ) * time_scale );
}


int CBaro::Y(double a)
{ a = max ( a, alti_min );
  a = min ( a, alti_max );
  return yoffset + IROUND ( ( alti_max - a ) * alti_scale );
}


int CBaro::yAlt(CFlightBase *pB, int i)
{ return Y ( pB->ptA[i].Altitude );
}


int CBaro::yGPS(CFlightBase *pB, int i)
{ return Y ( pB->ptA[i].AltiGPS );
}



int CBaro::yENL(double ENL)
{ return yoffset + IROUND ( ( RangeENL - ENL ) * ScaleENL );
}
