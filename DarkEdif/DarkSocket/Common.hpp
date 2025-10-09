#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only
#define JSON_COMMENT_MACRO Extension::Version, STRIFY(CONFIG)

#include "DarkEdif.hpp"

#ifdef _WIN32
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <AF_Irda.h>
#pragma comment(lib, "ws2_32.lib")
#endif

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

// Include the structs/enums/etc used later on in the object
#include "Structures.hpp"
#include "Extension.hpp"
