#include "Global.h"
#include "Runs.h"

CRuns Runs;


void CalcRuns(void)
{
  CFlight *pF = 0;
  if      ( sbMaxFF )  pF = sbMaxFF->pOwner;
  else if ( sbMaxT  )  pF = sbMaxT->pOwner;
  if ( !pF )  return;

  if ( debugRuns )
  { printf ( "\n" );
    printf ( "***********************************************\n" );
    printf ( "RUNS EVALUATION \n" );
    printf ( "\n" );
    Flush();
  }

  Runs.ReadStretches();
  if ( Runs.nStretches < 1 )
  { report_error ( RM_NO_STRETCHES, NULL );
    return;
  }

  for ( int i = 0;   i < Runs.nStretches;   i ++ )
  { CSoarBlock *pSB = pF->SB[0];
    for ( int i = 0;   i < pF->nSB;   i ++ )
    { pSB = pF->SB[i];
      if ( debugRuns )
      { printf ( "***********************************************\n" );
        printf ( "STRETCH ANALYSIS FLIGHT #%d, SOAR BLOCK #%d\n", pF->nFlight + 1, i + 1 );
      }
      Runs.Evaluate ( pSB, i );
    }
  }

  if ( debugRuns )
  { printf ( "\n" );
    printf ( "END RUNS EVALUATION\n" );
    printf ( "***********************************************\n" );
    printf ( "\n" );
    Flush();
  }
}



/*****************************************************************
/***   CStretch class
/*****************************************************************/

CRuns::CRuns(void)
{ Stretches = NULL;
  nStretches = nAR = 0;
}



CRuns::~CRuns(void)
{ free ( (void**)&Stretches );
}


void CRuns::ReadStretches(void)
{ char Line[MAXLEN] = { 0 };
  FILE *F = fopen ( file_stretches, "rt" );
  if ( F == 0 )  return;

  // Determine the number of airfilelds in the file
  // The first line contains the header, but that will be replaced by the unknown airfield code
  int nList = 0;
  while ( 1 )
  { if ( !fgets ( Line, MAXLEN, F ) )  break;
    if ( strlen ( Line ) < 20 )  continue;
    nList ++;
  }
  if ( nList < 2 )  goto Exit;

  rewind ( F );
  fgets ( Line, MAXLEN, F );  // Skip the header

  for ( int i = 1;  i < nList;   i ++ )
  { if ( !fgets ( Line, MAXLEN, F ) )  goto Exit;
    if ( !Read1 ( Line ) )
    { report_error ( RM_READ_ERROR, Line );
      goto Exit;
    }   
  }

 Exit:
  fclose ( (void**)&F );
  if ( nStretches < 1 ) report_error ( RM_NO_STRETCHES, NULL );
}


int CRuns::Read1(char *Line)
{ char st[MAXLEN];
  char st2[MAXLEN];
  char *c = Line;
  double D, M;

  sStretch S = { 0 };

  str_trim_right ( Line );

  // Read point 1 name
  if ( !ReadField ( &c, S.Name,  64 ) )  return 0;
  // Read latitude 1
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 2 );  D = atoi(st2);
  strcpy_sec ( st2, st, 2, 5 );  M = atoi(st2);
  S.pt[0].Lat = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[7] == 'S' ) S.pt[0].Lat *= (float)-1.;
  S.pt[0].Lat *= piDiv180;

  // Read longitude 1
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 3 );  D = atoi(st2);
  strcpy_sec ( st2, st, 3, 5 );  M = atoi(st2);
  S.pt[0].Lon = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[8] == 'W' ) S.pt[0].Lon *= (float)-1.;
  S.pt[0].Lon *= piDiv180;

  // Read point 2 name
  if ( !ReadField ( &c, st,  64 ) )  return 0;
  strcat ( S.Name, " - " );
  strcat ( S.Name, st );
  // Read latitude 2
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 2 );  D = atoi(st2);
  strcpy_sec ( st2, st, 2, 5 );  M = atoi(st2);
  S.pt[1].Lat = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[7] == 'S' ) S.pt[1].Lat *= (float)-1.;
  S.pt[1].Lat *= piDiv180;

  // Read longitude 2
  if ( !ReadField ( &c, st, 48 ) )  return 0;
  strcpy_sec ( st2, st, 0, 3 );  D = atoi(st2);
  strcpy_sec ( st2, st, 3, 5 );  M = atoi(st2);
  S.pt[1].Lon = (float)D + (float)( M / ( 1000. * 60. ) );   // in degrees
  if ( st[8] == 'W' ) S.pt[1].Lon *= (float)-1.;
  S.pt[1].Lon *= piDiv180;

  // Read radius
  if ( !ReadField ( &c, st, 8 ) )  return 0;
  S.Width = atof(st2) * 2;

  S.D = OrthoDist(&S.pt[0], &S.pt[1]);
  S.Track = OrthoTrk(&S.pt[0], &S.pt[1]);
  S.Sector[0] = S.Track + 180.;
  S.Track = OrthoTrk(&S.pt[1], &S.pt[0]);
  S.Sector[1] = S.Track;
  S.nLaps = 0;
  S.tTotal = 0.;

  if ( nStretches >= nAR )
  { realloc_s ( (void **)&Stretches, sizeof(sStretch), &nAR, 4 );
    if ( Stretches == NULL )  return 0;
  }
  Stretches[nStretches] = S;
  nStretches ++;
  return 1;
}






void CRuns::Evaluate(CSoarBlock *psb, int iA)
{
  int iCrossed = 0;   // Last crossed sector: 1: point 1, 2: point 2
  int nLaps = 0;   // Number of laps
  double tCrossed;
  double tTotal = 0.;
//  Cleanup();


  if ( !psb   )    return;
  pSB = psb;
  ptA = pSB->ptA;
  npt = pSB->npt;
  Stretch = Stretches[iA];

  for ( int i = 0;   i < npt;  i ++ )
  { if ( CheckLine ( i, 1 ) )
    { if ( iCrossed == 2 )
      { nLaps ++;
        tTotal += ptA[i].Time - tCrossed;
      }
      iCrossed = 1;
      tCrossed = ptA[i].Time;
    }
    else if ( CheckLine ( i, 2 ) )
    { if ( iCrossed == 1 )
      { nLaps ++;
        tTotal += ptA[i].Time - tCrossed;
      }
      iCrossed = 1;
      tCrossed = ptA[i].Time;
    }
  }

  if ( nLaps >  Stretches[iA].nLaps  ||  
       nLaps == Stretches[iA].nLaps  &&  tTotal < Stretches[iA].tTotal
     )
  { Stretches[iA].nLaps  = nLaps;
    Stretches[iA].tTotal = tTotal;
    Stretches[iA].aSpeed = Stretches[iA].D * nLaps / tTotal;
  }


  // Extract all data and fill TPT and Result
//  OptimizeTPT();
//  FillResult();
}





// Check if the line of sector n was croseed between i and i+1
// Width: total length of the line (not radius)
// i: index in flight points array
// n: stretch point
int CRuns::CheckLine(int i, int n)
{ if ( i > npt - 2 )  return 0;

  double x0, x1, y1, x2, y2;
  double d1 = wgs84    ( &Stretch.pt[n], &ptA[ i].pt );
  double d2 = wgs84    ( &Stretch.pt[n], &ptA[i+1].pt );
  double h1 = OrthoTrk ( &Stretch.pt[n], &ptA[ i ].pt );
  double h2 = OrthoTrk ( &Stretch.pt[n], &ptA[i+1].pt );

  // Step 1: rotate frame -> Sector = y axis
  //                      -> Line   = x axis
  double H1 = ( h1 - Stretch.Sector[n] ) * piDiv180;
  double H2 = ( h2 - Stretch.Sector[n] ) * piDiv180;

  // Step 2: calculate (x,y) coordinates of points (d1,h1), (d2,h2)
  x1 = d1 * sin(H1);    y1 = d1 * cos(H1);
  x2 = d2 * sin(H2);    y2 = d2 * cos(H2);

  // Step 3: check if x axis was crossed between (x1,y1), (x2,y2)
  // case y2=0: treated on next execution of loop in AnalyseTask
  if ( ( ( y1 > 0. )  &&  ( y2 >= 0. ) )   ||
       ( ( y1 < 0. )  &&  ( y2 <= 0. ) )
     ) return 0;

  // Take into account exception y1=y2=0
  if ( ( y1 == 0. )  &&  ( y2 == 0. ) ) return 0;


  // Step 4: The start/finish line (x axis) is crossed
  //         -> calculate distance of crossing to origin
  //         Singular case y1=y2 excluded due to step 3
  x0 = x2 - y2 * (x1-x2) / (y1-y2);

  // Step 5: Final check on limits of start/finish line
  //         Return 1 if crossing within limits of line
  if ( fabs(x0) <= ( Stretch.Width / 2. ) )
  { return 1;
  }
  return 0;
}

