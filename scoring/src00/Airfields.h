#ifndef AIRFIELDS_H
#define AIRFIELDS_H


const double AirfTO_Radius = 5.;     // The takeoff airfield must be located within this distance


struct sAirfield
{ sCoord pt;          // Latitude, longitude of airfield
  int    level;       // Level of importance of airfield
  int    sel;         // Airfield selection for plotting
  int    BNL;         // Airfield in BeNeLux if = 1
  char   name[40];    // Airfield name
  char   icao[5];     // Airfield ICAO code
};




class CAirfields
{
 public:
  CAirfields(void);
  ~CAirfields(void);

  sAirfield *List;
  int nList;

  void Read(void);
  void ResetSel(void);
  int Find(sCoord *pt, double r);   // Find the airfield closest to position pt, invalid (0) if beyond radius r
                                    // The first airfield in the list must be "zzzz"

 protected:
  int ReadAirfield(FILE *F, sAirfield *afd);
};

extern CAirfields Airfields;

#endif
