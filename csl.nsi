;Include Modern UI
!include "MUI.nsh"

!define CSL_VERSION 0.8.1.94

;The name of the installer
  Name "Cube Server Lister ${CSL_VERSION}"

;The file to write
  OutFile "CSL-Installer-${CSL_VERSION}.exe"

;The default installation directory
  InstallDir "$PROGRAMFILES\CSL"

;Registry key to check for directory (so if you install again, it will 
;overwrite the old one automatically)
  InstallDirRegKey HKLM "Software\CSL" "Install_Dir"

;Compressor
  SetCompressor /SOLID lzma


;Images
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP   "src\img\install_header.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "src\img\install_header.bmp"
  !define MUI_ICON                 "src\img\csl_48.ico"
  !define MUI_UNICON               "src\img\uninstall.ico"


;Finish page  
  !define MUI_FINISHPAGE_RUN $INSTDIR\csl.exe
  !define MUI_FINISHPAGE_RUN_TEXT $(Finish_Run)


;Pages
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES  
  !insertmacro MUI_PAGE_FINISH
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES


;Languages
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "German"

  LangString Finish_Run ${LANG_ENGLISH} "Start Cube Server Lister."
  LangString Finish_Run ${LANG_GERMAN} "Cube Server Lister jetzt starten."


;The stuff to install
  Section $(TITLE_Section1) SecProgram
    SectionIn RO
    ; Set output path to the installation directory.
    SetOutPath $INSTDIR
    File Release\csl.exe
    File Release\cslengine.dll
    File Release\cslguitools.dll
    File Release\cslplugin.dll
    File Release\csltools.dll
    File Release\wxmsw28u_vc_csl.dll
    SetOutPath $INSTDIR\plugins
    File Release\plugins\assaultcube.dll
    File Release\plugins\cube.dll
    File Release\plugins\redeclipse.dll
    File Release\plugins\sauerbraten.dll
    SetOutPath $INSTDIR\data
    File data\GeoIP.dat
  
    ; Write the installation path into the registry
    WriteRegStr HKLM SOFTWARE\CSL "Install_Dir" "$INSTDIR"  
    ; Write the uninstall keys for Windows
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "DisplayName" "CSL"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CSL" "NoRepair" 1
    WriteUninstaller "uninstall.exe"

    ; Register the URI
    WriteRegStr HKCR "CSL" "" "URL:csl Protocol Handler"
    WriteRegStr HKCR "CSL" "URL Protocol" ""
    WriteRegStr HKCR "CSL\DefaultIcon" "" '"$INSTDIR\csl.exe"'
    WriteRegStr HKCR "CSL\shell\open\command" "" '"$INSTDIR\csl.exe" "%1"'
  SectionEnd


  Section $(TITLE_Section2) SecMapCfgTool
    SetOutPath $INSTDIR
    File Release\cslmapcfgtool.exe
  SectionEnd


  Section $(TITLE_Section3) SecMapPreview
    SetOutPath $INSTDIR\data\maps
    File /r data\maps\*.cfg
    File /r data\maps\*.png
  SectionEnd


  Section $(TITLE_Section4) SecLang
    SetOutPath $INSTDIR\locale\cs
    File po\cs\csl.mo
    SetOutPath $INSTDIR\locale\de
    File po\de\csl.mo
    SetOutPath $INSTDIR\locale\nl
    File po\nl\csl.mo
  SectionEnd


  SectionGroup /e $(TITLE_Section5) SecShortcut

   Section $(TITLE_Section5a)
     ; Set output path to the installation directory again - necessary for shortcuts.
     SetOutPath $INSTDIR
     CreateDirectory "$SMPROGRAMS\Cube Server Lister"
     CreateShortCut "$SMPROGRAMS\Cube Server Lister\Cube Server Lister.lnk" "$INSTDIR\csl.exe" "" "$INSTDIR\csl.exe" 0
     CreateShortCut "$SMPROGRAMS\Cube Server Lister\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
   SectionEnd

   Section  $(TITLE_Section5b)
     ; Set output path to the installation directory again - necessary for shortcuts.
     SetOutPath $INSTDIR
     CreateShortCut "$DESKTOP\Cube Server Lister.lnk" "$INSTDIR\csl.exe" "" "$INSTDIR\csl.exe" 0
   SectionEnd

  SectionGroupEnd

  ;Descriptions
    LangString TITLE_Section1  ${LANG_ENGLISH} "Program"
    LangString TITLE_Section2  ${LANG_ENGLISH} "Map config tool"
    LangString TITLE_Section3  ${LANG_ENGLISH} "Map preview files"
    LangString TITLE_Section4  ${LANG_ENGLISH} "Language files"
    LangString TITLE_Section5  ${LANG_ENGLISH} "Shortcuts"
    LangString TITLE_Section5a ${LANG_ENGLISH} "Startmenu"
    LangString TITLE_Section5b ${LANG_ENGLISH} "Desktop"

    LangString TITLE_Section1  ${LANG_GERMAN} "Hauptprogramm"
    LangString TITLE_Section2  ${LANG_GERMAN} "Mapconfig Werkzeug"
    LangString TITLE_Section3  ${LANG_GERMAN} "Mapvorschaudateien"
    LangString TITLE_Section4  ${LANG_GERMAN} "Übersetzungen"
    LangString TITLE_Section5  ${LANG_GERMAN} "Verknüpfungen"
    LangString TITLE_Section5a ${LANG_GERMAN} "Startmenü"
    LangString TITLE_Section5b ${LANG_GERMAN} "Desktop"

    LangString DESC_Section1 ${LANG_ENGLISH} "Main progrom."
    LangString DESC_Section2 ${LANG_ENGLISH} "Tool to create map config files necessary for map preview."
    LangString DESC_Section3 ${LANG_ENGLISH} "Images and config files for map preview."
    LangString DESC_Section4 ${LANG_ENGLISH} "Some translations."
    LangString DESC_Section5 ${LANG_ENGLISH} "Program shortcuts for the startmenu and the desktop."

    LangString DESC_Section1 ${LANG_GERMAN} "Hauptprogramm."
    LangString DESC_Section2 ${LANG_GERMAN} "Ein Werkzeug, um die für die Mapvorschau benötigten Konfigurationsdateien zu erzeugen."
    LangString DESC_Section3 ${LANG_GERMAN} "Bilder und Konfigurationsdateien für die Mapvorschau."
    LangString DESC_Section4 ${LANG_GERMAN} "Einige Übersetzungen."
    LangString DESC_Section5 ${LANG_GERMAN} "Verknüpfungen für das Startmenü und den Desktop."

    ;Assign descriptions to sections
    !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram}    $(DESC_Section1)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMapCfgTool} $(DESC_Section2)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMapPreview} $(DESC_Section3)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecLang}       $(DESC_Section4)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcut}   $(DESC_Section5)
    !insertmacro MUI_FUNCTION_DESCRIPTION_END


; Uninstaller

Section "Uninstall"
  
  ; Remove files and uninstaller
  RMDir /r $INSTDIR

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Cube Server Lister\*.*"
  Delete "$DESKTOP\Cube Server Lister.lnk"
  ; Remove directories used
  RMDir "$SMPROGRAMS\Cube Server Lister"
  ; Remove URI handler
  DeleteRegKey HKCR "CSL"

SectionEnd
