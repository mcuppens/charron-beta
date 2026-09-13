#ifndef RELEASE_H
#define RELEASE_H

extern double tReleaseManual;

const double TmaxInterval     = 10.;    // Maximum (nominal) sampling interval allowed (s)
const double TInterval_Trsh   = 10.;    // Threshold for total time exceeding interval limit
const double turnrate_Trsh_Hi = 11.5;   // High turn rate threshold for start of turn (deg/s)
const double turnrate_Trsh_Lo =  4.;    // Low  turn rate threshold to stop turning (deg/s)
const double Tstraight_Trsh   =  7.;    // Time threshold for stopping turn phase (s)
const double Tturning_Trsh    =  0.;    // Time threshold for rate of turn detection (s)
const double hdrop_Trsh       = 50.;    // Maximum altitude drop to detect release (m)



class CRelease : public CFlightBase
{
 public:
  CRelease(void);
  ~CRelease(void);

  void Calc(CFlight *pF);

 protected:
  CFlight *pF;
  double MinAlt;                // Minimum alt of release. 0 or TO elevation + 200
  char   message[MAXLEN+1];
  int    i0, i, i1;
  BOOL   ErrInterv;             // TRUE if fix interval warning was issued
  double dt, vz, v, tr;
  double dtMin;                 // Minimum time between fixes
  double dtTooLong;             // Counter to check if fix interval isn't too long
  double a;                     // General purpose
  // Delta vz algo params
  double vzI_D;
  // vz Int algo params
  double vzI_I;
  // Alt drop algo params
  double hmin, hmax, hdrop;
  int    ihmax;
  // ROT algo params
  double tr0, ROT, Bank;
  int    iStraight, iTurning;
  // GS algo params
  double trMin, trMax;          // Low ground speed - minimum track
  int    nLowspeed;
  // Release triggering
  struct sTrigger
  { double dtLim;               // Limit dt to trigget release
    int    nLim;                // Minimum number of fixes to trigger release
    int    iRelease;            // The first fix triggering this type
  }        Trigger[4];          // One set of trigger limits  for every release type detection, seconds
  eReleaseType Type;

  void Init(void);
  BOOL LowAccuracy;
  void GetFixAccuracy(void);
  BOOL GetFix(void);            // get the next fix indexes for calulation
  void CalcVzI_D(void);
  void CalcVzI_I(void);
  void GetFixData(void);        // get ground speed, ROT, Vz, dAlt
  void CheckVz_D(void);
  void CheckVz_I(void);
  void CheckAD(void);           // Altitude drop
  void CheckROT(void);
  void CheckGS(void);
  BOOL CheckRelease(void);

  // Debugging data
  char sti[64];
};

extern void FindRelease(void);  // Find release of all flights

#endif
