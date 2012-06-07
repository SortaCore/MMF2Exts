# Microsoft Developer Studio Project File - Name="cxcore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cxcore - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cxcore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cxcore.mak" CFG="cxcore - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cxcore - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cxcore - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cxcore - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cxcore_Release"
# PROP Intermediate_Dir "..\..\_temp\cxcore_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W4 /Zi /O2 /Ob2 /I "." /I "..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "CVAPI_EXPORTS" /FR /Yu"_cxcore.h" /FD /Qopenmp /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:windows /dll /pdb:"..\..\bin/cxcore110.pdb" /debug /machine:I386 /nodefaultlib:"libmmd.lib" /out:"..\..\bin\cxcore110.dll" /implib:"../../lib/cxcore.lib"
# SUBTRACT LINK32 /profile /pdb:none /map

!ELSEIF  "$(CFG)" == "cxcore - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cxcore_Debug"
# PROP Intermediate_Dir "..\..\_temp\cxcore_Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W4 /Gm /Zi /Od /I "." /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "CVAPI_EXPORTS" /FR /Yu"_cxcore.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:windows /dll /pdb:"..\..\bin/cxcore110d.pdb" /debug /machine:I386 /nodefaultlib:"libmmdd.lib" /out:"..\..\bin\cxcore110d.dll" /implib:"../../lib/cxcored.lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cxcore - Win32 Release"
# Name "cxcore - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cxalloc.cpp
# End Source File
# Begin Source File

SOURCE=.\cxarithm.cpp
# End Source File
# Begin Source File

SOURCE=.\cxarray.cpp
# End Source File
# Begin Source File

SOURCE=.\cxcmp.cpp
# End Source File
# Begin Source File

SOURCE=.\cxconvert.cpp
# End Source File
# Begin Source File

SOURCE=.\cxcopy.cpp
# End Source File
# Begin Source File

SOURCE=.\cxcore.rc
# End Source File
# Begin Source File

SOURCE=.\cxdatastructs.cpp
# End Source File
# Begin Source File

SOURCE=.\cxdrawing.cpp
# End Source File
# Begin Source File

SOURCE=.\cxdxt.cpp
# End Source File
# Begin Source File

SOURCE=.\cxerror.cpp
# End Source File
# Begin Source File

SOURCE=.\cximage.cpp
# End Source File
# Begin Source File

SOURCE=.\cxjacobieigens.cpp
# End Source File
# Begin Source File

SOURCE=.\cxlogic.cpp
# End Source File
# Begin Source File

SOURCE=.\cxlut.cpp
# End Source File
# Begin Source File

SOURCE=.\cxmathfuncs.cpp
# End Source File
# Begin Source File

SOURCE=.\cxmatmul.cpp
# End Source File
# Begin Source File

SOURCE=.\cxmatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\cxmean.cpp
# End Source File
# Begin Source File

SOURCE=.\cxmeansdv.cpp
# End Source File
# Begin Source File

SOURCE=.\cxminmaxloc.cpp
# End Source File
# Begin Source File

SOURCE=.\cxnorm.cpp
# End Source File
# Begin Source File

SOURCE=.\cxouttext.cpp
# End Source File
# Begin Source File

SOURCE=.\cxpersistence.cpp
# End Source File
# Begin Source File

SOURCE=.\cxprecomp.cpp
# ADD CPP /Yc"_cxcore.h"
# End Source File
# Begin Source File

SOURCE=.\cxrand.cpp
# End Source File
# Begin Source File

SOURCE=.\cxsumpixels.cpp
# End Source File
# Begin Source File

SOURCE=.\cxsvd.cpp
# End Source File
# Begin Source File

SOURCE=.\cxswitcher.cpp
# End Source File
# Begin Source File

SOURCE=.\cxtables.cpp
# End Source File
# Begin Source File

SOURCE=.\cxutils.cpp
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\cxcore.h
# End Source File
# Begin Source File

SOURCE=..\include\cxcore.hpp
# End Source File
# Begin Source File

SOURCE=..\include\cxerror.h
# End Source File
# Begin Source File

SOURCE=..\include\cxmisc.h
# End Source File
# Begin Source File

SOURCE=..\include\cxtypes.h
# End Source File
# End Group
# Begin Group "Internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\_cxcore.h
# End Source File
# Begin Source File

SOURCE=.\_cxipp.h
# End Source File
# End Group
# End Group
# End Target
# End Project
