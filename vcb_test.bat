call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86


set INCLUDE=%INCLUDE%C:\Program Files\Microsoft Platform SDK\Include;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\include;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\include;C:\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0\include;
set LIB=%PSDK%\Lib;%LIB%C:\Program Files\Microsoft Platform SDK\Lib;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\lib;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\lib;C:\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0\lib;

vcbuild /r /u /platform:Win32 D:\Dexter\Documents\GitHub\fortressforever\cl_dll\client_scratch-2005.vcproj "Release FF|Win32"

pause