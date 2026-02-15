#pragma once

// Do not move XXXEXT after #include of DarkEdif.h!
// #define TGFEXT	// TGF2, Fusion 2.x Std, Fusion 2.x Dev
#define MMFEXT		// Fusion 2.x, Fusion 2.x Dev
// #define PROEXT	// Fusion 2.x Dev only

#include "DarkEdif.hpp"

#pragma pack (push, 1)
// Binary block used in Fusion editor and saved in the MFA/CCN/EXE files
struct EDITDATA final
{
	NO_DEFAULT_CTORS_OR_DTORS(EDITDATA);
	// Header - required, must be first variable in EDITDATA
	extHeader eHeader;

#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS
	// Object size in Fusion editor
	DarkEdif::Size objSize;
#endif
#if TEXT_OEFLAG_EXTENSION
	// Font set in font tab, only for font property tab extensions
	DarkEdif::EditDataFont font;
#endif

	// Extra object data needed in Fusion editor should be placed after this comment, before Props.
	// The majority of extensions will not need this!
	// If you do, look in DarkEdif help file for CreateObject(),
	// UserConverter() and MigrateMiddle().

	// Keep Properties variable last; its size varies.
	DarkEdif::Properties Props;
};
#pragma pack (pop)

#include "Extension.hpp"
