/*
 * Windows Glk Libraries
 * Startup code for Glk applications
 */

#include <windows.h>
#include "Glk.h"
#include "WinGlk.h"

int InitGlk(unsigned int iVersion);

/* Entry point for all Glk applications */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// CUSTOM FONT PATCH BEGIN -------------------------------------------
	// by Alessandro Schillaci	
	// Before load glk system, this patch will load a custom font
	// from the config file
	// i.e.
	// [CUSTOM]
	// CustomFont=xxx.ttf
	// xxx.ttf will be loaded from the system
	// 
	char  szPath[MAX_PATH];
	char* lpFilename;
	if ( GetModuleFileName( NULL, szPath, sizeof szPath ) == 0 ){
		return -1;
	}    
	lpFilename = ( strrchr( szPath, '\\' ) + 1 );
	
	// add ".cfg" extension
    char * config;
    config = strstr (lpFilename,".exe");
    strncpy (config,".cfg",4);

	// add ".\\" to load the cfg in the current directory
	char extension[50];
    strcpy(extension,  ".\\");
    strcat(extension,lpFilename);

	// load "CustomFont" from the config file
    TCHAR fontfile[32];
    int a = GetPrivateProfileString("CUSTOM", "CustomFont", "", fontfile, 32, extension);
    if (a == 0){
	    //MessageBox(NULL, TEXT("Error loading font name from config.ini"), TEXT("Error"), MB_ICONERROR | MB_OK);
    }
    else{
		// install the font
	    int err = AddFontResource(fontfile);
	    if (err == 0){
  		  MessageBox(NULL, TEXT(fontfile), TEXT("Error reading font file"), MB_ICONERROR | MB_OK);
	    }	
    }
  // PATCH END -------------------------------------------
  
  /* Attempt to initialise Glk */
  if (InitGlk(0x00000704) == 0)
    exit(0);

  /* Call the Windows specific initialization routine */
  if (winglk_startup_code(lpCmdLine) != 0)
  {
    /* Run the application */
    glk_main();

    /* There is no return from this routine */
    glk_exit();
  }

  return 0;
}

