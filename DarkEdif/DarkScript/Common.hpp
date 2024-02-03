#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only
#define JSON_COMMENT_MACRO Extension::Version

// Activates some Edif workarounds to get internal A/C/E parameters
#define DARKSCRIPT_EXTENSION

#include "DarkEdif.hpp"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	NO_DEFAULT_CTORS_OR_DTORS(EDITDATA);
	// Header - required
	extHeader		eHeader;

	// Object's data

//	short			swidth;
//	short			sheight;

	// Keep DarkEdif variables as last. Undefined behaviour otherwise.
	DarkEdif::Properties Props;
};

#include <regex>
#include "Extension.hpp"
