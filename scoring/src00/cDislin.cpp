#include "Global.h"
#include "dislin.h"
#include "cDislin.h"

#include "Airfields.h"

#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
//https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#endif

CDislin::CDislin(char *fn)
{ nAirf_maxplot = 20;
  Scale = 1;     // DIN A4
  //Scale = 2;  //  DIN A2
  //Scale = 4;  //  DIN A0
  tlMap.x =  210 * Scale;
  tlMap.y = 1930 * Scale;
  sMap.cx = 2470 * Scale;
  sMap.cy = 1800 * Scale;

  nAirfSel = 0;
  LatMin   = 1000.;   LatMax = -1000.;
  LonMin   = 1000.;   LonMax = -1000.;
  Lat      = NULL;
  Lon      = NULL;
  npt      = 0;
  nptR     = 0;
  strcpy ( FN, fn );
}



CDislin::~CDislin(void)
{ free ( (void**)&Lat );
  free ( (void**)&Lon );
  disfin();
}




void CDislin::StartPlot(void)
{ // Remove plot file if it exists already
  FILE *F = fopen ( FN, "rb" );
  if ( F )
  { fclose ( (void**)&F );
    remove ( FN );
    F = 0;
  }

  if ( debugDislin )
  { printf ( "CDislin::StartPlot %s\n", base_name );
  }

  metafl ( "PNG" );          // level 0
  setfil ( FN );             // level 0
  pagmod ( "NONE" );         // Automatic orientation ( ->landscape) level 0
//  sclfac ( 4. );
  switch ( Scale )
  { case 1:  break;                        // Din A4 (default)
    case 2:  setpag ( "DA2L" );   break;   // Din A0
    case 4:  setpag ( "DA0L" );   break;   // Din A0
    default: report_error ( EC_ARG_OUT_RANGE, " CDislin Invalid Scale, must be 1, 2 or 4" );
  }
  disini();
  axslen ( sMap.cx, sMap.cy ); 
  axspos ( tlMap.x, tlMap.y );
  projct ( "coni" );         // Projection type
  pagfll ( 255 );            // Page background color
  setvlt ( "rain");          // Select color table
  // Modify some palette values
  setind ( 1, 0.25, 0.25, 0.25 );
  setind ( 2, 0.37, 0.37, 0.37 );
  setind ( 3, 0.5,  0.5,  0.5  );
  setind ( 4, 0.62, 0.62, 0.62 );
  color  ( "black" );
  linwid ( 1 );              // Line width
  SetFont();

/*
  int rx, ry;
  int nx, ny;
  getres (&rx, &ry );
  getscr (&nx, &ny );
  int b = getbpp();
*/
}




void CDislin::GetArea(void) {}



void CDislin::GetPlotArea(void)
{
  float  LatMid,  LonMid;
  float  LonWidth;
  float  LatLower;
  float  ratio;
  float  cos_Phi;

  // Determine size of plot area
  ViewLatMin = LatMin - 0.05F * ( LatMax - LatMin );
  ViewLatMax = LatMax + 0.05F * ( LatMax - LatMin );
  ViewLonMin = LonMin - 0.05F * ( LonMax - LonMin );
  ViewLonMax = LonMax + 0.05F * ( LonMax - LonMin );
  LatMid = 0.5F * ( LatMin + LatMax );
  LonMid = 0.5F * ( LonMin + LonMax );
  
  if ( fabs(LatMin) < fabs(LatMax)) LatLower = LatMin;
  else                              LatLower = LatMax;
  cos_Phi = (float) cos ( LatLower * piDiv180 );

  ratio = (float)sMap.cy / (float)sMap.cx;    // Height / width ratio of viewport
  
  LonWidth  = ViewLonMax - ViewLonMin ;
  LatWidth  = ViewLatMax - ViewLatMin ;
  if ( LatWidth < LonWidth * cos_Phi * ratio )  LatWidth = LonWidth * cos_Phi * ratio;
  else                                          LonWidth = LatWidth / ( cos_Phi * ratio );

  ViewLatMin = LatMid - 0.5F * LatWidth;
  ViewLatMax = LatMid + 0.5F * LatWidth;
  ViewLonMin = LonMid - 0.5F * LonWidth;
  ViewLonMax = LonMid + 0.5F * LonWidth;
  
  PlotLatMin = LatMid - 0.49F * LatWidth;
  PlotLatMax = LatMid + 0.49F * LatWidth;
  PlotLonMin = LonMid - 0.49F * LonWidth;
  PlotLonMax = LonMid + 0.49F * LonWidth;

  if ( (LatWidth > 5.) && (LonWidth > 5.) )
  { LatStep = 5.;  LonStep = 5.;
    ticks ( 5, "XY" );
  }
  else if ( (LatWidth > 2.) && (LonWidth > 2.) )
  { LatStep = 2.;  LonStep = 2.;
    ticks ( 4, "xy" );
  }
  else
  { LatStep = 1.;  LonStep = 1.;
    ticks ( 6, "xy" );
  }
}





void CDislin::DrawFrame(char *Title)
{ // Plot the axis and labels
  color  ( "black" );
  pagera ();                 // Plot a border around the page
  disalf ();                 // Default font
  intax  ();                 // Label axis with integer
  labels ( "map", "xy" );    // Determine which label types will be plotted on an axis
  ticpos ( "revers","xy" );  // Defines the position of ticks
  ticlen ( 28, 14 );         // Set the length of major and minor ticks
  height ( 28 * Scale ) ;
//ViewLonMin = 8.5;
//ViewLonMax = 10.;
  if ( debugDislin )
  { printf ( "    CDislin::DrawFrame\n" );
    printf ( "    grafmp lon %0.4f, %0.4f / %0.4f, %0.4f\n", ViewLonMin, ViewLonMax, floor(ViewLonMin / LonStep) * LonStep, LonStep );
    printf ( "    grafmp lat %0.4f, %0.4f / %0.4f, %0.4f\n", ViewLatMin, ViewLatMax, floor(ViewLatMin / LatStep) * LatStep, LatStep );
  }

  grafmp ( ViewLonMin, ViewLonMax, floor(ViewLonMin / LonStep) * LonStep, LonStep, 
           ViewLatMin, ViewLatMax, floor(ViewLatMin / LatStep) * LatStep, LatStep );
  if ( debugDislin && OutF )  fflush ( OutF );
/*
TestLon(7.);
TestLon(8);
TestLon(9);
TestLon(10.39);
TestLon(10.);
*/
  // Plot the creation date and dislin version
  height ( 38 * Scale ) ;
  paghdr ( "CHARRON.LINE", "", 2,0 );
  SetFont();

  // Plot the title
  int x0, y0, sx, sy;
  getclp ( &x0, &y0, &sx, &sy );
  height ( 44 * Scale ) ;
  txtjus ( "cent" );
  messag ( Title, sx / 2, 44 * Scale );
  txtjus ( "left" );
  clpbor ( "axis" );         // To draw outside the border, call clpbor ( "page" )
}



void CDislin::TestLon(float lon)
{
  float Lat[2];
  float Lon[2];

  Lat[0] = 53.;
  Lat[1] = 52.;
  Lon[0] = Lon[1] = lon;

  setclr ( 1 );
  linwid ( 3 );
  curvmp ( Lon, Lat, 2 ) ;
}





void CDislin::SelectAirfields(void)
{ // Select airfields within map plot area and of level of importance sufficiently
  // high to limit nbr. of airfields in plot
  // Doesn't actually plot
  int i;
  int min_level = 0;
  double mLat = PlotLatMin * piDiv180;
  double mLon = PlotLonMin * piDiv180;
  double MLat = PlotLatMax * piDiv180;
  double MLon = PlotLonMax * piDiv180;
  do
  { Airfields.ResetSel();
    min_level++;
    nAirfSel = 0;
    
    for (  i= 0;   i < Airfields.nList;  i ++ )
    { if ( ( Airfields.List[i].pt.Lat > mLat ) && ( Airfields.List[i].pt.Lat < MLat )   &&
           ( Airfields.List[i].pt.Lon > mLon ) && ( Airfields.List[i].pt.Lon < MLon )   &&
           ( Airfields.List[i].level >= min_level )
         )
      { Airfields.List[i].sel = 1;
        nAirfSel++;
      }
    }
  }
  while ( ( nAirfSel > nAirf_maxplot )  &&  ( min_level < 5 ) );
}





void CDislin::DrawAirfields(void)
{ // Plot selected airfields 
  float Lat, Lon;
  int xp, yp;

  if ( debugDislin )
  { printf ( "    CDislin::DrawAirfields\n" );
    if ( OutF )  fflush ( OutF );
  }

  color  ( "black" );
  simplx ();
  height ( 28 * Scale );
  hsymbl ( 20 );
  
  for ( int i = 0;   i < Airfields.nList;   i ++ )
  { if ( Airfields.List[i].sel )
    { Lat = dec ( Airfields.List[i].pt.Lat );
      Lon = dec ( Airfields.List[i].pt.Lon );
      xp = IROUND ( x2dpos ( Lon, Lat ) );
      yp = IROUND ( y2dpos ( Lon, Lat ) );
      if ( xp > 0   &&   yp > 0 )  symbol ( 15, xp, yp );
      messag ( Airfields.List[i].name, xp + 30, yp );
    }
  } 
} 





void CDislin::DrawCountries(int c)
{ // Plot country boundaries: only for flights in Europe
//printf ( "DrawCountries, entry\n" );   // debugging: world() crashes on linux !!

  if ( debugDislin )
  { printf ( "    CDislin::DrawCountries\n" );
    if ( OutF )  fflush ( OutF );
  }

  mapbas ( "gshh" );         // This will cause a segmentation fault if no gshh file is present !!
  setclr ( c );
  if ( nAirfSel > 0 )        // If airfields selected, we are in Europe
  { int   nCountries    = 32;
    int  *Countries     = (int  *) calloc ( nCountries, sizeof(int)  );
    long *CountryShades = (long *) calloc ( nCountries, sizeof(long) );
    int  *CountryColors = (int  *) calloc ( nCountries, sizeof(int)  );

    if ( !Countries     )  report_error ( EC_MEMORY, "DrawCountries, Countries" );
    if ( !CountryShades )  report_error ( EC_MEMORY, "DrawCountries, CountryShades" );
    if ( !CountryColors )  report_error ( EC_MEMORY, "DrawCountries, CountryColors" );

    for ( int i = 0;  i < nCountries;  i++ )
    { Countries[i]     = i+1;
      CountryShades[i] = 0;
      CountryColors[i] = c;
    }

    shdeur ( Countries, CountryShades, CountryColors, nCountries );
    world();

    free ( (void**)&Countries );
    free ( (void**)&CountryShades );
    free ( (void**)&CountryColors );
  }
  else world();
//printf ( "DrawCountries, exit\n" );
  if ( debugDislin )
  { printf ( "    CDislin::DrawCountries done\n" );
    if ( OutF )  fflush ( OutF );
  }
}




void CDislin::PlotScale(void)
{ char st[256];
  double ScaleDist;
  double ydist = ( LatWidth * piDiv180 ) * rEarth;

  if ( debugDislin )
  { printf ( "    CDislin::PlotScale\n" );
    if ( OutF )  fflush ( OutF );
  }

  if      ( ydist > 2500. ) ScaleDist = 500.;
  else if ( ydist >  500. ) ScaleDist = 100.;
  else if ( ydist >  250. ) ScaleDist =  50.;
  else if ( ydist >   50. ) ScaleDist =  10.;
  else if ( ydist >   25. ) ScaleDist =   5.;
  else                      ScaleDist =   1.;

  double ScaleLat = (float) ( ( ScaleDist / rEarth ) * c180divPi );
  int    ScaleLen = (int) floor ( ScaleLat / LatWidth * sMap.cy );
  
  sprintf ( st, "%.0f km", ScaleDist );
    
//  plot_scale ( 430, 1770, (float)ScaleLen, st );
  
  int x0 = tlMap.x + 50;
  int y0 = tlMap.y - 70;
  int x1 = x0 + ScaleLen;
  
  color ( "black" );
  line ( x0, y0,    x1, y0    );
  line ( x0, y0-10, x0, y0+10 );
  line ( x1, y0-10, x1, y0+10 );
     
  disalf();
  height(36);
  messag ( st, x1+20, y0-18 );
  SetFont();
  if ( debugDislin )
  { printf ( "    CDislin::PlotScale done\n" );
    if ( OutF )  fflush ( OutF );
  }
}





void CDislin::SetFont(void)
{ // Set the default font
  // Postscript fonts don't work on png
  //disalf ();                 // Default font
  complx ();                 // Complex font
  //psfont ( "Helvetica" );  // Doesn't work with png files
}




void CDislin::Symbol(int n, int h, float Lat, float Lon)
{ int xp = IROUND ( x2dpos ( Lon, Lat ) );
  int yp = IROUND ( y2dpos ( Lon, Lat ) );
  hsymbl ( h );
  if ( xp > 0   &&   yp > 0 )  symbol ( n, xp, yp );
}



void CDislin::AllocLatLon(int n)
{ if ( n > nptR )
  { int GrowBy = n - nptR;
    int nptR0;
    nptR0 = nptR;    realloc_s ( (void **)&Lat, sizeof(float), &nptR, GrowBy );
    nptR = nptR0;    realloc_s ( (void **)&Lon, sizeof(float), &nptR, GrowBy );
  }
  npt = n;
}



void CDislin::AllocLatLon(sFlightPoint *ptA, int n)
{ AllocLatLon ( n );
  for ( int i = 0;   i < n;   i ++,  ptA ++ )
  { Lat[i] = dec ( ptA->pt.Lat );
    Lon[i] = dec ( ptA->pt.Lon );
  }
}





void CDislin::DrawPalette(void)
{ // Debugging only: draw the currently selected palette
  char  st[8];
  int   nCol = 256;
  int   nx = 18;
  int   ny = ( nCol + nx  - 1 ) / nx;
  int   ix, iy, x, y, x0, y0;
  int   sx, sy;
  int   nc = 0;
  float r, g, b, c;
  clpbor ( "page" );
  getclp ( &ix, &iy, &sx, &sy );
  shdpat ( 16 );
  txtjus ( "cent" );
  height ( 30 );
  disalf ();

  double dx = (double)sx / nx;
  double dy = (double)sy / ny;

  y0 = 0;
  for ( iy = 0;   iy < ny;   iy ++ )
  { y = IROUND ( ( iy + 1 ) * dy );
    x0 = 0;
    for ( ix = 0;   ix < nx;   ix ++,  nc ++ )
    { if ( nc >= nCol ) break;
      x = IROUND ( ( ix + 1 ) * dx );
      setclr ( nc );
      rectan ( x0, y0, x - x0, y - y0 );
      getrgb ( &r, &g, &b );
      c = ( r + g + b ) / 3.F;
      if ( c < 0.5 ) color ( "white" );
      else           color ( "black" );
      sprintf ( st, "%d", nc );
      messag ( st, ( x + x0 ) / 2, ( y + y0 ) / 2 );
      x0 = x + 1;
    }
    y0 = y + 1;
  }
}




void CDislin::DrawSymbols(void)
{ char st[8];
  int   ix, iy, x, y;
  int   sx, sy;
  int   n = 0;
  int   dx = 400;
  int   dy = 400;
  clpbor ( "page" );
  getclp ( &ix, &iy, &sx, &sy );
  color  ( "black" );
  hsymbl ( 60 );
  txtjus ( "left" );
  height ( 44 );
  y = 100;
  for ( iy = 0;   iy < 4;  iy ++,  y += dy )
  { x = 100;
    for ( ix = 0;   ix < 6;  ix ++,  x += dy,  n ++ )
    { sprintf ( st, "%d", n );
      symbol ( n, x, y );
      messag ( st, x + 60, y - 30 );
    }
  }
}




void CDislin::DebugWorld(void)
{ // Obsolete function to debug the segmentation fault when drawing world
  // -> was due to the missing file gshh
  ViewLonMin =  4.;
  ViewLatMin = 50.;
  ViewLonMax = ViewLonMin + 2.F;
  ViewLatMax = ViewLatMin + 2.F;
  LonStep = LatStep = 1.;

  color  ( "black" );
  disalf ();                 // Default font
  height ( 28 * Scale ) ;
  grafmp ( ViewLonMin, ViewLonMax, ViewLonMin, LonStep, 
           ViewLatMin, ViewLatMax, ViewLatMin, LatStep );
  nAirfSel = 1;
  DrawCountries ( 11 );
}



