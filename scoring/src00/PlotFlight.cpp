#include "Global.h"
#include "dislin.h"
#include "PlotFlight.h"

#include "Airfields.h"
#include "Brussel.h"
#include "Triangle.h"

#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
//https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#endif


void PlotFlight(void)
{ 
  CFlight *pF = 0;
  if      ( sbMaxT  )   pF = sbMaxT->pOwner;
  else if ( FlightA )   pF = FlightA[0];
  if ( !pF ) return;

  CPlotFlight P ( pF, file_plot );
}




CPlotFlight::CPlotFlight(CFlight *pf, char *fn) : CDislin ( fn )
{ pF = pf;

  if ( debugPlot )
  { printf ( "\b>>>>Drawing map\n" );
    if ( OutF )  fflush ( OutF );
  }

  StartPlot();          // Must be called befoare GetPlotArea
//DebugWorld();   return;   // Debugging: trace cause of world segmentation fault
//DrawPalette();  return;   // Debugging: display palette
//DrawSymbols();  return;   // Debugging: display all available symbols
  Init();
  GetArea();
  GetPlotArea();
  SelectAirfields();    // Must be done before DrawCountries
  DrawFrame();
  DrawCountries ( 4 );
  PlotScale();
  DrawDT();
  DrawFF();
  DrawTRI();
  if ( debugFF )
  { DrawSoarBlock ( sbMaxFF );
    DrawKnicks();
  }
  else DrawFlight();
  if ( debugTRI )
  { //DrawKnicksTRI();
    DrawTRIbf();
  }
  DrawBRU();
  DrawAirfields();
}



CPlotFlight::~CPlotFlight(void)
{
}




void CPlotFlight::Init(void)
{ // Must be called after StartPlot
  bDrawTask  = FALSE;    // Will be set TRUE in GetAera if task is in map area
  yLegend    = tlMap.y - sMap.cy;
  // Indexes in color palette table
  // Palette is selected via setvlt in StartPlot
  // See file Rain.png or plot palette via DrawPalette()
//  setind ( 1, 220, 220, 220 );
  setind ( 1, 0.6, 0.6, 0.65 );
  cFlight    =  11;
  cRelease   =  1;//75;
  cEngine    = 223;
  cFF        = 249;
  cFFpt      = 245;
  setind ( 205, 1., 0.3, 1. );
  cTRI       = 205;   // TODO
  cFT        =  87;   // TODO
  cDT        = 119;
  cBru       = 209;
}


void CPlotFlight::GetArea(void)
{ int i;
  if ( GetAreaManual() ) return;

  // Get the flight area
  sFlightPoint *p = pF->ptA;
  int npt = pF->npt;
  for ( i = 0;   i < npt;   i++,  p ++ )
  { LatMin = min ( (float)p->pt.Lat, LatMin );
    LatMax = max ( (float)p->pt.Lat, LatMax );
    LonMin = min ( (float)p->pt.Lon, LonMin );
    LonMax = max ( (float)p->pt.Lon, LonMax );
  }

  // Merge the task area
  if ( Task.Valid  &&  sbMaxDT->DT.TotDist >= 30. )
  { float Latm =  10000.;
    float LatM = -10000.;
    float Lonm =  10000.;
    float LonM = -10000.;
  
    for ( i = 1; i <= Task.nPoints;  i ++ )
    { if ( sbMaxDT   &&   sbMaxDT->DT.ScoreDist > 0. )
      { Latm = min ( (float)Task.TPT[i].pt.Lat, Latm );
        LatM = max ( (float)Task.TPT[i].pt.Lat, LatM );
        Lonm = min ( (float)Task.TPT[i].pt.Lon, Lonm );
        LonM = max ( (float)Task.TPT[i].pt.Lon, LonM );
      }
    }
    // Check if the task isn't outside the flight area
    if ( LatM < LatMin   ||
         Latm > LatMax   ||
         LonM < LonMin   ||
         Lonm > LonMax
       )
    { // Outside, don't draw the task
    }
    else
    { // Inside: include the task area in the graph
      LatMin = min ( LatMin, Latm );
      LatMax = max ( LatMax, LatM );
      LonMin = min ( LonMin, Lonm );
      LonMax = max ( LonMax, LonM );
      bDrawTask = TRUE;  // Draw the task
    }
  }
  LatMin = dec ( LatMin );
  LatMax = dec ( LatMax );
  LonMin = dec ( LonMin );
  LonMax = dec ( LonMax );
}





int CPlotFlight::GetAreaManual(void)
{ if ( LatMaxPlot == 999.   &&
       LatMinPlot == 999.   &&
       LonMaxPlot == 999.   &&
       LonMinPlot == 999.
     )
    return 0;
  if ( LatMaxPlot == 999.   ||
       LatMinPlot == 999.   ||
       LonMaxPlot == 999.   ||
       LonMinPlot == 999.
       )
  { // Not all boundaries specified
    report_error ( EC_CL_PL_AREA, NULL ); 
    return 0;
  }
  if ( LatMaxPlot < LatMinPlot + 0.5   ||
       LonMaxPlot < LonMinPlot + 0.5
     )
  { 
    report_error ( EC_CL_PL_AREA_ERROR, NULL ); 
    return 0;
  }
  LatMax = LatMaxPlot;
  LatMin = LatMinPlot;
  LonMax = LonMaxPlot;
  LonMin = LonMinPlot;
  return 1;
}





void CPlotFlight::DrawFrame(void)
{ char st[256];

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawFrame\n" );
    if ( OutF )  fflush ( OutF );
  }

  sprintf ( st, "%02d-%02d-%02d   %s   %s (%s)", Trace.Day, Trace.Month, Trace.Year, Trace.Pilot, Trace.GliderType, Trace.GliderReg) ;
  CDislin::DrawFrame ( st );

  double d = 0.;
  if ( sbMaxFF ) d = sbMaxFF->FF.ScoreDist;
  sprintf ( st, "Vrije Vlucht: %6.1f km", d );
  clpbor ( "page" );
  height ( 42 );
  messag ( st, tlMap.x, 2025*Scale );
  clpbor ( "axis" );
  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawFrame done\n" );
    if ( OutF )  fflush ( OutF );
  }
}




void CPlotFlight::Legend(char * st, int c)
{ clpbor ( "page" );
  txtjus ( "left" );
  setclr ( c );
  linwid ( 2 );
  height ( 36 * Scale ) ;
  messag ( st, tlMap.x + sMap.cx + 30 * Scale, yLegend * Scale );
  linwid ( 1 );
  clpbor ( "axis" );
  yLegend += 80;
}



void CPlotFlight::SelectAirfields(void)
{ CDislin::SelectAirfields();
  int i, j, n;
  double mLat = PlotLatMin * piDiv180;
  double mLon = PlotLonMin * piDiv180;
  double MLat = PlotLatMax * piDiv180;
  double MLon = PlotLonMax * piDiv180;

  if ( debugPlot )
  { printf ( "    CPlotFlight::SelectAirfields\n" );
    if ( OutF )  fflush ( OutF );
  }

  // Select always the airfield at (i.e. near) the points of take-off, landing, declared task and the turn points.
  // Select airfield within maxdist_airf km of take-off point
  for ( i = 0;  i < Airfields.nList;  i ++ )
  { if ( Airfields.List[i].sel )  continue;
    if ( ( OrthoDist ( &pF->ptA[0].pt, &Airfields.List[i].pt ) <= maxdist_airf )      &&
         ( Airfields.List[i].pt.Lat > mLat ) && ( Airfields.List[i].pt.Lat < MLat )   &&
         ( Airfields.List[i].pt.Lon > mLon ) && ( Airfields.List[i].pt.Lon < MLon )
       )
      Airfields.List[i].sel = 1;
  }
    
  // Select airfield within maxdist_airf km of landing point
  for ( i = 0;   i < Airfields.nList;  i ++ )
  { if ( Airfields.List[i].sel )  continue;
    if ( ( OrthoDist ( &pF->ptA[pF->npt-1].pt, &Airfields.List[i].pt ) <= maxdist_airf )          &&
         ( Airfields.List[i].pt.Lat > mLat ) && ( Airfields.List[i].pt.Lat < MLat ) &&
         ( Airfields.List[i].pt.Lon > mLon ) && ( Airfields.List[i].pt.Lon < MLon )
       )
      Airfields.List[i].sel = 1;
  }
        
  // Select airfields within maxdist_turnpt km of declared task points
  if ( bDrawTask )
  { for ( i = 0;  i < Airfields.nList;  i ++ )
    { for ( j = 1;   j <= Task.nPoints;  j ++ )
      { if ( Airfields.List[i].sel )  continue;
        if ( ( OrthoDist ( &Task.TPT[j].pt, &Airfields.List[i].pt ) <= maxdist_turnpt )                &&
             ( Airfields.List[i].pt.Lat > mLat ) && ( Airfields.List[i].pt.Lat < MLat )  && 
             ( Airfields.List[i].pt.Lon > mLon ) && ( Airfields.List[i].pt.Lon < MLon )
           )
          Airfields.List[i].sel = 1;
      }
    }
  }

  // Select airfields within maxdist_turnpt km of optimized turn points
  if ( sbMaxFF )
  { for ( i = 0;  i < Airfields.nList;  i ++ )
    { for ( j = 0;   j < sbMaxFF->FF.npt;   j ++ )
      { if ( Airfields.List[i].sel )  continue;
        n = sbMaxFF->FF.TptA[j].i;
        if ( ( OrthoDist ( &sbMaxFF->ptA[n].pt, &Airfields.List[i].pt ) <= maxdist_turnpt )       &&
             ( Airfields.List[i].pt.Lat > mLat ) && ( Airfields.List[i].pt.Lat < MLat ) &&
             ( Airfields.List[i].pt.Lon > mLon ) && ( Airfields.List[i].pt.Lon < MLon )
           )
          Airfields.List[i].sel = 1;
      }
    }
  }
}





void CPlotFlight::DrawFlight(void)
{ // Plot logger trace
  int i, i0, n;

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawFlight\n" );
    if ( OutF )  fflush ( OutF );
  }

  AllocLatLon ( pF->ptA, pF->npt );

  setclr ( cFlight );
  linwid ( 2 );
  curvmp ( Lon, Lat, pF->npt );    // Draw the whole flight

  if ( sbMaxFF   &&   sbMaxFF->FT.i2 )
  { //Draw the speed comp in another color
    setclr ( cFT );
    i0 = sbMaxFF->Start + sbMaxFF->FT.i0;
    n  = sbMaxFF->FT.i2 - sbMaxFF->FT.i0 + 1;
    linwid ( 2 );
    curvmp ( &Lon[i0], &Lat[i0], n );
    Legend ( "Speed", cFT );
  }

  // Draw portion before StartSoar in cyan
  if ( pF->StartSoar > 0 )
  { setclr ( cRelease );
    linwid ( 2 );
    curvmp ( Lon, Lat, pF->StartSoar );  
  }

  // Draw engine run sections on orange
  if ( pF->EngRuns > 0 )
  { setclr ( cEngine );
    for ( i = 0;  i < pF->EngRuns;  i ++ )
    { n = pF->EngA[i].Off - pF->EngA[i].On;
      if ( n > 1 )
      { linwid ( 2 );
        curvmp ( &Lon[pF->EngA[i].On], &Lat[pF->EngA[i].On], n );  
      }
    }
    Legend ( "Engine", cEngine );
  }
}





void CPlotFlight::DrawSoarBlock(CSoarBlock *pSB)
{ if ( !pSB ) return;
  AllocLatLon ( pSB->ptA, pSB->npt );

  setclr ( cFlight );
  linwid ( 2 );
  curvmp ( Lon, Lat, pSB->npt );    // Draw the whole soaring block
}





void CPlotFlight::DrawDT(void)
{ // Plot declared task
  if ( !bDrawTask )  return;
  if ( !sbMaxDT   )  return;
  if ( sbMaxFF )
  { if ( sbMaxDT->DT.TotDist < sbMaxFF->FF.Dist / 2.5 )  return;
  }
  else
  { if ( sbMaxDT->DT.TotDist < 30. )  return;
  }
  float Lat[nTaskArray];
  float Lon[nTaskArray];

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawDT\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 1;  i <= Task.nPoints;  i ++ )
  { Lat[i] = dec ( Task.TPT[i].pt.Lat );
    Lon[i] = dec ( Task.TPT[i].pt.Lon );
  }
  setclr  ( cDT );
  linwid ( 3 );
  curvmp ( &Lon[1], &Lat[1], Task.nPoints );
  Legend ( "OP", cDT );
  linwid ( 1 );
}





void CPlotFlight::DrawFF(void)
{ // Plot free flight

  if ( !sbMaxFF  ||   !sbMaxFF->FF.npt )  return;
  if ( sbMaxDT   &&   sbMaxFF->FF.Dist < sbMaxDT->DT.TotDist * 1.2 )  return;

  int i, n;
  float Lat[nTaskArray];
  float Lon[nTaskArray];

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawFF\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( i = 0;  i < sbMaxFF->FF.npt;  i ++ )
  { n = sbMaxFF->FF.TptA[i].i;
    Lat[i] = dec ( sbMaxFF->ptA[n].pt.Lat );
    Lon[i] = dec ( sbMaxFF->ptA[n].pt.Lon );
  }
  setclr ( cFF );
  linwid ( 3 );
  curvmp ( Lon, Lat, sbMaxFF->FF.npt ) ;

  // Plot the turn point symbols
  if (debugDislin)
  { printf ("    Plotting optimised turn points\n");
    if ( OutF )  fflush ( OutF );
  }

  setclr ( cFFpt );
  Symbol ( 2, 40, Lat[0], Lon[0] );
  Symbol ( 5, 40, Lat[sbMaxFF->FF.npt-1], Lon[sbMaxFF->FF.npt-1] );
  for ( i = 1;   i < ( sbMaxFF->FF.npt - 1 );  i ++ )
  { Symbol ( 15, 40, Lat[i], Lon[i] );
  }
  Legend ( "VV", cFF );
}




void CPlotFlight::DrawTRI(void)
{
  if ( !sbMaxFF  ||   TriM.TotDist < 100. )  return;
  if ( TriM.TotDist < sbMaxFF->FF.Dist / 3. )  return;
  float Lat[4];
  float Lon[4];

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawTRI\n" );
    if ( OutF )  fflush ( OutF );
  }

  for ( int i = 0;  i < 3;  i ++ )
  { Lat[i] = dec ( sbMaxFF->ptA[TriM.Index[i]].pt.Lat );
    Lon[i] = dec ( sbMaxFF->ptA[TriM.Index[i]].pt.Lon );
  }
  Lat[3] = Lat[0];
  Lon[3] = Lon[0];

  setclr  ( cTRI );
  linwid ( 3 );
  curvmp ( Lon, Lat, 4 );
  Legend ( "TRI", cTRI );
  linwid ( 1 );
}




void CPlotFlight::DrawTRIbf(void)
{
  if ( !sbMaxFF  ||   TriBF.TotDist < 100. )  return;
  if ( TriBF.TotDist < sbMaxFF->FF.Dist / 3. )  return;
  float Lat[4];
  float Lon[4];

  for ( int i = 0;  i < 3;  i ++ )
  { Lat[i] = dec ( sbMaxFF->ptA[TriBF.Index[i]].pt.Lat );
    Lon[i] = dec ( sbMaxFF->ptA[TriBF.Index[i]].pt.Lon );
  }
  Lat[3] = Lat[0];
  Lon[3] = Lon[0];

  setclr  ( cTRI + 1 );
  linwid ( 3 );
  curvmp ( Lon, Lat, 4 );
  Legend ( "TRI", cTRI + 1 );
  linwid ( 1 );
}





void CPlotFlight::DrawKnicks(void)
{ if ( !sbMaxFF  ||  !sbMaxFF->nKnicks )  return;
  setclr ( cFF );
  sCoord *pt;
  for ( int i = 0;   i < sbMaxFF->nKnicks;   i ++ )
  { pt = &sbMaxFF->ptA[sbMaxFF->Knicks[i]].pt;
    Symbol ( 4, 60, (float)( pt->Lat * c180divPi ), (float)( pt->Lon * c180divPi ) );
  }
}




void CPlotFlight::DrawKnicksTRI(void)
{ if ( TriM.TotDist < 60.  ||  !TriDebug.nKnicks )  return;
  setclr ( cFF );
  sCoord *pt;
  for ( int i = 0;   i < TriDebug.nKnicks;   i ++ )
  { pt = &sbMaxFF->ptA[TriDebug.Knicks[i]].pt;
    Symbol ( 4, 60, (float)( pt->Lat * c180divPi ), (float)( pt->Lon * c180divPi ) );
  }
}




void CPlotFlight::DrawBRU(void)
{ if ( !sbMaxFF )  return;
  if ( fabs ( sbMaxFF->BRU.Angle ) < 150. )  return;

  if ( debugPlot )
  { printf ( "    CPlotFlight::DrawBRU\n" );
    if ( OutF )  fflush ( OutF );
  }

  int    xa, ya, xb, yb, x0, y0, x1, y1, x2, y2, dx1, dy1, dx2, dy2;
  double d1, d2, dm, f;
  sCoord p;
  float  Lat, Lon;
  setclr ( cBru );
  linwid ( 2 );
  hsymbl ( 60 );
  sBrussel *B = &sbMaxFF->BRU;

  // The symbol at Brussels
  Lat = (float) ( CBrussel::ptCtr.Lat * c180divPi );
  Lon = (float) ( CBrussel::ptCtr.Lon * c180divPi );
  x0 = IROUND ( x2dpos ( Lon, Lat ) );
  y0 = IROUND ( y2dpos ( Lon, Lat ) );
  if ( x0 <= 0   ||   y0 <= 0 ) return;  // Don't draw if Bru not on map
  symbol ( 5, x0, y0 );

  // Calculate symbol & Line 1: first point -> BRU
  p = sbMaxFF->ptA[B->i1].pt;
  Lat = (float) ( p.Lat * c180divPi );
  Lon = (float) ( p.Lon * c180divPi );
  x1  = IROUND ( x2dpos ( Lon, Lat ) );
  y1  = IROUND ( y2dpos ( Lon, Lat ) );
  dx1 = x1 - x0;
  dy1 = y1 - y0;
  d1  = sqrt ( (double)dx1 * dx1 + dy1 * dy1 );


  // Calculate symbol & Line 2: last point -> BRU
  p = sbMaxFF->ptA[B->i2].pt;
  Lat = (float) ( p.Lat * c180divPi );
  Lon = (float) ( p.Lon * c180divPi );
  x2  = IROUND ( x2dpos ( Lon, Lat ) );
  y2  = IROUND ( y2dpos ( Lon, Lat ) );
  dx2 = x2 - x0;
  dy2 = y2 - y0;
  d2  = sqrt ( (double)dx2 * dx2 + dy2 * dy2 );

  // Draw line bru -> first point
  dm = min ( d1, d2 ) * 0.3;
  f  = dm / d1;
  xa = x0 + ( x1 - x0 ) * 11 / 10;          // Extend the line past the point
  ya = y0 + ( y1 - y0 ) * 11 / 10;          // Extend the line past the point
  xb = x0 + IROUND ( ( x1 - x0 ) * f );
  yb = y0 + IROUND ( ( y1 - y0 ) * f );
  symbol ( 4, x1, y1 );
  line ( x1, y1, xa, ya );                  // Lines with begin & end point out of screen are not drawn
  line ( x1, y1, xb, yb );                  // => split in 2

  // Draw line bru -> last point
  f  = dm / d2;
  xa = x0 + ( x2 - x0 ) * 11 / 10;          // Extend the line past the point
  ya = y0 + ( y2 - y0 ) * 11 / 10;          // Extend the line past the point
  xb = x0 + IROUND ( ( x2 - x0 ) * f );
  yb = y0 + IROUND ( ( y2 - y0 ) * f );
  symbol ( 4, x2, y2 );
  line ( x2, y2, xa, ya );                  // Lines with begin & end point out of screen are not drawn
  line ( x2, y2, xb, yb );                  // => split in 2

  // Draw spiral 1 -> 2
  double r, dr;
  double da = 6.;                           // angle step
  double a  = B->t1;
  if ( fabs ( B->Angle ) > 350. )
  { r = d1 * 0.45;                          // radius at point 1
    dr = ( r * 0.2 ) / 360. * da;           // radius increment / step
  }
  else
  { r = d1 * 0.55;                          // radius at point 1
    dr = 0.;                                // circle iso spiral
  }
  if ( B->Angle < 0 ) da = -da;
  int n = (int)floor ( B->Angle / da );
  xa = x0 + IROUND ( r * sin ( a * piDiv180 ) );
  ya = y0 - IROUND ( r * cos ( a * piDiv180 ) );
  for ( int i = 1;  i < n;   i ++ )
  { a += da;
    r += dr;
    xb = x0 + IROUND ( r * sin ( a * piDiv180 ) );
    yb = y0 - IROUND ( r * cos ( a * piDiv180 ) );
    line ( xa, ya, xb, yb );
    xa = xb;
    ya = yb;
  }
  r += dr;
  xb = x0 + IROUND ( r * sin ( B->t2 * piDiv180 ) );
  yb = y0 - IROUND ( r * cos ( B->t2 * piDiv180 ) );
  line ( xa, ya, xb, yb );

  Legend ( "Ronde", cBru );
}
