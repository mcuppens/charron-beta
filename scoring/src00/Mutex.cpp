#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#ifdef WIN32
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include "Global.h"
#include "Mutex.h"



#ifdef WIN32
struct timeval
{ long tv_sec;
  long tv_usec;
};

int gettimeofday(timeval *tv, void *tz)
{ tv->tv_sec  = (int)GetTickCountOwn();
  tv->tv_usec = ( tv->tv_sec % 1000 ) * 1000;
  tv->tv_sec /= 1000;
  return 0;
}

#endif



CMutex::CMutex(char *name)
{ F = 0;
  // Make sure the mutex is created in the general files directory
  strcpy ( Name, base_name );
  char *c = strrchr ( Name, PathDelimiter );
  if ( c )
  { c ++;
    *c = 0;
  }
  else *Name = 0;
  strcat ( Name, name );
}



CMutex::~CMutex(void)
{ Close();
}




BOOL CMutex::Wait(DWORD dwMilliseconds)
{ timeval tv;
  DWORD  tc2, tc;

  if (  F > 0   ) return TRUE;
  if ( !Name[0] )
  { printf ( "Mutex not initialised correctly\n" );
    return FALSE;
  }

  if ( gettimeofday ( &tv, NULL ) != 0 )
  { printf ( "Mutex error gettimeofday\n" );
    return FALSE;   // Error
  }
  tc2 = tv.tv_sec * 1000 + tv.tv_usec / 1000 + dwMilliseconds;

printf ( "Attempting to create mutex file, File name:\n%s\n", Name );
  while ( 1 )
  {
    if ( OpenFile() ) 
    { // The process now owns the mutex
printf ( "Mutex file created OK, mutex owned\n" );
      return TRUE;
    }

    // The mutex is owned by another thread / process
    // Wait until timeout, or 250 ms, whichever is less
    if ( gettimeofday ( &tv, NULL ) != 0 )
    { 
printf ( "\n" );
      printf ( "Mutex error gettimeofday\n" );
      return FALSE;   // Error
    }
    tc = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    if ( tc >= tc2 )
    {
printf ( "\n" );
      printf ( "Mutex timeout\n" );
/*
printf( "Checking if file exists\n" );
F = open ( Name, O_RDONLY );
if ( F > 0)
{ printf ( "File exists\n" );
  close ( F );
  F = 0;
  printf( "Attempting to remove file\n" );
  if ( remove ( Name ) )
  { printf( "Error removing mutex file\n" );
  }
}
else
{ printf ( "File did not exist or error opening, error code: %d\n", errno );
}
*/
      return FALSE;                    // Timeout
    }
    tc = tc2 - tc;
    if ( tc > 250 )  tc = 250;
    SleepOwn ( tc );
  }
}



int CMutex::OpenFile(void)
{
 #ifdef LOWLEVEL
  F = open ( Name, O_WRONLY | O_CREAT | O_EXCL, S_IWRITE );   // O_RDWR ??
  if ( F > 0 )  return 1;
  switch ( errno )
  { case EACCES: printf ( "A" );  break;
    case EEXIST: printf ( "E" );  break;
    case EINVAL: printf ( "I" );  break;
    case EMFILE: printf ( "F" );  break;
    case ENOENT: printf ( "N" );  break;
    default: printf ( "?" );
  }
 #else
  F = fopen ( Name, "r" );
  if ( F == 0 )
  { // The file did not exist: the mutex can be created and owned by this process
    F = fopen ( Name, "w" );
    if ( F == 0 )
    { // Error !! proceed as if mutex was created
      printf ( "Error creating mutex, proceeding as if created\n" );
    }
    return 1;
  }
  else
  { printf ( "E" );
    fclose ( (void**)&F );
  }
 #endif
  return 0;
}








void CMutex::Close(void)
{
 #ifdef LOWLEVEL
  if ( F >= 0 )
  { close ( F );
    F = 1;
  }
 #else
  if ( F != 0 )
  { fclose ( (void**)&F );
    F = (FILE*)1;
  }
 #endif
  if ( F > 0 )
  { F = 0;
printf ( "Closing mutex file\n" );
    if ( remove ( Name ) )   printf ( "Error closing mutex\n" );
  }
}

