#ifndef TRACE_H
#define TRACE_H

#include "FlightBase.h"

/*
CTrace
This class contains all the points og the logger trace
It may contain severeal flights
All invalid fixes are removed
- non 3D fixes
- Position jumps
*/
class CTrace : public CFlightBase
{
 public:
  CTrace(void);
  ~CTrace(void);

  char Pilot[LREC];       // Name of pilot in header
  char GliderType[LREC];  // Type of glider
  char GliderReg[LREC];   // Registration of glider
  char CompClass[LREC];   // Competition class
  char MOPsensor[LREC];
  int  Year,Month,Day;    // Date of flight
  BOOL bMOP;              // TRUE if file contains MOP detection

  void Init(void);        // Get basic flight data
  void AddFix(sFlightPoint *pt);

 protected:
  double Time0;             // For fix time validity checking
  double d24;               // Correction for passing midnight
  int    bInterupted;       // Will be set true if log file was interupted

  void GetFlights(void);
  void AddFlight(int Start, int End, BOOL bGap);
  void DeleteFix(int n);
};

extern CTrace Trace;

#endif