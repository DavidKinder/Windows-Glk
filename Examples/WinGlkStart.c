/*
  WinGlkStart.c
  Example demonstrating how to use Windows Glk functions at startup.
*/

#include "glk.h"
#include "WinGlk.h"
#include "resource.h"

#include <stdio.h>

char* filename = 0;

/* Windows Glk specific startup code */
int winglk_startup_code(const char* cmdline)
{
  /* Set the application name */
  winglk_app_set_name("WinGlkStart");

  /* Set the application window title */
  winglk_window_set_title("Windows Glk Startup Example");

  /* Set up the custom menu */
  winglk_set_gui(IDR_EXAMPLE);

  /* Process the command line and get a file name from that.
     If that fails, prompt the user with a file dialog. */
  filename = (char*)winglk_get_initial_filename(cmdline,"WinGlk Test",
    "C Source Files (*.c)|*.c|All Files (*.*)|*.*||");

  return 1;
}

/* Main Glk entry point */
void glk_main(void)
{
  winid_t window;
  char buffer[256];

  /* Open a Glk window */
  window = glk_window_open(0,0,0,wintype_TextBuffer,0);

  /* Set the output stream to the window */
  glk_set_window(window);

  /* Did the user select a file? */
  if (filename != 0)
  {
    glk_put_string("You selected the file \"");
    glk_put_string(filename);
    glk_put_string("\".\n");
  }
  else
    glk_put_string("No file selected.\n");

  /* Input event loop */
  while (1)
  {
    /* Get the next event */
    event_t ev;
    glk_select(&ev);
    switch (ev.type)
    {
    /* Windows specific GUI event */
    case winglk_evtype_GuiInput:
      sprintf(buffer,"Menu item selected, identifier is %d\n",ev.val1);
      glk_put_string(buffer);
      if (ev.val1 == ID_MENUITEM2)
      {
        glk_put_string("Exiting...\n");
        return;
      }
      break;
    }
  }
}
