#ifndef LOGGER_H
#define LOGGER_H

#include "Global.h"

// Logger data
//
// To add a logger:
// - Increment NLOGGERS, below
// - Add a logger enumeration in eLoggerType, below
// - Add the logger parameters in the definition of 
//   LoggerList in Logger.cpp
//   It is not required to add the logger in LoggerList in the same
//   sequence as in eLoggerType

const int NLOGGERS = 23;

enum eLoggerType {
  // IGC certified loggers
  LT_AIRCOTEC,
  LT_CAMBRIDGE,
  LT_CLEARNAV,
  LT_DATASWAN,
  LT_EW,
  LT_FILSER,
  LT_FLARM,
  LT_FLARM_NON_IGC,
  LT_GARRECHT,
  LT_IMI,
  LT_LX_NAV_V,
  LT_LX_NAV,
  L_NAVITER,
  LT_NEWTECH,
  LT_NIELSEN_KELLERMAN,
  LT_PESCHGES,
  LT_PFE,
  LT_PRINT,
  LT_SCHEFFEL,
  LT_SDI,
  LT_TRIADIS,
  LT_WESTERBOER,
  LT_ZANDER,
  /* Non IGC certified loggers */
  LT_UNKNOWN,
  LT_XXX,
  LT_BAL,
  LT_BORGELT,
  LT_COLIBRI,
  LT_DELVER,
  LT_GARMIN,
  LT_GRIFFIN,
  LT_ILEC,
  LT_LX4000,
  LT_LXGPS,
  LT_SOAR,
  LT_VOLKSLOGGER,
  LT_X
} ;

struct sLogger
{ enum  eLoggerType Type;
  int   Security;
  int   TextFile;
  int   GoodValue, BadValue;
  char  Code[4];
  char  Letter;
  char  Name[LREC];
};

extern sLogger  LoggerList[NLOGGERS];
extern sLogger  LoggerX;       // Unknown logger type
extern sLogger *pLogger;       // Pointer of the actual logger type in the list
extern char LoggerId[5];       // Unique logger serial number (alphanumeric)

#endif
