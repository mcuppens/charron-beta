#ifndef WIN32
# include <sys/time.h>
#endif
#include <time.h>


#ifndef WIN32
#include <unistd.h>
#include "Linux.h"

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
unsigned int GetTickCountOwn(void) 
{
  timespec now;
  if ( clock_gettime ( CLOCK_MONOTONIC, &now ) )
    return 0;
  return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

void SleepOwn(DWORD ms)
{ usleep ( ( ms + 500 ) / 1000 );
}


void DebugBreakOwn(void)
{ int *ip = NULL;
//  int i = *ip;
}


#endif



