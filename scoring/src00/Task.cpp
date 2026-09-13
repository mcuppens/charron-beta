#include "Global.h"
#include "WriteText.h"


double dtMinLegDist  = 10.;    // minimum length of the legs
int    MaxTaskPoints = 7;      // Maximum nbr task points (incl. start & finish)
BOOL   CheckDecTime  = TRUE;

CTask Task;



void CalcTask(void)
{ double Max = 0.;
  CFlight *pF = 0;
  
  if ( !Task.CheckDeclaration() )  return;
  if      ( sbMaxFF )  pF = sbMaxFF->pOwner;
  else if ( sbMaxT  )  pF = sbMaxT->pOwner;
  if ( !pF )  return;
  if ( !Task.CheckValid ( pF ) )  return;

  CSoarBlock *pSB = pF->SB[0];
  sbMaxDT = pSB;
  for ( int i = 0;   i < pF->nSB;   i ++ )
  { pSB = pF->SB[i];
    if ( debugDT )
    { printf ( "***********************************************\n" );
      printf ( "DECLARED TASK EVALUATION FLIGHT #%d, SOAR BLOCK #%d\n", pF->nFlight + 1, i + 1 );
    }
    Task.Evaluate ( pSB );
    if ( pSB->DT.ScoreDist < 5. )  continue;
    if ( pSB->DT.ScoreDist > Max )
    { Max = pSB->DT.ScoreDist;
      sbMaxDT = pSB;
    }
    else if ( fabs ( Max - pSB->DT.ScoreDist ) < epsilon )
    { // Probably both rounded, store the fastest
      if ( pSB->DT.AvgSpeed > sbMaxDT->DT.AvgSpeed )
      { sbMaxDT = pSB;
      }
    }
  }
  if ( debugDT )
  { printf ( "END DECLARED TASK EVALUATION\n" );
    printf ( "***********************************************\n" );
  }
}



CTask::CTask(void)
{
  Declared  = FALSE;
  nPoints   = nTPT = 0;
  TotDist   = 0;
  Valid     = FALSE;
  bArrayWarned = FALSE;
  memset ( TPT, 0, sizeof(TPT) );
  Cleanup();
}



CTask::~CTask(void)
{ Cleanup();
}





void CTask::Cleanup(void)
{ // Cleanup data, but not messages
  memset ( &RM, 0, sizeof(RM) );
  bBeercan  = FALSE;
}


int CTask::CheckDeclaration(void)
{
  if ( debugDT )
  { printf ( "Original declaration: %d points\n", nTPT );
    for ( int i = 0;   i < nTPT;   i ++ )
    { char st[256];
      double d = 0.;
      if ( i < nTPT-1 ) d = OrthoDist ( &TPT[i].pt, &TPT[i+1].pt );
      sprintf ( st, "%s            ", TPT[i].Name );
      st[12] = 0;
      printf ( "Point %d: %s %s %7.1f\n", i+1, st, stPosTask(i), d );
    }
    printf ( "\n" );
  }
/*
  // Remove zero fixes
  for ( int i = 0;   i < nTPT;   i ++ )
  { if ( IsZeroFix ( &TPT[i].pt ) )
    { if ( debugDT )  printf ( "Removing zero fix at position %d\n", i );
      for ( int j = i;   j < nTPT - 1;   j ++ )
        TPT[j] = TPT[j+1];
      nTPT --;
      nPoints --;
      i --;
    }
  }
*/
  // Remove zero legs.  Not before start / after finish
  for ( int i = 1;   i < nTPT-2;   i ++ )
  { if ( OrthoDist ( &TPT[i].pt, &TPT[i+1].pt ) < 1.E-3 )
    { if ( debugDT )  printf ( "Removing zero leg at position %d\n", i );
      for ( int j = i;   j < nTPT - 1;   j ++ )
        TPT[j] = TPT[j+1];
      nTPT --;
      nPoints --;
      i --;
    }
  }

  // Check proper task declaration
  if ( ( nPoints >= 2 )   ||  ( nTPT >= 4 ) )
  { // IGC task declaration = task points + take-off & landing
    // where task points = turn points + start & finish
    Declared = TRUE;
    if ( nTPT != ( nPoints + 2 ) )
    { report_error ( TM_TASK_INCONSIST, NULL );
      nPoints = nTPT - 2;
      if ( nPoints < 2 )
      { Declared = FALSE;
      }
    }
  }
  else
  { // No task declaration records (nPoints = 0)
    // or just empty declaration   (nPoints = 2)
    Declared = FALSE;
    report_error ( TM_NO_TASK, NULL );
  }
  if ( !Declared )
  { Cleanup();
    return FALSE;
  }
  if ( debugDT )
  { printf ( "Task declaration consistent, %d points\n", nTPT );
    printf ( "Cleaned declaration: %d points\n", nTPT );
    for ( int i = 0;   i < nTPT;   i ++ )
    { char st[256];
      double d = 0.;
      if ( i < nTPT-1 ) d = OrthoDist ( &TPT[i].pt, &TPT[i+1].pt );
      sprintf ( st, "%s            ", TPT[i].Name );
      st[12] = 0;
      printf ( "Point %d: %s %s %7.1f\n", i+1, st, stPosTask(i), d );
    }
    printf ( "\n" );
  }

  GetData();
  return TRUE;
}





// Calculate tracks, distance and sector data
void CTask::GetData(void)
{ int i;
  double t1, t2;

  // Calculate leg bearings & distances
  for ( i = 0;  i < nTPT;   i ++ )
  { TPT[i].Dist  = wgs84    ( &TPT[i].pt, &TPT[i+1].pt );
    TPT[i].Track = OrthoTrk ( &TPT[i].pt, &TPT[i+1].pt );
  }

  // Calculate total distance
  TotDist = 0.;
  for ( i = 1;  i < nTPT - 2;   i ++ )
    TotDist += TPT[i].Dist;

  // Calculate sector axises
  // Start sector
  TPT[1].Sector = TPT[1].Track + 180.;
  if ( TPT[1].Sector > 360. ) TPT[1].Sector -= 360.;
  
  // Turnpoint bisectors
  for ( i = 2;   i < Task.nPoints;   i ++ )
  { t1 = OrthoTrk ( &TPT[i].pt, &TPT[i-1].pt ) + 180.;
    if ( t1 > 360. ) t1 -= 360.;

    t2 = TPT[i].Track - 180.;
    if ( t1 - t2 > 180. ) t2 -= 360.;

    TPT[i].Sector = ( t1 + t2 ) / 2.; 
    if ( TPT[i].Sector < 0. ) TPT[i].Sector += 360.;
  }

  // Finish sector
  TPT[nPoints].Sector = TPT[nPoints-1].Track;
  if ( TPT[nPoints].Sector > 360. ) TPT[nPoints].Sector -= 360.;

}




int CTask::CheckValid(CFlight *pF)
{ char   st[MAXLEN];
  char   stD[MAXLEN];
  char   stF[MAXLEN];
  int    Dd, Dm, Dy;
  int    Fh, Fm, Fs;
  int    nR = FALSE;
  double t;

  Valid = TRUE;
  // Check total task distance
  if ( TotDist < MinTaskDist )
  { report_error ( TM_TASK_TOO_SMALL, NULL );
    Valid = FALSE;
    if ( debugDT )   printf ( "Task invalid: total distance too small: %0.1f\n", TotDist );
    return FALSE;
  }

  // Check minimum leg distance
  for ( int i = 1;  i < nTPT - 2;   i ++ )
  { if ( TPT[i].Dist < dtMinLegDist )
    { if ( i == 1    &&
           ( TPT[0].Dist < epsilon   ||   IsZeroFix ( &TPT[0].pt ) )
         )
      { // The take-off field may be included twice or invalid, remove first point
        if ( debugDT )  printf ( "Removing double take-off location\n" );
        report_error ( TM_CORRECTED_START, NULL );
        for ( int j = 0;   j < nTPT - 1;   j ++ )   TPT[j] = TPT[j+1];
        nTPT --;
        nPoints --;
        i --;
        nR ++;
        continue;
      }
      if ( nTPT > 4   &&   i == nTPT - 3   &&
           ( TPT[i+1].Dist < epsilon   ||   IsZeroFix ( &TPT[i+2].pt ) )
         )
      { // The landing field may be included twice or invalid, remove last point
        if ( debugDT )  printf ( "Removing double landing location\n" );
        report_error ( TM_CORRECTED_FINISH, NULL );
        nTPT --;
        nPoints --;
        nR ++;
        break;
      }
      report_error ( TM_TURNPT_TOO_CLOSE, NULL );
      Valid = FALSE;
      if ( debugDT )   printf ( "Task invalid: leg %d distance too small: %0.1f\n", i, TPT[i].Dist );
      return FALSE;
    }
  }
  if ( nR )
  { // Some point was deleted, recalculate the whole task
    GetData();
    if ( debugDT )
    { printf ( "Removed %d short legs, new: %d points\n", nR, nTPT );
      for ( int i = 0;   i < nTPT;   i ++ )
      { char st[256];
        double d = 0.;
        if ( i < nTPT-1 ) d = OrthoDist ( &TPT[i].pt, &TPT[i+1].pt );
        sprintf ( st, "%s            ", TPT[i].Name );
        st[12] = 0;
        printf ( "Point %d: %s %s %7.1f\n", i+1, st, stPosTask(i), d );
      }
      printf ( "\n" );
    }
  }

  if ( TotDist > 5000 )
  { report_error ( TM_TASK_DIST_ERR, NULL );
    Valid = FALSE;
    if ( debugDT )   printf ( "Task invalid: total distance above 5000km: %0.1f\n", TotDist );
    return FALSE;
  }

  if ( TotDist < TaskExtraPtDist )  MaxTaskPoints = 6;
  else                              MaxTaskPoints = 7;

  if ( nPoints > MaxTaskPoints )
  { report_error ( TM_NTURNPT_INVALID, NULL );
    Valid = FALSE;
    if ( debugDT )   printf ( "Task invalid: number of points above max (%d): %d\n", MaxTaskPoints, nPoints );
    return FALSE;
  }

  if ( pF )
  { if ( CheckDecTime )
    { // Check if the task was declared before the start of the flight
      // Only possible at this stage: find_flight had to be executed first
      // Convert the declaration & flight start date & time to int values
      strncpy ( st,  DeclareDate,    2 );      Dd = atoi(st);
      strncpy ( st, &DeclareDate[2], 2 );      Dm = atoi(st);
      strncpy ( st, &DeclareDate[4], 2 );      Dy = atoi(st);
      t  = pF->ptA[0].Time;
      Fh = (int) t;
      t  = ( t - Fh ) * 60.;
      Fm = (int) t;
      t  = ( t - Fm ) * 60. + 0.5;
      Fs = (int) t;
      sprintf ( stF, "%02d%02d%02d_%02d%02d%02d", Trace.Year % 100, Trace.Month, Trace.Day, Fh, Fm, Fs );
      sprintf ( stD, "%02d%02d%02d_%s", Dy, Dm, Dd, Task.DeclareTime );
//TODO      if ( debugDT )
if ( debugAny )
      { printf ( "Checking task declaration time\n" );
        printf ( "Start of flight: /%s/\n", stF );
        printf ( "Declaration:     /%s/\n", stD );
      }
      if ( strcmp ( stD, stF ) > 0 )
      { // Task was declared after start of flight
        double Buffer = 60.;
if ( debugAny )
        { printf ( "Task invalid: declared after start of flight\n");
          printf ( "Check if within buffered time\n");
          printf ( "Applied buffer %0.0f seconds\n", Buffer);
        }
        t = pF->ptA[0].Time + Buffer / 3600.;
        Fh = (int) t;
        t  = ( t - Fh ) * 60.;
        Fm = (int) t;
        t  = ( t - Fm ) * 60. + 0.5;
        Fs = (int) t;
        sprintf ( stF, "%02d%02d%02d_%02d%02d%02d", Trace.Year % 100, Trace.Month, Trace.Day, Fh, Fm, Fs );
if ( debugAny )
      { printf ( "Checking task declaration time with buffer applied\n" );
        printf ( "Limit time:      /%s/\n", stF );
        printf ( "Declaration:     /%s/\n", stD );
      }
        if ( strcmp ( stD, stF ) > 0 )
        {
if ( debugAny )
          { printf ( "Task invalid: declared after start of flight including buffer\n");
          }
          report_error ( TM_DECL_AFTER_START, NULL );
          Valid = FALSE;
          return FALSE;
        }
      }
    }
    if ( debugDT )  printf ( "Task Valid\n\n");
  }
  else Valid = FALSE;
if ( debugAny  &&  Valid )
      { printf ( "Declaration valid\n\n" );
      }
  return Valid;
}




BOOL CTask::Append(CTask::sPoint *pt)
{ pt->Sector = 0.;
  pt->Dist   = 0.;
  pt->Track  = 0.;

  // Add task point to declaration list
  if ( nTPT >= nTaskArray )    // Check if number of task points within array sizes
  { if ( !bArrayWarned )
    { if ( debugDT )
      { printf ( "Task declaration array out of bounds\n" );
      }
      bArrayWarned = TRUE;
      report_error ( TM_NTURNPT_ALLOWED, NULL );
    }
    return FALSE;
  }

  memcpy ( &TPT[nTPT], pt, sizeof(sPoint) );
  nTPT ++;
  return TRUE;
}






void CTask::Evaluate(CSoarBlock *psb)
{ Cleanup();

  if ( !Declared ) return;
  if ( !psb   )    return;
  pSB = psb;
  ptA = pSB->ptA;
  npt = pSB->npt;

  bBeercan = FALSE;
  if ( !CheckRounded() )
  { bBeercan  = TRUE;
    CheckRounded();
  }

  // Extract all data and fill TPT and Result
  OptimizeTPT();
  FillResult();
}






int CTask::CheckRounded(void)
{ int    i;
  int    NextStartLoop = 0;
  int    ThisStartLoop = -1;
  BOOL   bRounded = FALSE;
  double LowestAlt = 1.E10;             // Lowest altitude before crossing the start line
  if ( debugDT )
  { if ( bBeercan ) printf ( ">>>>> Starting evaluation using beercan\n" );
    else            printf ( ">>>>> Starting evaluation without beercan\n" );
  }
char st[64];  
  while ( ThisStartLoop != NextStartLoop )
  { ThisStartLoop = NextStartLoop;
    tpt = 1;
    memset ( &R, 0, sizeof(R) );
    R.BC = bBeercan;
//    if ( debugDT )  if ( bBeercan ) printf ( "LOOP Starting at %s\n", pSB->stHour ( ThisStartLoop ) );

    for ( i = ThisStartLoop;   i < npt;  i ++ )
    { LowestAlt = min ( LowestAlt, ptA[i].Altitude );
strcpy ( st, pSB->stHour(i) );
if ( ptA[i].Time >= 16+54/60. + 13/3600. )
i=i;
      d = wgs84 ( &ptA[i].pt, &TPT[1].pt );    // Distance from the current fix to the start point
      if ( debugDT )
      {
        //printf ( "tpt: %d  i:%6d  d: %0.6f\n", 1, i, d );
      }
      if ( CheckStart ( i ) )
      { // the start sector has been rounded at point i
        if ( tpt == 1 )                // Start sector rounded first time
        { i += Offset;                 // Offset = 1: not this, but the next point was at the sector side
          R.TPT[1] = i;
          R.BD     = 1.E10;            // Trigger a calculation of max result, below
          R.iB     = i;
          R.SA     = LowestAlt;
          tpt      = 2;                // First time in start sector
          if ( debugDT )
          { printf ( "Startpoint rounded: %s\n", PointString ( ptA, i ) );
          }
        }
        else if ( tpt == 2 )
        { // were flying the first leg -> restart immediately from here
          // Max result was already checked at the end of the previous loop
          NextStartLoop = i;
          if ( debugDT )
          { printf ( "Restarted: %s\n", PointString ( ptA, i ) );
          }
//          if ( debugDT )  if ( bBeercan ) printf ( "LOOP Starting from first leg at %s\n", pSB->stHour ( NextStartLoop ) );
          break;
        }
        else if ( NextStartLoop == ThisStartLoop )
        { // The first leg was completed.
          // Continue the evaluation of the current loop,
          // prepare the next loop
          NextStartLoop = i;
        }
      }

      if ( tpt < 2 )   continue;       // No valid start yet, can't proceed further

      d = wgs84 ( &ptA[i].pt, &TPT[tpt].pt );    // Distance from the current fix to the target turnpoint
      if ( debugDT )
      {
//        printf ( "tpt: %d  i:%6d  d: %0.6f\n", tpt, i, d );
      }
      if ( tpt < nPoints )             // target != finish
      { // Check if inside the sector of the next point
        if ( CheckSector ( tpt, i ) )
        { // inside the sector
          R.TPT[tpt] = i;              // Store result
          R.BD       = 1.E10;          // Trigger a calculation of max result, below
          R.iB       = i;
          tpt ++;
          d = wgs84 ( &ptA[i].pt, &TPT[tpt].pt );
          if ( debugDT )
          {
//            printf ( "Inside sector %d: %s\n", tpt, PointString ( ptA, i ) );
          }
        }
      }
      if ( tpt == nPoints )
      { if ( CheckFinish ( i ) )
        { i += Offset;                 // Offset = 1: not this, but the next point was at the sector side
          R.TPT[tpt]   = i;
          R.TPT[tpt+1] = 1;            // This will help in GetMaxResult
          R.Rounded    = TRUE;
          bRounded = TRUE;
          GetMaxResult();              // Store the result
          break;                       // Check for a new start
        }
      }
      if ( d < R.BD )
      { R.BD = d;
        R.iB = i;
        GetMaxResult();                // Task was started but not rounded.  Store result
      }
    }   // for i
    if ( !bRounded   &&  tpt == Task.nPoints )
    { i --;
      // this is the case if the finish was not rounded,
      d = wgs84 ( &ptA[i].pt, &TPT[tpt].pt );
      if ( d <= MaxLandingFinish )
      { // but the landing was within a given distance from it
        R.TPT[nPoints] = i;
        R.TPT[tpt+1]   = 1;            // This will help in GetMaxResult
        R.Rounded      = TRUE;
        bRounded       = TRUE;
        GetMaxResult();                // Store the result
      }
    }
  }  // while ( ThisStartLoop != NextStartLoop )
  return bRounded;
}




int CTask::CheckStart(int i)
{ Offset = 0;
  if ( CheckSector ( 1, i, 45. ) )  return 1;   // Inside sector
  if ( CheckLine ( 1, i, StartLineWidth, 1 ) )    return 1;
  return 0;
}



int CTask::CheckSector(int n, int i)
{ if ( CheckSector ( n, i, 45. ) )  return 1;   // Inside sector
  return 0;
}




int CTask::CheckFinish(int i)
{ Offset = 0;
  if ( CheckSector ( nPoints, i, 45. ) )  return 1;   // Inside sector
  if ( CheckLine ( nPoints, i, FinishLineWidth, 0 ) )    return 1;
  return 0;
}



void CTask::GetMaxResult(void)
{ int i2;                                        // Will receive the last fix
  // Calculate the total distance flown, including use of beercan
  double dBC = R.BC * BeercanRadius;             // Correction for distance, if beercan used
  R.TD = 0.;
  for ( int i = 1;  i < nPoints;   i ++ )
  { if ( R.TPT[i+1] )
    { // Leg was rounded
      if ( i == 1   ||   i +1 == nPoints )       // first or last leg
           R.DMG[i] = TPT[i].Dist - dBC;
      else R.DMG[i] = TPT[i].Dist - dBC * 2.;
      R.TD += R.DMG[i];
    }
    else                                         // Leg not rounded
    { R.DMG[i] = max ( 0., TPT[i].Dist - dBC - R.BD );
      R.TD += R.DMG[i];
      break;
    }
  }

  if ( R.Rounded )  i2 = R.TPT[nPoints];
  else              i2 = R.iB;
/*
  // Get the start altitude
  FF.StartAlt = ptA[0].Altitude;
  for ( i = i,  pt = ptA;   i <= FF.TptA[0].i;   i ++,  pt ++ )
  { FF.StartAlt = min ( FF.StartAlt, pt->Altitude );
  }
*/
  R.TT = ptA[i2].Time - ptA[R.TPT[1]].Time;      // Calculate the total time flown
  R.FA = ptA[i2].Altitude;                       // Finish altitude
  R.HL = R.SA - R.FA;                            // Calculate the height loss
  if ( R.HL > FreeHeightLoss )                   // Calculate the score distance
  { R.SD = R.TD - ( R.HL - FreeHeightLoss ) * LDnominal / 1000.;
  }
  else R.SD = R.TD;
  if ( R.Rounded   &&  !RM.Rounded )             // if the current result is better than the max: save
  { memcpy ( &RM, &R, sizeof(RM) );
  }
  else if ( R.SD > RM.SD )
  { memcpy ( &RM, &R, sizeof(RM) );
  }
  else if ( fabs ( R.SD - RM.SD ) < epsilon )    // Probably rounded, retain the fastest run
  { if ( R.TT < RM.TT )
    { memcpy ( &RM, &R, sizeof(RM) );
    }
  }
}




/****************************************************************************
 * check_Sector : Checks FAI turnpoint sector
 *
 * arguments : n  = index of the turnpoint to be checked
 *             i  = index of the current flight point
 *             aM = allowed angle from the sector axis
 * 
 * return = 1: - angle h within 45 left or right of the bisector of turnpoint n
 *             - a straight ine from the previous fix to the current fix passes through teh sector
 *          0: outside sector
 ****************************************************************************/

int CTask::CheckSector(int n, int i, double aM)
{ if ( d < epsilon )  return 1;        // On the point.  Some calculations below may cause exceptions
  if ( bBeercan  &&  d < BeercanRadius   &&  // Within beercan
       n != 1   &&   n != nPoints            // But not for start or finish
     ) return 1;

  double h = OrthoTrk ( &TPT[n].pt, &ptA[i].pt );
  double angle = fabs ( TPT[n].Sector - h );
  if ( angle > 180. ) angle = 360. - angle;

  // Check if inside the specified degrees "a"
  if ( angle <= aM )  return 1;

  // Has the sector been crossed, without a fix being logged inside?
  if ( !i ) return 0;                  // we need from the previous point...
  // if d > certain value: skipp this step
  // value:  300 kmh at log interval of 6" -> 75 m/s * 6" = 450 m => 500 m
  if ( d > 0.5 )  return 0;            // Outside the set limit
  // calculate the distance / bearing from the turnpoint to the previous fix
  int    i0 = i - 1;                   // index of the previous fix
  double d0 = wgs84    ( &TPT[n].pt, &ptA[i0].pt );
  double h0 = OrthoTrk ( &TPT[n].pt, &ptA[i0].pt );
  double x1, x0, y1, y0, a, b, x;      // rotate h and h0 -> Bisector   = x axis
  h0 -= TPT[n].Sector;    h0 *= piDiv180;
  h  -= TPT[n].Sector;    h  *= piDiv180;
  // Calculate the coordinates of the points in an orthonormal reference
  x1  = d  * cos ( h );       y1  = d  * sin ( h );
  x0  = d0 * cos ( h0 );      y0  = d0 * sin ( h0 );
  // exclude all obvious negative results
  if ( y0 > 0.   &&   y1  > 0. )   return 0;   // both in quadrant 1 or 2
  if ( y0 < 0.   &&   y1  < 0. )   return 0;   // both in quadrant 3 or 4
  if ( x0 < 0.   &&   x1  < 0. )   return 0;   // both in quadrant 2 or 3
  // return obvious positive result
  if ( x0 > 0.   &&   x1  > 0.   &&
       y0 * y1 < 0.
     )
    return 1;                          // one in quadrant 1, other in quadrant 4
  // arriving here requires deeper analysis
  // Get carthesian equation line p0 - p1
  // if line crosses x axis at x >= 0 => sector crossed
  a = ( y1 - y0 ) / ( x1 - x0 );
  if ( abs ( a ) < epsilon )  return 0;  // line parallel to x-axis
  b = y0 - x0 * a;
  x = -b / a;
  if ( x >= 0. )  return 1;
  return 0;
}



// CTask::CheckLine
// Check if the line of sector n was croseed between i and i+1
// Width: total length of the line (not radius)
// bSense: if true, the line must be crossed towards the next turnpoint
int CTask::CheckLine(int n, int i, double Width, int bSense)
{ if ( i > npt - 2 )  return 0;

  double x0, x1, y1, x2, y2;
  double d1 = wgs84    ( &TPT[n].pt, &ptA[ i].pt );
  double d2 = wgs84    ( &TPT[n].pt, &ptA[i+1].pt );
  double h1 = OrthoTrk ( &TPT[n].pt, &ptA[ i ].pt );
  double h2 = OrthoTrk ( &TPT[n].pt, &ptA[i+1].pt );

  // Step 1: rotate frame -> Sector = y axis
  //                      -> Line   = x axis
  double H1 = ( h1 - TPT[n].Sector ) * piDiv180;
  double H2 = ( h2 - TPT[n].Sector ) * piDiv180;

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

  if ( bSense )
  { // Check direction of crossing:  Should be away from sector.
    //  start or finish?
    if ( n == 1 ) { if ( y1 < 0  ||  y2 >= 0 )  return 0; }  // Start
    else          { if ( y1 > 0  ||  y2 <= 0 )  return 0; }
  }

  // Step 4: The start/finish line (x axis) is crossed
  //         -> calculate distance of crossing to origin
  //         Singular case y1=y2 excluded due to step 3
  x0 = x2 - y2 * (x1-x2) / (y1-y2);

  // Step 5: Final check on limits of start/finish line
  //         Return 1 if crossing within limits of line
  if ( fabs(x0) <= ( Width / 2. ) )
  { if ( y2 > 0 ) Offset = 1;  // Next point on sector side
    return 1;
  }
  return 0;
}




void CTask::FillResult(void)
{ // Store the result in pSB->DT
  sTaskAnalysis *pA = &pSB->DT;
  memset ( pA, 0, sizeof(sTaskAnalysis) );
  pA->Finished      = RM.Rounded;
  pA->BeercanUsed   = RM.BC;
  pA->StartAlt      = RM.SA;
  pA->FinishAlt     = RM.FA;
  pA->HeightLoss    = RM.HL;
  pA->TotTime       = RM.TT;
  pA->AvgSpeed      = ( RM.TT > epsilon ) ?  RM.TD / RM.TT  :  0.;
  pA->TotDist       = RM.TD;
  pA->ScoreDist     = RM.SD;
  if ( !RM.Rounded )
  { pA->BreakPointPos  = ptA[RM.iB].pt;
    pA->BreakPointTime = ptA[RM.iB].Time;
    pA->BreakPointAlt  = ptA[RM.iB].Altitude;
    pA->BreakDistance  = RM.BD;
  }
  int i2;
  for ( int i = 1;  i <= nPoints;   i ++  )
  { pA->Leg[i].Index = RM.TPT[i];
    pA->Leg[i].Time  = ptA[RM.TPT[i]].Time;
    pA->Leg[i].Alt   = ptA[RM.TPT[i]].Altitude;
    if ( i < nPoints )                          // Exclude finish
    { pA->Leg[i].DMG = RM.DMG[i];
      pA->Leg[i].Completed = RM.TPT[i+1] > 0;
      if ( pA->Leg[i].Completed ) i2 = RM.TPT[i+1];
      else                        i2 = RM.iB;
      pA->Leg[i].ET    = ptA[i2].Time - pA->Leg[i].Time;
      if ( pA->Leg[i].ET > epsilon )  pA->Leg[i].Speed = pA->Leg[i].DMG / pA->Leg[i].ET;
      else                            pA->Leg[i].Speed = 0.;
    }
  }
}



// Optimise the turnpoints by obtaining the last rounding
// RM.TPT contain the first rounding, => optimimum = ( firts + last ) / 2
void CTask::OptimizeTPT(void)
{ int i, i2;
  for ( int t = 2;   t < Task.nPoints;   t ++ )
  { if ( !RM.TPT[t] )  break;
    if ( RM.TPT[t+1] )  i2 = RM.TPT[t+1];
    else i2 = RM.iB;
    for ( i = i2;   i > RM.TPT[t];   i -- )
    { d = wgs84 ( &ptA[i].pt, &TPT[t].pt );    // Distance from the current fix to the target turnpoint
      if ( CheckSector ( t, i ) )
      { RM.TPT[t] += i;
        RM.TPT[t] /= 2;
        break;
      }
    }
  }
}