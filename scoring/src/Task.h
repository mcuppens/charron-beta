#ifndef TASK_H
#define TASK_H

const int     nTaskArray       = 16;
const double  TaskExtraPtDist  = 200.; // Below this distance, one less tpt is allowed
const double  MinTaskDist      = 20.;
const double  BeercanRadius    = 0.5;
const double  StartLineWidth   = 1.;   // Width of Start line (km) (24/01/2015: 2 -> 1 )
const double  FinishLineWidth  = 1.;   // Width of Finish line (km)
const double  MaxLandingFinish = 1.;   // Maximum distance to finish for landing finish (km)
extern double dtMinLegDist;            // minimum length of the legs
extern int    MaxTaskPoints;           // Maximum nbr task points (incl. start & finish)
extern BOOL   CheckDecTime;


// Declared task evaluation results

struct sTaskLeg // Analysis results
{ int    Index;                   // Fix index of start, turnpoints, finish in Soarblock ptA
                                  // Set to 0 if point not rounded
                                  // i=0:start, i=nPoints-1:finish (landing/engine)
  double Time;                    // Times over point (UTC)
  double Alt;                     // Altitude over point
  int    Completed;               // TRUE if leg was completed
  double DMG;                     // Distance flown on this leg
  double ET;                      // Elapsed time over leg
  double Speed;                   // Speed flown over this leg
};

struct sTaskAnalysis
{ sTaskLeg Leg[nTaskArray];       // turnpoint & leg results
  double TotDist;                 // Total distance flown
  double TotTime;                 // Time (finish or breakpoint) - start (h)
  double AvgSpeed;                // Average speed over TotDist (km/h)
  int    Finished;                // 0: task not finished, 1: task finished
  int    BeercanUsed;             // 1 if Beercan fotosector used
  sCoord BreakPointPos;           // Position of the break point
  double BreakPointTime;          // Time of Breakpoint (h UTC)
  double BreakPointAlt;           // Altitude of Breakpoint
  double BreakDistance;           // Distance breakpoint -> missed turnpoint (km)
  double StartAlt;                // Start altitude. Lowest alt start soaring block -> start point
  double FinishAlt;
  double HeightLoss;              // Altitude loss StartAlt -> finish (breakpoint) (h)
  double ScoreDist;               // Scored distance, accounted for HeightLoss (km)
};



class CFlight;

class CTask
{
public:
  CTask(void);
  ~CTask(void);

  int  Declared;
  char DeclareDate[LREC] ;             // Date of declaration in logger
  char DeclareTime[LREC] ;             // Time (UTC) of declaration
  int  Valid;                          // Declaration is valid or not
  int  nPoints;                        // Nbr. of turnpoints +2 in declared in first C record
                                       // !!!  incl. start & finish,
                                       // !!!  excl. take off & landing
  int  nTPT;                           // Number of points including take off & landing
  double TotDist;                      // Total distance of declared task
  struct sPoint
  { char   Name[LREC];
    sCoord pt;
    double Sector;                     // Bisector angles for task points
    // Leg data to the next point:
    double Dist;                       // Distance to next point
    double Track;                      // Track to next point
  } TPT[nTaskArray];

  int  CheckDeclaration(void);         // Checks if a task is declared and if it is consistent
  int  CheckValid(CFlight *pF);        // Checks minimu task length, number of points and declaration time
  void Evaluate(CSoarBlock *psb);
  BOOL Append(sPoint *pt);

 protected:
  CSoarBlock *pSB;
  sFlightPoint *ptA;                  // points to pSB->ptA, for easier editing
  int npt;                            // copy of pSB->npt, for easier editing
  int bArrayWarned;
  // Calculation parameters
  struct sCalc
  { int    TPT[nTaskArray+1];         // Index of point in pSB->ptA
    double DMG[nTaskArray+1];         // Distance made good of each leg
    int    Rounded;
    int    BC;                        // 1 if beercan was used
    double SA;                        // Start altitude: lowest altitude until start
    double FA;                        // Finish altitude: altitude upon finish or break-point
    double HL;                        // Height loss
    double TD;                        // Total distance flown
    double SD;                        // Score distance, taking into account height loss
    double TT;                        // Total time flown
    double BD;                        // Break distance
    int    iB;                        // Fix index of the break point
  }   R, RM;
  int tpt;
  int Offset;                         // Start & finish line crossing to/from sector
  double d;                           // The distance from the current fix to the target waypoint
  int bBeercan;

  void Cleanup(void);
  void GetData(void);
  int  CheckRounded(void);            // Check if the task was rounded
  inline int  CheckStart(int i);      // Check if the start sector was rounded in pt i
  inline int  CheckSector(int n, int i); // Check if sector nr n was rounded in pt i
  inline int  CheckFinish(int i);     // Check if the finish sector was rounded in pt i
  inline int  CheckSector(int n, int i, double a);
  inline int  CheckLine(int n, int i, double Width, int bSense);
  inline void GetMaxResult(void);      // Get the task result and break distance, store i bigger than Result
  void FillResult(void);              // Store the result in pSB->DT
  void OptimizeTPT(void);             // Optimise the turnpoints
};

extern CTask Task;
extern void CalcTask(void);

#endif
