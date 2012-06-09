# Microsoft Developer Studio Project File - Name="V2Template32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=V2Template32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "V2Template32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "V2Template32.mak" CFG="V2Template32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "V2Template32 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Run_Only" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Release_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Run_Only_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Vitalize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "V2Template32 - Win32 Vitalize_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "V2Template32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "V2TEMPLATE32_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Release/packx.cox"

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "V2TEMPLATE32_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /Fr /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /debug /machine:I386 /out:".\Debug/packx.cox"
# Begin Custom Build - *** Transferring your extension to MMF's directory! ***
InputDir=.\Debug
WkspDir=.
WkspName=V2Template32
InputPath=.\Debug\packx.cox
SOURCE="$(InputPath)"

"$(WkspName)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(WkspDir)\$(InputDir)\transfer.bat

# End Custom Build

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Run_Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "V2Template32___Win32_Run_Only"
# PROP BASE Intermediate_Dir "V2Template32___Win32_Run_Only"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Run_Only"
# PROP Intermediate_Dir ".\Run_Only"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Run_Only/packx.cox"

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Release_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "V2Template32___Win32_Release_Small"
# PROP BASE Intermediate_Dir "V2Template32___Win32_Release_Small"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Small"
# PROP Intermediate_Dir ".\Release_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /W3 /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COMPILE_SMALL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Release/V2Template32.cox"
# ADD LINK32 kernel32.lib user32.lib msvcrt.lib ..\lib\cncs32.lib shell32.lib /nologo /dll /machine:I386 /nodefaultlib /out:".\Release_Small/packx.cox"
# Begin Custom Build - *** Transferring your extension to MMF's directory! ***
InputDir=.\Release_Small
WkspDir=.
WkspName=V2Template32
InputPath=.\Release_Small\packx.cox
SOURCE="$(InputPath)"

"$(WkspName)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(WkspDir)\$(InputDir)\transfer.bat

# End Custom Build

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Run_Only_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "V2Template32___Win32_Run_Only_Small"
# PROP BASE Intermediate_Dir "V2Template32___Win32_Run_Only_Small"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Run_Only_Small"
# PROP Intermediate_Dir ".\Run_Only_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /YX /FD /c
# ADD CPP /nologo /W3 /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "COMPILE_SMALL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY"
# ADD RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Run_Only/V2Template32.cox"
# ADD LINK32 kernel32.lib user32.lib msvcrt.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /nodefaultlib /out:".\Run_Only_Small/packx.cox"

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Vitalize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "V2Template32___Win32_Vitalize"
# PROP BASE Intermediate_Dir "V2Template32___Win32_Vitalize"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Vitalize"
# PROP Intermediate_Dir ".\Vitalize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "VITALIZE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY"
# ADD RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Run_Only/V2Template32.cox"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /out:".\Vitalize\V2Template32.mox"

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Vitalize_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "V2Template32___Win32_Vitalize_Small"
# PROP BASE Intermediate_Dir "V2Template32___Win32_Vitalize_Small"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Vitalize_Small"
# PROP Intermediate_Dir ".\Vitalize_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "COMPILE_SMALL" /YX /FD /c
# ADD CPP /nologo /W3 /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "COMPILE_SMALL" /D "VITALIZE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY"
# ADD RSC /l 0x409 /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib msvcrt.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /nodefaultlib /out:".\Run_Only_Small/V2Template32.cox"
# ADD LINK32 kernel32.lib user32.lib msvcrt.lib ..\lib\cncs32.lib /nologo /dll /machine:I386 /nodefaultlib /out:".\Vitalize_Small/V2Template32.mox"

!ENDIF 

# Begin Target

# Name "V2Template32 - Win32 Release"
# Name "V2Template32 - Win32 Debug"
# Name "V2Template32 - Win32 Run_Only"
# Name "V2Template32 - Win32 Release_Small"
# Name "V2Template32 - Win32 Run_Only_Small"
# Name "V2Template32 - Win32 Vitalize"
# Name "V2Template32 - Win32 Vitalize_Small"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Edittime.cpp
# End Source File
# Begin Source File

SOURCE=.\Extobj.rc

!IF  "$(CFG)" == "V2Template32 - Win32 Release"

# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Debug"

# PROP Intermediate_Dir ".\Debug"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Run_Only"

# PROP BASE Intermediate_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Release_Small"

# PROP BASE Intermediate_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Run_Only_Small"

# PROP BASE Intermediate_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Vitalize"

# PROP BASE Intermediate_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "V2Template32 - Win32 Vitalize_Small"

# PROP BASE Intermediate_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Extobj32.def
# End Source File
# Begin Source File

SOURCE=.\General.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\RunOnly.cpp
# End Source File
# Begin Source File

SOURCE=.\Runtime.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Common.h
# End Source File
# Begin Source File

SOURCE=.\Main.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\EXTICON.BMP
# End Source File
# Begin Source File

SOURCE=.\EXTIMG.BMP
# End Source File
# End Group
# Begin Group "Information"

# PROP Default_Filter "txt"
# End Group
# Begin Group "MMF Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\Vc98\Include\Basetsd.h"
# End Source File
# Begin Source File

SOURCE=..\Inc\Ccx.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Ccxhdr.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Mmf\Cncf.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Mmf\Cncy.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Cnpdll.h
# End Source File
# Begin Source File

SOURCE=..\Inc\colors.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Mmf\Evtccx.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Ptype.h
# End Source File
# Begin Source File

SOURCE=..\Inc\Surface.h
# End Source File
# Begin Source File

SOURCE=..\Inc\WinMacro.h
# End Source File
# End Group
# End Target
# End Project
