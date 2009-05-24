!define PRODUCT_NAME "Fortress Forever"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "Fortress Forever Team"
!define PRODUCT_WEB_SITE "http://www.fortress-forever.com"

!define APPID 215

!define MODDIR "FortressForever"
;(i.e. cstrike)

!define LOCALDIR "D:\Steam\SteamApps\SourceMods\FortressForever"
;This value should be set depending on the PC this will be compiled under

!define MUI_ICON "${LOCALDIR}\Fortress Forever.ico"
;This is the location on the local computer where the icon for the installer executable is.

!define FULL_GAME_NAME "Fortress Forever"
;This is the name of the game defined in gameinfo.txt
;Remove all colons from this (as well as any other invalid char in a file name)

!define DESKICO "${LOCALDIR}\Fortress Forever.ico"
;This is the location on the local computer where the icon for the desktop icon for the game is.
;The icon name must be the same as the one in gameinfo.txt
;Remove all colons from this (as well as any other invalid char in a file name)

;Uncomment this if you do not wish to have a custom desktop icon:
;!define NO_DESKTOP_ICON

;Uncomment this if you want to use ZipDLL
;The zip file should be in the LOCALDIR for this
;!define ZIPDLL


;======DO NOT EDIT BEYOND THIS POINT======

!ifdef ZIPDLL
	!include "zipdll.nsh"
!endif

var ICONDIR
var STEAMEXE

!include "MUI.nsh"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME}.exe"
ShowInstDetails show

Section "Mod Files" FILES
	SetOverwrite ifdiff
	SetOutPath "$INSTDIR"
!ifdef ZIPDLL
	File "${LOCALDIR}\${MODDIR}.zip"
	StrCpy $R0 "$INSTDIR\${MODDIR}.zip"
	ZipDLL::extractall "$R0" "$INSTDIR"
	Delete "$R0"
!else
	File /r "${LOCALDIR}"
!endif

!ifndef NO_DESKTOP_ICON
	SetOutPath "$ICONDIR"
	File "${DESKICO}"
!endif

SectionEnd
!ifndef NO_DESKTOP_ICON
Section "Desktop Shortcut" SHORTCUT
	SetOutPath "$DESKTOP"
	CreateShortcut "${FULL_GAME_NAME}.lnk" $STEAMEXE \
		'-applaunch ${APPID} -game "$INSTDIR\${MODDIR}"' "$ICONDIR\${FULL_GAME_NAME}.ico"
SectionEnd
!endif
Page custom Finish
Function Finish
  MessageBox MB_OK|MB_ICONEXCLAMATION "Steam must be restarted for the game to show on the games list."
FunctionEnd
Function .onInit
	ReadRegStr $R0 HKLM "Software\Valve\Steam" InstallPath
	ReadRegStr $R1 HKCU "Software\Valve\Steam" SourceModInstallPath
	IfErrors lbl_error 0
	StrCpy $INSTDIR "$R1"
!ifndef NO_DESKTOP_ICON
	StrCpy $ICONDIR "$R0\steam\games"
	SectionSetFlags ${SHORTCUT} 0
	StrCpy $STEAMEXE "$R0\steam.exe"
!endif
	SectionSetFlags ${FILES} 17
	Return
	lbl_error:
		ClearErrors
		SectionSetFlags ${FILES} 17
FunctionEnd