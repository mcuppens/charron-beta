#ifndef FAST_TRACK_H
#define FAST_TRACK_H

extern int    SpeedComp;         // Flag set via command line: true: calculate, default = false
extern double FastTrackTime;     // Duration time lapse for fastest track calculation (decimal hour)
extern void CalcFastTrack(void);

struct sFastTrack
{ int    i0;                     // Index of the start of the fast track
  int    i2;                     // Index of the end  of the fast track
  double t0;                     // Time at i0
  double t2;                     // Time at i2
  double Dist;                   // Distance of the fast track, over the free flight turnpoints
  double Time;                   // time from i0 to i2, decimal hours
  double Speed;                  // Dist / Time
  int    Alt0;                   // Altitude at i0
  int    Alt2;                   // Altitude at i2
  int    HeightLoss;             // Alt0 - Alt2;
};


class CSoarBlock;

class CFastTrack : public CFlightBase
{
public:
  CFastTrack(CSoarBlock *psb);
  ~CFastTrack(void);

  sFreeFlight FF;
  sFastTrack  FT;

  void Calc(void);

 protected:
  CSoarBlock *pSB;
  int  ffi2;    // Last  available point: FF.TptA[FF.npt-1].i

  double GetDist(void);
};


#endif
