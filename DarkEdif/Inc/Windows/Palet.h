#pragma once
#include <WinDef.h>
#include <WinGDI.h>

struct csPalette
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
};

extern "C"
{
	FusionAPIImport void csPalette_Remove(csPalette * pCsPal);

	// Initialize and free palette manager
	FusionAPIImport void FusionAPI Init_Palettes();
	FusionAPIImport void FusionAPI Free_Palettes();

	// Create list of palettes attached to the current thread
	FusionAPIImport void FusionAPI Create_CurrentPalList();

	// Delete list of palettes attached to the current thread
	FusionAPIImport void FusionAPI Delete_CurrentPalList();

	// Create / Delete CSPalette
	FusionAPIImport csPalette * FusionAPI csPalette_Create(LPLOGPALETTE pLogPal);
	FusionAPIImport void 		FusionAPI csPalette_Delete(csPalette * pCsPal);		// another name for csPalette_SubRef
	FusionAPIImport csPalette * FusionAPI csPalette_NormalizePalette(csPalette * pCsPal, LPBYTE pTabRemap);

	// Create default 256 color palette
	// This palette must be deleted using csPalette_Delete or csPalette_SubRef
	FusionAPIImport csPalette * FusionAPI  csPalette_CreateDefaultPalette();

	// GetNearColorIndex with 16bit cache (warning: small loss of color possible)
	FusionAPIImport int FusionAPI csPalette_GetNearColorIndex(csPalette * ptCsPal, COLORREF color);
	FusionAPIImport void FusionAPI csPalette_GetPaletteEntries(csPalette * ptCsPal, LPPALETTEENTRY pe, int start, int count);

	// GetNearColorIndex slow (but no loss of color) - Optimized for MMX machines -
	// Warning: if nColors == 256, this routine assumes the first color is RGB(0,0,0)
	FusionAPIImport int FusionAPI GetNearestColorIndex(LPPALETTEENTRY ppe, UINT nColors, COLORREF color);

	// Normalize palette: add system colors and replace black>0 by OPAQUE_BLACK
	FusionAPIImport BOOL FusionAPI NormalizePalette(PALETTEENTRY * destPal, PALETTEENTRY * srcPal, LPBYTE ptabRemap);

	// Copy RGB to BGR and reverse
	FusionAPIImport void FusionAPI CopyRGBToBGR(RGBQUAD * dest, PALETTEENTRY * src, UINT nbColors);
	FusionAPIImport void FusionAPI CopyBGRToRGB(PALETTEENTRY * dest, RGBQUAD * src, UINT nbColors);

	// Default palettes
	extern	FusionAPIImport BYTE Palet16[], Palet256[];

} // extern "C"
