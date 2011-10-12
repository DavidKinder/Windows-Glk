; Installer for Windows Glulxe
;--------------------------------

!define GLULXE_VERSION "0.4.7.143"

;--------------------------------
;Configuration

!include "MUI.nsh"

SetCompressor /SOLID lzma
RequestExecutionLevel admin

; The name of the installer
Name "Windows Glulxe"
Caption "Windows Glulxe ${GLULXE_VERSION} Setup"
BrandingText "NullSoft Install System"

; The file to write
OutFile "WinGlulxeInstaller.exe"

; The default installation directory
InstallDir $PROGRAMFILES\WinGlulxe

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\WinGlulxe "Install_Dir"

!define MUI_ICON "..\Glulxe\res\Glulx.ico"
!define MUI_UNICON "..\Glulxe\res\Glulx.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "Back.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Windows Glulxe"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY SOFTWARE\WinGlulxe
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME Startmenu_Folder_Name2
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

Page custom chooseAssoc

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\Glulxe.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Var UserType
Var InstallFor

Function getUserType
	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $0
	UserInfo::GetAccountType
	Pop $UserType
        Return
	
	Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
	   StrCpy $UserType "Admin"
FunctionEnd

Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "WinGluxeMutex") i .r1 ?e'
  Pop $R0
 
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

  InitPluginsDir
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT Glulxe.ini
FunctionEnd

Function chooseAssoc
  Push $R0
  !insertmacro MUI_HEADER_TEXT "Choose Options" "Choose how Windows Glulxe is configured."
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY_RETURN Glulxe.ini
  Pop $R0

  StrCmp $R0 "success" +1 leaveThis
  
  ReadINIStr $R0 "$PLUGINSDIR\Glulxe.ini" "Field 6" "State" ; Reads the Install for ALL option state

  StrCpy $InstallFor "current"
  StrCmp $R0 "0" leaveThis
    Call getUserType
    StrCmp $UserType "Admin" +1 notAdmin
      SetShellVarContext all
      StrCpy $InstallFor "all"
      goto leaveThis
notAdmin:
    MessageBox MB_OK "You do not have administrator rights. A single user install will be performed."
 
leaveThis:

FunctionEnd


;--------------------------------

; The stuff to install
Section "MainInstall" ;No components page, name is not important

  SectionIn RO

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\WinGlulxe "Install_Dir" "$INSTDIR"

  ; Write the install type into the registry
  WriteRegStr HKLM SOFTWARE\WinGlulxe "InstallFor" "$InstallFor"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put files there
  File ..\Executables\Release\Glk.dll
  File ..\Executables\Release\GlkEspañol.dll
  File ..\Executables\Release\GlkItaliano.dll
  File ..\Executables\Release\Glulxe.exe
  File ..\Executables\Release\Glulxe.chm
  File ..\Executables\Release\ScaleGfx.dll

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinGlulxe" "DisplayName" "Windows Glulxe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinGlulxe" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteUninstaller "Uninstall.exe"
  
  ; Write out IFDB keys so that the IFDB Download Advisor can find us
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "Version" ${GLULXE_VERSION}
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "RunGame" '"$INSTDIR\Glulxe.exe" "%1"'
  WriteRegStr HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "Version" ${GLULXE_VERSION}
  WriteRegStr HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com" "RunGame" '"$INSTDIR\Glulxe.exe" "%1"'

  ; Delete old start menu shortcuts
  Delete "$SMPROGRAMS\WinGlulxe\WinGlulxe.lnk"
  Delete "$SMPROGRAMS\WinGlulxe\WinGlulxe Help.lnk"
  Delete "$SMPROGRAMS\WinGlulxe\(www) Download Glulx games.lnk"
  Delete "$SMPROGRAMS\WinGlulxe\Uninstall WinGlulxe.lnk"
  RMDir "$SMPROGRAMS\WinGlulxe"

SectionEnd ; end the section

Section "Start Menu Shortcuts"

  SectionIn RO

  StrCpy $R9 ""

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

  StrCpy $R0 $STARTMENU_FOLDER
  StrCpy $R9 "$SMPROGRAMS\$R0"
  IfFileExists "$R9" startmenuFolderExists
  CreateDirectory "$R9"
startmenuFolderExists:
  CreateShortCut "$R9\Windows Glulxe.lnk" "$INSTDIR\Glulxe.exe"  
  CreateShortCut "$R9\Windows Glulxe Help.lnk" "$INSTDIR\Glulxe.chm"  
  CreateShortCut "$R9\(www) Download Glulx games.lnk" "http://mirror.ifarchive.org/indexes/if-archiveXgamesXglulx.html"  
  CreateShortCut "$R9\Uninstall Windows Glulxe.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0

  !insertmacro MUI_STARTMENU_WRITE_END

  ; Write the start menu path into the registry
  WriteRegStr HKLM SOFTWARE\WinGlulxe "Startmenu_Folder" $R9
  WriteRegStr HKCU SOFTWARE\WinGlulxe "Startmenu_Folder" $R9
  WriteRegStr HKLM SOFTWARE\WinGlulxe "Startmenu_Folder_Name2" $STARTMENU_FOLDER

SectionEnd

Var Extension
Var GlulxIcon
Var ZRegSection
Var ZDescription
Var LastSection

Function AddAssoc

  ; back up old value of file association
  ReadRegStr $1 HKCR "$Extension" ""
  StrCmp $1 "" noPreviousAssoc
    StrCmp $1 "$ZRegSection" noPreviousAssoc
      WriteRegStr HKCR "$Extension" "backup_val" $1
noPreviousAssoc:
  WriteRegStr HKCR "$Extension" "" "$ZRegSection"
  StrCmp "$ZRegSection" "$LastSection" alreadyWrittenSection
  WriteRegStr HKCR "$ZRegSection" "" "$ZDescription"
  WriteRegStr HKCR "$ZRegSection\shell" "" "open"
  WriteRegStr HKCR "$ZRegSection\DefaultIcon" "" '"$INSTDIR\Glulxe.exe",$GlulxIcon'
  WriteRegStr HKCR "$ZRegSection\shell\open\command" "" '"$INSTDIR\Glulxe.exe" "%1"'
  StrCpy $LastSection $ZRegSection
alreadyWrittenSection:

FunctionEnd

Section "File Associations"

  SectionIn RO

  IntOp $GlulxIcon 2 + 0
  StrCpy $ZRegSection "WGlulxGame"
  StrCpy $ZDescription "Glulx Game"

  ReadINIStr $R1 "$PLUGINSDIR\Glulxe.ini" "Field 1" "State" ; Reads the user's .ulx file association selection
  StrCmp $R1 "0" skipAssoc1
    StrCpy $Extension ".ulx"
    Call AddAssoc

  skipAssoc1:

  IntOp $GlulxIcon 1 + 0
  StrCpy $ZRegSection "BlorbedWGlulxGame"
  StrCpy $ZDescription "Blorbed Glulx Game"

  ReadINIStr $R1 "$PLUGINSDIR\Glulxe.ini" "Field 2" "State" ; Reads the user's .blb file association selection
  StrCmp $R1 "0" skipAssoc2
    StrCpy $Extension ".blb"
    Call AddAssoc

  skipAssoc2:

  ReadINIStr $R1 "$PLUGINSDIR\Glulxe.ini" "Field 3" "State" ; Reads the user's .blorb file association selection
  StrCmp $R1 "0" skipAssoc3
    StrCpy $Extension ".blorb"
    Call AddAssoc

  skipAssoc3:

  ReadINIStr $R1 "$PLUGINSDIR\Glulxe.ini" "Field 4" "State" ; Reads the user's .glb file association selection
  StrCmp $R1 "0" skipAssoc4
    StrCpy $Extension ".glb"
    Call AddAssoc

  skipAssoc4:

  ReadINIStr $R1 "$PLUGINSDIR\Glulxe.ini" "Field 5" "State" ; Reads the user's .gblorb file association selection
  StrCmp $R1 "0" skipAssoc5
    StrCpy $Extension ".gblorb"
    Call AddAssoc

  skipAssoc5:

SectionEnd


;--------------------------------
; Uninstaller stuff

Function un.getUserType
	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $0
	UserInfo::GetAccountType
	Pop $UserType
        Return
	
	Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
	   StrCpy $UserType "Admin"
FunctionEnd

Function un.RemoveAssoc
  ReadRegStr $1 HKCR "$Extension" ""
  StrCmp $1 "WGlulxGame" ReadyToKill 0 ; only do this if we own it
  StrCmp $1 "BlorbedWGlulxGame" ReadyToKill NoOwn ; only do this if we own it
  ReadyToKill:
    ReadRegStr $3 HKCR "$Extension" "backup_val"
    StrCmp $3 "" 0 RestoreBackup ; if backup == "" then delete the whole key
      DeleteRegKey HKCR "$Extension" ; Delete .z? section
    Goto NoOwn
  RestoreBackup:
      WriteRegStr HKCR "$Extension" "" $3
      DeleteRegValue HKCR "$Extension" "backup_val"
  NoOwn:
FunctionEnd

Section "Uninstall"

ReadRegStr $R9 HKLM SOFTWARE\WinGlulxe "Startmenu_Folder"
  StrCmp $R1 "" 0 +1
    ReadRegStr $R9 HKCU SOFTWARE\WinGlulxe "Startmenu_Folder"

ReadRegStr $R1 HKLM SOFTWARE\WinGlulxe "InstallFor"
  StrCmp $R1 "all" +1 noProblemUninstall
    Call un.getUserType
    StrCmp $UserType "Admin" allUninstallOk
    MessageBox MB_OK "You need administrator rights to uninstall Windows Glulxe."
    goto skipUninstall
allUninstallOk:  
  SetShellVarContext all
  
noProblemUninstall:  
  ; remove files and uninstaller
  Delete $INSTDIR\Glk.dll
  Delete $INSTDIR\GlkEspañol.dll
  Delete $INSTDIR\GlkItaliano.dll
  Delete $INSTDIR\Glulxe.exe
  Delete $INSTDIR\Glulxe.chm
  Delete $INSTDIR\ScaleGfx.dll
  Delete $INSTDIR\Uninstall.exe

  ; remove shortcuts, if any
  StrCmp $R9 "" skipRemoveStart
  Delete "$R9\Windows Glulxe.lnk"
  Delete "$R9\Windows Glulxe Help.lnk"
  Delete "$R9\(www) Download Glulx games.lnk"
  Delete "$R9\Uninstall Windows Glulxe.lnk"

  ; remove directories used
  RMDir "$R9"
skipRemoveStart:
  RMDir "$INSTDIR"

  ; remove registry keys

  StrCpy $Extension ".ulx"
  Call un.RemoveAssoc

  StrCpy $Extension ".blb"
  Call un.RemoveAssoc

  StrCpy $Extension ".blorb"
  Call un.RemoveAssoc

  StrCpy $Extension ".glb"
  Call un.RemoveAssoc

  StrCpy $Extension ".gblorb"
  Call un.RemoveAssoc

  DeleteRegKey HKCR "WGlulxGame"
  DeleteRegKey HKCR "BlorbedWGlulxGame"

  DeleteRegKey HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\WinGlulxe
  DeleteRegKey HKLM SOFTWARE\WinGlulxe

  DeleteRegKey HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com"
  DeleteRegKey HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WinGlulxe.zarf.eblong.com"

skipUninstall:

SectionEnd

