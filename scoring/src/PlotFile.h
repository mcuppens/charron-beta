#ifndef DAY_FILE_H
#define DAY_FILE_H

#include "cDislin.h"
#include "Mutex.h"
#include "Region.h"

const char PlotFileVersion[] = "3.0";

extern void ExportFlight(void);


struct sFlightHeader
{ char  Delimiter[16];
  int   npt;
  int   PointIndex;       // Index to the coordinates.  Will be set upon reading
  float ffDist;
  float dtDist;
  float LatMin, LatMax;   // in radians
  float LonMin, LonMax;
  sCoord pt0;             // 
  float Reserved[4];
};



class CPlotList
{public:
  CPlotList(void);
  ~CPlotList(void);

  struct sList
  { char    FF0[MAXLEN];   // The name of the flights file without path
    char    FF[MAXLEN];    // The name of the flights file including path
    char    FP0[MAXLEN];   // The name of the plot without path
    char    FP[MAXLEN];    // The name of the plot including path
    char    TL[MAXLEN];    // The title of the plot
    eRegion Region;
  } *List;
  int nList;

  int Add(char *name, char *rgn);

  // Current flight data
  sFlightHeader H;
  float *Lat;
  float *Lon;
  double Resolution;
  int LoadFlight(void);

 protected:
  int nListR;
};

extern CPlotList PlotList;





class CPlotFile : public CDislin
{
 public:
  CPlotFile(char *fn, int n);
  ~CPlotFile(void);


 protected:
  FILE *F;
  sFlightHeader *HA;
  int   nH;
  int   nHR;
  float MaxFF;
  CPlotList::sList *pPlot;
  CMutex *pMutex;

  void TreatFlight();
  int  GetFlights(void);
  void GetArea(void);
//  void Plot(void);
  void DrawFlights(void);
  void nFlts(void);
};


#endif
