#ifndef SOARBLOCK_H
#define SOARBLOCK_H

#include "FreeFlight.h"
#include "FastTrack.h"
#include "Task.h"
#include "Triangle.h"
#include "Brussel.h"


class CFlight;        // Forward declaration in order to be able to declare CSoarBlock::pOwner

class CSoarBlock : public CFlightBase
{
public:
  CSoarBlock(void);
  ~CSoarBlock(void);

  CFlight *pOwner;    // Pointer to the Flight object that owns this block
  int      nrSeq;     // Sequence number of this block in the flight (zero based)

  // Altitude gain variables
  double MaxAlt;      // Maximum absolute altitude;
  int    iLow;        // Index of the lowest point of the altitude gain
  int    iHigh;       // Index of the lowest point of the altitude gain
  double AltGain;     // Maximum altitude gain

  void CalcAltGain(void);
  void CalcFF(void);  // Caolculates various details after optimalisation by CFreeFlight

  sFreeFlight   FF;
  sFastTrack    FT;
  sTaskAnalysis DT;
  sTriangle     TRI;
  sBrussel      BRU;

  // For debugging only
  int *Knicks;
  int nKnicks;
};


extern CSoarBlock *sbMaxT;    // This pointer will receive the soaring block with the longest time
extern CSoarBlock *sbMaxAG;   // This pointer will receive the soaring block with the biggest altitude gain
extern CSoarBlock *sbMaxFF;   // The soaring block containing the maximum soaring distance
extern CSoarBlock *sbMaxDT;   // The soaring block containing the best declared task performance
extern CSoarBlock *sbMaxTRI;  // The soaring block containing the best triangle performance
extern void FindLongestSoarBlock(void);
extern void LimitFixes(void);
extern void CalcAltGain(void);

#endif
