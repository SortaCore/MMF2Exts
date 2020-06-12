#pragma once

// #define TGFEXT	// TGF2, MMF2, MMF2 Dev
#define MMFEXT		// MMF2, MMF2 Dev
// #define PROEXT	// MMF2 Dev only

#ifdef RUN_ONLY
	#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#else
	const extern struct _json_value & CurrentLanguage();
	#define CurLang CurrentLanguage()
#endif
#define JSON_COMMENT_MACRO Extension::Version

#pragma comment(lib, "..\\Lib\\Windows\\dbghelp.lib")

#include "Edif.h"
#include "Resource.h"
#include "DarkEdif.h"
#include <time.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#pragma warning (push)
#pragma warning (disable: 4091) // random crud about typedef before enum ignored
#include <dbghelp.h>
#pragma warning (pop)

// If your extension will be using multithreading, remove the #if and #endif lines here.
#define MULTI_THREADING
#include "MultiThreading.h"
#include <atomic>

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// DarkEdif properties
	int				DarkEdif_Prop_Size;
	char			DarkEdif_Props[];

	// DarkEdif functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	std::tstring GetPropertyStr(const char * propName);
	std::tstring GetPropertyStr(int propID);
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


