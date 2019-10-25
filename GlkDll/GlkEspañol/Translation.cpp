/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// Translation
// Stub DLL entry point for Glk translation DLLs
//
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
	return 1;
}

extern "C" __declspec(dllexport) BOOL IsEnabled(VOID)
{
  return (GetACP() == 1252); // ANSI Latin 1; Western European (Windows)
}
