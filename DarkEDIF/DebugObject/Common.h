#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

#ifdef RUN_ONLY
	#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#else
	const extern struct _json_value & CurrentLanguage();
	#define CurLang CurrentLanguage()
#endif
#define JSON_COMMENT_MACRO Extension::Version

#define DLLExport   __stdcall
#pragma comment(lib, "..\\Lib\\mmfs2.lib")
#pragma comment(lib, "..\\Lib\\dbghelp.lib")

#include    "Edif.h"
#include	"Resource.h"
#include	"DarkEdif.h"
#include	<time.h>
#include	<sstream>
#include	<iostream>
#include	<iomanip>

#pragma warning (push)
#pragma warning (disable: 4091) // random crud about typedef before enum ignored
#include	<dbghelp.h>
#pragma warning (pop)

// If your extension will be using multithreading, remove the #if and #endif lines here.
#define MULTI_THREADING
#include "MultiThreading.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
	bool EnableAtStart;
	bool DoMsgBoxIfPathNotSet;
	char InitialPath [MAX_PATH+1];
	bool ConsoleEnabled;
	bool PauseForDebugger;
	
	// TODO: Extend properties to default handling type?
	int				DarkEDIF_Prop_Size;
	char			DarkEDIF_Props[0];

	// DarkEDIF functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	const char * GetPropertyStr(const char * propName);
	const char * GetPropertyStr(int propID);
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
	FILE * FileHandle;
	volatile bool ReadingThis, ReleaseConsoleInput;
	bool DebugEnabled, DoMsgBoxIfPathNotSet, ConsoleEnabled;
	time_t rawtime;
	struct tm * timeinfo;
	char * TimeFormat;
	char * RealTime;
	unsigned char NumUsages;
	HANDLE ConsoleIn, ConsoleOut;
	std::string ConsoleReceived;
	unsigned char ConsoleBreakType;
	std::string MiniDumpPath;
	int MiniDumpType; // See MINIDUMP_TYPE enum

	// Allows debugging of what had the last lock
	#ifdef _DEBUG
		std::string LastLockFile;
		int LastLockLine;
		#define OpenLock() \
			while (Data->ReadingThis) \
				Sleep(0); \
			Data->ReadingThis = true; \
			Data->LastLockFile = __FILE__; \
			Data->LastLockLine = __LINE__
		#define CloseLock() Data->ReadingThis = false
	#else
		#define OpenLock() \
			while (Data->ReadingThis) \
				Sleep(0); \
			Data->ReadingThis = true
		#define CloseLock() Data->ReadingThis = false
	#endif

	// Handle exceptions
	char HandleExceptionVia, ContinuesCount, ContinuesMax;
	const enum {
		HANDLE_VIA_QUIT,
		HANDLE_VIA_INFINITE_WAIT,
		HANDLE_VIA_IGNORE,
		HANDLE_VIA_CONTINUE,
		HANDLE_VIA_EMAIL,
		HANDLE_VIA_BOOT_PROCESS,
		HANDLE_VIA_FORCE_DEBUG_BREAK,

		HANDLE_VIA_MAX
	};
};

#define GlobalID _T("DebugObject")
extern class Extension * GlobalExt;

extern BOOL WINAPI HandlerRoutine(DWORD ControlType);
extern DWORD WINAPI ReceiveConsoleInput(void *);
extern LONG WINAPI UnhandledExceptionCatcher(PEXCEPTION_POINTERS pExceptionPtrs);

#include "Extension.h"


