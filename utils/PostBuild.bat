@echo off
setlocal enabledelayedexpansion
IF "%SteamPath%" EQU "" (
    FOR /f "tokens=1,2*" %%E in ('reg query "HKEY_CURRENT_USER\Software\Valve\Steam"') DO (
        IF "%%E"=="SteamPath" (
            set SteamPath=%%G
        )
    )
)
IF "%SteamPath%" EQU "" (
    echo Not able to determine Steam install path
    exit /b 0
)

if NOT EXIST "%SteamPath%\steamapps\libraryfolders.vdf" (
    echo|set /p="%SteamPath%\steamapps\libraryfolders.vdf not found"
    exit /b 0
)

for /F "usebackq tokens=*" %%L in ("%SteamPath%\steamapps\libraryfolders.vdf") do (
    for /F "tokens=1*" %%A in ("%%L") do (
        if "%%B" NEQ "" (
            if "%%~A" EQU "path" (
                set "CurPath=%%~B"
                set "CurPath=!CurPath:\\=\!"
            )
            :: 253530 is the FF appid
            if "%%~A" EQU "253530" (
                set "FFBinPath=!CurPath!\steamapps\common\Fortress Forever\FortressForever\bin"
            )
        )
    )
)

if "%FFBinPath%" EQU "" (
    echo|set /p="Fortress Forever appid not found in %SteamPath%\steamapps\libraryfolders.vdf"
    exit /b 0
)

XCOPY /R /Y /V %1 "%FFBinPath%"
