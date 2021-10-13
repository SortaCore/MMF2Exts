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
#define JSON_COMMENT_MACRO lacewing::relayclient::buildnum, STRIFY(CONFIG)

// Lacewing-required imports for accessing Windows sockets
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")
#endif

// #define lw_import
// #define _lacewing_static
// These two are now defined in project settings; they're absolutely required.

#include "../Inc/Shared/DarkEdif.h"

#include "../Lib/Shared/Lacewing/Lacewing.h"
#include "LacewingFunctions.h"

#ifdef _WIN32
#include <stdlib.h>
#include <crtdbg.h>
#pragma comment(lib, "..\\Lib\\Windows\\zlib.lib")
#include "..\Inc\Windows\zlib.h"
#else
#include <zlib.h>
// libz included in project settings

// Prevent errors with IntelliSense
#include <string_view>
using namespace std::string_view_literals;
#endif
#include <iomanip>

#ifdef __APPLE__
#include "../Inc/iOS/MMFiOSMasterHeader.h"
#endif

#ifdef _DEBUG
	extern std::stringstream CriticalSection;
#define EnterCriticalSectionDebug(x) \
		(x)->edif_lock(); \
		::CriticalSection << "Thread "sv << std::this_thread::get_id() << " : Entered on "sv \
			<< __FILE__ << ", line "sv << __LINE__ << ".\r\n"sv

#define LeaveCriticalSectionDebug(x) \
		::CriticalSection << "Thread "sv << std::this_thread::get_id() << " : Left on "sv \
			<< __FILE__ << ", line "sv << __LINE__ << ".\r\n"sv; \
		(x)->edif_unlock()
#else
#define EnterCriticalSectionDebug(x)  (x)->edif_lock()
#define LeaveCriticalSectionDebug(x)  (x)->edif_unlock()
#endif

// Enable DarkEdif's utility
#define MULTI_THREADING
#include "MultiThreading.h"

#include "Edif.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required
	extHeader			eHeader;

	// Because Lacewing Blue can be interchanged with Lacewing Relay by replacing the MFXes,
	// we don't use DarkEdif's automatic property system (except for the menus), instead
	// matching the Relay's memory layout, and changing Edittime.cpp.

	// Any random edittime stuff? Place here.
	char pad0[5];				// For matching Relay Client extension
	bool automaticClear,
		 multiThreading;
	bool isGlobal;
	char edGlobalID[255];
	bool timeoutWarningEnabled;
	bool fullDeleteEnabled;
	char pad1[256];

	// Note: To match Lacewing Relay Client, this struct's size must be 544 bytes

#ifndef NOPROPS
	// Keep as last or risk overwriting by functions accessing this address
	std::uint32_t DarkEdif_Prop_Size;
	char DarkEdif_Props[];

	// DarkEdif functions, use within Extension ctor.
	bool IsPropChecked(int propID);
	std::tstring GetPropertyStr(const char * propName);
	std::tstring GetPropertyStr(int propID);
#endif
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
DWORD LacewingLoopThread(void * ThisExt);
DWORD ObjectDestroyTimeoutFunc(void * ThisGlobalInfo);

#define COMMON_H
#include "Extension.h"
#undef COMMON_H
