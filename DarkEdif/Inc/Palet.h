
#ifndef _Palet_h
#define	_Palet_h

#define SURFACES_API __declspec(dllimport)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32_WCE
#define PC_NOCOLLAPSE	4
#endif

typedef	struct csPalette
{
	// LOGPALETTE
	WORD			palVersion;
	WORD			palNumEntries;
	PALETTEENTRY	palEntry[256];

	// 32K cache table for quick remap
	LPBYTE			Cache15b;

	// Windows HPALETTE
	HPALETTE		hPal;

	// Index of opaque black (256 colors only)
	UINT			opaqueBlackIndex;

	// Ref count
	UINT			refCount;

} csPalette;
typedef	csPalette *	LPCSPALETTE;

void csPalette_Remove(LPCSPALETTE pCsPal);

// Initialize and free palette manager
SURFACES_API void WINAPI Init_Palettes();
SURFACES_API void WINAPI Free_Palettes();

// Create list of palettes attached to the current thread
void 		FAR	Create_CurrentPalList();

// Delete list of palettes attached to the current thread
void 		FAR	Delete_CurrentPalList();

// Create / Delete CSPalette
SURFACES_API LPCSPALETTE 	FAR	WINAPI csPalette_Create (LPLOGPALETTE pLogPal);
SURFACES_API void 		FAR	WINAPI csPalette_Delete (LPCSPALETTE pCsPal);		// another name for csPalette_SubRef
SURFACES_API LPCSPALETTE  WINAPI csPalette_NormalizePalette(LPCSPALETTE pCsPal, LPBYTE pTabRemap);

// Create default 256 color palette
// This palette must be deleted using csPalette_Delete or csPalette_SubRef
SURFACES_API LPCSPALETTE 	FAR	WINAPI  csPalette_CreateDefaultPalette();

// GetNearColorIndex with 16bit cache (warning: small loss of color possible)
SURFACES_API int 			FAR csPalette_GetNearColorIndex(LPCSPALETTE ptCsPal, COLORREF color);
SURFACES_API void 		FAR WINAPI csPalette_GetPaletteEntries(LPCSPALETTE ptCsPal, LPPALETTEENTRY pe, int start, int count);

// GetNearColorIndex slow (but no loss of color) - Optimized for MMX machines -
// Warning: if nColors == 256, this routine assumes the first color is RGB(0,0,0)
int GetNearestColorIndex (LPPALETTEENTRY ppe, UINT nColors, COLORREF color);

// Normalize palette: add system colors and replace black>0 by OPAQUE_BLACK
SURFACES_API BOOL 		FAR	WINAPI NormalizePalette (PALETTEENTRY *destPal, PALETTEENTRY *srcPal, LPBYTE ptabRemap);

// Copy RGB to BGR and reciprocally
void 		FAR	CopyRGBToBGR ( RGBQUAD *dest, PALETTEENTRY *src, UINT nbColors );
void 		FAR	CopyBGRToRGB ( PALETTEENTRY *dest, RGBQUAD *src, UINT nbColors );

// Default palettes
extern	BYTE	Palet16[], Palet256[];


#ifdef __cplusplus
}
#endif

#endif	// _Palet_h
