; The name of the installer
Name "CSL Installer Version 0.7"

; The file to write
OutFile "CSL-Installer-v0.7.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\CSL"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\CSL" "Install_Dir"

; Compressor
SetCompressor lzma

XPStyle on
ShowInstDetails show
CRCCheck on

LicenseText "CSL terms of usage"
LicenseData "COPYING"

;--------------------------------

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------


; The stuff to install

Section "Program (required)"
  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  File release\csl.exe
  SetOutPath $INSTDIR\data
  File data\GeoIP.dat
  File data\csl_start_sb.ogz
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\CSL "Install_Dir" "$INSTDIR"  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "DisplayName" "CSL"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "UninstallString" '"$INSTDIR\csluninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "NoRepair" 1
  WriteUninstaller "csluninstall.exe"
SectionEnd


Section "Translations"
  SetOutPath $INSTDIR\lang\de
  File po\csl.mo
SectionEnd


SectionGroup /e "Shortcuts"

Section "Startmenu"
  ; Set output path to the installation directory again - necessary for shortcuts.
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\Cube Server Lister"
  CreateShortCut "$SMPROGRAMS\Cube Server Lister\Cube Server Lister.lnk" "$INSTDIR\csl.exe" "" "$INSTDIR\csl.exe" 0
  CreateShortCut "$SMPROGRAMS\Cube Server Lister\Uninstall.lnk" "$INSTDIR\csluninstall.exe" "" "$INSTDIR\csluninstall.exe" 0
SectionEnd

Section "Desktop"
  ; Set output path to the installation directory again - necessary for shortcuts.
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Cube Server Lister.lnk" "$INSTDIR\csl.exe" "" "$INSTDIR\csl.exe" 0
SectionEnd

SectionGroupEnd


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove files and uninstaller
  RMDir /r $INSTDIR

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Cube Server Lister\*.*"
  Delete "$DESKTOP\Cube Server Lister.lnk"
  ; Remove directories used
  RMDir "$SMPROGRAMS\Cube Server Lister"

SectionEnd
