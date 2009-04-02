;*
;* RetroCode Installer
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
	
	!include "FileFunc.nsh"
	!include "WordFunc.nsh"
	
	!insertmacro Locate
	!insertmacro VersionCompare

;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;General
	;Name and file
	!define INST_NAME	"RetroCode(tm) and RetroFID(tm)"
	!define INST_VER 	"1.0.0.1"

	!define DLL_VER 	"9.0.21022.8"
	
	;!define DLL_VER "9.0.21022.222"

	var _RUNTIME_PATH

	Name "${INST_NAME}"
	OutFile "RetroPack_Installer.exe"
	XPStyle on
	
	VIProductVersion "${INST_VER}"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${INST_NAME} Installer"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
	VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${INST_NAME} are trademarks of MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© MMSGURU"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${INST_NAME} Setup"
	VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${INST_VER}"
	
	;Default installation folder
	InstallDir "$PROGRAMFILES\MMSGURU\RetroCode"
	
	;Get installation folder from registry if available
	InstallDirRegKey HKCU "Software\RetroCode" ""
	
	
	
;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING

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
;Installer Sections

!ifndef NOINSTTYPES ; only if not defined
	InstType "Full"
	InstType "RetroCode only"
	InstType "RetroFID only"
	;InstType "Custom"
	;InstType /NOCUSTOM
	InstType /COMPONENTSONLYONCUSTOM
!endif

AutoCloseWindow false
ShowInstDetails show

;--------------------------------


Section "" SecMain
SectionEnd

Section "Microsoft Runtime" SecMsvcrt
	SectionIn 1 2 3

	SetOutPath "$INSTDIR\ThirdParty"
	File /a "..\ThirdParty\vcredist_x86.exe"

	DetailPrint "Checking if the runtime is installed..."
	StrCpy $1 $WINDIR
	${Locate} "$1" "/L=F /M=MSVCR90.DLL /S=0B" "LocateCallback"

	IfErrors 0 CheckResult
		MessageBox MB_OK "Error" idok InstallRuntime
		
CheckResult:
	StrCmp "$_RUNTIME_PATH" "0" 0 IsInstalled
	
InstallRuntime:
	StrCpy $1 "vcredist_x86.exe"
	Call InstallRuntime
	goto DoneInstalling
	
IsInstalled:
	DetailPrint "DLL located at: $_RUNTIME_PATH"

DoneInstalling:
SectionEnd 


Section "RetroCode" SecCode
	SectionIn 1 2
	
	SetOutPath "$INSTDIR"
	
	File /a "..\Release\code.exe"
	File /a "..\Documentation\readme.txt"
	
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=RetroCode_Documentation.rtf" "..\Documentation\RetroCode_Documentation_V005.rtf"

SectionEnd

Section "RetroFID" SecFID

	SectionIn 1 3
	
	SetOutPath "$INSTDIR"
	
	File /a "..\..\fid\Release\fid.exe"
	
	SetOutPath "$INSTDIR\Documentation"
	File "/oname=RetroFID_Documentation.rtf" "..\..\fid\Documentation\RetroFID_Documentation_V001.rtf"

SectionEnd



Function InstallRuntime

		DetailPrint "Installing Runtime..."
		
		;Exec '"$1"'
  		ExecWait '"$1 /quiet"' $0
  		DetailPrint "Result: $0"
  		;Sleep 500
  		;BringToFront
	
FunctionEnd

Function LocateCallback
		GetDllVersion "$R9" $R0 $R1
		IntOp $R2 $R0 / 0x00010000
		IntOp $R3 $R0 & 0x0000FFFF
		IntOp $R4 $R1 / 0x00010000
		IntOp $R5 $R1 & 0x0000FFFF
		StrCpy $0 "$R2.$R3.$R4.$R5"
	
        ${VersionCompare} "$0" "${DLL_VER}" $R1

        StrCmp $R1 0 0 new
        StrCpy $_RUNTIME_PATH $R9
		StrCpy "$0" StopLocate
		goto leave

      new:
        StrCmp $R1 1 0 old
        StrCpy $_RUNTIME_PATH "0"
        goto leave

      old:
        StrCmp $R1 2 0 leave
        StrCpy $_RUNTIME_PATH "0"
        
	leave:
		Push "$0"
FunctionEnd


Section "Base" SecBase
	SectionIn 1 2 3
	
	SetOutPath "$INSTDIR"
	File /a "..\Release\retroBase.dll"
	SetOutPath "$INSTDIR\License"
	File "/oname=License.txt" "..\License\gpl-3_0.txt" 

	;Store installation folder
	WriteRegStr HKCU "Software\RetroCode" "" $INSTDIR
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	
SectionEnd

Section "Validators" SecValidators
	SectionIn 1 3
	
	SetOutPath "$INSTDIR\validators"
	
	File /a "..\..\fid\Release\validators\retroGraphics.dll"
	File /a "..\..\fid\Release\validators\retroMono.dll"
	File /a "..\..\fid\Release\validators\retroTheme.dll"
	File /a "..\..\fid\Release\validators\retroMidi.dll"
SectionEnd

Section "Codecs" SecCodecs
	SectionIn 1 2 3

	SetOutPath "$INSTDIR"
	File /a "..\Release\avcodec-51.dll" 
	File /a "..\Release\avformat-52.dll"
	File /a "..\Release\avutil-49.dll"
	File /a "..\Release\libfaac.dll"
	File /a "..\Release\libfaad2.dll"
	File /a "..\Release\libmp3lame-0.dll"
	File /a "..\Release\libogg-0.dll"
	File /a "..\Release\libvorbis-0.dll"	
	File /a "..\Release\qscl.dll"
	
	SetOutPath "$INSTDIR\codecs"
	
	File /a "..\Release\codecs\retroApple.dll"
	File /a "..\Release\codecs\retroG711.dll"
	File /a "..\Release\codecs\retroBeatnik.dll"
	File /a "..\Release\codecs\retroMpeg.dll"
	File /a "..\Release\codecs\retroPanasonic.dll"
	File /a "..\Release\codecs\retroQualcomm.dll"
	File /a "..\Release\codecs\retroWave.dll"
	File /a "..\Release\codecs\retroYamaha.dll"
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
  Delete "$INSTDIR\qscl.dll"
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
  RMDir "$INSTDIR\Documentation"

  Delete "$INSTDIR\License\License.txt"
  RMDir "$INSTDIR\License"

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
