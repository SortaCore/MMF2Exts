#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only

#define JSON_COMMENT_MACRO lacewing::relayserver::buildnum, STRIFY(CONFIG)
#include "DarkEdif.h"

#ifdef _WIN32
// Lacewing-required imports for accessing Windows sockets
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")
#endif

#ifndef _lacewing_static
	#error Required Lacewing definitions are not in project settings.
#endif

#include "Lacewing.h"
#include "LacewingFunctions.h"
#include "MultiThreading.h"

#ifdef _WIN32
#pragma comment(lib, "..\\Lib\\Windows\\zlib.lib")
	extern std::stringstream CriticalSection;
#define EnterCriticalSectionDebug(x) \
		(x)->edif_lock(); \
		::CriticalSection << "Thread "sv << std::this_thread::get_id() << " : Entered on "sv \
			<< __FILE__ << ", line "sv << __LINE__ << ".\r\n"sv

#define LeaveCriticalSectionDebug(x) \
		::CriticalSection << "Thread "sv << std::this_thread::get_id() << " : Left on "sv \
			<< __FILE__ << ", line "sv << __LINE__ << ".\r\n"sv; \
		(x)->edif_unlock()
#include "..\Inc\Windows\zlib.h"
#else
#define EnterCriticalSectionDebug(x)  (x)->edif_lock()
#define LeaveCriticalSectionDebug(x)  (x)->edif_unlock()
#include <zlib.h>
// libz included in project settings
#endif
#include <iomanip>


// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	// Header - required, must be first variable in EDITDATA
	extHeader			eHeader;

	// Because Lacewing Blue can be interchanged with Lacewing Relay by replacing the MFXes,
	// we don't use DarkEdif's automatic property system (except for the menus), instead
	// matching the Relay's memory layout, and changing Edittime.cpp.

	// Any random edittime stuff? Place here.
	char pad0[5];				// For matching old extension
	bool automaticClear;
	bool multiThreading;
	bool isGlobal;
	// UTF-8 global ID
	char edGlobalID[255];
	bool timeoutWarningEnabled;
	bool fullDeleteEnabled;
	bool enableInactivityTimer;
	char pad1[249];

	// To match Lacewing Relay Server, this struct's size must be 536 bytes.

#ifndef NOPROPS
	// Keep DarkEdif variables as last. Undefined behaviour otherwise.
	int				DarkEdif_Prop_Size;
	char			DarkEdif_Props[];

	// =====
	// DarkEdif functions, use within Extension ctor.
	// =====

	// Returns property checked or unchecked.
	bool IsPropChecked(int propID);
	// Returns std::tstring property setting from property name.
	std::tstring GetPropertyStr(const char * propName);
	// Returns std::tstring property string from property ID.
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
DWORD LacewingLoopThread(Extension * ThisExt);

#define COMMON_H
#include "Extension.h"
#undef COMMON_H
