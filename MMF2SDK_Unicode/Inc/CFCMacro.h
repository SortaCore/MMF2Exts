
#ifndef _CFCMacro_h
#define _CFCMacro_h

#ifdef _DEBUG
#include <stdarg.h>
#endif

#ifndef _WIN32_WCE
	#ifdef WIN32
		#include	<crtdbg.h>
	#else
		#define _CrtCheckMemory() TRUE
		#include	<assert.h>
	#endif
#else
	#define _CrtCheckMemory() TRUE
#endif

// ASSERT & DEBUG MEMORY

//#if defined(_DEBUG) && defined(WIN32)
//	#define	_CRTDBG_MAP_ALLOC	1
//#endif

#ifndef DEBUG_NEW

	#ifdef _DEBUG
		#define	_ASSERTON
	#endif

	#if defined(_ASSERTON)
		#define DEBUG_NEW	new
		#ifndef _WIN32_WCE
			#ifdef WIN32
					#define	ASSERT(exp)	\
						do { if (!(exp) && \
								(1 == _CrtDbgReport(_CRT_ASSERT, THIS_FILE, __LINE__, NULL, NULL))) \
							_CrtDbgBreak(); } while (0)
			#else	
				#define	ASSERT(exp) assert(exp)
			#endif
		#endif
	#else
		#ifndef _WIN32_WCE
		#define ASSERT(exp)
		#endif
	#endif

	// TRACE
	#if !defined(TRACE)
		#if defined(_DEBUG) && defined(_WINDOWS)
			//void TraceDebug(LPCSTR lpszFormat, ...)
			//{
			//	char szBuffer[512];
			//	va_list args;
			//	va_start(args, lpszFormat);
			//
			//	vsprintf(szBuffer, lpszFormat, args);
			//	OutputDebugString(szBuffer);
			//	va_end(args);
			//}
			extern void TraceDebug(LPCSTR lpszFormat, ...);
			#define TRACE TraceDebug
		#else
			#define TRACE
		#endif	// _DEBUG
	#endif

#endif // DEBUG_NEW

// MAC macros
#ifdef _MAC
#define SWAPDWORD(a) {a=(unsigned long)((((unsigned long)(a))>>24)|(((unsigned long)(a))<<24)|((((unsigned long)(a))&0xFF00)<<8)|((((unsigned long)(a))&0xFF0000)>>8)); }
#define SWAPWORD(a) {a=(unsigned short) ((((unsigned short)(a))>>8)|(((unsigned short)(a))<<8)); }
#else
#define SWAPDWORD(a) 
#define SWAPWORD(a) 
#endif // _MAC

#define SWAPDWORD_ALWAYS(a) {a=(unsigned long)((((unsigned long)(a))>>24)|(((unsigned long)(a))<<24)|((((unsigned long)(a))&0xFF00)<<8)|((((unsigned long)(a))&0xFF0000)>>8)); }
#define SWAPWORD_ALWAYS(a) {a=(unsigned short) ((((unsigned short)(a))>>8)|(((unsigned short)(a))<<8)); }

// Directory separator
#ifdef WIN32
#define CDIR_SEP '\\'
#define SDIR_SEP "\\"
#define WILDCARD_ALLFILE "*.*"
#else
#define CDIR_SEP '/'
#define SDIR_SEP "/"
#define WILDCARD_ALLFILE "*"
#endif // WIN32

// DWORDSTR
#ifndef B2L
#define B2L(a,b,c,d)    ((ULONG)(((ULONG)((unsigned char)(d))<<24)|((ULONG)((unsigned char)(c))<<16)|((ULONG)((unsigned char)(b))<<8)|(ULONG)((unsigned char)(a))))
#endif


#endif // _CFCMacro_h
