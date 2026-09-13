#ifndef PLOT_FLIGHT_H
#define PLOT_FLIGHT_H

#include "cDislin.h"

const double maxdist_airf   = 5.;   // Maximum distance of airfield to select take-off/landing
const double maxdist_turnpt = 20.;  // Maximum distance of airfield selected by turn points

class CPlotFlight : public CDislin
{
public:
  CPlotFlight(CFlight *pf, char *fn);
  ~CPlotFlight(void);

  void Init(void);
  virtual void GetArea(void);
  virtual void DrawFrame(void);
  virtual void SelectAirfields(void);
  void DrawFlight(void);
  void DrawSoarBlock(CSoarBlock *pSB);
  void DrawDT(void);
  void DrawFF(void);
  void DrawTRI(void);
  void DrawTRIbf(void);
  void DrawBRU(void);
  void DrawKnicks(void);
  void DrawKnicksTRI(void);

 protected:
  CFlight *pF;
  BOOL     bDrawTask;
  // Color table
  int cFlight;
  int cRelease;
  int cEngine;
  int cFF;
  int cTRI;
  int cFFpt;
  int cFT;
  int cDT;
  int cBru;

  int  yLegend;              // The y coordinate of the first legend line
  void Legend(char * st, int c);

  int  GetAreaManual(void);
};


extern void PlotFlight(void);

#endif
