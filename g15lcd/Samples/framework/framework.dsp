# Microsoft Developer Studio Project File - Name="framework" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=framework - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "framework.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "framework.mak" CFG="framework - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "framework - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "framework - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "framework - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\shared\lcdui" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /libpath:"..\..\lib\x86"

!ELSEIF  "$(CFG)" == "framework - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\shared\lcdui" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\lib\x86"

!ENDIF 

# Begin Target

# Name "framework - Win32 Release"
# Name "framework - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Framework.cpp
# End Source File
# Begin Source File

SOURCE=.\Framework.rc
# End Source File
# Begin Source File

SOURCE=.\FrameworkDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LCDSampleScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Framework.h
# End Source File
# Begin Source File

SOURCE=.\FrameworkDlg.h
# End Source File
# Begin Source File

SOURCE=.\LCDSampleScreen.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\btn_dn.bmp
# End Source File
# Begin Source File

SOURCE=.\res\framework.ico
# End Source File
# End Group
# Begin Group "LCD UI Framework Classes"

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
# Begin Group "lglcd API"

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
