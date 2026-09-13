#ifndef BRUSSEL_H
#define BRUSSEL_H

#include "SoarBlock.h"


struct sBrussel
{ double t1;            // First angle (min or max, whichever is first
  double t2;            // Second angle (min or max, whichever is first
  double Angle;         // t2 - t1.  After calc, t1 & t2 are caseted 0 -> 360
  int    i1;            // Index of t1 in SoarBlock
  int    i2;            // Index of t2 in SoarBlock
};



class CBrussel
{
public:
  CBrussel(CSoarBlock *psb);
  ~CBrussel(void);

  static sCoord ptCtr;

  void Calc(void);

 protected:
  CSoarBlock *pSB;
};

extern void Charronde(void);


#endif // BRUSSEL_H
