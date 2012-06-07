# Microsoft Developer Studio Project File - Name="cvaux" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cvaux - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cvaux.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cvaux.mak" CFG="cvaux - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cvaux - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cvaux - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cvaux - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cvaux_Release"
# PROP Intermediate_Dir "..\..\_temp\cvaux_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "cvaux_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /Zi /O2 /Ob2 /I "." /I ".." /I "../include" /I "../../cxcore/include" /I "../../cv/include" /I "../../cv/src" /D "NDEBUG" /D "CVAPI_EXPORTS" /D "WIN32" /D "_WINDOWS" /Yu"_cvaux.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 cv.lib cxcore.lib kernel32.lib user32.lib gdi32.lib /nologo /dll /pdb:"..\..\bin/cvaux110.pdb" /debug /machine:I386 /nodefaultlib:"libmmd.lib" /out:"..\..\bin/cvaux110.dll" /implib:"..\..\lib/cvaux.lib" /libpath:"..\..\lib"
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "cvaux - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cvaux_Debug"
# PROP Intermediate_Dir "..\..\_temp\cvaux_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "cvaux_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /Zi /Od /I "." /I ".." /I "../include" /I "../../cxcore/include" /I "../../cv/include" /I "../../cv/src" /D "_DEBUG" /D "CVAPI_EXPORTS" /D "WIN32" /D "_WINDOWS" /FR /Yu"_cvaux.h" /FD /GZ /Zm200 /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cvd.lib cxcored.lib kernel32.lib user32.lib gdi32.lib /nologo /dll /pdb:"..\..\bin/cvaux110d.pdb" /debug /machine:I386 /nodefaultlib:"libmmdd.lib" /out:"..\..\bin/cvaux110d.dll" /implib:"..\..\lib/cvauxd.lib" /pdbtype:sept /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cvaux - Win32 Release"
# Name "cvaux - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "VideoSurveillance"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vs\bgfg_estimation.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackanalysis.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackanalysishist.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackanalysisior.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackanalysistrackdist.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackgen1.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackgenyml.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingauto.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingcc.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingccwithcr.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingkalman.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackinglist.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingmsfg.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackingmsfgs.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackpostprockalman.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackpostproclinear.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\blobtrackpostproclist.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\enteringblobdetection.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\enteringblobdetectionreal.cpp
# End Source File
# Begin Source File

SOURCE=.\vs\TestSeq.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\camshift.cpp
# End Source File
# Begin Source File

SOURCE=.\cv3dtracker.cpp
# End Source File
# Begin Source File

SOURCE=.\cvaux.cpp
# End Source File
# Begin Source File

SOURCE=.\cvaux.rc
# End Source File
# Begin Source File

SOURCE=.\cvauxutils.cpp
# End Source File
# Begin Source File

SOURCE=.\cvbgfg_acmmm2003.cpp
# End Source File
# Begin Source File

SOURCE=.\cvbgfg_codebook.cpp
# End Source File
# Begin Source File

SOURCE=.\cvbgfg_common.cpp
# End Source File
# Begin Source File

SOURCE=.\cvbgfg_gaussmix.cpp
# End Source File
# Begin Source File

SOURCE=.\cvcalibfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\cvcorrespond.cpp
# End Source File
# Begin Source File

SOURCE=.\cvcorrimages.cpp
# End Source File
# Begin Source File

SOURCE=.\cvcreatehandmask.cpp
# End Source File
# Begin Source File

SOURCE=.\cvdpstereo.cpp
# End Source File
# Begin Source File

SOURCE=.\cveigenobjects.cpp
# End Source File
# Begin Source File

SOURCE=.\cvepilines.cpp
# End Source File
# Begin Source File

SOURCE=.\cvface.cpp
# End Source File
# Begin Source File

SOURCE=.\cvfacedetection.cpp
# End Source File
# Begin Source File

SOURCE=.\cvfacetemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\cvfindface.cpp
# End Source File
# Begin Source File

SOURCE=.\cvfindhandregion.cpp
# End Source File
# Begin Source File

SOURCE=.\cvhmm.cpp
# End Source File
# Begin Source File

SOURCE=.\cvhmm1d.cpp
# End Source File
# Begin Source File

SOURCE=.\cvhmmobs.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlcm.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlee.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlevmar.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlevmarprojbandle.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlevmartrif.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlines.cpp
# End Source File
# Begin Source File

SOURCE=.\cvlmeds.cpp
# End Source File
# Begin Source File

SOURCE=.\cvmorphcontours.cpp
# End Source File
# Begin Source File

SOURCE=.\cvmorphing.cpp
# End Source File
# Begin Source File

SOURCE=.\cvprewarp.cpp
# End Source File
# Begin Source File

SOURCE=.\cvscanlines.cpp
# End Source File
# Begin Source File

SOURCE=.\cvsegment.cpp
# End Source File
# Begin Source File

SOURCE=.\cvsubdiv2.cpp
# End Source File
# Begin Source File

SOURCE=.\cvtexture.cpp
# End Source File
# Begin Source File

SOURCE=.\cvtrifocal.cpp
# End Source File
# Begin Source File

SOURCE=.\cvvecfacetracking.cpp
# End Source File
# Begin Source File

SOURCE=.\cvvideo.cpp
# End Source File
# Begin Source File

SOURCE=.\precomp.cpp
# ADD CPP /Yc"_cvaux.h"
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\_cvaux.h
# End Source File
# Begin Source File

SOURCE=.\_cvfacedetection.h
# End Source File
# Begin Source File

SOURCE=.\_cvvectrack.h
# End Source File
# Begin Source File

SOURCE=.\_cvvm.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\cvaux.h
# End Source File
# Begin Source File

SOURCE=..\include\cvaux.hpp
# End Source File
# Begin Source File

SOURCE=..\include\cvvidsurv.hpp
# End Source File
# End Group
# End Group
# End Target
# End Project
