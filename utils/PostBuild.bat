@echo off
IF "%SteamPath%" EQU "" (
    FOR /f "tokens=1,2*" %%E in ('reg query "HKEY_CURRENT_USER\Software\Valve\Steam"') DO (
        IF "%%E"=="SteamPath" (
            set SteamPath=%%G
        )
    )
)
IF "%SteamPath%" NEQ "" (
	XCOPY /R /Y /V %1 "D:\Games\Steam Library\steamapps\common\Fortress Forever\FortressForever\bin"
) ELSE (
	echo "Not able to determine Steam install path"
)