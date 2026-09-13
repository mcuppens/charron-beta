#include "Region.h"

#include "Airfields.h"


eRegion GetRegion(sCoord pt)
{ int AirfTO = Airfields.Find ( &pt, AirfTO_Radius );
  if ( Airfields.List[AirfTO].BNL )  return BENELUX;

  // Based upon ICA code
  char st[4] = { 0 };
  strncpy ( st, Airfields.List[AirfTO].icao, 2 );
  str_upper ( st );
  if ( !strcmp ( st, "LF" ) )  return FRANCE;
  if ( !strcmp ( st, "GM" ) )  return MOROCCO;
  if ( *st == 'K' )            return USA;

  // Based upon coordinates
  if ( pt.Lat >  35.  &&  pt.Lon > -10. ) return EUROPE;
  if ( pt.Lat < -17.  &&  pt.Lat > -37.  &&  pt.Lon > 10.  &&  pt.Lon < 43. ) return SOUTH_AFRICA;
  if ( pt.Lat < -10.  &&  pt.Lon > 110. ) return AUSTRALIA;
  if ( pt.Lat >  22.  &&  pt.Lon <  40. ) return USA;

  return WORLD;
}



int IncludeRegion(eRegion Flt, eRegion Plot)
{ if ( Plot == Flt    )   return TRUE;
  if ( Plot == EUROPE )
  { if ( Flt == BENELUX ) return TRUE;
    if ( Flt == FRANCE  ) return TRUE;
  }
  if ( Plot == WORLD )   return 1;

  return 0;
}



