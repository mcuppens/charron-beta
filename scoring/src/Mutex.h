#ifndef MUTEX_H
#define MUTEX_H

// For interprocess mutexing.  See:
// http://www.ibm.com/developerworks/linux/library/l-ipc2lin3/index.html


//#define LOWLEVEL

// This class will make sure hat everything is cleaned up upon exit
class CMutex
{
 public:
  CMutex(char *Name);
  ~CMutex(void);

  BOOL Wait(DWORD TimeOut);   // ms
  void Close(void);

 protected:
  char Name[256];
 #ifdef LOWLEVEL
  int  F;
 #else
  FILE *F;
 #endif

 int OpenFile(void);
};


#endif
