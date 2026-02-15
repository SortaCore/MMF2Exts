#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only

#define JSON_COMMENT_MACRO Extension::Version

#include "DarkEdif.hpp"

#include "..\Inc\Windows\ksnd.h"
// Static linking of KSND
#pragma comment(lib, "..\\Lib\\Windows\\ksndstatic.lib")
#pragma comment(lib, "winmm.lib")

#define CheckForPlayer(makeError, funcName, ...) \
	if (!curPlayer) { \
		if constexpr (makeError) \
			CreateError(_T(""## funcName##": No player selected!")); \
		return __VA_ARGS__; \
	}

#pragma pack (push, 1)
// Binary block used in Fusion editor and saved in the MFA/CCN/EXE files
struct EDITDATA final
{
	NO_DEFAULT_CTORS_OR_DTORS(EDITDATA);
	// Header - required, must be first variable in EDITDATA
	extHeader eHeader;

	// Keep Properties variable last; its size varies.
	DarkEdif::Properties Props;
};
#pragma pack (pop)

#include "Extension.hpp"
