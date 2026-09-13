#include "Global.h"


char VersionSt[256] = { 0 };

/****************************/
/* Program output data      */
/****************************/

FILE       *OutF = NULL;
//ePlotFormat PlotFormat = PF_PNG ;

/******************************/
/* Program control variables  */
/******************************/

int debug             = 0;
int debugIGC          = 0;
int debugFlt          = 0;
int debugRelease      = 0;
int debugEngine       = 0;
int debugAlt          = 0;
int debugFF           = 0;
int debugDT           = 0;
int debugTRI          = 0;
int debugBru          = 0;
int debugRuns         = 0;
int debugDislin       = 0;
int debugPlot         = 0;
int debugAny          = 0;
int debugTime         = 0;   // For algoritm execution time checking: generate a time delay of the given number of seconds
int debugtTot         = 0;   // For algoritm execution time checking: generate a time delay of the given number of seconds
int debugFile         = 0;
int noGraph           = 0;
int releaseManual     = 0;
int ScoreFF           = 1;
int ScoreDT           = 1;
int ScoreBru          = 1;
int ScoreTRI          = 1;
int ScoreRuns         = 1;
double triCloseD      = 1.;  // Distance between fixes for closed loop checking. 0: disable loop checking
int triBF             = 0;
int triBFstep         = 15;
int triBFmns          = 0;
int FLT_Automatic     = 1;
double tStartSoar     = 0.;
double tEndSoar       = 0.;
double FreeHeightLoss = 1000.;  // Maximum height loss without penalisation (m)
double LDnominal      = 100.;
BOOL   bPause         = FALSE;
float  LatMaxPlot     = 999.;    // 999 -> auto
float  LatMinPlot     = 999.;
float  LonMaxPlot     = 999.;
float  LonMinPlot     = 999.;


/******************************/
/* Input/output file names    */
/******************************/
char file_igc[MAXLEN]       = { 0 };
char file_text[MAXLEN]      = { 0 };
char file_data[MAXLEN]      = { 0 };
char file_plot[MAXLEN]      = { 0 };
char file_baro[MAXLEN]      = { 0 };
char file_airf[MAXLEN]      = { 0 };
char file_stretches[MAXLEN] = { 0 };
char dir_country[MAXLEN]    = { 0 };
char base_name[MAXLEN]      = { 0 };
char IGC_name[MAXLEN]       = { 0 };
char score_dir[MAXLEN]      = { 0 };




/******************************/
/* Debug output data          */
/******************************/
int  nDebugMsg        = 0;
char DebugMsg[10][64] = { 0 };

BOOL AddDebugMsg(char *st)
{ if ( nDebugMsg >= 9 )  return FALSE;
  strcpy ( DebugMsg[nDebugMsg], st );
  nDebugMsg ++;
  return TRUE;
}


const char *GetDebugMsg(int n)
{ if ( n < 0   ||   n > 9 ) return NULL;
  return DebugMsg[n];
}



// Safe realloc
// p: in:  previous pointer
//    out: new pointer
// s: in:  sizeof 1 element
// n: in:  previous number of elements
//    out: new number of elements
// growby: number of elements to add
// bZero: if set, zeroes the new elements
void realloc_s(void **p, int s, int *n, int growby/* = 1*/, BOOL bZero/* = FALSE*/)
{ int n0 = *n;
  int n2 = n0 + growby;
  void *tmp = *p;

  if ( growby < 1 )
  { char st[256];
    sprintf ( st, "realloc_s invalid growby %d\n", growby );
    report_error ( EC_MEMORY, st );
  }

  if ( !tmp )  tmp = malloc  ( n2 * s );
  else         tmp = realloc ( tmp, n2 * s );
  if ( !tmp )
  { char st[256];
    sprintf ( st, "Memory allocation error %d bytes\n", n2 * s );
    report_error ( EC_MEMORY, st );
  }

  if ( bZero ) memset ( (char *)tmp + n0 * s, 0, growby * s );
  *p = tmp;
  *n = n2;
}





double OrthoDist(sCoord *p1, sCoord *p2) 
{ double cosB = cos(p1->Lat) * cos(p2->Lat) * cos(p2->Lon-p1->Lon) + sin(p1->Lat) * sin(p2->Lat) ;
  if ( cosB > 1.0 ) cosB = 1.0;
  return rEarth * acos(cosB);
}


double LoxoDist(sCoord *p1, sCoord *p2)
{ // Calculate loxo dist between points in ptL
  double ym = ( p2->Lat + p1->Lat ) / 2.;
  double dy =   p2->Lat - p1->Lat;
  double dx = ( p2->Lon - p1->Lon ) * cos ( ym );
  return sqrt ( dx * dx + dy * dy ) * rEarth;
}




BOOL IsZeroFix(sCoord *pt)
{ return fabs(pt->Lat) < epsilon  && fabs(pt->Lon) < epsilon ;
}



// Flight ellipsoid distance (WGS84)  --- not used yet ---
// Uses approximation of Vincenty (1975)
double wgs84(sCoord *p1, sCoord *p2) 
{
  const    double a = 6378.137;          // WGS84 equatorial radius
  const    double f = 1.0/298.257223563; // WGS84 ellipsoid flattening
  const    double EPSILON = 0.00000000005;
  const    int    MAXITER = 100;
   
  int  iter=0;
  register double lambda1, phi1 ;
  register double lambda2, phi2 ;
  register double tU1, tU2, cU1, cU2, sU1, sU2;
  register double cU12, sU12, sc12, cs12;
  register double L0, L, C;
  register double sL, cL, sinS = 0., cosS = 0., tanS, Sigma = 0.;
  register double sinA, cos2A = 0., cosM = 0.;
  register double u2, A, B, dSigma, distance;

  // Geodetic latitude & longitude of points p1 and p2
  phi1 = p1->Lat ;  lambda1 = p1->Lon;
  phi2 = p2->Lat ;  lambda2 = p2->Lon;
  if ( ( fabs ( lambda2 - lambda1 )  < epsilon )   &&
       ( fabs ( phi2    - phi1    )  < epsilon )
     ) return 0.;
  
  // Reduced latitudes U1 and U2
  tU1 = (1.0-f)*tan(phi1);         // tan( U1 )
  tU2 = (1.0-f)*tan(phi2);
  cU1 = 1.0/sqrt(1.0 + tU1*tU1);   // cos( U1 )
  cU2 = 1.0/sqrt(1.0 + tU2*tU2);
  sU1 = tU1*cU1;                   // sin( U1 )
  sU2 = tU2*cU2;
  
  // Products to be calculated only once
  cU12 = cU1*cU2;
  sU12 = sU1*sU2;
  cs12 = cU1*sU2;
  sc12 = sU1*cU2;
  
  // Initial estimate for longitude difference on sphere
  L0 = lambda2 - lambda1;
  
  // Loop until no significant change in L
  L  = L0 + 1.0;  // force one pass
  
  while ( (fabs(L-L0) > EPSILON) && (iter < MAXITER) )
  {
    L0 = L;
    iter++;
    
    sL = sin( L );
    cL = cos( L );
    
    sinS  = sqrt( pow(cU2*sL,2) + pow(cs12-sc12*cL,2) );
    cosS  = sU12 + cU12*cL;
    tanS  = sinS/cosS;
    Sigma = acos(cosS);
    
    sinA  = cU12 * sL / sinS;
    cos2A = 1-sinA*sinA;
    cosM = cosS - (2*sU12/cos2A);
    
    C = (0.625*f)*cos2A*(4+f*(4-3*cos2A));
    L = (lambda2-lambda1) 
      + (1-C)*f*sinA*(Sigma+C*sinS*(cosM+C*cosS*(-1+2*cosM*cosM)));
  }
  
  u2 = cos2A * (1/pow(1-f,2) - 1);
  A  = 1 + u2/256 * (64+u2*(-12+5*u2));
  B  = u2/512 * (128+u2*(-64+37*u2));
  dSigma = B * sinS * ( cosM + 0.25 * B * cosS * ( -1. + 2. * cosM * cosM ) );
  
  distance = (1-f)*a * A * (Sigma-dSigma);
  
  return distance;
}





double OrthoTrk(sCoord *p1, sCoord *p2) 
{ double Trk;
  double cosD = cos(p1->Lat) * cos(p2->Lat) * cos ( p2->Lon - p1->Lon ) + sin(p1->Lat) * sin(p2->Lat);
  if ( cosD > 1.0 ) cosD = 1.0;
  double D = acos ( cosD );
  double d = sin(D) * cos ( p1->Lat );
  
  double cosB = ( sin(p2->Lat) - sin(p1->Lat) * cosD ) / d;
  if (cosB >  1.0) cosB =  1.0 ;
  if (cosB < -1.0) cosB = -1.0 ;
    
  if ( sin ( p2->Lon - p1->Lon ) > 0.0 ) Trk = acos ( cosB );
  else                                   Trk = 2. * pi - acos ( cosB );
   
  return Trk * c180divPi;
}



 // MakeHour : converts decimal hours to hour, min, sec 
void MakeHour(double time, int *hr, int *min, int *sec)
{ double b;
  
  *hr  = (int)floor ( time );
  b    = ( time -( *hr ) ) * 60.;
  *min = (int)floor ( b );
  b    = ( b - (*min) ) * 60. + 0.5;
  *sec = (int)floor ( b );
  
  if (*sec == 60)
  { *sec  = 0;
    *min += 1;
  }
  
  if (*min == 60)
  { *min  = 0;
    *hr  += 1;
  }
  
  if ( *hr >= 24 ) *hr -= 24;  /* Remove 24 h offset (flights passing midnight) */
}
  




char *stHour(double time)
{ static char st[4][24];    // st must be persistent
  static int i = 0;         // Pointer for rotating buffer: if stHour is called from
  i ++;                     // within the sam prontf command, the same st would be transfered every time
  i &= 0x03;
  int h, m, s;
  MakeHour ( time, &h, &m, &s );
  sprintf ( st[i], "%02d:%02d:%02d", h, m, s );
  return st[i];
}





char *PointString(sFlightPoint *ptA, int i)
{ static char st[64];
  sprintf ( st, "%4d %s", i, stHour ( ptA[i].Time ) );
  return st;
}


/****************************************************************************
 * Extra string handling functions
 ****************************************************************************/


// str_lower : Converts a string to all lowercase.
void str_lower(char *st)
{ char *c = st;
  while( *c )
  { *c = (char)tolower ( *c );
    c ++;
  }
}



// str_upper : Converts a string to all uppercase.
void str_upper(char *st)
{ char *c = st;
  while( *c )
  { *c = (char)toupper ( *c );
    c ++;
  }
}


// str_trim_left : Trims a string at left end
void str_trim_left(char *st)
{ char *sp ;

  sp = st ;
  while (*sp!=0 && (*sp<=32 || *sp>=127)) sp++ ;
  strcpy(st, sp) ;
}






// str_trim_right : Trims a string at right end
void str_trim_right(char *st)
{ int i = (int)strlen(st) ;
  if (i<=0) return ; i-- ;
  
  while (i)
  { if (st[i]>' ' && st[i]<127) return ;
    st[i]='\0' ;
    i-- ;
  }
}





// strcpy_sec : Copies section of string : n characters starting from First
//              If n == 0 : string copied till the end
void strcpy_sec(char *Dest, char *Source, int First, int n)
{ int i,j;
  if ( !n ) n = (int)strlen(Source)-First;
  
  for ( i = First,  j=0;    j < n;   i ++,  j ++) Dest[j] = Source[i];
  Dest[j] = '\0';
}





// append_spaces : Appends spaces to a string to given length.
void AppendSpaces(char *string, int length)
{ int i, len ;
  
  len = (int)strlen(string) ;
  for ( i = len;   i < length;   i++ )
    string[i] = ' ' ;
  string[length] = '\0' ;
}


// read a string from char c until the next tab or end of string,
// nMax: Maximum number of chars excluding the trailing zero
// return: c points the first char after the next tab or NULL
int ReadField(char **c, char *st, int nMax)
{ char *c0 = *c;
  if ( !*c ) return 0;
  *c = strchr ( *c, '\t' );
  if ( *c )
  { **c = 0;
    (*c) ++;   // Upon exit, c points to the first char after the tab
  }
  if ( (int)strlen ( c0 ) > nMax )   c0[nMax] = 0;
  strcpy ( st, c0 );
  return 1;
}



// FreeOwn: sets the memory block to NULL to avoid double frees or realloc corruption
#undef  free
void FreeOwn(void **M)
{ if ( !*M )  return;
  free ( *M );
  *M = NULL;
}

// FcloseOwn: sets the File to NULL to avoid double fcloses
#undef fclose
int  FcloseOwn(void **F)
{ if ( !*F )  return 0;
  int r = fclose ( (FILE*)*F );
  if ( !r )   *F = 0;
  return r;
}



void Flush(void)
{ if ( debugFile ) fflush ( stdout );
}
