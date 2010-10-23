/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkUnicode
// GLK Unicode case conversions, taken from Andrew Plotkin's CheapGlk
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_UNICODE_H_
#define WINGLK_UNICODE_H_

#define CASE_UPPER (0)
#define CASE_LOWER (1)
#define CASE_TITLE (2)
#define CASE_IDENT (3)

#define COND_ALL (0)
#define COND_LINESTART (1)

extern "C"
{
#include "glk.h"
}

glui32 buffer_change_case(glui32 *buf, glui32 len,
  glui32 numchars, int destcase, int cond, int changerest);

#endif // WINGLK_UNICODE_H_
