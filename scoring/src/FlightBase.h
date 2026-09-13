#ifndef FLIGHT_BASE_H
#define FLIGHT_BASE_H

#include "Global.h"

struct sCoord
{ double Lat, Lon;
};


struct sFlightPoint
{ sCoord pt;
  double Time;         // Decimal hours
  int    Altitude;     // pressure altitude
  int    AltiGPS;      // GPS altitude
  int    ENL;          // Engine ENL/RPM/EON
  int    MOP;
  double fENL;         // Filtered ENL / MOP
  WORD   Flag;         // See table below.  Combination of all values
  WORD   f3D;          // 3D fix accuracy
};

#define  FLT_ENG_ON       0x01
#define  FLT_ALT_BAD      0x02
#define  FLT_ALT_GPS_BAD  0x04


class CFlightBase
{
 public:
  CFlightBase(void);
  ~CFlightBase(void);

  sFlightPoint *ptA;
  int    npt;
  int    Start;                             // index of the first point of this object in ptA of the owner
  int    nAltBad;                           // The number of fixes with bad altitude
  double pAltBad;                           // The percentage of fixes with bad altitude

  void SetFlag(int i, DWORD Flag, BOOL bSet);
  BOOL GetFlag(int i, DWORD Flag)  { return ( ptA[i].Flag & Flag ) != 0; }
  void CountAltBad(void);

  int FixAtTime(double time, int d);        // Find the fix at the given decimal hour
  int FixAtdTime(int i0, double dTime, int d);  // Find the fix at dTime after fix i0
  int FixAtDist(int i0, double Dist);       // Find the closest fix at at least Dist from fix i0

  double OrthoDist(int p1, int p2);
  double OrthoTrk(int p1, int p2);
  double wgs84(int p1, int p2);;            // Calculate the distance using the WGS84 ellipsoide
  double dTime(int i1, int i2, double u = 1.);    // Delta time, in decimal hours
  double dAlt(int i1, int i2);       // From fix i1 to fix i2
  double Speed(int i);               // From fix i to next fix
  double Speed(int i1, int i2);      // From fix i1 to fix i2
  double Vz(int i);                  // From fix i to next fix
  double Vz(int i1, int i2);         // From fix i1 to fix i2
  char  *stHour(int i);

 protected:
  int nptR;
};



#endif
