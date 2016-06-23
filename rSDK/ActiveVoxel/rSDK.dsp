# Microsoft Developer Studio Project File - Name="rSDK" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=rSDK - Win32 Vitalize_Small
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rSDK.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rSDK.mak" CFG="rSDK - Win32 Vitalize_Small"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rSDK - Win32 Vitalize_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Vitalize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Release_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Run_Only" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rSDK - Win32 Run_Only_Small" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Vitalize_Small"
# PROP BASE Intermediate_Dir ".\Vitalize_Small"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Vitalize_Small"
# PROP Intermediate_Dir ".\Vitalize_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O1 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "COMPILE_SMALL" /D "VITALIZE" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "RUN_ONLY" /D "COMPILE_SMALL" /D "VITALIZE" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib /out:".\Vitalize_Small\Template.mox" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib /out:".\Vitalize_Small\Template.mox" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Vitalize"
# PROP BASE Intermediate_Dir ".\Vitalize"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Vitalize"
# PROP Intermediate_Dir ".\Vitalize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "VITALIZE" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "RUN_ONLY" /D "VITALIZE" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY" /d "VITALIZE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Vitalize\Template.mox" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Vitalize\Template.mox" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release_Small"
# PROP BASE Intermediate_Dir ".\Release_Small"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Small"
# PROP Intermediate_Dir ".\Release_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O1 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COMPILE_SMALL" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "COMPILE_SMALL" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /FR /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "EDITOR"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib /out:".\Release_Small\Template.mfx" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"c:\program files\multimedia fusion developer 2\extensions\rsdk.mfx" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /I "..\..\Inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\..\Inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /FR /YX /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "_DEBUG" /d "EDITOR"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "_DEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:IX86 /out:".\Debug\Template.mfx" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"c:\program files\multimedia fusion developer 2\extensions\rsdk.mfx" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /FR /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "EDITOR"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "EDITOR"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Release\Template.mfx" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"c:\program files\multimedia fusion developer 2\extensions\rsdk.mfx" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Run_Only"
# PROP BASE Intermediate_Dir ".\Run_Only"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Run_Only"
# PROP Intermediate_Dir ".\Run_Only"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "RUN_ONLY" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Run_Only\Template.mfx" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Run_Only\Template.mfx" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Run_Only_Small"
# PROP BASE Intermediate_Dir ".\Run_Only_Small"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Run_Only_Small"
# PROP Intermediate_Dir ".\Run_Only_Small"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O1 /I "..\..\Inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "RUN_ONLY" /D "COMPILE_SMALL" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\Inc" /D "NDEBUG" /D "RUN_ONLY" /D "COMPILE_SMALL" /D "WIN32" /D "_WINDOWS" /D "STRICT" /D "COXSDK" /D "IN_EXT_VERSION2" /D "RSDK" /FR /YX"TemplateInc.h" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /D "RUN_ONLY" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY"
# ADD RSC /l 0x409 /i "..\..\Inc" /d "NDEBUG" /d "RUN_ONLY"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib ..\..\lib\mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib /out:".\Run_Only_Small\Template.mfx" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mmfs2.lib /nologo /subsystem:windows /dll /machine:IX86 /out:".\Run_Only_Small\Template.mfx" /pdbtype:sept /libpath:"..\..\lib\\"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "rSDK - Win32 Vitalize_Small"
# Name "rSDK - Win32 Vitalize"
# Name "rSDK - Win32 Release_Small"
# Name "rSDK - Win32 Debug"
# Name "rSDK - Win32 Release"
# Name "rSDK - Win32 Run_Only"
# Name "rSDK - Win32 Run_Only_Small"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=Edittime.cpp
DEP_CPP_EDITT=\
	"..\..\Inc\Ccx.h"\
	"..\..\Inc\Ccxhdr.h"\
	"..\..\Inc\Cncf.h"\
	"..\..\Inc\cncr.h"\
	"..\..\Inc\Cncy.h"\
	"..\..\Inc\Cnpdll.h"\
	"..\..\Inc\colors.h"\
	"..\..\Inc\EVTCCX.H"\
	"..\..\Inc\Fill.h"\
	"..\..\Inc\MagicRDATA.h"\
	"..\..\Inc\oop_ext.h"\
	"..\..\Inc\Palet.h"\
	"..\..\Inc\PictEdDefs.h"\
	"..\..\Inc\Props.h"\
	"..\..\Inc\PTYPE.H"\
	"..\..\Inc\rCreateObject.h"\
	"..\..\Inc\rTemplate.h"\
	"..\..\Inc\Surface.h"\
	"..\..\Inc\SurfaceDefs.h"\
	"..\..\Inc\TemplateInc.h"\
	"..\..\Inc\WinMacro.h"\
	".\Common.h"\
	".\Data.h"\
	".\FlagsPrefs.h"\
	".\Information.h"\
	".\Menu.h"\
	
NODEP_CPP_EDITT=\
	"..\..\..\dll\cnpdll.h"\
	"..\..\..\events\evtccx.h"\
	"..\..\..\runtime\cncr.h"\
	"..\..\Inc\CValue.h"\
	"..\..\Inc\empty.h"\
	"..\..\Inc\win32.h"\
	

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# ADD CPP /nologo /GX /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=General.cpp
DEP_CPP_GENER=\
	"..\..\Inc\Ccx.h"\
	"..\..\Inc\Ccxhdr.h"\
	"..\..\Inc\Cncf.h"\
	"..\..\Inc\cncr.h"\
	"..\..\Inc\Cncy.h"\
	"..\..\Inc\Cnpdll.h"\
	"..\..\Inc\colors.h"\
	"..\..\Inc\EVTCCX.H"\
	"..\..\Inc\Fill.h"\
	"..\..\Inc\MagicRDATA.h"\
	"..\..\Inc\oop_ext.h"\
	"..\..\Inc\Palet.h"\
	"..\..\Inc\PictEdDefs.h"\
	"..\..\Inc\Props.h"\
	"..\..\Inc\PTYPE.H"\
	"..\..\Inc\rGetDependencies.h"\
	"..\..\Inc\rTemplate.h"\
	"..\..\Inc\Surface.h"\
	"..\..\Inc\SurfaceDefs.h"\
	"..\..\Inc\TemplateInc.h"\
	"..\..\Inc\WinMacro.h"\
	".\Common.h"\
	".\Data.h"\
	".\FlagsPrefs.h"\
	".\Information.h"\
	".\Menu.h"\
	
NODEP_CPP_GENER=\
	"..\..\..\dll\cnpdll.h"\
	"..\..\..\events\evtccx.h"\
	"..\..\..\runtime\cncr.h"\
	"..\..\Inc\CValue.h"\
	"..\..\Inc\empty.h"\
	"..\..\Inc\win32.h"\
	

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# ADD CPP /nologo /GX /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=Main.cpp
DEP_CPP_MAIN_=\
	"..\..\Inc\Ccx.h"\
	"..\..\Inc\Ccxhdr.h"\
	"..\..\Inc\Cncf.h"\
	"..\..\Inc\cncr.h"\
	"..\..\Inc\Cncy.h"\
	"..\..\Inc\Cnpdll.h"\
	"..\..\Inc\colors.h"\
	"..\..\Inc\EVTCCX.H"\
	"..\..\Inc\Fill.h"\
	"..\..\Inc\MagicRDATA.h"\
	"..\..\Inc\oop_ext.h"\
	"..\..\Inc\Palet.h"\
	"..\..\Inc\PictEdDefs.h"\
	"..\..\Inc\Props.h"\
	"..\..\Inc\PTYPE.H"\
	"..\..\Inc\rTemplate.h"\
	"..\..\Inc\Surface.h"\
	"..\..\Inc\SurfaceDefs.h"\
	"..\..\Inc\TemplateInc.h"\
	"..\..\Inc\WinMacro.h"\
	".\Common.h"\
	".\Data.h"\
	".\FlagsPrefs.h"\
	".\Information.h"\
	".\Menu.h"\
	
NODEP_CPP_MAIN_=\
	"..\..\..\dll\cnpdll.h"\
	"..\..\..\events\evtccx.h"\
	"..\..\..\runtime\cncr.h"\
	"..\..\Inc\CValue.h"\
	"..\..\Inc\empty.h"\
	"..\..\Inc\win32.h"\
	

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# ADD CPP /nologo /GX /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=Runtime.cpp
DEP_CPP_RUNTI=\
	"..\..\Inc\Ccx.h"\
	"..\..\Inc\Ccxhdr.h"\
	"..\..\Inc\Cncf.h"\
	"..\..\Inc\cncr.h"\
	"..\..\Inc\Cncy.h"\
	"..\..\Inc\Cnpdll.h"\
	"..\..\Inc\colors.h"\
	"..\..\Inc\EVTCCX.H"\
	"..\..\Inc\Fill.h"\
	"..\..\Inc\MagicRDATA.h"\
	"..\..\Inc\oop_ext.h"\
	"..\..\Inc\Palet.h"\
	"..\..\Inc\PictEdDefs.h"\
	"..\..\Inc\Props.h"\
	"..\..\Inc\PTYPE.H"\
	"..\..\Inc\rCreateRunObject.h"\
	"..\..\Inc\rTemplate.h"\
	"..\..\Inc\Surface.h"\
	"..\..\Inc\SurfaceDefs.h"\
	"..\..\Inc\TemplateInc.h"\
	"..\..\Inc\WinMacro.h"\
	".\Common.h"\
	".\Data.h"\
	".\FlagsPrefs.h"\
	".\Information.h"\
	".\Menu.h"\
	
NODEP_CPP_RUNTI=\
	"..\..\..\dll\cnpdll.h"\
	"..\..\..\events\evtccx.h"\
	"..\..\..\runtime\cncr.h"\
	"..\..\Inc\CValue.h"\
	"..\..\Inc\empty.h"\
	"..\..\Inc\win32.h"\
	

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# ADD CPP /nologo /GX /O1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=Common.h
# End Source File
# Begin Source File

SOURCE=.\Data.h
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=Ext.def
# End Source File
# Begin Source File

SOURCE=.\FlagsPrefs.h
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Information.h
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Menu.h
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=Ext.rc

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP Exclude_From_Build 1
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409

!ENDIF 

# End Source File
# Begin Source File

SOURCE=EXTICON.BMP

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=EXTIMG.BMP

!IF  "$(CFG)" == "rSDK - Win32 Vitalize_Small"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Vitalize"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release_Small"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Debug"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Release"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only"

!ELSEIF  "$(CFG)" == "rSDK - Win32 Run_Only_Small"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=Resource.h
# End Source File
# End Group
# Begin Group "MMF Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\..\..\..\Program Files\Microsoft Visual Studio\Vc98\Include\Basetsd.h"
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Ccx.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Ccxhdr.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\CfcError.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\CfcFile.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Cncf.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\cncr.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Cncy.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Cnpdll.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\colors.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\EVTCCX.H
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Fill.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\FilterMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\ImageFlt.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\ImgFlt.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\MagicRDATA.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\oop_ext.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Palet.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\PictEdDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Props.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\PTYPE.H
# End Source File
# Begin Source File

SOURCE=..\..\Inc\rCreateObject.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\rCreateRunObject.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\rGetDependencies.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\rTemplate.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\SoundFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\SoundFilterMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\Surface.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\SurfaceDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\TemplateInc.h
# End Source File
# Begin Source File

SOURCE=..\..\Inc\WinMacro.h
# End Source File
# End Group
# End Target
# End Project
