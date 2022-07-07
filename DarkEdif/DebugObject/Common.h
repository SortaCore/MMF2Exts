#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only

#define JSON_COMMENT_MACRO Extension::Version

#pragma comment(lib, "..\\Lib\\Windows\\dbghelp.lib")

#include "DarkEdif.h"
#include <time.h>
#include <iostream>
#include <iomanip>

#pragma warning (push)
#pragma warning (disable: 4091) // random crud about typedef before enum ignored
#include <dbghelp.h>
#pragma warning (pop)

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	NO_DEFAULT_CTORS(EDITDATA);
	// Header - required, must be first variable in EDITDATA
	extHeader		eHeader;

	// Keep DarkEdif variables as last. Undefined behaviour otherwise.
	DarkEdif::Properties Props;
};

class Extension;

struct RUNDATA
{
	// Main header - required
	HeaderObject rHo;

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
//	rCom			rc;				// Common structure for movements & animations
//	rMvt			rm;				// Movements
//	rSpr			rs;				// Sprite (displayable objects)
//	rVal			rv;				// Alterable values

	// Required
	Extension * pExtension;

	/*
		You shouldn't add any variables or anything here. Add them as members
		of the Extension class (Extension.h) instead.
	*/
};
struct GlobalData {
	FILE * fileHandle;
	std::atomic<bool> readingThis, releaseConsoleInput;
	bool debugEnabled, doMsgBoxIfPathNotSet, consoleEnabled;
	time_t rawtime;
	struct tm * timeinfo;
	TCHAR timeFormat[128];
	TCHAR realTime[128];
	unsigned char numUsages;
	HANDLE consoleIn, consoleOut;
	std::tstring consoleReceived;
	unsigned char consoleBreakType;
	std::tstring miniDumpPath;
	int miniDumpType; // See MINIDUMP_TYPE enum

	// Allows debugging of what had the last lock
	#ifdef _DEBUG
		std::string lastLockFile;
		int lastLockLine;
		#define OpenLock() \
			while (data->readingThis) \
				Sleep(0); \
			data->readingThis = true; \
			data->lastLockFile = __FILE__; \
			data->lastLockLine = __LINE__
		#define CloseLock() data->readingThis = false
	#else
		#define OpenLock() \
			while (data->readingThis) \
				Sleep(0); \
			data->readingThis = true
		#define CloseLock() data->readingThis = false
	#endif

	// Handle exceptions
	enum HandleType : char {
		HANDLE_VIA_QUIT,
		HANDLE_VIA_INFINITE_WAIT,
		HANDLE_VIA_IGNORE,
		HANDLE_VIA_CONTINUE,
		HANDLE_VIA_EMAIL,
		HANDLE_VIA_BOOT_PROCESS,
		HANDLE_VIA_FORCE_DEBUG_BREAK,

		HANDLE_VIA_MAX
	};
	HandleType handleExceptionVia;
	char continuesRemaining, continuesMax;
};

#define GlobalID _T("DebugObject")
extern class Extension * GlobalExt;

extern BOOL WINAPI HandlerRoutine(DWORD ControlType);
extern DWORD WINAPI ReceiveConsoleInput(void *);
extern LONG WINAPI UnhandledExceptionCatcher(PEXCEPTION_POINTERS pExceptionPtrs);
extern bool AttachDebugger();

#include "Extension.h"


