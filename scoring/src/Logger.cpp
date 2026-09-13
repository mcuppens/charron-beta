#include "Logger.h"

sLogger *pLogger = NULL;
char LoggerId[5] = { 0 };       // Unique logger serial number (alphanumeric)


sLogger LoggerList[NLOGGERS]=
{ // It is not required to add a new logger in LoggerList in the same
  // sequence as in eLoggerType
  LT_AIRCOTEC,          0,  0,  0x00, 0x00, "ACT", 'i', "Aircotec",
  LT_CAMBRIDGE,         1,  0,  0x00, 0x00, "CAM", 'c', "Cambridge",
  LT_CLEARNAV,          1,  0,  0x00, 0x00, "CNI", 'k', "Clearnav",
  LT_DATASWAN,          1,  0,  0x00, 0x00, "DSX", 'd', "Data Swan",
  LT_EW,                1,  0,  0x00, 0x00, "EWA", 'e', "EW",
  LT_FILSER,            1,  0,  0x41, 0x42, "FIL", 'f', "Filser",
  LT_FLARM,             0,  0,  0x41, 0x42, "FLA", 'g', "Flarm",
  LT_FLARM_NON_IGC,     0,  0,  0x41, 0x42, "XFL", 'x', "Flarm, non IGC",
  LT_GARRECHT,          1,  0,  0x00, 0x00, "GCS", 'a', "Garrecht",
  LT_IMI,               0,  0,  0x00, 0x00, "IMI", 'm', "IMI gliding equipment",
  LT_LX_NAV_V,          1,  0,  0x00, 0x00, "LXV", 'v', "LX Navigation d.o.o.",
  LT_LX_NAV,            1,  0,  0x00, 0x00, "LXN", 'l', "LX Navigation",
  L_NAVITER,            1,  0,  0x00, 0x00, "NAV", ' ', "Naviter d.o.o.",
  LT_NEWTECH,           0,  0,  0x00, 0x10, "NTE", 'n', "New Technologies",
  LT_NIELSEN_KELLERMAN, 0,  0,  0x00, 0x00, "NKL", '_', "Nielsen Kellerman",
  LT_PESCHGES,          1,  0,  0x00, 0x00, "PES", 'p', "Peschges",
  LT_PFE,               1,  0,  0x00, 0x00, "PFE", ' ', "Press Finish Electronics",
  LT_PRINT,             1,  0,  0x00, 0x00, "PRT", 'r', "Print Technik",
  LT_SCHEFFEL,          0,  0,  0x00, 0x00, "SCH", 'h', "Scheffel",
  LT_SDI,               1,  0,  0x00, 0x00, "SDI", 's', "Streamline Digital Instr.",
  LT_TRIADIS,           0,  0,  0x00, 0x00, "TRI", 't', "Triadis Engineering GmbH",
  LT_WESTERBOER,        0,  0,  0x00, 0x00, "WES", 'w', "Westerboer",
  LT_ZANDER,            1,  0,  0x00, 0x10, "ZAN", 'z', "Zander"
};

sLogger LoggerX = // Unknown logger type
{ LT_X,  // eLoggerType Type
  0,     // Security
  0,     // TextFile
  0 , 0, // GoodValue, BadValue
  "---", // Code
  ' ',   // Letter
  "Unknow logger type"
};
