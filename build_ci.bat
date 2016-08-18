@ECHO OFF

call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall.bat"  x86

set PSDK=C:\Program Files\Microsoft Platform SDK

set othersinclude=C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\include;C:\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0\include;
set otherslib=C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\lib;C:\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0\lib;

set INCLUDE=%PSDK%\Include;%INCLUDE%%othersinclude%
set LIB=%PSDK%\Lib;%LIB%%otherslib%

set UseEnv=true
set useenv=true

echo lib = %LIB%
echo include = %include%

rem /property:VCBuildUseEnvironment=true
msbuild Game_Scratch-2005.sln /v:diag /p:useenv=true /p:Configuration="Release FF" 
pause