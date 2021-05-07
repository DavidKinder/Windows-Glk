/*
 * Windows Glk Libraries
 * Startup code for Glk applications
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "glk.h"
#include "WinGlk.h"

/* Entry point for all Glk applications */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int show)
{
  /* Attempt to initialise Glk */
  if (InitGlk(0x00000704) == 0)
    exit(0);

  /* Call the Windows specific initialization routine */
  if (winglk_startup_code(cmdLine) != 0)
  {
    /* Run the application */
    glk_main();

    /* There is no return from this routine */
    glk_exit();
  }

  return 0;
}
