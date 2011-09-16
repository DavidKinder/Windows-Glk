/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkTime
// GLK Time related functions. Ideas on how to improve the accuracy beyond
// the ~15ms provided by the internal Windows clock are taken from Mozilla.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_TIME_H_
#define WINGLK_TIME_H_

#include "glk.h"

// Get the current time in UTC
FILETIME GetNow(void);
// Convert a UTC time to the local time zone
FILETIME ToLocal(const FILETIME& ft);

// Convert a time to the Glk format
void ToGlkTime(const FILETIME& ft, glktimeval_t* time);
// Convert a time from the Glk format
FILETIME FromGlkTime(const glktimeval_t* time);
// Convert a time to the simple Glk format
glsi32 ToSimpleTime(const FILETIME& ft, glui32 factor);
// Convert a time from the simple Glk format
FILETIME FromSimpleTime(glsi32 time, glui32 factor);

// Convert a date to the Glk format
void ToGlkDate(const FILETIME& ft, glkdate_t* date);
// Convert a date from the Glk format
FILETIME FromGlkDate(const glkdate_t* date);

// Get the different between two tick counts
DWORD TickCountDiff(DWORD later, DWORD earlier);

#endif // WINGLK_TIME_H_
