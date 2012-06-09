#pragma once

   #define TGFEXT       // TGF2, MMF2, MMF2 Dev
// #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only

#include    "Edif.h"
#include	"Resource.h"

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
//	short			swidth;
//	short			sheight;



    // Give menu items a tag (first parameter of a menu item in the
    // JSON) and you'll be able to show/hide them with these functions.
    // This is useful when the properties enable/disable features.

    void ShowMenuItems(const char * Tag);
    void HideMenuItems(const char * Tag);

    // At the end to save screwing up the offset of your variables

	Edif::SDK Edif;
};

class Extension;

struct RUNDATA
{
	// Main header - required
	headerObject rHo;

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
//	rCom			rc;				// Common structure for movements & animations
//	rMvt			rm;				// Movements
//	rSpr			rs;				// Sprite (displayable objects)
//	rVal			rv;				// Alterable values

    // Required
	Extension * pExtension;

    /*
        You shouldn't add any variables or anything here. Add them as members
        of the Extension class (Extension.h) instead.
    */
};

#include "Extension.h"
