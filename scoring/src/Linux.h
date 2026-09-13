#ifndef LINUX_H
#define LINUX_H


typedef void *HANDLE;
typedef unsigned int   UINT;
typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef int BOOL;
typedef long LONG;

struct sPOINT { LONG  x;  LONG  y; };
struct sSIZE  { LONG cx;  LONG cy; };

#define FALSE  0
#define TRUE   1

extern unsigned int GetTickCountOwn(void);
extern void SleepOwn(DWORD ms);

#endif
