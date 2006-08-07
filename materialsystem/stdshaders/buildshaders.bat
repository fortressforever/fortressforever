@echo off
REM ****************
REM usage: buildshaders <shaderProjectName> [-xbox]
REM ****************

setlocal
set xbox_args=
set targetdir=..\..\..\game\hl2\shaders
set SrcDirBase=..\..
set ChangeToDir=../../../game/bin
set shaderDir=shaders
set FFArgs=

if "%1" == "" goto usage
set inputbase=%1

if /i "%2" == "-xbox" goto set_xbox_args
if /i "%2" == "-game" goto set_mod_args
goto build_shaders

REM ****************
REM USAGE
REM ****************
:usage
echo.
echo "usage: buildshaders <shaderProjectName> [-xbox or -game] [gameDir if -game was specified] [-source sourceDir]"
echo "       gameDir is where gameinfo.txt is (where it will store the compiled shaders)."
echo "       sourceDir is where the source code is (where it will find scripts and compilers)."
echo "ex   : buildshaders myshaders"
echo "ex   : buildshaders myshaders -game c:\steam\steamapps\sourcemods\mymod -source c:\mymod\src"
goto :end

REM ****************
REM XBOX ARGS
REM ****************
:set_xbox_args
set shaderDir=shaders_xbox
set xbox_args=-xbox -shaderOutputDir %shaderDir%
set targetdir=%vproject%\shaders
set ChangeToDir=%vproject%\..\bin
goto build_shaders

REM ****************
REM MOD ARGS - look for -game or the vproject environment variable
REM ****************
:set_mod_args

if not exist %sourcesdk%\bin\shadercompile.exe goto NoShaderCompile
set ChangeToDir=%sourcesdk%\bin

if /i "%4" NEQ "-source" goto NoSourceDirSpecified
set SrcDirBase=%~5

REM ** use the -game parameter to tell us where to put the files
set targetdir=%~3\shaders
set FFArgs=-nompi -game "%~3"

if not exist "%~3\gameinfo.txt" goto InvalidGameDirectory
goto build_shaders

REM ****************
REM ERRORS
REM ****************
:InvalidGameDirectory
echo -
echo Error: "%~3" is not a valid game directory.
echo (The -game directory must have a gameinfo.txt file)
echo -
goto end

:NoSourceDirSpecified
echo ERROR: If you specify -game on the command line, you must specify -source.
goto usage
goto end

:NoShaderCompile
echo -
echo - ERROR: shadercompile.exe doesn't exist in %sourcesdk%\bin
echo -
goto end


REM ****************
REM BUILD SHADERS
REM ****************
:build_shaders

echo --------------------------------
echo %inputbase%
echo --------------------------------
REM make sure that target dirs exist
REM files will be built in these targets and copied to their final destination
if not exist %shaderDir% mkdir %shaderDir%
if not exist %shaderDir%\fxc mkdir %shaderDir%\fxc
if not exist %shaderDir%\vsh mkdir %shaderDir%\vsh
if not exist %shaderDir%\psh mkdir %shaderDir%\psh
REM Nuke some files that we will add to later.
if exist filelist.txt del /f /q filelist.txt
if exist filestocopy.txt del /f /q filestocopy.txt

REM ****************
REM Generate a makefile for the shader project
REM ****************
"%SrcDirBase%\devtools\bin\perl" "%SrcDirBase%\devtools\bin\updateshaders.pl" %xbox_args% -source "%SrcDirBase%" %inputbase% 

REM ****************
REM Run the makefile, generating minimal work/build list for fxc files, go ahead and compile vsh and psh files.
REM ****************
rem nmake /S /C -f makefile.%inputbase% clean > clean.txt 2>&1
nmake /S /C -f makefile.%inputbase%

REM ****************
REM Add the executables to the worklist.
REM ****************
echo %SrcDirBase%\dx9sdk\utilities\vsa.exe >> filestocopy.txt
echo %SrcDirBase%\dx9sdk\utilities\psa.exe >> filestocopy.txt
echo %SrcDirBase%\dx9sdk\utilities\fxc.exe >> filestocopy.txt
echo %SrcDirBase%\devtools\bin\perl.exe >> filestocopy.txt
echo %SrcDirBase%\devtools\bin\perl58.dll >> filestocopy.txt

REM ****************
REM Add the XBox executables to the worklist.
REM ****************
if /i NOT "%2" == "-xbox" goto skip_xbox_add_files
echo %SrcDirBase%\devtools\bin\fxc_xbox.pl >> filestocopy.txt
echo %SrcDirBase%\devtools\bin\xsasm.exe >> filestocopy.txt
:skip_xbox_add_files

REM ****************
REM Cull duplicate entries in work/build list
REM ****************
if exist filestocopy.txt type filestocopy.txt | sort | "%SrcDirBase%\devtools\bin\perl" "%SrcDirBase%\devtools\bin\uniq.pl" > uniquefilestocopy.txt
if exist filelist.txt type filelist.txt | sort | "%SrcDirBase%\devtools\bin\perl" "%SrcDirBase%\devtools\bin\uniq.pl" > uniquefilelist.txt
if exist uniquefilelist.txt move uniquefilelist.txt filelist.txt

REM ****************
REM Execute distributed process on work/build list
REM ****************
rem echo "%SrcDirBase%\devtools\bin\perl" "%SrcDirBase%\materialsystem\stdshaders\runvmpi.pl" %xbox_args% -changetodir "%ChangeToDir%" %FFArgs%
"%SrcDirBase%\devtools\bin\perl" "%SrcDirBase%\materialsystem\stdshaders\runvmpi.pl" %xbox_args% -changetodir "%ChangeToDir%" %FFArgs%

REM ****************
REM Copy the generated files to the output dir using ROBOCOPY
REM ****************
if not exist "%SrcDirBase%\devtools\bin\robocopy.exe" goto DoXCopy
"%SrcDirBase%\devtools\bin\robocopy" %shaderDir% "%targetdir%" /e > robocopy.txt 2>&1
goto end

REM ****************
REM Copy the generated files to the output dir using XCOPY
REM ****************
:DoXCopy
if not exist "%targetdir%" md "%targetdir%"
xcopy %shaderDir%\*.* "%targetdir%" /e
goto end

REM ****************
REM END
REM ****************
:end

