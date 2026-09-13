#include "Global.h"
#include "dislin.h"
#include "PlotFile.h"

#include "Airfields.h"

#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
//https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#endif

struct sPlotFileHeader
{ char stVersion[8];
  int  sPFH;   // sizeof(sPlotFileHeader)
  int  sFH;    // sizeof(sFlightHeader)
} PlotFileHeader;


//char DayFileName[MAXLEN] = { 0 };
const char stDelimiter[] = { "Flight header" };


// Write the flight to the day file
void ExportFlight(void)
{ int ExitCode = 0;
  int i;
  char *c, Dir[MAXLEN];
  FILE *F = 0;

  if ( !PlotList.LoadFlight() )   return;

  strcpy (  PlotFileHeader.stVersion, PlotFileVersion );
  PlotFileHeader.sPFH = sizeof(PlotFileHeader);
  PlotFileHeader.sFH  = sizeof(sFlightHeader);

  sprintf ( PlotList.List[0].FF0, "day_%04d%02d%02d",            Trace.Year, Trace.Month, Trace.Day );
  sprintf ( PlotList.List[0].FP0, "day_%04d%02d%02d.png",        Trace.Year, Trace.Month, Trace.Day );
  sprintf ( PlotList.List[0].TL,  "DAGOVERZICHT %02d-%02d-%02d", Trace.Day,  Trace.Month, Trace.Year );

  strcpy ( Dir, base_name );
  c = strrchr ( Dir, PathDelimiter );
  if ( c == 0 ) Dir[0] = 0;                       // No directory
  else { c ++; *c = 0; }

  for ( i = 0;   i < PlotList.nList;   i ++ )
  {
    if ( *Dir )
    { sprintf ( PlotList.List[i].FF, "%s%s", Dir, PlotList.List[i].FF0 );
      sprintf ( PlotList.List[i].FP, "%s%s", Dir, PlotList.List[i].FP0 );
    }
    else
    { strcpy ( PlotList.List[i].FF, PlotList.List[i].FF0 );
      strcpy ( PlotList.List[i].FP, PlotList.List[i].FP0 );
    }

    CPlotFile PF ( PlotList.List[i].FP, i );
  }
/*
  sprintf ( st, "mutex_day%04d%02d%02d", Trace.Year, Trace.Month, Trace.Day );
  CMutex Mutex ( st );
  if ( !Mutex.Wait ( 10000 ) )
  { report_error ( EC_OUT_DAYFILE, "Grabbing mutex" );
    goto Exit;
  }

  F = fopen ( DayFileName, "rb" );               // Try to open the day file to check if it exists
  if ( !F )                                      // Does not exist. Create & write the version
  { F = fopen ( DayFileName, "wb" );
    if ( !F )  goto Exit;
    fwrite ( DayFileVersion,  sizeof(DayFileVersion), 1, F );
  }
  fclose ( (void**)&F );                         // Must be reopened for writing
  F = fopen ( DayFileName, "ab" );               // Open the day file for appending.
  if ( fwrite ( &H, sizeof(sDayHeader), 1, F ) != 1 )      goto Exit;
  if ( fwrite ( Lat, sizeof(float), H.npt, F ) != H.npt )  goto Exit;
  if ( fwrite ( Lon, sizeof(float), H.npt, F ) != H.npt )  goto Exit;
  fclose ( (void**)&F );

  { strcpy ( st, DayFileName );
    strcat ( st, ".png" );
    CDayFile DF ( st );                          // Within CMutex !!
  }
  ExitCode = 1;
 Exit:
  if ( !ExitCode )
  { printf ( "Error exporting flight\n" );
  }
//  fclose ( (void**)&F );
*/
  // The mutex will be released via the CMutex destructor
}




CPlotList PlotList;


CPlotList::CPlotList(void)
{ List = 0;
  nList = nListR = 0;
  memset ( &H, 0, sizeof(sFlightHeader) );
  strcpy ( H.Delimiter, stDelimiter );
  Lat = Lon = 0;
  Resolution = 60. / 3600.;   // Seconds -> decimal hours
  Add ( "", "BNL" );   // Temporary name: Trace data not known yet. Adjust via ExportFlight
}


CPlotList::~CPlotList(void)
{
  free ( (void**)&List );
  free ( (void**)&Lat );
  free ( (void**)&Lon );
}





int CPlotList::Add(char *name, char *rgn)
{ char rgu[32];
  char stR[32];
  if ( nList >= nListR )
  { // Grow the List
    realloc_s ( (void **)&List, sizeof(sList), &nListR, 4, TRUE );
  }
  strcpy ( rgu, rgn );
  str_upper ( rgu );
  if      ( !strcmp ( rgu, "WLD") )  { List[nList].Region = WORLD;         strcpy ( stR, " World" ); }
  else if ( !strcmp ( rgu, "EUR") )  { List[nList].Region = EUROPE;        strcpy ( stR, " Europe" ); }
  else if ( !strcmp ( rgu, "FRA") )  { List[nList].Region = FRANCE;        strcpy ( stR, " France" ); }
  else if ( !strcmp ( rgu, "SAF") )  { List[nList].Region = SOUTH_AFRICA;  strcpy ( stR, " South Africa" ); }
  else if ( !strcmp ( rgu, "MOR") )  { List[nList].Region = MOROCCO;       strcpy ( stR, " Morocco" ); }
  else if ( !strcmp ( rgu, "AUS") )  { List[nList].Region = AUSTRALIA;     strcpy ( stR, " Australia" ); }
  else if ( !strcmp ( rgu, "USA") )  { List[nList].Region = USA;           strcpy ( stR, " USA" ); }
  else                               { List[nList].Region = BENELUX;       strcpy ( stR, " BNL" ); }
  sprintf ( List[nList].TL, "%s %s",     name, stR );

  // replace all underscores in the plot file by blanks
  char *c;
  c = strchr ( List[nList].TL, '_' );
  while ( c )
  { *c = ' ';
     c = strchr ( List[nList].TL, '_' );
  }

  strcpy  ( List[nList].FF0, name );
  sprintf ( List[nList].FP0, "%s_%s.png", name, rgu );


/*
  // replace all blanks in the flights file name by underscores
  c = strchr ( List[nList].FF, ' ' );
  while ( c )
  { *c = '_';
     c = strchr ( List[nList].FF, ' ' );
  }
  // replace all blanks in the plot file name by underscores
  c = strchr ( List[nList].FP, ' ' );
  while ( c )
  { *c = '_';
     c = strchr ( List[nList].FP, '_' );
  }
*/

  nList ++;
  return 1;
}




int CPlotList::LoadFlight(void)
{ int    i;
  double t = -999.;

  // check if the flight is elegible for export
  if ( !sbMaxFF ) return 0;
  CFlight *pF = sbMaxFF->pOwner;
  if ( !pF ) return 0;

  if ( sbMaxFF->FF.ScoreDist < 75. )     return 0;    // Only flights of at least n km are plotted

  // Create the header & the points array
  H.LatMax = -1000.F;
  H.LonMax = -1000.F;
  H.LatMin =  1000.F;
  H.LonMin =  1000.F;
  H.ffDist = (float) ( sbMaxFF  ?  sbMaxFF->FF.ScoreDist  :  0. );
  H.dtDist = (float) ( sbMaxDT  ?  sbMaxDT->DT.ScoreDist  :  0. );
  H.pt0    = pF->ptA[0].pt;
  Lat = (float *) malloc ( sizeof(float) * pF->npt );
  if ( !Lat )  report_error ( EC_MEMORY, "CPlotFlight::DrawFlight, Lat" );
  Lon = (float *) malloc ( sizeof(float) * pF->npt );
  if ( !Lon )  report_error ( EC_MEMORY, "CPlotFlight::DrawFlight, Lon" );

  sFlightPoint *fp = pF->ptA;
  for ( i = 0;   i < pF->npt;   i ++,  fp ++ )
  { if ( i < pF->npt-1   &&   fp->Time < t ) continue;
    Lat[H.npt] = (float) ( fp->pt.Lat * c180divPi );
    Lon[H.npt] = (float) ( fp->pt.Lon * c180divPi );
    H.LatMin = min ( H.LatMin, Lat[H.npt] );   // In radians !
    H.LatMax = max ( H.LatMax, Lat[H.npt] );
    H.LonMin = min ( H.LonMin, Lon[H.npt] );
    H.LonMax = max ( H.LonMax, Lon[H.npt] );
    H.npt ++;
    t = fp->Time + Resolution;
/*
    if ( Lat[i] < 49.   ||   Lat[i] > 52.   ||
         Lon[i] <  3.   ||   Lon[i] >  7.
       )
       i = i;
*/
  }
  return 1;
}




CPlotFile::CPlotFile(char *fn, int n) : CDislin ( fn )
{ F      = 0;
  HA     = NULL;
  nH     = nHR = 0;
  MaxFF  = 0;
  pMutex = 0;
  if ( n >= PlotList.nList )  return;
  pPlot = &PlotList.List[n];
  TreatFlight();
}



CPlotFile::~CPlotFile(void)
{
  fclose ( (void**)&F );
  free ( (void**)&HA );
  if ( pMutex ) delete pMutex;  // The mutex will be released via the CMutex destructor
}






void CPlotFile::TreatFlight()
{ char st[MAXLEN];


  sprintf ( st, "mutex_%s", pPlot->FF );
  pMutex = new CMutex(st);
  if ( !pMutex   ||   !pMutex->Wait ( 10000 ) )  // DEBUG  TODO reset to 5000
  { report_error ( EC_OUT_PLOTFILE, "Grabbing mutex" );
    printf ( "Plotfile %s not created\n", pPlot->FF );
    return;
  }

  if ( !GetFlights() )  return;
  if ( !IncludeRegion ( GetRegion ( PlotList.H.pt0 ), pPlot->Region ) )  return;

  GetArea();
  StartPlot();          // Must be called before GetPlotArea
  GetPlotArea();
  SelectAirfields();
  CDislin::DrawFrame ( pPlot->TL );
  DrawCountries ( 3 );
  PlotScale();
  DrawFlights();
  DrawAirfields();
  nFlts();
}





int CPlotFile::GetFlights(void)
{ // Read all flight headers in an array, add the current flight
  int ExitCode = FALSE;
  sPlotFileHeader PFH;
  sFlightHeader H;
  int bError = TRUE;

  // Always list the flight to the file, correct region or not
  F = fopen ( pPlot->FF, "r+b" );     // Open the file for reading & writing
  if ( F <= 0 )
  { F = fopen ( pPlot->FF, "w+b" );   // File didn't exist: create
    if ( F <= 0 )  goto Exit;
  }
  if ( fread ( &PFH, sizeof(sPlotFileHeader), 1, F ) == 1   &&
       !memcmp ( &PFH, &PlotFileHeader, sizeof(sPlotFileHeader) )
     )
  { // The version = OK, read all flight headers into memory
    bError = FALSE;
    while ( 1 )
    { int p = (int)ftell ( F );
      if ( fread ( &H, sizeof(H), 1, F ) != 1 )
      { // End of file, make sure the file pointer is at the start of the next header!
        fseek ( F, p, SEEK_SET );
        break;
      }
      if ( strcmp ( H.Delimiter, stDelimiter ) )
      { report_error ( EC_OUT_PLOTFILE, "Delimiter mismatch" );
        bError = TRUE;
        break;
      }
      H.PointIndex = ftell ( F );
      if ( IncludeRegion ( GetRegion ( H.pt0 ), pPlot->Region ) )
      { if ( nH >= nHR )
          realloc_s ( (void **)&HA, sizeof(H), &nHR, 16, TRUE );
        memcpy ( &HA[nH], &H, sizeof(H) );
        nH ++;
      }
      fseek ( F, sizeof(float) * H.npt * 2, SEEK_CUR ); // Skip the coordinates
    }
  }

  if ( bError )
  { // The version != OK: discard all data, Write fileheader
    fclose ( (void**)&F );
    F = fopen ( pPlot->FF, "w+b" );
    if ( F <= 0 )  goto Exit;
    if ( fwrite ( &PlotFileHeader, sizeof(sPlotFileHeader), 1, F ) != 1 ) goto Exit;
  }

  // Write the current flight to the file
  PlotList.H.PointIndex = ftell ( F ) + sizeof(sFlightHeader);
  if ( fwrite ( &PlotList.H, sizeof(sFlightHeader), 1, F ) != 1 )                     goto Exit;
  if ( fwrite ( PlotList.Lat, sizeof(float), PlotList.H.npt, F ) != PlotList.H.npt )  goto Exit;
  if ( fwrite ( PlotList.Lon, sizeof(float), PlotList.H.npt, F ) != PlotList.H.npt )  goto Exit;

  // Add the current flight header to the array
  if ( nH >= nHR )
    realloc_s ( (void **)&HA, sizeof(H), &nHR, 16, TRUE );
  memcpy ( &HA[nH], &PlotList.H, sizeof(H) );
  nH ++;

  ExitCode = TRUE;
 Exit:
  if ( !nH )
  { report_error ( EC_OUT_PLOTFILE, "No data found" );
    fclose ( (void**)&F );
  }
  else if ( !ExitCode )
  { printf ( "Error reading flghts file\n" );
    fclose ( (void**)&F );
    nH = 0;
  }
  return ExitCode;
}



void CPlotFile::GetArea(void)
{ sFlightHeader *ph = HA;
  for ( int i = 0;   i < nH;   i++,  ph ++ )
  { LatMin = min ( LatMin, ph->LatMin );
    LatMax = max ( LatMax, ph->LatMax );
    LonMin = min ( LonMin, ph->LonMin );
    LonMax = max ( LonMax, ph->LonMax );
    MaxFF  = max ( MaxFF,  ph->ffDist );
  }
}




void CPlotFile::DrawFlights(void)
{ sFlightHeader *ph = HA;
  int i = 0;

  linwid ( 1 );  
  setclr ( 40 );

  for ( ;   i < nH;   i ++, ph ++ )
  { AllocLatLon ( ph->npt );
    fseek ( F, ph->PointIndex, SEEK_SET );
    if ( fread ( Lat, sizeof(float), ph->npt, F ) != ph->npt )  goto Error;
    if ( fread ( Lon, sizeof(float), ph->npt, F ) != ph->npt )  goto Error;
    curvmp ( Lon, Lat, ph->npt );    // Draw the whole flight
  }
  return;

 Error:
  char st[MAXLEN];
  sprintf ( st, "Error reading points flight #%d", i );
  report_error ( EC_OUT_PLOTFILE, st );
}




void CPlotFile::nFlts(void)
{ char st[64];
  sprintf ( st, "%d vluchten", nH );
  clpbor ( "page" );
  txtjus ( "right" );
  setclr ( 3 );
  linwid ( 2 );
  height ( 32 * Scale ) ;
  messag ( st, tlMap.x + sMap.cx + 160 * Scale, 48 );
  linwid ( 1 );
  clpbor ( "axis" );
}

