#ifndef MESSAGES_H
#define MESSAGES_H

#include "Global.h"

/**********************/
/* Error codes        */
/**********************/

#define  NERRORS (int) 64    // Number of error codes defined below

#define  EC_NIL              (int) 0     // No error
#define  EC_UNDEFINED        (int) 1     // Undefined error
#define  EC_NO_ARGUMENTS     (int) 2     // No arguments given
#define  EC_UNDEF_ARGUMENT   (int) 3     // Argument undefined
#define  EC_ARG_OUT_RANGE    (int) 4     // Argument out of range
#define  EC_MEMORY           (int) 5     // Memory allocation error
#define  EC_ERRORCODE        (int) 6     // Undefined error code given
#define  EC_MUTEX_TO         (int) 7     // Mutex timeout

// Command line parameter errors
#define  EC_CL_NO_FILE       (int) 100   // No IGC file specified
#define  EC_CL_FN_SHORT      (int) 101   // IGC file name too short
#define  EC_CL_FN_LONG       (int) 102   // IGC file name too long
#define  EC_CL_PN_LONG       (int) 103   // SCOREDIR/filename.igc too long
#define  EC_CL_PL_AREA       (int) 104   // Plot area definition not complete
#define  EC_CL_PL_AREA_ERROR (int) 105   // Inconsistent plot area definition

// File IO errors
#define  EC_IGC_NOT_OPEN     (int) 110   // IGC file could not be opened
#define  EC_IGC_INVALID      (int) 111   // IGC file security invalid
#define  EC_IGC_TOO_LONG     (int) 112   // IGC file dataline too long
#define  EC_AIRF_NOT_OPEN    (int) 113   // Airfield file could not be opened
#define  EC_AIRF_F_EMTY      (int) 114   // Airfield file empty
#define  EC_AIRF_F_ERROR     (int) 115   // Error reading airfield file
  
// Logger errors
#define  EC_LOGGER           (int) 120   // Undefined logger type
#define  EC_GPS_DATUM        (int) 121   // GPS datum != WGS-1984
#define  EC_SHORT_FLIGHT     (int) 122   // Flight too short
#define  EC_RELEASE_POINT    (int) 123   // No release point found
#define  EC_SAMPLE_TOOLONG   (int) 124   // Sampling interval exceeds limit
#define  EC_NO_ENL           (int) 125   // No ENL signal recorded
#define  EC_NO_VALID3DFIX    (int) 126   // No single valid 3D fix
#define  EC_NO_ENL_OVRD      (int) 127   // No ENL signal, scoring forced
#define  EC_GPS_DATUM_NSTD   (int) 128   // GPS datum format not standard
#define  EC_IGC_FILE_ERROR   (int) 129   // Invalid character or line length in IGC file   // new
 
// Declaration errors
#define  EC_PILOT_NAME       (int) 130   // Undefined pilot name
#define  EC_GLIDER_TYPE      (int) 131   // Undefined glider type
#define  EC_GLIDER_REGN      (int) 132   // Undefined glider registration
#define  EC_DECLAR_INCMPL    (int) 133   // Declaration incomplete, but checked
#define  EC_DECLAR_LINE      (int) 134   // Found declaration line in invalid place   // new

// Engine errors
#define  EC_ENG_NO_TESTRUN   (int) 140   // No engine testrun made
#define  EC_ENG_ENL_PARAMS   (int) 141   // Invalid ENLM params
#define  EC_ENG_NO_MOP       (int) 142   // No MOP detection with bMOPonly flag set
  
// Flight recording errors
#define  EC_FR_TOO_SHORT     (int) 150   // Flight too short
#define  EC_FR_GPS_SIGNAL    (int) 151   // No GPS signal after take-off
#define  EC_FR_NO_FLIGHT     (int) 152   // No flight found
#define  EC_FR_NO_SOAR_B     (int) 153   // No or soaring block found
#define  EC_FR_SOAR_SHORT    (int) 154   // Soaring block found to short

// Output errors
#define  EC_OUT_DATAFILE     (int) 190   // Data file could not be opened
#define  EC_OUT_TEXTFILE     (int) 191   // Text file could not be opened
#define  EC_OUT_SCOREDIR     (int) 192   // Environment variable not found
#define  EC_OUT_BAROFILE     (int) 193   // Baro file could not be opened
#define  EC_OUT_PLOTFILE     (int) 194   // Error in plot file
#define  EC_OUT_PLOTPLOT     (int) 195   // Error drawing the plot

// Triangle messages
#define  TRI_NOT_CLOSED      (int) 200

// Task messages
#define  TM_FIRST            (int) 300
#define  TM_NO_TASK          (int) 300   // No task declaration
#define  TM_OK               (int) 301   // Valid task declaration
#define  TM_INVALID          (int) 302   // Invalid declaration, unknown error
#define  TM_NTURNPT_ALLOWED  (int) 303   // More turnpoints than allowed by array
#define  TM_NTURNPT_INVALID  (int) 304   // Zero or too many turnpoints (by rules)
#define  TM_TURNPT_TOO_CLOSE (int) 305   // Turnpoint separation < MinLegDistance
#define  TM_CORRECTED_START  (int) 306   // Short leg: corrected start point
#define  TM_CORRECTED_FINISH (int) 307   // Short leg: corrected finish point
#define  TM_TASK_TOO_SMALL   (int) 308   // Task distance < minimum required
#define  TM_TASK_DIST_ERR    (int) 309   // Task distance erratic
#define  TM_TASK_INCONSIST   (int) 310   // Task declaration inconsistent (warning)
#define  TM_BEERCAN_USED     (int) 311   // Same as Task OK, beercan fotosectors used
#define  TM_DECL_AFTER_START (int) 312   // Task was declared after the start of flight
#define  TM_NO_GID           (int) 313   // No GID was present in the header
#define  TM_LAST             (int) 399

// Runs messages
#define  RM_NO_STRETCHES     (int) 400   // Stretches file not found, not valid or empty
#define  RM_READ_ERROR       (int) 401   // Error while reading Stretches file
// TODO

struct sMessage
{ int  code ;
  char text[MAXLEN] ;
};

extern int      nCritErrors;   // Number of critical errors
extern int      nErrors;       //           non-critical errors
extern sMessage *CritErrors;   // List of critical errors
extern sMessage *Errors;       //         non-critical errors

extern void report_error(int code, const char *extra_info, int bExit=TRUE);
extern void exit_error(int code, const char *message);
extern void  list_errors(char *fn);


#endif
