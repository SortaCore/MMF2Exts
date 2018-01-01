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
#define JSON_COMMENT_MACRO lacewing::relayserver::buildnum

#define DLLExport   __stdcall
#pragma comment(lib, "..\\Lib\\mmfs2.lib")

// Lacewing-required imports for accessing Windows sockets
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")

// #define lw_import
// #define _lacewing_static
// These two are now defined in project settings; they're absolutely required.

// == end lacewing

#include <sstream>
#include <algorithm>
#include "zlib.h"
#pragma comment(lib, "..\\Lib\\zlib.lib")

#ifdef _DEBUG
	extern std::stringstream CriticalSection;
#define EnterCriticalSectionDerpy(x) { \
		EnterCriticalSection(x); \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Entered on " \
		<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
	}

#define LeaveCriticalSectionDebug(x) { \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Left on " \
			<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
		LeaveCriticalSection(x); \
	}
#else
#define EnterCriticalSectionDerpy(x)  EnterCriticalSection(x)
#define LeaveCriticalSectionDebug(x)  LeaveCriticalSection(x)
#endif

#include "Lacewing.h"
#include "LacewingFunctions.h"
#include "ChannelCopy.h"
#include "ClientCopy.h"

#include "Edif.h"
#include "Resource.h"

// Enable DarkEDIF's utility
#define MULTI_THREADING
#include "MultiThreading.h"

#include "DarkEDIF.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader			eHeader;

	// Because Lacewing Blue can be interchanged with Lacewing Relay by replacing the MFXes,
	// we don't use DarkEDIF's automatic property system (except for the menus), instead
	// matching the Relay's memory layout, and changing Edittime.cpp.

	// Any random edittime stuff? Place here.
	char pad0[5];				// For matching old extension
	bool AutomaticClear,
		 MultiThreading;
	bool Global;
	char edGlobalID[255];
	bool TimeoutWarningEnabled,
		FullDeleteEnabled;

	// Keep as last or risk overwriting by functions accessing this address
	size_t DarkEDIF_Prop_Size;
	char DarkEDIF_Props[0];

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
//	Sprite			rs;				// Sprite (displayable objects)
//	AltVals			rv;				// Alterable values

    // Required
	Extension * pExtension;

    /*
        You shouldn't add any variables or anything here. Add them as members
        of the Extension class (Extension.h) instead.
    */
};
DWORD WINAPI LacewingLoopThread(void * ThisExt);

#define COMMON_H
#include "Extension.h"
#undef COMMON_H
