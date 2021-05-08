@echo off

"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsGlk.zip Glk.c WinGlk.html
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\WindowsGlk.zip Executables\Release\Glk*.*
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\WindowsGlk.zip GlkDll\Glk.def
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\WindowsGlk.zip Examples\* Include\*

"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsGlkSrc.zip Glk.c WinGlk.html MakeDist.bat
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\WindowsGlkSrc.zip GlkDll\* Include\*
pushd \Programs
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\libmodplug\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\mfc\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\ScaleGfx\*.h
popd

"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Scare.exe
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Glk*.dll
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Scare\README Scare\ChangeLog
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Scare\COPYING Scare\doc\RUNNING
pushd Scare
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\scare_win.zip win32\*
popd
