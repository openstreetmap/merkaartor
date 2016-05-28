;Merkaartor installer script
;Based on NSIS Modern User Interface Basic Example Script, by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "FileFunc.nsh"
  !include "LogicLib.nsh"
  !include "StrFunc.nsh"
  !include "version.nch"


  SetCompressor lzma 

;--------------------------------
;General

  ;Name and file
  Name "Merkaartor"
  OutFile "merkaartor-${VER}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES${BITS}\Merkaartor"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Merkaartor" ""
  


  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
; Interface settings
  !define MUI_ICON "..\Icons\Merkaartor_48x48.ico"
  ;!define MUI_HEADERIMAGE
  ;!define MUI_HEADERIMAGE_BITMAP "..\Icons\Merkaartor_100x100.png"
  ;!define MUI_HEADERIMAGE_RIGHT
  !define MUI_ABORTWARNING

  !define MUI_WELCOMEPAGE_TEXT "This setup will install Merkaartor in version ${VER}."
  !define MUI_WELCOMEFINISHPAGE_BITMAP "..\Icons\Merkaartor_installer.bmp"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\LICENSE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  

;--------------------------------
;Installer Sections

Section "Merkaartor" SecMerkaartor
  ; This section is mandatory
  SectionIn RO

  SetOutPath "$INSTDIR"

  SetShellVarContext all

  
  ;ADD YOUR OWN FILES HERE...
  File ..\binaries\bin\*
  File /r ..\binaries\bin\platforms

  ; Menu shortcut
  CreateDirectory "$SMPROGRAMS\Merkaartor"
  CreateShortcut  "$SMPROGRAMS\Merkaartor\Merkaartor.lnk" "$INSTDIR\merkaartor.exe"
  CreateShortcut  "$SMPROGRAMS\Merkaartor\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Merkaartor" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  !define ARP      "Software\Microsoft\Windows\CurrentVersion\Uninstall\Merkaartor"
  WriteRegStr HKLM "${ARP}"      "DisplayName"    "Merkaartor"
  WriteRegStr HKLM "${ARP}"      "DisplayVersion"  "${VER}"
  WriteRegStr HKLM "${ARP}"      "DisplayVersion"  "${VER}"
  WriteRegStr HKLM "${ARP}"      "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKLM "${ARP}"      "DisplayIcon" "$\"$INSTDIR\merkaartor.exe$\""
 
  ; Estimated size
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize" "$0"

SectionEnd

Section "Background plugins" SecBackgroundPlugins

  SetOutPath "$INSTDIR\plugins"
  File /r ..\binaries\bin\plugins\background

SectionEnd

Section "Translations" SecTranslations

  SetOutPath "$INSTDIR"
  File /r ..\binaries\bin\translations

SectionEnd

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMerkaartor ${LANG_ENGLISH} "The program and necessary libraries."
  LangString DESC_SecBackgroundPlugins ${LANG_ENGLISH} "Access to some background layers, including Bing."
  LangString DESC_SecTranslations ${LANG_ENGLISH} "Install translations for various languages."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMerkaartor} $(DESC_SecMerkaartor)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBackgroundPlugins} $(DESC_SecBackgroundPlugins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecTranslations} $(DESC_SecTranslations)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

; -------------------------------
; Version info
;  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Merkaartor"
;  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© The Merkaartor Developers"
;  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "A good OpenStreetMap editor."
;  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VER}"
;  VIProductVersion "${VER}.0"
;  VIFileVersion    "${VER}.0"


Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  SetShellVarContext all
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR" 
  RMDir /r "$SMPROGRAMS\Merkaartor"
  DeleteRegKey HKCU "Software\Merkaartor"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Merkaartor"

SectionEnd
