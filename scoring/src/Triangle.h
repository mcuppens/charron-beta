#pragma once


// Triangle results
struct sTriangle
{ int    Index[3];
  double Legs[3];
  double f[3];
  double TotDist;
};
extern sTriangle TriM;
extern sTriangle TriBF;  // Brute Force



class CTriangle
{
public:
  CTriangle(void);
  ~CTriangle(void);

  void Evaluate(CSoarBlock *psb);
  void EvaluateBF(CSoarBlock *psb);  // Brute Force algo

  int *Knicks;
  int nKnicks;

 protected:
  CSoarBlock *psb;
  int nKnicksR;

  int i0, i2, npt;

  struct sTable
  { int *A;
    int nA, nAR;
  };
  sTable *tS;                       // Source table
  sTable *tD;                       // Destination table
  void tInsert(int iT, int isb);    // Insert point isb at tD->A[iT]
  void MakeKnicks(void);

  sTriangle tri;
  sTriangle tM[3];
  int       ntM;
  void SetMax(void);
  BOOL FindClosed(void);
  void Optimise(sTriangle *pM, int r, int step);
  void Optimise1(sTriangle *pM, int r, int step);
  void Calc(void);

};


extern void CalcTriangle(void);



class CTriDebug
{ // Stores knicktable used for triangle calculation
  // Uses the destructor to cleanup upon shutdown
 public:
  CTriDebug(void);
  ~CTriDebug(void);

  int *Knicks;
  int nKnicks;
};

extern CTriDebug TriDebug;
