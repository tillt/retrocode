;*
;* RetroPack Installer
;* Copyright (C) 2008-2009, MMSGURU - written by Till Toenshoff
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
	!include "MUI2.nsh"
	
	
;--------------------------------
;General
	;Name "${INST_NAME}"
	Name "RetroCode and RetroFID"
	OutFile "RetroPack_Installer.exe"
	;XPStyle on

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE "..\License\gpl-3_0.txt"
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

	!define INST_NAME	"RetroCode(tm) and RetroFID(tm)"
	!define INST_VER 	"1.0.0.1"

	VIProductVersion "${INST_VER}"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${INST_NAME} Installer"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "written by Till Toenshoff"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${INST_NAME} are trademarks of MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${INST_NAME} Setup"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${INST_VER}"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${INST_VER}"
	
	;Default installation folder
	InstallDir "$PROGRAMFILES\MMSGURU\RetroCode"
	
	;Get installation folder from registry if available
	InstallDirRegKey HKCU "Software\RetroCode" ""
	
;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING


;--------------------------------
;Installer Sections

!ifndef NOINSTTYPES ; only if not defined
	InstType "Full"
	InstType "RetroCode only"
	InstType "RetroFID only"
	;InstType /NOCUSTOM
	InstType /COMPONENTSONLYONCUSTOM
!endif

AutoCloseWindow false
ShowInstDetails show

;--------------------------------
	;!include "FileFunc.nsh"
	;!include "WordFunc.nsh"
	;!insertmacro Locate
	;!insertmacro VersionCompare
	;!insertmacro un.Locate
	;!insertmacro un.VersionCompare	


Section "" SecMain
SectionEnd

Section "-Microsoft Runtime Check" SecCheckMsvcrt
	SectionIn 1 2 3

	DetailPrint "Preparing Microsoft Runtime test..."
	SetOutPath "$TEMP\instDLLTest"
	File /a "..\Release\retroBase.dll"
	File /a "..\Release\codecs\retroWave.dll"
	;try to access the version information	
  	System::Call 'retroWave::pQueryLibrary(i) i(1) t .r0 ?u'
  	SetOutPath "$INSTDIR"
  	Delete "$TEMP\instDLLTest\retroBase.dll"
  	Delete "$TEMP\instDLLTest\retroWave.dll"
	RMDir "$TEMP\instDLLTest"

  	;in case of an error, install the runtime...
  	StrCmp $0 "error" DoInst
 	DetailPrint "Testing for usable Microsoft Runtime: ok"
 	goto DoneInst
  	
 DoInst:
 	DetailPrint "Testing for usable Microsoft Runtime: failed"
	SetOutPath "$INSTDIR\ThirdParty"
	DetailPrint "Installing Microsoft Runtime..."
	File /a "..\ThirdParty\vcredist_x86.exe"
	ExecWait '"$INSTDIR\ThirdParty\vcredist_x86.exe" /q:a /c:$\"msiexec /i vcredist.msi /qn /l*v $TEMP\vcredist_x86.log$\"'

 DoneInst:

SectionEnd 


Section "Base" SecBase
	SectionIn 1 2 3
	
	SetOutPath "$INSTDIR"
	File /a "..\..\Release\retroBase.dll"
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=ChangeLog" "..\..\ChangeLog"
	File "/oname=AUTHORS" "..\..\AUTHORS"
	SetOutPath "$INSTDIR\License"
	File "/oname=License.txt" "..\License\gpl-3_0.txt" 

	;Store installation folder
	WriteRegStr HKCU "Software\RetroCode" "" $INSTDIR
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd


Section "RetroCode" SecCode
	SectionIn 1 2
	
	SetOutPath "$INSTDIR"
	
	File /a "..\..\Release\code.exe"
	File /a "..\Documentation\readme.txt"
	
	File /a "..\ringback_config.xml"
	
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=RetroCode_Documentation.rtf" "..\Documentation\RetroCode_Documentation.rtf"
	File "/oname=RetroCode_Quickstart.txt" "..\Documentation\RetroCode_Quickstart.txt"
	
	
SectionEnd

Section "RetroFID" SecFID
	SectionIn 1 3
	
	SetOutPath "$INSTDIR"
	
	File /a "..\..\Release\fid.exe"
	
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=RetroFID_Documentation.rtf" "..\..\fid\Documentation\RetroFID_Documentation.rtf"
	File "/oname=RetroFID_Quickstart.txt" "..\..\fid\Documentation\RetroFID_Quickstart.txt"
	File "/oname=RetroFID_XML_Schema_V100.xsd" "..\..\fid\Documentation\RetroFID_XML_Schema_V100.xsd"
SectionEnd


Section "Validators" SecValidators
	SectionIn 1 3
	
	SetOutPath "$INSTDIR\validators"
	
	File /a "..\..\Release\validators\retroGraphics.dll"
	File /a "..\..\Release\validators\retroMono.dll"
	File /a "..\..\Release\validators\retroTheme.dll"
	File /a "..\..\Release\validators\retroMidi.dll"
SectionEnd

Section "Codecs" SecCodecs
	SectionIn 1 2 3

	SetOutPath "$INSTDIR"
	File /a "..\..\Release\avcodec-51.dll" 
	File /a "..\..\Release\avformat-52.dll"
	File /a "..\..\Release\avutil-49.dll"
	File /a "..\..\Release\libfaac.dll"
	File /a "..\..\Release\libfaad2.dll"
	File /a "..\..\Release\libmp3lame-0.dll"
	File /a "..\..\Release\libogg-0.dll"
	File /a "..\..\Release\libvorbis-0.dll"	
;	File /a "..\..\Release\qscl.dll"
	
	SetOutPath "$INSTDIR\codecs"
	
	File /a "..\..\Release\codecs\retroApple.dll"
	File /a "..\..\Release\codecs\retroG711.dll"
	File /a "..\..\Release\codecs\retroBeatnik.dll"
	File /a "..\..\Release\codecs\retroMpeg.dll"
	File /a "..\..\Release\codecs\retroPanasonic.dll"
	File /a "..\..\Release\codecs\retroQualcomm.dll"
	File /a "..\..\Release\codecs\retroWave.dll"
	File /a "..\..\Release\codecs\retroYamaha.dll"
SectionEnd


;--------------------------------
;Descriptions

  ;Language strings
;  LangString DESC_SecComplete ${LANG_ENGLISH} "Complete Package."

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${SecComplete} $(DESC_SecComplete)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\avcodec-51.dll"
  Delete "$INSTDIR\avformat-52.dll"
  Delete "$INSTDIR\avutil-49.dll"
  Delete "$INSTDIR\libfaac.dll"
  Delete "$INSTDIR\libfaad2.dll"
  Delete "$INSTDIR\libmp3lame-0.dll"
  Delete "$INSTDIR\libogg-0.dll"
  Delete "$INSTDIR\libvorbis-0.dll"
;  Delete "$INSTDIR\qscl.dll"
  Delete "$INSTDIR\retroBase.dll"
  Delete "$INSTDIR\code.exe"
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
  Delete "$INSTDIR\validators\retroGraphics.dll"
  Delete "$INSTDIR\validators\retroTheme.dll"
  RMDir "$INSTDIR\validators"

  Delete "$INSTDIR\Documentation\RetroFID_Documentation.rtf"
  Delete "$INSTDIR\Documentation\RetroCode_Documentation.rtf"
  Delete "$INSTDIR\Documentation\AUTHORS"
  Delete "$INSTDIR\Documentation\ChangeLog"
  RMDir "$INSTDIR\Documentation"

  Delete "$INSTDIR\License\License.txt"
  RMDir "$INSTDIR\License"
  
  Delete "$INSTDIR\ThirdParty\vcredist_x86.exe"
  RMDir "$INSTDIR\ThirdParty"

  Delete "$INSTDIR\Uninstall.exe"
  
  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\RetroCode"

SectionEnd

Function .onInit
	InitPluginsDir
	File /oname=$PLUGINSDIR\splash.bmp "mmsguru_splash.bmp"
	advsplash::show 1000 600 400 0xFF00FF $PLUGINSDIR\splash
	Pop $0 
	Delete $PLUGINSDIR\splash.bmp
FunctionEnd

Function myGuiShowQuickstart
  !insertmacro MUI_HEADER_TEXT "QuickStart" ""
  ;!insertmacro MUI_INSTALLOPTIONS_DISPLAY "ioFile.ini"
FunctionEnd
