
// Definitions and compilation options for Release_Small,
// Run_Only_Small and Vitalize_Small configurations
//
// z33z modifications
#ifdef COMPILE_SMALL
#ifdef NDEBUG

#ifdef _OPT_MAX_					// Not set by default: optimisation options should be defined in the project settings
#pragma optimize("gsy",on)
#endif // _OPT_MAX_

#pragma comment(linker,"/RELEASE")

#if _MSC_VER >= 1000				// Slightly slower loading time under Windows 9x, but may save some Kb
#pragma comment(linker,"/opt:nowin98")
#endif // _MSC_VER >= 1000

#endif // NDEBUG
#endif // COMPILE_SMALL

// end of z33z stuff

// MAIN INCLUDES
// =============
#ifndef ccxhdr_h
#define ccxhdr_h

	// C
#include	<windows.h>
#include	<mmsystem.h>
#include	<stdio.h>
#ifdef WIN32
#include	<io.h>
#else
#include	<dos.h>
#endif
#include	<string.h>
#include	<stdlib.h>
#include	<stddef.h>
#include	<ctype.h>
#include	<tchar.h>
#include	<math.h>
#include	<limits.h>
#ifndef _LINUX
#include	<crtdbg.h>
#else
#include	<assert.h>
#endif

#if !defined(ASSERT)
	#ifdef _DEBUG
		#define	_ASSERTON
	#endif

	#if defined(_ASSERTON)
		#define DEBUG_NEW	new
	#endif

	#if defined(_ASSERTON) && !defined(_68K_)
		#define DEBUG_NEW	new

		#ifdef WIN32
				#define	ASSERT(exp)	\
					do { if (!(exp) && \
							(1 == _CrtDbgReport(_CRT_ASSERT, THIS_FILE, __LINE__, NULL, NULL))) \
						_CrtDbgBreak(); } while (0)
		#else
			#define	ASSERT(exp)	\
						if (exp)	\
							NULL;	\
						else		\
							_assert("", THIS_FILE, __LINE__)
		#endif
	#else
		#define ASSERT(exp)
	#endif
#endif

// For DLL include files
#define STDDLL_IMPORTS
#define IMGFLTMGR_IMPORTS
#define SNDMGR_IMPORTS
#define SURFACES_IMPORTS
#define ZCOMPDLL_IMPORTS

// For runtime include files
#define	IN_KPX
#define	RUN_TIME

// Runtime include files
#include	"ptype.h"
#include	"WinMacro.h"
#include	"Colors.h"

#ifdef COXSDK
#include	"cnpdll.h"
#include	"cncr.h"
#include	"evtccx.h"
#include	"Props.h"
#else
#include	"..\..\dll\cnpdll.h"
#include	"..\..\runtime\cncr.h"
#include	"..\..\events\evtccx.h"
#include	"..\inc\props.h"
#endif

// Other definitions
#include	"ccx.h"

#endif // ndef ccxhdr_h
