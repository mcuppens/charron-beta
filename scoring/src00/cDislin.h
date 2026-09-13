#ifndef DISLIN_H
#define DISLIN_H



class CDislin
{
 public:
  CDislin(char *fn);
  ~CDislin(void);

  // parameters
  int    nAirf_maxplot;                // Maximum number of airfields to plot selected by level
  int Scale;

  virtual void StartPlot(void);
  virtual void GetArea(void);
  virtual void GetPlotArea(void);
  virtual void SelectAirfields(void);
  virtual void DrawAirfields(void);
  virtual void DrawCountries(int c);
  virtual void PlotScale(void);
  void DrawFrame(char *Title);
  void DrawPalette(void);
  void DrawSymbols(void);
void TestLon(float lon);

  float dec(double d) { return (float) ( d * c180divPi ); }

 protected:
  char   FN[MAXLEN];
  float  LatMin, LonMin, LatMax, LonMax;
  float  PlotLatMin, PlotLatMax, PlotLonMin, PlotLonMax;
  float  ViewLatMin, ViewLatMax, ViewLonMin, ViewLonMax;
  float  LatStep, LonStep;
  float  LatWidth;
  sPOINT tlMap;                        // Top left point on the map (plot coordinates)
  sSIZE  sMap;                         // Size of the map (plot coordinates)
  int    nAirfSel;
  float *Lat, *Lon;
  int    npt, nptR;

  void SetFont(void);                  // Set the default font
  void Symbol(int n, int h, float Lat, float Lon);
  void AllocLatLon(int n);
  void AllocLatLon(sFlightPoint *ptA, int n);
  void DebugWorld(void);
};


#endif
