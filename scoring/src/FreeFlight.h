#ifndef FREE_FLIGHT_H
#define FREE_FLIGHT_H

#include "ffArray.h"

const  int    ffMaxTpt     =  7;
extern double ffMinLegDist;
extern int    nKnicksBase;

struct sffTable                        // table that defines the number of turnpoints ifo flown distance
{ double Min;                          // At or obove this distance, use the number of tpt set in this record
  int    npt;                          // The number of allowed turnpoints, including start & finish
};
extern sffTable ffTable[5];            // Setup in InitffTable()
extern void InitffTable(void);         // Must be done after reading command line. Do in CalcFreeFlight

struct sffPt
{ int    i;                            // Index of the point in the soarblock ptA
  double ti;                           // Decimal time at this point
  double tLeg;                         // Decimal time over the leg from this ^point to the next
  double Dist;                         // Distance from this point to the next (wgs84)
  double Trk;                          // Track from this point to the next
  double Spd;                          // Speed from this point to the next
};



struct sFreeFlight
{ sffPt  TptA[ffMaxTpt];
  int    npt;                          // Number of points inTptA, including start and finish
  double Dist;                         // Total distance
  double Time;                         // Total time (decimal)
  double Speed;                        // Average speed
  double StartCrossAlt;                // Altitude at start of FF
  double StartAlt;                     // Lowest altitude between start soaring and StartCrossAlt
  double FinishCrossAlt;               // Altitude at finish of FF
  double FinishAlt;                    // Highest altitude between FinishCrossAlt and end of soaring
  double HeightLoss;
  double ScoreDist;                    // Including penalty for height loss
};


extern void CalcFreeFlight(void);

class CSoarBlock;

class CFreeFlight : public CFlightBase
{
 public:
  CFreeFlight(CSoarBlock *psb);
  ~CFreeFlight(void);

  void Calc(void);

  CffArray A;                          // Actual Free flight

 protected:
  CSoarBlock *pSB;
  sLoxo   *ptL;
  int      nptLR;                      // Number of points reserved in ptL
  CffArray K;                          // Knick points
  double  *DT;                         // Distance table
  int      nDT;                        // Number of reserved points in DT
  int dtIndex(int i, int j)   { return j * K.nA + i; }  // Index in the distance of i;j table
  double dtDist(int i, int j) { return DT [ dtIndex ( i, j ) ]; }    // Distnce i->j
  struct skOpt
  { int i;
    double Dist;                       // Dist from this point to the next
    double Sum;                        // Sum of all the legs previous to this point
  }    kOpt[ffMaxTpt],                 // Optimised knicks
       kOM[ffMaxTpt];                  // Debugging: contains knicks list of max dist
  void kRecurseBase1(void);            // Get the maximum dist along the knicks.  no ffMinLegDist
  void kRecurseBase2(void);            // Get the maximum dist along the knicks.  ffMinLegDist != 0
  inline void kRecurseB1(int k);       // Get the maximum dist along the knicks.  no ffMinLegDist
  inline void kRecurseB2(int k);       // Get the maximum dist along the knicks.  ffMinLegDist != 0
  inline void kOptDist(int i);         // Returns distance from kOpt[i]->kOpt[i+1]
  void MakeLoxo(void);                 // Generate loxodromic points array from ptA
  void MakeKnicks(void);               // Generate the knick points list (K)
  void FindA(int ntgt);                // Find the max free flight distance over ntgt turnpoints
  void Optimize(void);

  void DebugA(void);
  void DebugkOpt(void);
};


#endif
