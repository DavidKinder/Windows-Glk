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

"%ProgramFiles(x86)%\Zip\zip" -j \Temp\wingit.zip Executables\Release\Git.exe
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\wingit.zip Executables\Release\Git.chm
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\wingit.zip Executables\Release\Glk*.dll
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\wingit.zip Git\README.txt

"%ProgramFiles(x86)%\Zip\zip" \Temp\wingit-src.zip Git\git_windows.c
"%ProgramFiles(x86)%\Zip\zip" \Temp\wingit-src.zip Git\Makefile.win
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\wingit-src.zip Git\win\* Git\help\*

"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Scare.exe
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Glk*.dll
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Scare\README Scare\ChangeLog
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\scare_win.zip Scare\COPYING Scare\doc\RUNNING
pushd Scare
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\scare_win.zip win32\*
popd
