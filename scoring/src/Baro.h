#ifndef BARO_H
#define BARO_H

#include "gd.h"

class CBaro
{
public:
  CBaro(void);
  ~CBaro(void);

  void Draw(void);

 protected:
  gdImagePtr im;
  int    Scale;
  int    xoffset;
  int    xmax;
  int    yoffset;
  int    ymax;
  int    xLegend;
  int    Red, green, blue, black, white, Orange, cyan, pink, yellow, ltyellow, magenta, SkyBlue, ltSkyBlue, BlueSpd, grey, greylt, greendk, maroon;
  int    styleDashed[6];
  BOOL   bENL;
  BOOL   bMOP;
  double RangeENL;
  double ScaleENL;
  double alti_min, alti_max, alti_scale;
  double time_first, time_last, time_scale;
  gdFontPtr Font;

  BOOL Init(void);
  void DrawFlights(int Color);
  void DrawSoaring(int Color);
  void DrawEngine(int Color);
  void DrawENL(int Color);
  void DrawMOP(int Color);
  void DrawMOPline(int Color);
  void DrawENLThreshold(int Color);
  void DrawfENL(int Color);
  void DrawRelease(int Color);
  void DrawGPS(int Color);
  void DrawAlt(int Color);
  void DrawSpeed(int Color);
  void DrawAltGrid(int Color);
  void DrawTimeGrid(int Color);
  void WriteTitle(int Color);
  void DrawAltGain(int Color);
  void DrawFF(int Color);
  void DrawDT(int Color);
  void gdImagePngOwn(gdImagePtr im, FILE *F);
  void WriteLegend(const char *st, int Color);

 private:
  int X(CFlightBase *pB, int i);
  int X(double t);
  int Y(double a);
  int yAlt(CFlightBase *pB, int i);
  int yGPS(CFlightBase *pB, int i);
  int yENL(double ENL);
};

extern void PlotBaro(void);

#endif // BARO_H

