#pragma once

// #define TGFEXT	   // TGF2, MMF2, MMF2 Dev
   #define MMFEXT	   // MMF2, MMF2 Dev
// #define PROEXT	   // MMF2 Dev only

#define JSON_COMMENT_MACRO Extension::Version

// Enable DarkEdif's utility
// #define MULTI_THREADING
// #include "MultiThreading.hpp"

#include "DarkEdif.hpp"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	NO_DEFAULT_CTORS_OR_DTORS(EDITDATA);
	// Header - required
	extHeader			eHeader;

	// Keep DarkEdif variables as last. Undefined behaviour otherwise.
	DarkEdif::Properties Props;
};

#include "Extension.hpp"
