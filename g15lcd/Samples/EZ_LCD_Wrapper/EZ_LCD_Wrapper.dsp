# Microsoft Developer Studio Project File - Name="EZ_LCD_Wrapper" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=EZ_LCD_Wrapper - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EZ_LCD_Wrapper.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EZ_LCD_Wrapper.mak" CFG="EZ_LCD_Wrapper - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EZ_LCD_Wrapper - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "EZ_LCD_Wrapper - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/PC/LCD/Src/Shared/EZ_LCD_Wrapper", LGIDAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EZ_LCD_Wrapper - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W4 /GX /O2 /I "." /I "..\Shared\lcdui" /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../Lib/x86"

!ELSEIF  "$(CFG)" == "EZ_LCD_Wrapper - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "." /I "..\Shared\lcdui" /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../Lib/x86"

!ENDIF 

# Begin Target

# Name "EZ_LCD_Wrapper - Win32 Release"
# Name "EZ_LCD_Wrapper - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\EZ_LCD.cpp
# SUBTRACT CPP /I "." /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\EZ_LCD_Wrapper.cpp
# SUBTRACT CPP /I "." /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\EZ_LCD.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "LCDUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\shared\lcdui\LCDAnimatedBitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDAnimatedBitmap.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDBase.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDBase.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDBitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDBitmap.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDCollection.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDCollection.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDGfx.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDGfx.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDIcon.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDManager.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDManager.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDOutput.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDOutput.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDScrollingText.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDScrollingText.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDStreamingText.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDStreamingText.h
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDText.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\lcdui\LCDText.h
# End Source File
# End Group
# Begin Group "SDK Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\lglcd.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
