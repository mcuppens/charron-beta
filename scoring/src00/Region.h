#pragma once

#include "Global.h"
#include "Flight.h"

enum eRegion { WORLD, BENELUX, FRANCE, EUROPE, SOUTH_AFRICA, MOROCCO, AUSTRALIA, USA };
// Strings:    WLD    BNL      FRA     EUR     SAF           MOR      AUS        USA

extern eRegion GetRegion(sCoord pt);
extern int IncludeRegion(eRegion Flt, eRegion Plot);

