#ifdef WIN32

#ifndef WIN32_H
#define WIN32_H

#include <io.h>
#include <conio.h>

#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable: 4996)
#pragma warning (disable: 4305)   // 'function' : truncation from 'double' to 'float'

#define NULL   0

#define open  _open
#define close _close
#define O_CREAT   _O_CREAT
#define O_EXCL    _O_EXCL
#define S_IWRITE  _S_IWRITE

extern void ExitWin32(void);

#endif

#endif
