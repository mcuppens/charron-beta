#ifndef FREE_FLIGHT_H
#define FREE_FLIGHT_H


const int    ffMaxTpt      = 7;
const double ffExtraPtDist = 200.;     // Below this distance, one less tpt is allowed


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


class CFreeFlight : public CFlightBase
{
 public:
  CFreeFlight(sFlightPoint *pta, int n);
  ~CFreeFlight(void);

  sffPt  TPT[ffMaxTpt];                // Don't use sFreeFlight for easier writing
  int    nTPT;
  int    ntgt;                         // The target number of turnpoints
  double Dist;

  void Calc(void);

 protected:
  int SeqA[ffMaxTpt];                  // Keep tracks of the creation sequence of the turnpoints.  Allows to delete the oldest pt
  int nSeq;                            // The sequence number of the last adde point in TPT

  void Optimize(void);
  void InsertMaxPt(void);              // Insert the point that yields the maximum distance

  // Basic TPT array functions
  void Insert(int Index, int p);       // Insert point p at index
  void Insert(int p);                  // Insert point p, finds out where to insert
  void Delete(int Index);              // Delete the point at index
  void DeleteOldest(void);             // Delete the oldest point
  void CalcDist(int Index);            // Calculates the distance of leg Index -> Index + 1.  If Index = -1: Calculates all legs
};


#endif
