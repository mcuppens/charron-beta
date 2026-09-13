#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#ifdef WIN32
#include "Win32.h"
#endif
#include "Linux.h"

#define YES   1
#define NO    0

#define MAXLEN    (int) 256
const int LREC     = 72;

#include "Logger.h"
#include "Messages.h"
#include "Flight.h"
#include "FlightBase.h"
#include "Task.h"


const  char Version[] = { "24.03" };
extern char VersionSt[256];

const double pi        = 3.14159265359,
             pi2       = 6.28318530718,
             piDiv2    = 1.5707963268,
             piDiv180  = 0.0174532925199,
             c180divPi = 57.2957795131,
             epsilon   = 1.0e-12;
const double rEarth    = 6371.0 ;  // Radius FAI sphere

#define DUTCH

#ifdef WIN32
 const char PathDelimiter = '\\';
#else
 const char PathDelimiter = '/';
#endif

/*************************************/
/* Global macros and basic functions */
/*************************************/

#define IROUND(i)   (int)(i>0 ? i+0.5 : i-0.5)
#define NELEMS(X)   sizeof(X)/sizeof(*X)
#define min(X,Y)  (X<Y?X:Y)
#define max(X,Y)  (X>Y?X:Y)
#define free   FreeOwn
#define fclose FcloseOwn
extern void FreeOwn(void **M);
extern int  FcloseOwn(void **F);
extern void Flush(void);
extern void DebugBreakOwn(void);

/****************************/
/* Program output data      */
/****************************/
extern FILE   *OutF;

enum eSecurity {LS_NONE, LS_OK, LS_BAD};

enum ePlotFormat {PF_PNG, PF_PDF, PF_EPS};
//extern ePlotFormat PlotFormat ;


/******************************/
/* Program control variables  */
/******************************/
extern int debug;
extern int debugIGC;          // provide debug info for the IGC file checking / input
extern int debugFlt;          // provide debug info for flight detection algo
extern int debugRelease;      // provide debug info for release detection algo
extern int debugEngine;       // provide debug info for engine detection algo
extern int debugAlt;          // provide debug info for altimeter & altitude gain algo
extern int debugFF;           // provide debug info for free flight algo
extern int debugDT;           // provide debug info for declared task algo
extern int debugTRI;          // provide debug info for triangle algo
extern int debugBru;          // provide debug info for Charronde algo
extern int debugRuns;         // provide debug info for Charronde algo
extern int debugDislin;       // provide debug info for picture output
extern int debugPlot;         // provide debug info for plots output
extern int debugAny;          // True if any of the above debug switches is set
extern int debugTime;         // For algoritm execution time checking: generate a time delay of the given number of seconds
extern int debugtTot;         // For algoritm execution time checking: generate a time delay of the given number of seconds
extern int debugFile;         // Write debug info to file *.out, rather than console
extern int noGraph;           // True: doesn't draw any graphical output
extern int releaseManual;
extern int ScoreFF;
extern int ScoreDT;
extern int ScoreBru;
extern int ScoreTRI;
extern int ScoreRuns;
extern double triCloseD;      // Distance between fixes for closed loop checking. 0: disable loop checking
extern int triBF;             // Calcularte triangle Brute Force (for verifying of standard algo
extern int triBFstep;         // Step size (seconds) used in triangle brute force algo. default = 15
extern int triBFmns;          // Max number of steps used in triangle brute force algo
extern int FLT_Automatic;     // if FALSE: set startflight end endflight via command line; Release = StartSoar = StartFlight
extern double tStartSoar;     // For manual flt setting, via command line
extern double tEndSoar;       // For manual flt setting, via command line
extern float LatMaxPlot;
extern float LatMinPlot;
extern float LonMaxPlot;
extern float LonMinPlot;

extern int    nFixMax;        // Maximum number of fixes in a soaring block
extern double FreeHeightLoss; // Maximum height loss without penalisation (m)
extern double LDnominal;      // Height loss above FreeHeightLoss will be penalised using this value
extern BOOL   bPause;         // If TRUE, will request key press upon end of execution

/******************************/
/* Input/output file names    */
/******************************/
extern char file_igc[];
extern char file_text[];
extern char file_data[];
extern char file_plot[];
extern char file_baro[];
extern char file_airf[];
extern char file_stretches[];
extern char dir_country[];
extern char base_name[];
extern char IGC_name[];
extern char score_dir[];

/******************************/
/* Debug output data          */
/******************************/
extern int  nDebugMsg;
extern BOOL AddDebugMsg(char *st);
extern const char *GetDebugMsg(int n);
extern void StartTiming(void);
extern void EndTiming(char *msg);

extern void realloc_s(void **p, int s, int *n, int growby = 1, BOOL bZero = FALSE);  // Safe realloc

extern BOOL   IsZeroFix(sCoord *pt);
extern double wgs84(sCoord *p1, sCoord *p2);
extern double OrthoDist(sCoord *p1, sCoord *p2);
extern double LoxoDist(sCoord *p1, sCoord *p2);
extern double OrthoTrk(sCoord *p1, sCoord *p2);
extern void  MakeHour(double time, int *hr, int *min, int *sec);

extern char *stHour(double time);
extern char *PointString(sFlightPoint *ptA, int i);

// Extra string handling functions
extern void str_lower(char *st);
extern void str_upper(char *st);
extern void str_trim_left(char *st);
extern void str_trim_right(char *st);
extern void strcpy_sec(char *Dest, char *Source, int First, int n);
extern void AppendSpaces(char *string, int length);
extern int ReadField(char **c, char *st, int nMax);

#endif
