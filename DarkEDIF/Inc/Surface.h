//////////////////////////////////////////////////////////////////////////////
//
// cSurface class
//

#ifndef _Surface_h
#define _Surface_h

//#include "Fill.h"
#include "Palet.h"
#include <CommDlg.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "..\\Lib\\mmfs2.lib")
//kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;
#define SURFACES_API __declspec(dllimport)

// Convention : SfSrc.FilterBlit(SfDest, MyCallBack, param)
// ==========		will call MyCallBack(pixelDest, pixelSrc, param)
 	
typedef	COLORREF (CALLBACK * FILTERBLITPROC)(COLORREF, COLORREF, unsigned long);
typedef	COLORREF (CALLBACK * MATRIXFILTERBLITPROC)(COLORREF FAR *, COLORREF FAR *, unsigned long);

// Display mode
typedef	struct DisplayMode {
	int		mcx;
	int		mcy;
	int		mDepth;
//	int		mNearestBitCount;
//	int		mDriver;
} DisplayMode;

typedef	BOOL (CALLBACK * LPENUMSCREENMODESPROC)(DisplayMode*, LPVOID);

#ifdef HWABETA
// Lost device callback function
typedef	void (CALLBACK * LOSTDEVICECALLBACKPROC)(cSurface*, LPARAM);
#endif

// System colors
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION	28
#endif

// Forwards
class cSurface;
struct cSurfaceImplementation;
struct CFillData;
class CInputFile;

// Types
//typedef	cSurface FAR * cSurface *;

// Blit modes
typedef enum {
	BMODE_OPAQUE,
	BMODE_TRANSP,
	BMODE_MAX
} BlitMode;

// Blit operations
typedef enum {
	BOP_COPY,					// None
	BOP_BLEND,					// dest = ((dest * coef) + (src * (128-coef)))/128
	BOP_INVERT,					// dest = src XOR 0xFFFFFF
	BOP_XOR,					// dest = src XOR dest
	BOP_AND,					// dest = src AND dest
	BOP_OR,						// dest = src OR dest
	BOP_BLEND_REPLACETRANSP,	// dest = ((dest * coef) + ((src==transp)?replace:src * (128-coef)))/128
	BOP_DWROP,
	BOP_ANDNOT,
	BOP_ADD,
	BOP_MONO,
	BOP_SUB,
	BOP_BLEND_DONTREPLACECOLOR,
	BOP_EFFECTEX,
	BOP_MAX,
	BOP_MASK = 0xFFF,
	BOP_RGBAFILTER = 0x1000,
} BlitOp;

#define ALPHATOSEMITRANSP(a) ((a==0) ? 128:(255-a)/2)
#define SEMITRANSPTOALPHA(s) ((s==128) ? 0:(255-s*2))

typedef unsigned long RGBAREF;
#define COLORREFATORGBA(c,a) ((c & 0x00FFFFFF) | (a << 24))

// Surface capabilities
typedef enum 
{
	// Capabilities
	SC_GETPIXEL,
	SC_SETPIXEL,
	SC_FILL,
	SC_DRAW,
	SC_ROTATE,
	SC_STRETCH,
	SC_MAX
} SurfaceCap;

// SetPalette actions
typedef enum
{
	SPA_NONE,					// Just update palette
	SPA_REMAPSURFACE,			// remap current surface pixels to new palette
	SPA_MAX
} SetPaletteAction;

// Surface types
enum
{
	ST_MEMORY,					// Buffer only
	ST_MEMORYWITHDC,			// Buffer + DC (i.e. DIBSection, DDRAW surface, etc...
	ST_MEMORYWITHPERMANENTDC,	// Buffer + permanent DC (i.e. DIBDC)
	ST_DDRAW_SYSTEMMEMORY,		// Surface Direct Draw en mémoire systeme
	ST_HWA_SCREEN,				// Screen surface in HWA mode
	ST_HWA_RTTEXTURE,			// Render target texture in HWA mode
	ST_HWA_ROUTEXTURE,			// HWA texture created in video memory, unmanaged (lost when device is lost)
	ST_HWA_ROMTEXTURE,			// HWA texture created in video memory, managed (automatically reloaded when the device is lost)
	ST_MAX
};

// Drivers for memory + DC surfaces
enum
{
	SD_DIB,						// DIB (standard driver: DIBSection)
	SD_WING,					// WinG
	SD_DDRAW,					// Direct Draw
	SD_BITMAP,					// Win 3.1 bitmap
	SD_3DFX,					// 3DFX
	SD_D3D9,					// Direct3D9
	SD_D3D8,					// Direct3D8
	SD_MAX
};

typedef enum 			// Warning, bit mask, not enumeration!
{
	LI_NONE=0x0000,
	LI_REMAP=0x0001,
	LI_CHANGESURFACEDEPTH=0x0002,
	LI_DONOTNORMALIZEPALETTE = 0x0004	// do not normalize palette
} LIFlags;

typedef enum 
{
	SI_NONE=0x0000,
	SI_ONLYHEADER=0x0001,
	SI_SAVEALPHA=0x0002
} SIFlags;

enum {
	LBF_DONOTCHANGESURFACEDEPTH = 0x0001
};

// Blilt options
enum {
	BLTF_ANTIA				= 0x0001,		// Anti-aliasing
	BLTF_COPYALPHA			= 0x0002,		// Copy alpha channel to destination alpha channel instead of applying it
#ifdef HWABETA
	BLTF_SAFESRC			= 0x0010,
	BLTF_TILE				= 0x0020
#endif
};

// Stretch & BlitEx options
enum {
	STRF_RESAMPLE			= 0x0001,		// Resample bitmap
	STRF_RESAMPLE_TRANSP	= 0x0002,		// Resample bitmap, but doesn't resample the transparent color
	STRF_COPYALPHA			= 0x0004,		// Copy (stretch) alpha channel to destination alpha channel instead of applying it
#ifdef HWABETA
	STRF_SAFESRC			= 0x0010,
	STRF_TILE				= 0x0020
#endif
};


// Transparent monochrome mask for collisions
typedef struct sMask
{
	int		mkSize;
	int		mkWidth;
	int		mkHeight;
	UINT	mkWidthBytes;
	int		mkXSpot;
	int		mkYSpot;
	unsigned long	mkFlags;
	RECT	mkRect;
} sMask;
typedef sMask *LPSMASK;

#define SCMF_FULL		0x0000
#define SCMF_PLATFORM	0x0001

#ifndef PI
#define     PI  ((double)3.141592653589)
#endif

// Allocate/Free surface
SURFACES_API cSurface *	WINAPI NewSurface();
SURFACES_API void		WINAPI DeleteSurface(cSurface * pSurf);

// Get surface prototype
SURFACES_API BOOL WINAPI		GetSurfacePrototype (cSurface * FAR *proto, int depth, int st, int drv);

// DIB
SURFACES_API unsigned long WINAPI GetDIBHeaderSize(int depth);
SURFACES_API unsigned long WINAPI GetDIBWidthBytes ( int width, int depth );
SURFACES_API unsigned long WINAPI GetDIBSize ( int width, int height, int depth );
SURFACES_API unsigned char * WINAPI GetDIBBitmap ( LPBITMAPINFO pBmi );

// cSurface class
class SURFACES_API cSurface
{
public:
		// ======================
		// Creation / Destruction
		// ======================
		cSurface();
		~cSurface();

		// Init
		static void InitializeSurfaces();
		static void FreeSurfaces();
		#ifdef HWABETA
			static void FreeExternalModules();
		#endif

		// Operators
		cSurface FAR & operator= (const cSurface FAR & source);

		// Create surface implementation from surface prototype
		void Create (int width, int height, cSurface * prototype);

		// Create pure DC surface from DC
		void Create (HDC hDC);

		// Create pure DC surface from a window
		void Create (HWND hWnd, BOOL IncludeFrame);

		// Create screen surface (fullscreen mode)
		BOOL CreateScreenSurface();
		BOOL IsScreenSurface();

		// Valid?
		BOOL IsValid ();

		// Get driver & type
		int		GetType();
		int		GetDriver();
		unsigned long	GetDriverInfo(void * pInfo);

		// Clone surface (= create with same size + Blit)
		void Clone (const cSurface & pSrcSurface, int newW = -1, int newH = -1);

		// Delete surface implementation (before to create another one)
		void Delete();

		// ======================
		// Error codes
		// ======================
		int		GetLastError(void);

		// ======================
	    // Surface info
		// ======================
		int		GetWidth(void) const;
		int		GetHeight(void) const;
		int		GetDepth(void) const;
		BOOL	GetInfo(int FAR & width, int FAR & height, int FAR & depth) const;

		// ======================
	    // Surface coordinate management
		// ======================

		void	SetOrigin(int x, int y);
		void	SetOrigin(POINT c);

		void	GetOrigin(POINT FAR &pt);
		void	GetOrigin(int FAR & x, int FAR & y);

		void	OffsetOrigin(int dx, int dy);
		void	OffsetOrigin(POINT delta);

		// ======================
	    // Raster operations
		// ======================
		LPBYTE	LockBuffer();
		void	UnlockBuffer(LPBYTE spBuffer);
		int		GetPitch() const;

		// ======================
		// Double-buffer handling
		// ======================
		void	SetCurrentDevice();
		int		BeginRendering(BOOL bClear, RGBAREF dwRgba);
		int		EndRendering();
		BOOL	UpdateScreen();
		#ifdef HWABETA
			cSurface* GetRenderTargetSurface();
			void	ReleaseRenderTargetSurface(cSurface* psf);
			void	Flush(BOOL bMax);
			void	SetZBuffer(float z2D);
		#endif

		// ======================
	    // Device context for graphic operations
		// ======================
		HDC		GetDC(void);
		void	ReleaseDC(HDC dc);
		void	AttachWindow (HWND hWnd);

		// ======================
	    // Clipping
		// ======================
		void	GetClipRect(int FAR & x, int FAR & y, int FAR & w, int FAR & h);
		void	SetClipRect(int x, int y, int w, int h);
		void	ClearClipRect(void);

		// ======================
		// LoadImage (DIB format) / SaveImage (DIB format)
		// ======================
		#undef LoadImage

		BOOL	LoadImageA (HFILE hf, unsigned long lsize, LIFlags loadFlags = LI_NONE);
		BOOL	LoadImageA (LPCSTR fileName, LIFlags loadFlags = LI_NONE);
#ifdef _WINDOWS
		BOOL	LoadImageA (HINSTANCE hInst, int bmpID, LIFlags loadFlags = LI_NONE);
#endif // _WINDOWS
		BOOL	LoadImageA (LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LIFlags loadFlags = LI_NONE);

		BOOL	LoadImageW (HFILE hf, unsigned long lsize, LIFlags loadFlags = LI_NONE);
		BOOL	LoadImageW (LPCWSTR fileName, LIFlags loadFlags = LI_NONE);
#ifdef _WINDOWS
		BOOL	LoadImageW (HINSTANCE hInst, int bmpID, LIFlags loadFlags = LI_NONE);
#endif // _WINDOWS
		BOOL	LoadImageW (LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LIFlags loadFlags = LI_NONE);

		#ifdef _UNICODE
		#define LoadImage LoadImageW
		#else
		#define LoadImage LoadImageA
		#endif

		BOOL	SaveImage (HFILE hf, SIFlags saveFlags = SI_NONE);
		BOOL	SaveImage (LPCSTR fileName, SIFlags saveFlags = SI_NONE);
		BOOL	SaveImage (LPCWSTR fileName, SIFlags saveFlags = SI_NONE);
		BOOL	SaveImage (LPBITMAPINFO pBmi, LPBYTE pBits, SIFlags saveFlags = SI_NONE);

		unsigned long	GetDIBSize ();

		// ======================
		// Pixel functions
		// ======================
		// Set pixel
		void	SetPixel(int x, int y, COLORREF c);
		void	SetPixel(int x, int y, BYTE R, BYTE G, BYTE B);
		void	SetPixel(int x, int y, int index);

		// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
		void	SetPixelFast(int x, int y, COLORREF c);
		void	SetPixelFast8(int x, int y, int index);

		// Get pixel
		BOOL	GetPixel(int x, int y, COLORREF FAR & c) const;
		BOOL	GetPixel(int x, int y, BYTE FAR & R, BYTE FAR & G, BYTE FAR & B) const;
		BOOL	GetPixel(int x, int y, int FAR & index) const;

		// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
		COLORREF	GetPixelFast(int x, int y);
		int			GetPixelFast8(int x, int y);

		// ======================
		// Blit functions
		// ======================
		// Blit surface to surface
		BOOL	Blit(cSurface FAR & dest) const;

		BOOL	Blit(cSurface FAR & dest, int destX, int destY, 
					  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0,
					  unsigned long dwBlitFlags = 0) const;

		// Blit rectangle to surface
		BOOL	Blit(cSurface FAR & dest, int destX, int destY, 
					  int srcX, int srcY, int srcWidth, int srcHeight,
					  BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0,
					  unsigned long dwBlitFlags = 0) const;

		// Extended blit : can do stretch & rotate at the same time
		// Only implemented in 3D mode
#ifdef HWABETA
		BOOL	BlitEx(cSurface FAR & dest, float dX, float dY, float fScaleX, float fScaleY,
						int sX, int sY, int sW, int sH, LPPOINT pCenter, float fAngle, 
						BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, unsigned long dwFlags = 0) const;
#endif
		// Scrolling
		BOOL	Scroll (int xDest, int yDest, int xSrc, int ySrc, int width, int height);

		// Blit via callback
		BOOL	FilterBlit (cSurface FAR & dest, int destX, int destY,
							int srcX, int srcY, int srcWidth, int srcHeight,
							BlitMode bm, FILTERBLITPROC fbProc, LPARAM lUserParam) const;

		BOOL	FilterBlit (cSurface FAR & dest, FILTERBLITPROC fbProc, 
							LPARAM lUserParam, BlitMode bm = BMODE_OPAQUE) const;

		// Matrix blit via callback
		BOOL	MatrixFilterBlit (cSurface FAR & dest, int destX, int destY,
									int srcX, int srcY, int srcWidth, int srcHeight,
									int mWidth, int mHeight, int mDXCenter, int mDYCenter,
									MATRIXFILTERBLITPROC fbProc, LPARAM lUserParam) const;

		// Stretch surface to surface
		BOOL	Stretch(cSurface FAR & dest, unsigned long dwFlags = 0) const;

		// Stretch surface to rectangle
		BOOL	Stretch(cSurface FAR & dest, int destX, int destY, int destWidth, int destHeight,
						BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0, unsigned long dwFlags = 0) const;

		// Stretch rectangle to rectangle
		BOOL	Stretch(cSurface FAR & dest, int destX, int destY, int destWidth, int destHeight,
						int srcX, int srcY, int srcWidth, int srcHeight,
						BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0, unsigned long dwFlags = 0) const;

		// Revert surface horizontally
		BOOL	ReverseX();

		// Revert rectangle horizontally
		BOOL	ReverseX(int x, int y, int width, int height);

		// Revert surface vertically
		BOOL	ReverseY();

		// Revert rectangle vertically
		BOOL	ReverseY(int x, int y, int width, int height);

		// Remove empty borders
		BOOL	GetMinimizeRect(RECT FAR*);
		BOOL	Minimize();
		BOOL	Minimize(RECT FAR* r);

		// =============================
		// Blit from screen or memory DC
		// =============================

		static BOOL CaptureDC(HDC srcDC,HDC dstDC,LONG srcX,LONG srcY,LONG dstX,LONG dstY,LONG srcWidth,LONG srcHeight,LONG dstWidth,LONG dstHeight,BOOL bFlushMessage,BOOL bKeepRatio);

		// ======================
		// Fill
		// ======================
		
		// Fill surface
		BOOL	Fill(COLORREF c);
		BOOL	Fill(CFillData FAR * fd);
		BOOL	Fill(int index);
		BOOL	Fill(int R, int G, int B);

		// Fill block
		BOOL	Fill(int x, int y, int w, int h, COLORREF c);
		BOOL	Fill(int x, int y, int w, int h, CFillData FAR * fd);
		BOOL	Fill(int x, int y, int w, int h, int index);
		BOOL	Fill(int x, int y, int w, int h, int R, int G, int B);
#ifdef HWABETA
		BOOL	Fill(int x, int y, int w, int h, COLORREF* pColors, unsigned long dwFlags);
#endif

		// ======================
		// Geometric Primitives
		// ======================

		// 1. Simple routines : call GDI with Surface DC
		// =============================================

		BOOL	Ellipse(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Ellipse(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/, 
			COLORREF crOutl /*= BLACK*/, BOOL Fill = TRUE);

		BOOL	Rectangle(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/, 
			COLORREF crOutl /*= 0*/, BOOL bFill = TRUE);

		BOOL	Polygon(LPPOINT pts, int nPts, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, COLORREF crFill, int thickness = 0, 
			COLORREF crOutl = 0, BOOL bFill = TRUE);

		BOOL  Line(int x1, int y1, int x2, int y2, int thickness = 1, COLORREF crOutl = 0); 

		// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
		// ==============================================================================

		BOOL	Ellipse(int left, int top, int right, int bottom, int thickness, CFillData FAR * fdOutl, BOOL AntiA = FALSE,  
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Ellipse(int left, int top, int right, int bottom, CFillData FAR * fdFill,  
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);
		
		BOOL	Ellipse(int left, int top, int right, int bottom, CFillData FAR * fdFill, int thickness, CFillData FAR * fdOutl, 
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);
		
		BOOL	Rectangle(int left, int top, int right, int bottom, int thickness, CFillData FAR * fdOutl, BOOL AntiA = FALSE,  
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, CFillData FAR * fdFill,  
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, CFillData FAR * fdFill, int thickness, CFillData FAR * fdOutl, 
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);
		
		BOOL	Polygon(LPPOINT pts, int nPts, int thickness, CFillData FAR * fdOutl, BOOL AntiA = FALSE,  
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, CFillData FAR * fdFill,  
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, CFillData FAR * fdFill, int thickness, CFillData FAR * fdOutl, 
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);
		
		BOOL  Line(int x1, int y1, int x2, int y2, int thickness, CFillData FAR * fdOutl, BOOL AntiA, 
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		// Filled Primitives

		BOOL	FloodFill(int x, int y, int FAR & left, int FAR & top, int FAR & right, int FAR & bottom, COLORREF crFill, BOOL AntiA = FALSE, 
			int tol = 0, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	FloodFill(int x, int y, COLORREF crFill, BOOL AntiA = FALSE,  int tol = 0,
							BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		// ======================
		// Rotation
		// ======================
		// Rotate surface
		BOOL	Rotate(cSurface FAR& dest, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// radian
		BOOL	Rotate(cSurface FAR& dest, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// degree
		BOOL	Rotate90(cSurface FAR& dest, BOOL b270);

		// Create rotated surface
		BOOL	CreateRotatedSurface (cSurface FAR& ps, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);
		BOOL	CreateRotatedSurface (cSurface FAR& ps, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);

#ifdef HWABETA
		static void GetSizeOfRotatedRect (int FAR *pWidth, int FAR *pHeight, float angle);
#else
		static void GetSizeOfRotatedRect (int FAR *pWidth, int FAR *pHeight, int angle);
#endif

		// ======================
		// Text
		// ======================

#undef TextOut

		int		TextOutA(LPCSTR text, unsigned long dwCharCount,int x,int y,unsigned long alignMode,LPRECT pClipRc, COLORREF color=0, HFONT hFnt=(HFONT)NULL, BlitMode bm=BMODE_TRANSP, BlitOp=BOP_COPY, LPARAM param=0, int AntiA=0);
		int		TextOutW(LPCWSTR text, unsigned long dwCharCount,int x,int y,unsigned long alignMode,LPRECT pClipRc, COLORREF color=0, HFONT hFnt=(HFONT)NULL, BlitMode bm=BMODE_TRANSP, BlitOp=BOP_COPY, LPARAM param=0, int AntiA=0);

#ifdef _UNICODE
	#define TextOut TextOutW
#else
	#define TextOut TextOutA
#endif

#undef DrawText

		int		DrawTextA(LPCSTR text, unsigned long dwCharCount,LPRECT pRc, unsigned long dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
						  BlitMode bm=BMODE_TRANSP, BlitOp bo=BOP_COPY, LPARAM param=0, int AntiA=0,unsigned long dwLeftMargin=0,unsigned long dwRightMargin=0,unsigned long dwTabSize=8);
		int		DrawTextW(LPCWSTR text, unsigned long dwCharCount,LPRECT pRc, unsigned long dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
						  BlitMode bm=BMODE_TRANSP, BlitOp bo=BOP_COPY, LPARAM param=0, int AntiA=0,unsigned long dwLeftMargin=0,unsigned long dwRightMargin=0,unsigned long dwTabSize=8);

#ifdef _UNICODE
	#define DrawText DrawTextW
#else
	#define DrawText DrawTextA
#endif

		// ======================
		// Color / Palette functions
		// ======================
		// Is transparent
		BOOL	IsTransparent();
		
		// Replace color
		BOOL	ReplaceColor (COLORREF newColor, COLORREF oldColor);

		// Test collision fine entre deux surfaces
		BOOL	IsColliding(cSurface FAR & dest, int xDest, int yDest, int xSrc, int ySrc, int rcWidth, int rcHeight);

		// Create icon
		HICON	CreateIcon(int iconWidth, int iconHeight, COLORREF transpColor,POINT *pHotSpot);

	    // Palette support
		BOOL	Indexed(void);

		BOOL	SetPalette(LPLOGPALETTE palette, SetPaletteAction action=SPA_NONE);
		BOOL	SetPalette (LPCSPALETTE pCsPal, SetPaletteAction action=SPA_NONE);
		BOOL	SetPalette (cSurface FAR& src, SetPaletteAction action=SPA_NONE);
		BOOL	SetPalette(HPALETTE palette, SetPaletteAction action=SPA_NONE);

		void	Remap(cSurface FAR& src);
		void	Remap(LPBYTE remapTable);

		LPCSPALETTE	GetPalette();
		UINT		GetPaletteEntries(LPPALETTEENTRY paletteEntry, int index, int nbColors);

		int			GetNearestColorIndex(COLORREF rgb);
		COLORREF	GetRGB(int index);
		int			GetOpaqueBlackIndex();

		// ======================
		// Full screen
		// ======================
		void	EnumScreenModes(LPENUMSCREENMODESPROC pProc, LPVOID lParam);
		BOOL	SetScreenMode(HWND hWnd, int width, int height, int depth);
		void	RestoreWindowedMode(HWND hWnd);
		void	CopyScreenModeInfo(cSurface* pSrc);

#ifdef HWABETA
		BOOL	SetAutoVSync(int nAutoVSync);
#endif
		BOOL	WaitForVBlank();

		// System colors
		static COLORREF	GetSysColor(int colorIndex);
		static void		OnSysColorChange();

		// Transparent color
		void		SetTransparentColor(COLORREF rgb);
		COLORREF	GetTransparentColor();
		int			GetTransparentColorIndex();

		// Alpha channel
		BOOL		HasAlpha();
		LPBYTE		LockAlpha();
		void		UnlockAlpha();
		int			GetAlphaPitch();
		void		CreateAlpha();
		void		SetAlpha(LPBYTE pAlpha, int nPitch);
		void		AttachAlpha(LPBYTE pAlpha, int nPitch);
		LPBYTE		DetachAlpha(LPLONG pPitch);
		cSurface*	GetAlphaSurface();
		void		ReleaseAlphaSurface(cSurface* pAlphaSf);

		// Transparent monochrome mask
		unsigned long	CreateMask(LPSMASK pMask, UINT dwFlags);

		// Lost device callback
#ifdef HWABETA
		void		OnLostDevice();
		void		AddLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
		void		RemoveLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
#endif

	// Friend functions
	// ----------------
		SURFACES_API friend cSurfaceImplementation FAR * WINAPI GetSurfaceImplementation(cSurface FAR &cs);
		SURFACES_API friend void WINAPI SetSurfaceImplementation(cSurface FAR &cs, cSurfaceImplementation FAR *psi);

	protected:
#ifdef _WINDOWS
		static void		BuildSysColorTable();
#endif // _WINDOWS

	// Clipping
	//----------
		HRGN SetDrawClip(HDC hDC);
		void RestoreDrawClip(HDC hDC,HRGN hOldClipRgn);
	
	// Private functions
	// -----------------
	private:
		BOOL	LoadPicture (CInputFile FAR * pFile, unsigned long bitmapSize, LIFlags loadFlags);
		BOOL	LoadDIB (LPBITMAPINFO pBmi, LPBYTE pBits, LIFlags loadFlags);

	// Private data
	// ------------
	private:
		cSurfaceImplementation FAR *m_actual;
		POINT						origin;

		// System colors
		static BOOL			m_bSysColorTab;
		static COLORREF		m_sysColorTab[COLOR_GRADIENTINACTIVECAPTION+1];

	protected:
		int		m_error;		// Est-ce qu'on doit toujours garder ça?
};

// maximum opacity
#define OP_MAX 128			// 100 

extern "C" SURFACES_API BOOL	WINAPI BuildRemapTable	(LPBYTE, LOGPALETTE FAR *, LOGPALETTE FAR *, WORD);

#endif	//  _Surface_h
