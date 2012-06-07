
#ifndef _PaletCpp_h
#define	_PaletCpp_h

#include "Palet.h"


// Increment / Decrement CSPalette recount
SURFACES_API void WINAPI		FAR csPalette_AddRef(LPCSPALETTE pCsPal);
SURFACES_API void WINAPI		FAR csPalette_SubRef(LPCSPALETTE pCsPal);

// Get Windows palette handle
SURFACES_API HPALETTE WINAPI 	FAR	csPalette_GetPaletteHandle(LPCSPALETTE pCsPal);

// Init cache for fast GetNearColorIndex
SURFACES_API int WINAPI 			FAR csPalette_InitCache(LPCSPALETTE ptCsPal);

// GetNearColorIndex with 16bit cache (warning: small loss of color possible)
SURFACES_API int WINAPI FAR csPalette_GetNearColorIndex_Fast (LPCSPALETTE ptCsPal, COLORREF color);



#endif	// _PaletCpp_h
