#pragma once

// #define TGFEXT	   // TGF2, MMF2, MMF2 Dev
   #define MMFEXT	   // MMF2, MMF2 Dev
// #define PROEXT	   // MMF2 Dev only

#define JSON_COMMENT_MACRO Extension::Version

#include "DarkEdif.hpp"

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
