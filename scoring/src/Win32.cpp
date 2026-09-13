#ifdef WIN32

#include "Global.h"


void ExitWin32(void)
{ if ( OutF != NULL )
  { OutF = freopen ( "CON", "w", stdout );
//    printf ( "Console output was directed to text file\n" );
  }

  if ( bPause )
  { while ( _kbhit() )  _getch ();
    printf ( "\nPress any key" );
    _getch ();
  }
}


#include <windows.h>


unsigned int GetTickCountOwn(void)
{ return GetTickCount();
}

void SleepOwn(DWORD ms)
{ Sleep ( ms );
}


void DebugBreakOwn(void)
{ int *ip = NULL;
//  int i = *ip;
//DebugBreak();
//throw(0);
}


#endif // WIN32
