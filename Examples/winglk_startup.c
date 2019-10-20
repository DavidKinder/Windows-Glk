/*
   winglk_startup.c: Windows-specific code for the Glk examples.
*/

#include "Glk.h"
#include "WinGlk.h"

int winglk_startup_code(const char* cmdline)
{
  winglk_app_set_name("Glk Example");
  return 1;
}
