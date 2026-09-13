#ifndef CFF_ARRAY_H
#define CFF_ARRAY_H


struct sffA
{ int    i;                                 // Index of the point in the soarblock ptA
  int    Seq;
  double Dist;                              // Distance from this point to the next
  double Loxo;                              // Loxo Distance from this point to the next
};

struct sLoxo { double x, y; };

class CffArray : public CFlightBase
{
public:
  CffArray(void);
  ~CffArray(void);

  void Init(sLoxo *pl, sFlightPoint *pta, int npta, int nR, int nTarget);

  sffA  *A;
  int    nA;                                // Actual number of valid points in A
  int    ntgt;                              // Target number of points in A
  double Dist;                              // Total distance over A
  double dMin;                              // Minimum distance of a leg

  int  InsertMaxPt(int i0 = -1, int i2 = -1); // Insert the point that yields the maximum distance
  int  InsertMaxPtLoxo(int i0 = -1, int i2 = -1); // Insert the point that yields the maximum distance
  void CalcDist(int Index);                 // Calculates the distance of leg Index -> Index + 1.  If Index = -1: Calculates all legs
  BOOL SplitOptimal(void);
  void SplitLeg(int i);                     // Insert two waypoints the leg i

  // Basic TPT array functions
  void Insert(int Index, int p);            // Insert point p at index
  int  Insert(int p);                       // Insert point p, finds out where to insert
  void SplitLegs(double dM, int nkM);       // Spit long legs
  void Delete(int Index);                   // Delete the point at index
  void DeleteOldest(void);                  // Delete the oldest point
  void DeleteLeast(void);                   // Delete the least relevant turnpoint

 protected:
  double Loxo;                              // Loxo Dist
  int    nR;                                // Reserved number of points in A
  int    Seq;                               // Creation sequence counter
  struct sSplit
  { int    i;                               // The from turnpoint of the leg
    int    s1, s2;                          // The split points
    double Gain;                            // The gain obtained by this split
  } Split;

  sLoxo *ptL;                               // Loxodromic points array. Not allocated but set to address

 private:
  double LoxoDist(int i1, int i2);          // Calculate loxo dist between points in ptL
};

#endif // CFF_ARRAY_H
