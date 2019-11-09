; Install script for Windows Glulxe

!include "MUI2.nsh"
!include "WinGlulxe.nsh"

Name "Windows Glulxe"
Caption "Windows Glulxe ${GLULXE_VERSION} Setup"
BrandingText "NullSoft Install System"

SetCompressor /SOLID lzma
RequestExecutionLevel admin
OutFile "WinGlulxeInstaller.exe"

InstallDir "$PROGRAMFILES\Windows Glulxe"
InstallDirRegKey HKLM "SOFTWARE\David Kinder\Glulxe\Install" "Directory"

!define MUI_ICON "..\Glulxe\res\Glulx.ico"
!define MUI_UNICON "..\Glulxe\res\Glulx.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "Back.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\Glulxe.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "DoInstall"

  SetOutPath "$INSTDIR"
  File ..\Executables\Release\Glk*.dll
  File ..\Executables\Release\Glulxe*.exe
  File ..\Executables\Release\Glulxe.chm
  File ..\Executables\Release\ScaleGfx.dll
  WriteUninstaller "Uninstall.exe"

  ; Registry keys for the IFDB Download Advisor
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "Version" ${GLULXE_VERSION}
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "RunGame" '"$INSTDIR\Glulxe.exe" "%1"'

  SetShellVarContext all    
  ; Remove old Start Menu folders
  RMDir /r "$SMPROGRAMS\WinGlulxe"
  RMDir /r "$SMPROGRAMS\Windows Glulxe"
  CreateShortCut "$SMPROGRAMS\Windows Glulxe.lnk" "$INSTDIR\Glulxe.exe"
  SetShellVarContext current
  
  WriteRegStr HKLM "SOFTWARE\David Kinder\Glulxe\Install" "Directory" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsGlulxe" "DisplayName" "Windows Glulxe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsGlulxe" "DisplayIcon" "$INSTDIR\Glulxe.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsGlulxe" "UninstallString" '"$INSTDIR\Uninstall.exe"'

  WriteRegStr HKCR ".ulx" "" "Glulx.ulx"
  WriteRegStr HKCR "Glulx.ulx" "" "Glulx Game"
  WriteRegStr HKCR "Glulx.ulx\DefaultIcon" "" "$INSTDIR\Glulxe.exe,2"
  WriteRegStr HKCR "Glulx.ulx\shell" "" "open"
  WriteRegStr HKCR "Glulx.ulx\shell\open\command" "" '"$INSTDIR\Glulxe.exe" "%1"'

  WriteRegStr HKCR ".blb" "" "Glulx.blorb"
  WriteRegStr HKCR ".blorb" "" "Glulx.blorb"
  WriteRegStr HKCR ".gblorb" "" "Glulx.blorb"
  WriteRegStr HKCR "Glulx.blorb" "" "Blorbed Glulx Game"
  WriteRegStr HKCR "Glulx.blorb\DefaultIcon" "" "$INSTDIR\Glulxe.exe,1"
  WriteRegStr HKCR "Glulx.blorb\shell" "" "open"
  WriteRegStr HKCR "Glulx.blorb\shell\open\command" "" '"$INSTDIR\Glulxe.exe" "%1"'

SectionEnd

Section "Uninstall"

  Delete $INSTDIR\Glk*.dll
  Delete $INSTDIR\Glulxe*.exe
  Delete $INSTDIR\Glulxe.chm
  Delete $INSTDIR\ScaleGfx.dll
  Delete $INSTDIR\Uninstall.exe
  RMDir "$INSTDIR"

  SetShellVarContext all
  Delete "$SMPROGRAMS\Windows Glulxe.lnk"
  SetShellVarContext current

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsGlulxe"
  DeleteRegKey HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com"

  DeleteRegKey HKCR ".ulx"
  DeleteRegKey HKCR ".blb"
  DeleteRegKey HKCR ".blorb"
  DeleteRegKey HKCR ".gblorb"

  DeleteRegKey HKCR "Glulx.ulx"
  DeleteRegKey HKCR "Glulx.blorb"

SectionEnd
