#pragma once

struct sStretch
{ char Name[64];
  sCoord pt[2];
  double Width;  // Total width (= radius x 2)
  double D, Track;
  double Sector[2];
  double tTotal;
  double aSpeed;
  int    nLaps;
};




class CRuns
{
 public:
  CRuns(void);
  ~CRuns(void);

  sStretch *Stretches;
  int nStretches;

  void ReadStretches(void);
  void Evaluate(CSoarBlock *psb, int iA);

 protected:
  sStretch Stretch;
  CSoarBlock *pSB;
  sFlightPoint *ptA;                  // points to pSB->ptA, for easier editing
  int npt;                            // copy of pSB->npt, for easier editing
  int nAR;

  int Read1(char *Line);
  int CheckLine(int i, int n);
};


extern CRuns Runs;
extern void CalcRuns(void);
