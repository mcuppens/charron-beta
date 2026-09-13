#ifndef WRITE_TEXT_H
#define WRITE_TEXT_H

extern void WriteText(void);

extern char *stPos(CFlightBase *p, int i, BOOL bSplit = FALSE);
extern char *stPosTask(int i, BOOL bSplit = FALSE);
extern char *stPos(sCoord *p, BOOL bSplit = FALSE);

#endif
