#ifndef ENGINE_H
#define ENGINE_H


const double EngineTimeThreshold = 36.;   // Below this time, engine run is rejected (seconds)

enum eEngineDet
{ ED_NONE,               // None
  ED_ENL,                // ENL only
  ED_MOP,                // MOP only
  ED_ENL_MOP,            // ENL & MOP
  ED_RPM,                // RPM
  ED_EON,                // ON-OFF
  ED_MAN                 // Manual
};


struct sEngine
{ int  On;                // First fix with engine switched ON
  int  Off;               // First fix with engine switched OFF
};



class CEngine
{
public:
  CEngine(void);
  ~CEngine(void);

  // Settings set via the command line
  BOOL   Installed;       // FALSE by default, must be selected TRUE via the command line
  BOOL   InstAuto;        // If TRUE, automatically determine if an engine is installed. FALSE by default
  BOOL   SkipTestRun;     // If TRUE, a test run is not required
  double Threshold_Min;
  double Threshold_Max;
  double Threshold_Man;
  eEngineDet Detection;   // Engine detection

  // Statistics resulting from analysis over all flights
  double MinENL, MaxENL;
  double Average;
  double Median;
  double Sigma;
  double Threshold;       // Threshold used for engine detection.  Can be ENL, RPM, ...

  BOOL Analyse(void);

 protected:
  void Filter(void);
  void Statistics(void);
  void GetThreshold(void);
  void SetFlag(void);
  void RemoveShort(double tt);
  void RemovePerformance(double vLim, double vzLim);
  void Consolidate(double tt1, double tt2);
  void SetFlights(void);
  void Debug(void);       // Lists all remining engine running blocks
};

extern CEngine Engine;

extern void AnalyseEngine(void);

#endif
