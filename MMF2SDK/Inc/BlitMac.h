
//////////////////////////////////////////////////////////////////////////////
//
// Macros
//

#ifndef _BlitMac_h
#define	_BlitMac_h

#ifndef __GNUC__
#pragma pack(push,1)
#endif // __GNUC__

//////////////////////////////////////////
//
// MAC
//
#ifdef _MAC
// 24 bit pixel in memory screen
typedef struct RGBSCREEN24 {
	BYTE	red;
	BYTE	green;
	BYTE	blue;
} RGBSCREEN24;
typedef RGBSCREEN24 FAR *LPRGBSCREEN24;

// 32 bit pixel in memory screen
typedef struct RGBSCREEN32 {
	BYTE	unused;
	BYTE	red;
	BYTE	green;
	BYTE	blue;
} RGBSCREEN32;
typedef RGBSCREEN32 FAR *LPRGBSCREEN32;

//////////////////////////////////////////
//
// PC
//
#else
// 24 bit pixel in memory screen
typedef struct RGBSCREEN24 {
	BYTE	blue;
	BYTE	green;
	BYTE	red;
#ifdef __GNUC__
} __attribute__((packed,aligned(1))) RGBSCREEN24;
#else
} RGBSCREEN24;
#endif // __GNUC__
typedef RGBSCREEN24 FAR *LPRGBSCREEN24;

// 32 bit pixel in memory screen
typedef struct RGBSCREEN32 {
	BYTE	blue;
	BYTE	green;
	BYTE	red;
	BYTE	unused;
#ifdef __GNUC__
} __attribute__((packed,aligned(1))) RGBSCREEN32;
#else
} RGBSCREEN32;
#endif // __GNUC__
typedef RGBSCREEN32 FAR *LPRGBSCREEN32;

#endif // _MAC

// Define (only) one of the following cstes dependending on the location of the unused byt in 32 bit true color mode
//#define	PIXEL32BIT_FIRSTBYTEUNUSED
#define	PIXEL32BIT_LASTBYTEUNUSED

#ifndef __GNUC__
#pragma pack(pop)
#endif

// MACROS

#ifdef _MAC
#define	PALETTEENTRY_TO_DWORD(pte) ((((DWORD)((LPPALETTEENTRY)pte)->peRed)<<16)|(((DWORD)((LPPALETTEENTRY)pte)->peGreen)<<8)|(((DWORD)((LPPALETTEENTRY)pte)->peBlue)))
#define	PALETTEENTRY_TO_COLORREF(pte) ((((DWORD)((LPPALETTEENTRY)pte)->peRed)<<16)|(((DWORD)((LPPALETTEENTRY)pte)->peGreen)<<8)|(((DWORD)((LPPALETTEENTRY)pte)->peBlue)))
#else
// Transforms a PALETTEENTRY to DWORD, removing the PALETTEENTRY flags
// Note: the placement if the colors in the DWORD is not important, it's not a COLORREF.
#define	PALETTEENTRY_TO_DWORD(pte) (*((LPDWORD)(pte)) & 0xFFFFFF)

// Transforms a PALETTEENTRY to COLORREF, removing the PALETTEENTRY flags
#define	PALETTEENTRY_TO_COLORREF(pte) (*((LPDWORD)(pte)) & 0xFFFFFF)
#endif // _MAC

// Transforms COLORREF to RGB 15 bit
#define	Rgb24To15(c) (UINT)((((c) & 0xF8) << 7) | (((c) & 0xF800) >> 6) | (((c) & 0xF80000) >> 19))

// Transforms COLORREF to RGB 16 bit
#define	Rgb24To16(c) (UINT)((((c) & 0xF8) << 8) | (((c) & 0xFC00) >> 5) | (((c) & 0xF80000) >> 19))

// Transforms RGB 15 bit to COLORREF
#define	Rgb15To24(w) (UINT)(((DWORD)((w) & 0x7C00) >> 7) | ((DWORD)((w) & 0x03E0) << 6) | ((DWORD)((w) & 0x001F) << 19))

// Transforms RGB 16 bit to COLORREF
#define	Rgb16To24(w) (UINT)(((DWORD)((w) & 0xF800) >> 8) | ((DWORD)((w) & 0x07E0) << 5) | ((DWORD)((w) & 0x001F) << 19))

// Mask to convert a PALETTEENTRY to a DWORD
#define COLORMASK_PALETTEENTRY_TO_DWORD 0x00FFFFFF

// Transforms PALETTEENTRY to RGB 15 bit
#define	PALETTEENTRY_TO_RGB15(p) (UINT)(((DWORD)((p)->peRed & 0xF8) << 7) | ((DWORD)((p)->peGreen & 0xF8) << 2) | ((DWORD)((p)->peBlue & 0xF8) >> 3))

// Transforms PALETTEENTRY to RGB 16 bit
#define	PALETTEENTRY_TO_RGB16(p) (UINT)(((DWORD)((p)->peRed & 0xF8) << 8) | ((DWORD)((p)->peGreen & 0xFC) << 3) | ((DWORD)((p)->peBlue & 0xF8) >> 3))

// Transforms RGBSCREEN24 to RGB 15 bit
#define	RGBSCREEN24_TO_RGB15(p) (UINT)(((DWORD)((p)->red & 0xF8) << 7) | ((DWORD)((p)->green & 0xF8) << 2) | ((DWORD)((p)->blue & 0xF8) >> 3))

// Transforms RGBSCREEN24 to RGB 16 bit
#define	RGBSCREEN24_TO_RGB16(p) (UINT)(((DWORD)((p)->red & 0xF8) << 8) | ((DWORD)((p)->green & 0xFC) << 3) | ((DWORD)((p)->blue & 0xF8) >> 3))

// Transforms RGBSCREEN24 to COLORREF
#define	RGBSCREEN24_TO_COLORREF(p) (UINT)(((DWORD)(p)->blue << 16) | ((DWORD)(p)->green << 8) | ((DWORD)(p)->red))

// Transforms RGBSCREEN24 to COLORREF
#define	RGBSCREEN32_TO_COLORREF(p) (UINT)(((DWORD)(p)->blue << 16) | ((DWORD)(p)->green << 8) | ((DWORD)(p)->red))

// Transforms RGBSCREEN32 to RGB 15 bit
#define	RGBSCREEN32_TO_RGB15(p) RGBSCREEN24_TO_RGB15(p)

// Transforms RGBSCREEN32 to RGB 16 bit
#define	RGBSCREEN32_TO_RGB16(p) RGBSCREEN24_TO_RGB16(p)

// Transforms COLORREF to DWORD containing an RGBSCREEN32 for instructions of type "*pdw++ = dw"
#define COLORREF_TO_DWRGBSCREEN32(c) ((DWORD)GetBValue(c) + ((DWORD)GetGValue(c) << 8) + ((DWORD)GetRValue(c) << 16))
#define COLORMASK_DWRGBSCREEN32 0x00FFFFFF

#endif	// _BlitMac_h
