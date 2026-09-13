#ifndef IGC_H
#define IGC_H

extern BOOL CheckDeclar;
extern BOOL bNoENL;        // Allow file without ENL registration
extern BOOL bNoMOP;        // Do not use MOP
extern BOOL bMOPonly;      // Analyse only flights with MOP detection
extern BOOL no_GID;        // Allow file without Glider ID

extern int readfile_IGC(void);

class CIGC
{public:
  CIGC(void);
  ~CIGC(void);

  void CheckFile(void);
  int Read(void);

 protected:
  FILE *F;
  char  Line[MAXLEN+1];
  int   nExtension;
  int   ExtFirst, ExtLast;

  int   dFR_ID;
  int   dPLT, dGID, dGTY, dCCL;

  int read_Hrecord(char *data);
  void GetSN(char *st, char *st2);
  void Check(void);
};

#endif
