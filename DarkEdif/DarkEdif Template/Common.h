#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only

#include "DarkEdif.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files
struct EDITDATA
{
	NO_DEFAULT_CTORS(EDITDATA);
	// Header - required
	extHeader		eHeader;

	// Object's data

//	short			swidth;
//	short			sheight;

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

#include "Extension.h"
