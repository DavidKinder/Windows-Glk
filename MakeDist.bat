@echo off

"\Program Files\Zip\zip" \Temp\WindowsGlk.zip Glk.c WinGlk.html
"\Program Files\Zip\zip" -j \Temp\WindowsGlk.zip Executables\Release\Glk*.* Executables\Release\ScaleGfx.dll
"\Program Files\Zip\zip" -j \Temp\WindowsGlk.zip GlkDll\Glk.def
"\Program Files\Zip\zip" -r \Temp\WindowsGlk.zip Examples\* Include\*

"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Glk.c WinGlk.html MakeDist.bat
"\Program Files\Zip\zip" -r \Temp\WindowsGlkSrc.zip GlkDll\* Include\*
pushd \Programs
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\libmodplug\*
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\mfc\*
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\ScaleGfx\*.h
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\ScaleGfx\*.cpp
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\ScaleGfx\*.def
"\Program Files\Zip\zip" \Temp\WindowsGlkSrc.zip Libraries\ScaleGfx\Makefile
popd

"\Program Files\Zip\zip" -j \Temp\WinGlulxe.zip Executables\Release\Glulxe.*
"\Program Files\Zip\zip" -j \Temp\WinGlulxe.zip Executables\Release\Glk*.dll
"\Program Files\Zip\zip" -j \Temp\WinGlulxe.zip Executables\Release\ScaleGfx.dll

"\Program Files\Zip\zip" -r \Temp\WinGlulxeSrc.zip Glulxe\help\* Glulxe\res\* Glulxe\msvc\*
"\Program Files\Zip\zip" -r \Temp\WinGlulxeSrc.zip Installer\*
"\Program Files\Zip\zip" \Temp\WinGlulxeSrc.zip Glulxe\resource.h
"\Program Files\Zip\zip" \Temp\WinGlulxeSrc.zip Glulxe\osdepend.c
"\Program Files\Zip\zip" \Temp\WinGlulxeSrc.zip Glulxe\winstart.c
"\Program Files\Zip\zip" \Temp\WinGlulxeSrc.zip Glulxe\Glulxe.rc
"\Program Files\Zip\zip" \Temp\WinGlulxeSrc.zip Glulxe\Makefile.win

"\Program Files\Zip\zip" -j \Temp\wingit.zip Executables\Release\Git.exe
"\Program Files\Zip\zip" -j \Temp\wingit.zip Executables\Release\Git.chm
"\Program Files\Zip\zip" -j \Temp\wingit.zip Executables\Release\Glk*.dll
"\Program Files\Zip\zip" -j \Temp\wingit.zip Executables\Release\ScaleGfx.dll
"\Program Files\Zip\zip" -j \Temp\wingit.zip Git\README.txt

"\Program Files\Zip\zip" \Temp\wingit-src.zip Git\git_windows.c
"\Program Files\Zip\zip" \Temp\wingit-src.zip Git\Makefile.win
"\Program Files\Zip\zip" -r \Temp\wingit-src.zip Git\win\* Git\help\*

"\Program Files\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Scare.exe
"\Program Files\Zip\zip" -j \Temp\scare_win.zip Executables\Release\Glk*.dll
"\Program Files\Zip\zip" -j \Temp\scare_win.zip Executables\Release\ScaleGfx.dll
"\Program Files\Zip\zip" -j \Temp\scare_win.zip Scare\README Scare\ChangeLog
"\Program Files\Zip\zip" -j \Temp\scare_win.zip Scare\COPYING Scare\doc\RUNNING
pushd Scare
"\Program Files\Zip\zip" -r \Temp\scare_win.zip win32\*
popd

pushd Installer
"\Program Files\NSIS\makensis" Glulxe.nsi
move *.exe \Temp
popd
