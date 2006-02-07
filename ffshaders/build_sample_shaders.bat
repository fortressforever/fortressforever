@echo off
setlocal


rem **** Make sure the parameters are specified.
if /i "%1" NEQ "-game" goto ShowUsage
set __GameDir=%~2
if not exist "%__GameDir%\gameinfo.txt" goto MissingGameInfo


rem **** Call the batch files to build our stuff.
call ..\materialsystem\stdshaders\buildshaders.bat ff_shaders -game "%__GameDir%" -source ..
goto end


:MissingGameInfo
echo Invalid -game parameter specified (no "%__GameDir%\gameinfo.txt" exists).
goto end

:ShowUsage
echo build_sample_shaders.bat -game [game directory]
goto end


:end
