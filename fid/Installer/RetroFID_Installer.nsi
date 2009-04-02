;*
;* RetroFID Installer
;* Copyright (C) 2007, MMSGURU - written by Till Toenshoff
;*
;* This program is free software: you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation, either version 3 of the License, or
;* (at your option) any later version.
;* 
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;* 
;* You should have received a copy of the GNU General Public License
;* along with this program.  If not, see <http://www.gnu.org/licenses/>.
;*
;Credits:
;NSIS Modern User Interface, Welcome/Finish Page Example Script, Written by Joost Verburg

;--------------------------------
;Include Modern UI
  !include "MUI.nsh"

;--------------------------------
;General
  ;Name and file
  Name "RetroFID(tm)"
  OutFile "RetroFID_Installer.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\MMSGURU\RetroFID"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\RetroFID" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\..\code\License\gpl-3_0.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "" SecMain
SectionEnd


Section "Complete" SecComplete

	SetOutPath "$INSTDIR"
	File /a "..\..\Release\avcodec.dll" 
	File /a "..\..\Release\avformat.dll"
	File /a "..\..\Release\avutil.dll"
	File /a "..\..\Release\libfaac.dll"
	File /a "..\..\Release\libfaad2.dll"
	File /a "..\..\Release\qscl.dll"
	File /a "..\..\Release\retroBase.dll"
	File /a "..\..\Release\msvcm80.dll"
	File /a "..\..\Release\msvcr80.dll"
	File /a "..\..\Release\msvcp80.dll"
	File /a "..\..\Release\Microsoft.VC80.CRT.manifest"
	File /a "..\..\Release\fid.exe"
	File /a "..\Documentation\readme.txt"	
	SetOutPath "$INSTDIR\codecs"
	File /a "..\..\Release\codecs\retroApple.dll"
	File /a "..\..\Release\codecs\retroG711.dll"
	File /a "..\..\Release\codecs\retroBeatnik.dll"
	File /a "..\..\Release\codecs\retroMpeg.dll"
	File /a "..\..\Release\codecs\retroPanasonic.dll"
	File /a "..\..\Release\codecs\retroQualcomm.dll"
	File /a "..\..\Release\codecs\retroWave.dll"
	File /a "..\..\Release\codecs\retroYamaha.dll"
	SetOutPath "$INSTDIR\validators"
	File /a "..\..\Release\validators\retroMono.dll"
	File /a "..\..\Release\validators\retroMidi.dll"
	File /a "..\..\Release\validators\retroGraphics.dll"
	File /a "..\..\Release\validators\retroTheme.dll"
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=Documentation.rtf" "..\Documentation\RetroFID_Documentation_V001.rtf"
	SetOutPath "$INSTDIR\Documentation\PHP Example"
	File /a "..\Documentation\PHP Example\parser_example.php"
	SetOutPath "$INSTDIR\License"
	File "/oname=License.txt" "..\..\code\License\gpl-3_0.txt" 

	;Store installation folder
	WriteRegStr HKCU "Software\RetroCode" "" $INSTDIR
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecComplete ${LANG_ENGLISH} "Complete Package."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecComplete} $(DESC_SecComplete)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\avcodec.dll"
  Delete "$INSTDIR\avformat.dll"
  Delete "$INSTDIR\avutil.dll"
  Delete "$INSTDIR\libfaac.dll"
  Delete "$INSTDIR\libfaad2.dll"
  Delete "$INSTDIR\qscl.dll"
  Delete "$INSTDIR\retroBase.dll"
  Delete "$INSTDIR\Microsoft.VC80.CRT.manifest"
  Delete "$INSTDIR\msvcm80.dll"
  Delete "$INSTDIR\msvcr80.dll"
  Delete "$INSTDIR\msvcp80.dll"
  Delete "$INSTDIR\fid.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\codecs\retroApple.dll"
  Delete "$INSTDIR\codecs\retroG711.dll"
  Delete "$INSTDIR\codecs\retroBeatnik.dll"
  Delete "$INSTDIR\codecs\retroMpeg.dll"
  Delete "$INSTDIR\codecs\retroPanasonic.dll"
  Delete "$INSTDIR\codecs\retroQualcomm.dll"
  Delete "$INSTDIR\codecs\retroWave.dll"
  Delete "$INSTDIR\codecs\retroYamaha.dll"
  RMDir "$INSTDIR\codecs"
  Delete "$INSTDIR\validators\retroMono.dll"
  Delete "$INSTDIR\validators\retroMidi.dll"
  Delete "$INSTDIR\validators\retroTheme.dll"
  Delete "$INSTDIR\validators\retroGraphics.dll"
  RMDir "$INSTDIR\codecs"
  Delete "$INSTDIR\Documentation\Documentation.rtf"
  RMDir "$INSTDIR\Documentation"
  Delete "$INSTDIR\Documentation\PHP Example\parser_example.php"
  RMDir "$INSTDIR\Documentation\PHP Example"
  Delete "$INSTDIR\License\License.txt"
  RMDir "$INSTDIR\License"

  Delete "$INSTDIR\Uninstall.exe"
  
  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\RetroFID"

SectionEnd

Function .onInit
	InitPluginsDir
	File /oname=$PLUGINSDIR\splash.bmp "..\..\code\Installer\mmsguru_splash.bmp"
	advsplash::show 1000 600 400 0xFF00FF $PLUGINSDIR\splash
	Pop $0 
	Delete $PLUGINSDIR\splash.bmp
FunctionEnd