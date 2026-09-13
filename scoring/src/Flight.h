#ifndef FLIGHT_H
#define FLIGHT_H

#include "Trace.h"
#include "Engine.h"
#include "SoarBlock.h"

enum eReleaseType { RT_DVZ, RT_VZI, RT_ROT, RT_ALT, RT_GS };  // Types of release detection

class CFlight : public CFlightBase
{
public:
  CFlight(int First, int Last, BOOL bGap);
  ~CFlight(void);

  int    nFlight;         // The number of this flight in FlightA
  int    nBaseLine;       // Number of base line fixes
  double dtBaseLine;      // Duration of the base line, minutes
  double AltBaseLine;     // Average altitude of the base line
  int    Release;
  eReleaseType ReleaseTrigger;  // Code for type of relase detection
  BOOL   bRelease;        // FALSE if release point not found
  double FlightTime;      // take-off to landing, decimal hours
  double FixInterval;     // Average over whole flight, seconds
  double SamplesPerHour;  // Average over whole flight

  int    AirfTO;          // Index of the take-off airfield in CAirfields::List.  -1 if not found

  // Engine related params & functions
  int      EngRuns;       // Number of engine runs
  int      EngTest;       // Index of the test run.  -1 if no test run
  double   EngTimeTot;    // Total engine run time
  double   EngTimeMax;    // Time of longest run
  double   EngClimbTot;   // Total altitude gain with engine
  double   EngClimbMax;   // Maximum altitude of a single run
  double   EngDistTot;    // Total distance flown with engine (straight lines / run)
  double   EngDistMax;    // Maximum distance flown of a single run (straight line)
  sEngine *EngA;

  void GetBaseLine(void); // Get the a&ltitude of the fixes before the take-off point
  void AdjustLdg(void);   // Find the correct landing time by running backwards from the end
  void AddEngine(sEngine *E);

  // SoarBlock realted params
  CSoarBlock **SB;
  int    nSB;
  int    StartSoar;
  double MaxAlt;          // Maximum absolute altitude;
  int    iAltGainMax;     // Index of the soaring block with the max altgain.  -1 if not set
  void   AddSoarBlock(int First, int Last );

  void   SetSoarBlocks(void);
  void   CalcAltGain(void);

 protected:
  int  nSBR;
  BOOL bGap;              // True if the flight was started after a gap in the logger trace

  void Init(void);
  void CheckAlt(void);     // Check altitude validity by comparing to previous (filtered) alt 
  BOOL CheckAltGpsDif(void);  // a single llop for checking altitude versus GPS & filter out bad fixes
  void CheckAltGps(void);  // Check gps altitude validity by comparing to previous (filtered) alt 
};


extern CFlight **FlightA;
extern int nFlights;


#endif

