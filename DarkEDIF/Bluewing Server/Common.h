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


// Lacewing-required lines.==

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")

// #define lw_import
// #define _lacewing_static
// Now in project settings; absolutely required.

// ==end lacewing

#pragma comment(lib, "..\\Lib\\mmfs2.lib")
#pragma comment(lib, "..\\Lib\\zlib.lib")
#include <sstream>
#include "zlib.h"
#include <algorithm>

#ifdef _DEBUG
	extern std::stringstream CriticalSection;
#define EnterCriticalSectionDerpy(x) { \
		EnterCriticalSection(x); \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Entered on " \
		<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
	}

#define LeaveCriticalSectionDerpy(x) { \
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Left on " \
			<< __FILE__ << ", line " << __LINE__ << ".\r\n"; \
		LeaveCriticalSection(x); \
	}
#else
#define EnterCriticalSectionDerpy(x)  EnterCriticalSection(x)
#define LeaveCriticalSectionDerpy(x)  LeaveCriticalSection(x)
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
