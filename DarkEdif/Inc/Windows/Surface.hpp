#pragma once
#include "AllPlatformDefines.hpp"
#include "WindowsDefines.hpp"

// Defined in Palet.hpp
struct csPalette;

// Surface errors
enum class SurfaceError : unsigned int
{
	NoError = 0,
	Min = 0x40010000,
	NotSupported = Min,
	SurfaceNotCreated,
	Internal,
	Max = 0x40020000,
};

//////////////////////////////////////////////////////////////////////////////
//
// cSurface class
//
#ifdef _WIN32
#include "Palet.h"
#include <CommDlg.h>
#pragma comment(lib, "gdi32.lib")
#endif

class cSurface;
class cSurfaceImplementation;
class CFillData;
class CInputFile;

// Convention : SfSrc.FilterBlit(SfDest, MyCallBack, param)
// ==========		will call MyCallBack(pixelDest, pixelSrc, param)

typedef	COLORREF (CALLBACK * FILTERBLITPROC)(COLORREF pixelDest, COLORREF pixelSource, ULONG userParam);
typedef	COLORREF (CALLBACK * MATRIXFILTERBLITPROC)(COLORREF * pixelDest, COLORREF * pixelSrc, ULONG userParam);

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
class cSurface;
// Lost device callback function
typedef	void (CALLBACK * LOSTDEVICECALLBACKPROC)(cSurface*, LPARAM);
#endif // HWABETA

// System colors
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION	28
#endif

// Forwards
class cSurface;
class cSurfaceImplementation;
class CFillData;
class CInputFile;

// Types
//typedef	cSurface * cSurface *;

// Blit modes
// Cannot rename, is used by DLL function signatures
enum BlitMode {
	BMODE_OPAQUE,
	BMODE_TRANSP,
	BMODE_MAX
};

// Blit operations. Cannot rename, is used by DLL function signatures
enum _Enum_is_bitflag_ BlitOp {
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
};

enum class _Enum_is_bitflag_ BlitOperation {
	// Copies by overwriting destination surface
	Copy,
	// Blend the destination RGB with source surface RGB
	// blend_coef is in range 0 to 128, 128 being a full merge
	// dest = ((dest * blend_coef) + (src * (128-blend_coef)))/128
	BLEND,
	// Invert the source RGB before overwriting destination RGB
	// src XOR 0xFFFFFF
	INVERT,
	// Combine the source RGB with dest RGB using XOR
	// src XOR dest
	XOR,
	// Combine the source RGB with dest RGB using AND
	// src AND dest
	AND,
	// Combine the source RGB with dest RGB using OR
	// src OR dest
	OR,
	// Blend the destination RGB with source surface RGB
	// dest = ((dest * blend_coef) + ((src==transp) ? replace : src * (128-blend_coef) ))/128
	BlendReplaceTransp,
	// TODO: Confirm this
	// Blits using the WinGDI raster operation (ROP) specified in the lParam
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt#:~:text=A%20raster-operation%20code
	DrawWithROP,
	// Applies INVERT then AND
	AndNot,
	// Combines the source RGB with the dest RGB by adding src and dest together by color channels
	Addition,
	// Gets the mean average of R,G,B and uses it for dest R,G,B
	Mono,
	// Combines the source RGB with the dest RGB by subtracting src from dest by color channels
	Subtract,
	// ?
	BlendDontReplaceColor,
	// ?
	EffectEx,
	// Max possible outside of a bitmask
	Max,
	// The bitmask for possible blit ops
	Mask = 0x0FFF,
	// ?
	RGBAFilter = 0x1000,
};
enum_class_is_a_bitmask(BlitOperation);

constexpr std::uint8_t AlphaToSemiTransp(const std::uint8_t a) {
	return a == 0 ? 128 : (255 - a) / 2;
}

constexpr std::uint8_t SemiTranspToAlpha(const std::uint8_t s) {
	return (s == 128) ? 0 : (255 - s * 2);
}
typedef ULONG RGBAREF;
constexpr RGBAREF ColorRefAToRGBA(const COLORREF c, const std::uint8_t a) {
	return ((c & 0x00FFFFFF) | (a << 24));
}

// Surface capabilities
enum class SurfaceCapabilities
{
	// Capabilities
	SC_GETPIXEL,
	SC_SETPIXEL,
	SC_FILL,
	SC_DRAW,
	SC_ROTATE,
	SC_STRETCH,
	SC_MAX
};

// SetPalette()
// Cannot be renamed, part of a linked DLL function
enum class SetPaletteAction
{
	// Just update palette
	None,
	// remap current surface pixels to new palette
	RemapSurface,
	Max
};
// Surface types
enum class SurfaceType : int
{
	// Buffer only
	Memory,
	// Buffer + Device context (i.e. DIBSection, DDRAW surface, etc...
	Memory_DeviceContext,
	// Buffer + permanent device context (i.e. DIBDC)
	Memory_PermanentDeviceContext,
	// Surface Direct Draw in system memory
	DirectDraw_SystemMemory,
	// Screen surface in HWA mode
	// TODO: YQ: What?
	HWA_Screen,
	// Render target texture in HWA mode
	HWA_RenderTargetTexture,
	// HWA texture created in video memory, unmanaged (lost when device is lost)
	HWA_UnmanagedTexture,
	// HWA texture created in video memory, managed (automatically reloaded when the device is lost)
	HWA_ManagedTexture,
	// HWA mosaic texture
	HWA_Mosaic,
	Max
};

// Drivers for memory + DC surfaces
enum class SurfaceDriver : int
{
	// DIB; Device Independent Bitmap, a bitmap that is not set to a specific output device
	// which would mean an implied "pixels per inch" resolution.
	DIB,

	// WinGDI; Windows Graphical Device Interface, the oldest, barebones graphics tech,
	// which creates the simple UI interface you can see in standard Windows applications
	// that don't make use of customizable UI frameworks.
	// Fusion is one of these, using purely GDI calls for the interface, with some optional
	// Direct3D use for rendering the frame editor.
	WinGDI,

	// Microsoft DirectDraw, an old tech, part of DirectX; since Vista, is now a translation layer to Direct3D
	DirectDraw,

	// Bitmap introduced in Windows 3.1 and still going strong
	Bitmap,

	// 3DFX Glide API. Deprecated.
	// The first popular Windows hardware-accelerated 3D tech, predating even Direct3D. Based on a subset of OpenGL v1.1
	// Eventually superceded by Direct3D and OpenGL.
	// TODO: YQ: Is this a ST_HWATEXTURE-compatible driver? Implied by
	// https://github.com/clickteam-plugin/Surface/blob/19968f04b67b58b1aaf3585f8558126df14e6fa5/Runtime.cpp#L138
	_3DFX,

	// Microsoft Direct3D v9, introduced 2002, last update July 2007.
	// Backwards compatible to Windows 98 (last supported 9.0c, Dec 2006).
	Direct3D9,

	// Microsoft Direct3D v8, deprecated.
	// The first version of Direct3D to take over DirectDraw, and take over 2D as well as 3D; the first version to
	// not use WinGDI, and instead call to the kernel directly.
	// Introduced in 2000, final release 2002. Backwards compatible to Windows 95 (last support v8.0a, 2001).
	Direct3D8,

	// Microsoft Direct3D v11, introduced 2009, and still maintained in 2022.
	// Compatible with Vista+.
	Direct3D11,

	// Max valid number
	Max
};


// SetAutoVSync() function flags.
enum class _Enum_is_bitflag_ SetAutoVSyncFlags : int
{
	VSync = 0x0001,
	ResetDevice = 0x0002,
	BackBuffers = 0x0004,
	// YQ: Why is this the same?!
	BackBufferShift = 0x0004,
};
enum_class_is_a_bitmask(SetAutoVSyncFlags);

// LoadImage() function flags. Cannot rename this; part of the function signatures for cSurface.
enum class _Enum_is_bitflag_ LIFlags : int
{
	None = 0x0000,
	Remap = 0x0001,
	ChangeSurfaceDepth = 0x0002,
	// Do not normalize palette
	DoNotNormalizePalette = 0x0004
};
typedef LIFlags LoadImageFlags;
enum_class_is_a_bitmask(LoadImageFlags);

// SaveImage() function flags. Cannot rename this; it's part of the function signatures for cSurface.
enum class _Enum_is_bitflag_ SIFlags : int
{
	None = 0x0000,
	OnlyHeader = 0x0001,
	SaveAlpha = 0x0002
};
typedef SIFlags SaveImageFlags;
enum_class_is_a_bitmask(SaveImageFlags);

// TODO: YQ: When is this used? What does LBF_ mean?
enum {
	LBF_DONOTCHANGESURFACEDEPTH = 0x0001
};

// Blit options
enum class _Enum_is_bitflag_ BlitOptions : ULONG {
	None			= 0x0000,
	// Anti-aliasing
	AntiAliasing	= 0x0001,
	// Copy alpha channel to destination alpha channel instead of applying it
	CopyAlpha		= 0x0002,
#ifdef HWABETA
	// TODO: YQ: What is a safe source?
	// HWA only
	SafeSource		= 0x0010,
	// TODO: YQ: Tile as in mosaic? Any margins?
	// HWA only
	Tile			= 0x0020
#endif // HWABETA
};
enum_class_is_a_bitmask(BlitOptions);


// Stretch & BlitEx options
enum class _Enum_is_bitflag_ StretchFlags : unsigned int {
	None			= 0x0000,
	// Resample bitmap
	Resample		= 0x0001,
	// Resample bitmap, but doesn't resample the transparent color
	ResampleTransparentColor = 0x0002,
	// Copy (stretch) alpha channel to destination alpha channel instead of applying it
	CopyAlpha		= 0x0004,
#ifdef HWABETA
	SafeSource		= 0x0010,
	Tile			= 0x0020
#endif
};
enum_class_is_a_bitmask(StretchFlags);


// Transparent monochrome mask for collisions
struct sMask
{
	// TODO: YQ: Is this size of CollisionMask (sMask) in bytes?
	int		mkSize;
	// Width in pixels
	int		mkWidth;
	// Height in pixels
	int		mkHeight;
	// TODO: YQ: Is this width in bytes... that wouldn't ame
	UINT	mkWidthBytes;
	// TODO: YQ: X/Y based on Hot Spot, Action Point, or something else?
	int		mkXSpot;
	int		mkYSpot;
	// TODO: YQ: MKFlags? Is this SCMF_XX
	ULONG	mkFlags;
	// TODO: YQ: What is this relative to?
	RECT	mkRect;
};
typedef sMask CollisionMask;

enum class CreateCollisionMaskFlags : std::uint16_t {
	Full,
	Platform
};

// TODO: YQ: Why is this defined?
#ifndef PI
	#define PI ((double)3.14159265358979323846)
#endif

// Allocate/Free surface
// TODO: YQ: new cSurface, or NewSurface()?
FusionAPIImport cSurface * FusionAPI NewSurface();
FusionAPIImport void FusionAPI DeleteSurface(cSurface * pSurf);

// Get surface prototype. You should NOT free the cSurface proto returned!
// TODO: YQ: If there is an error, where is it placed?
FusionAPIImport BOOL FusionAPI GetSurfacePrototype(cSurface * *proto, int depth, int st, int drv);

// DIB
static_assert(sizeof(ULONG) == 4, "32-bit long expected");
FusionAPIImport ULONG FusionAPI GetDIBHeaderSize(int depth);
FusionAPIImport ULONG FusionAPI GetDIBWidthBytes(int width, int depth);
FusionAPIImport ULONG FusionAPI GetDIBSize(int width, int height, int depth);
FusionAPIImport std::byte * FusionAPI GetDIBBitmap(LPBITMAPINFO pBmi);

// cSurface class
class FusionAPIImport cSurface
{
public:
		// ======================
		// Creation / Destruction
		// ======================

		// TODO: YQ: Prefered over NewSurface()?
		// TODO: YQ: Are copy and move constructors defined?
		cSurface();
		~cSurface();

		// TODO: YQ: Init what, exactly?
		static void InitializeSurfaces();
		static void FreeSurfaces();
		#if HWABETA
		// TODO: YQ: What is being freed?
		// HWA only
		static void FreeExternalModules();
		#endif

		// Operators

		cSurface & operator= (const cSurface & source);

		// Create surface implementation from surface prototype
		void Create (int width, int height, cSurface * prototype);

		// Create pure DC surface from DC
		void Create (HDC hDC);

		// Create pure DC surface from a window
		void Create (HWND hWnd, BOOL IncludeFrame);

		// TODO: YQ: What is a screen surface? What does creating one do?
		// Create screen surface (fullscreen mode)
		BOOL CreateScreenSurface();
		BOOL IsScreenSurface();

		// TODO: YQ: What makes a surface valid or invalid?
		// Valid?
		BOOL IsValid ();

		// TODO: YQ: What is type?
		// Get driver & type
		int		GetType();
		// TODO: YQ: What is driver?
		int		GetDriver();
		// TODO: YQ: What is driverinfo? What should be passed to pInfo?
		ULONG	GetDriverInfo(void * pInfo);

		// Clone surface (= create with same size + Blit)
		void Clone (const cSurface & pSrcSurface, int newW = -1, int newH = -1);

		// TODO: YQ: What is "before to create"? Reusing the cSurface *?
		// YQ: Does this delete this cSurface? Is it suitable for use with Clone() only? just Create()?
		// Delete surface implementation (before to create another one)
		void Delete();

		// ======================
		// error codes
		// ======================
		// This has never returned an error in scenarios where nothing is drawn (e.g. drawing a line 2px in D3D 11)
		int		GetLastError(void);

		// ======================
	    // Surface info
		// ======================
		// TODO: Width in pixels? What if invalid?
		int		GetWidth(void) const;
		// TODO: Height in pixels? What if invalid?
		int		GetHeight(void) const;

		// Number of bits per pixel. 8, 15, 16, 24 or 32 (BGRA).
		// 8 bits = 1 byte index in the color table
		// TODO: color table = pallete?
		// 15 = 16 bit per pixel; 5 bits per component, 1 bit ignored. Red mask = 0x7C00, green mask = 0x03E0, blue mask = 0x001F.
		// 16 = 16 bit per pixel; 5 bits for R/B, 6 for G. Red mask = 0xF800, green mask = 0x07E0, blue mask = 0x001F.
		// 24 = 8 bits per component, BGR layout.
		// 32 = 8 bits per component, BGRA layout; A is normally unused, but can be alpha.
		int		GetDepth(void) const;
		BOOL	GetInfo(int & width, int & height, int & depth) const;

		// ======================
	    // Surface coordinate management
		// ======================

		// TODO: YQ: What does Origin mean? Something relative to Hot Spot? Relative for blit operations?
		void	SetOrigin(int x, int y);
		void	SetOrigin(POINT c);

		void	GetOrigin(POINT &pt);
		void	GetOrigin(int & x, int & y);

		void	OffsetOrigin(int dx, int dy);
		void	OffsetOrigin(POINT delta);

		// ======================
	    // Raster operations
		// ======================

		// Locks the raw memory of the Surface, and returns the address to it. When you're done writing, use UnlockBuffer to return it.
		// TODO: YQ: Does this work for all surface types?
		LPBYTE	LockBuffer();

		// TODO: YQ: Why do you have to unlock with address, won't OOP make that pointless? Can you pass a different address?
		void	UnlockBuffer(LPBYTE spBuffer);
		// TODO: YQ: What is pitch? It's not Depth()?
		int		GetPitch() const;

		// ======================
		// Double-buffer handling
		// ======================

		// Sets this cSurface as the current one in a double-buffer... in what context? How are surfaces paired, and how do we know what the pairs target?
		void	SetCurrentDevice();
		// TODO: YQ: Rendering to GetRenderTargetSurface()? Where and why do we need to call BeginRendering()?
		int		BeginRendering(BOOL clear, RGBAREF rgba);
		// TODO: YQ: Rendering to GetRenderTargetSurface()?
		int		EndRendering();
		// TODO: YQ: update how?
		BOOL	UpdateScreen();
		#ifdef HWABETA
			// TODO: YQ: update how?
			cSurface* GetRenderTargetSurface();
			void	ReleaseRenderTargetSurface(cSurface* psf);
			// TODO: YQ: What does this do, how does Max affect it ?
			void	Flush(BOOL bMax);
			// TODO: YQ: I would guess this is the Z position of the buffer, but a float makes no sense here.
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

		// TODO: YQ: WHat does clipping apply to? Blit?
		void	GetClipRect(int & x, int & y, int & w, int & h);
		void	SetClipRect(int x, int y, int w, int h);
		void	ClearClipRect(void);

		// ======================
		// LoadImage (DIB format) / SaveImage (DIB format)
		// ======================
		#undef LoadImage

		BOOL	LoadImageA (HFILE hf, ULONG lsize, LoadImageFlags loadFlags = LoadImageFlags::None);
		// See the _FUSIONT macro for filename!
		BOOL	LoadImageA (LPCSTR fileName, LoadImageFlags loadFlags = LoadImageFlags::None);
		BOOL	LoadImageA (HINSTANCE hInst, int bmpID, LoadImageFlags loadFlags = LoadImageFlags::None);
		BOOL	LoadImageA (LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LoadImageFlags loadFlags = LoadImageFlags::None);

		BOOL	LoadImageW (HFILE hf, ULONG lsize, LoadImageFlags loadFlags = LoadImageFlags::None);
		// See the _FUSIONT macro for filename!
		BOOL	LoadImageW (const UShortWCHAR * fileName, LoadImageFlags loadFlags = LoadImageFlags::None);
		BOOL	LoadImageW (HINSTANCE hInst, int bmpID, LoadImageFlags loadFlags = LoadImageFlags::None);
		BOOL	LoadImageW (LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LoadImageFlags loadFlags = LoadImageFlags::None);

		#ifdef _UNICODE
		#define LoadImage LoadImageW
		#else
		#define LoadImage LoadImageA
		#endif

		BOOL	SaveImage (HFILE hf, SaveImageFlags saveFlags = SaveImageFlags::None);
		// See the _FUSIONT macro for filename!
		BOOL	SaveImage (LPCSTR fileName, SaveImageFlags saveFlags = SaveImageFlags::None);
		// See the _FUSIONT macro for filename!
		BOOL	SaveImage (const UShortWCHAR * fileName, SaveImageFlags saveFlags = SaveImageFlags::None);
		BOOL	SaveImage (LPBITMAPINFO pBmi, LPBYTE pBits, SaveImageFlags saveFlags = SaveImageFlags::None);

		// TODO: YQ: WHat does this mean?
		ULONG	GetDIBSize ();

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
		BOOL	GetPixel(int x, int y, COLORREF & c) const;
		BOOL	GetPixel(int x, int y, BYTE & R, BYTE & G, BYTE & B) const;
		BOOL	GetPixel(int x, int y, int & index) const;

		// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
		COLORREF	GetPixelFast(int x, int y);
		int			GetPixelFast8(int x, int y);

		// ======================
		// Blit functions
		// ======================
		// Blit surface to surface
		BOOL	Blit(cSurface & dest) const;

		// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
		// TODO: does it apply to this func?
		BOOL	Blit(cSurface & dest, int destX, int destY,
					  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0,
					  ULONG dwBlitFlags = 0) const;

		// Blit rectangle to surface
		// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
		// TODO: does it apply to this func?
		BOOL	Blit(cSurface & dest, int destX, int destY,
					  int srcX, int srcY, int srcWidth, int srcHeight,
					  BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0,
					  ULONG dwBlitFlags = 0) const;

#ifdef HWABETA
		// Extended blit : can do stretch & rotate at the same time
		// Only implemented in 3D mode
		// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
		// TODO: does it apply to this func?
		BOOL	BlitEx(cSurface & dest, float dX, float dY, float fScaleX, float fScaleY,
						int sX, int sY, int sW, int sH, LPPOINT pCenter, float fAngle,
						BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, ULONG dwFlags = 0) const;
#endif
		// Scrolling
		BOOL	Scroll (int xDest, int yDest, int xSrc, int ySrc, int width, int height);

		// Blit via callback
		BOOL	FilterBlit (cSurface & dest, int destX, int destY,
							int srcX, int srcY, int srcWidth, int srcHeight,
							BlitMode bm, FILTERBLITPROC fbProc, LPARAM lUserParam) const;

		BOOL	FilterBlit (cSurface & dest, FILTERBLITPROC fbProc,
							LPARAM lUserParam, BlitMode bm = BMODE_OPAQUE) const;

		// Matrix blit via callback
		BOOL	MatrixFilterBlit (cSurface & dest, int destX, int destY,
									int srcX, int srcY, int srcWidth, int srcHeight,
									int mWidth, int mHeight, int mDXCenter, int mDYCenter,
									MATRIXFILTERBLITPROC fbProc, LPARAM lUserParam) const;

		// Stretch surface to surface
		BOOL	Stretch(cSurface & dest, ULONG dwFlags = 0) const;

		// Stretch surface to rectangle
		BOOL	Stretch(cSurface & dest, int destX, int destY, int destWidth, int destHeight,
						BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0, ULONG dwFlags = 0) const;

		// Stretch rectangle to rectangle
		BOOL	Stretch(cSurface & dest, int destX, int destY, int destWidth, int destHeight,
						int srcX, int srcY, int srcWidth, int srcHeight,
						BlitMode bm /*= BMODE_OPAQUE*/, BlitOp bo = BOP_COPY, LPARAM param = 0, ULONG dwFlags = 0) const;

		// Revert surface horizontally
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		BOOL	ReverseX();

		// Revert rectangle horizontally
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		BOOL	ReverseX(int x, int y, int width, int height);

		// Revert surface vertically
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		BOOL	ReverseY();

		// Revert rectangle vertically
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		BOOL	ReverseY(int x, int y, int width, int height);

		// Remove empty borders
		BOOL	GetMinimizeRect(RECT *);
		BOOL	Minimize();
		BOOL	Minimize(RECT * r);

		// =============================
		// Blit from screen or memory DC
		// =============================

		static BOOL CaptureDC(HDC srcDC,HDC dstDC,LONG srcX,LONG srcY,LONG dstX,LONG dstY,LONG srcWidth,LONG srcHeight,LONG dstWidth,LONG dstHeight,BOOL bFlushMessage,BOOL bKeepRatio);

		// ======================
		// Fill
		// ======================

		// Fill surface
		BOOL	Fill(COLORREF c);
		BOOL	Fill(CFillData * fd);
		BOOL	Fill(int index);
		BOOL	Fill(int R, int G, int B);

		// Fill block
		BOOL	Fill(int x, int y, int w, int h, COLORREF c);
		BOOL	Fill(int x, int y, int w, int h, CFillData * fd);
		BOOL	Fill(int x, int y, int w, int h, int index);
		BOOL	Fill(int x, int y, int w, int h, int R, int G, int B);
#ifdef HWABETA
		// TODO: YQ: dwFlags is made from what enum/flags?
		// HWA only
		BOOL	Fill(int x, int y, int w, int h, COLORREF* pColors, ULONG dwFlags);

		// Fill with blit mode and RGBA coef
		// TODO: This may be CF2.5+ only, or D3D11 surfaces only
		BOOL	fxFill(int x, int y, int w, int h, COLORREF c, BlitMode bm, COLORREF rgbaCoef);
		BOOL	fxFill(int x, int y, int w, int h, COLORREF* pColors, ULONG dwFlags, BlitMode bm, COLORREF rgbaCoef);
#endif

		// ======================
		// Geometric Primitives
		// ======================

		// 1. Simple routines : call GDI with Surface DC
		// =============================================

		BOOL	Ellipse(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Ellipse(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/,
			COLORREF crOutl /* = BLACK */, BOOL Fill = TRUE);

		BOOL	Rectangle(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/,
			COLORREF crOutl /* = 0 */, BOOL bFill = TRUE);

		BOOL	Polygon(LPPOINT pts, int nPts, int thickness = 1, COLORREF crOutl = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, COLORREF crFill, int thickness = 0,
			COLORREF crOutl = 0, BOOL bFill = TRUE);

		BOOL  Line(int x1, int y1, int x2, int y2, int thickness = 1, COLORREF crOutl = 0);

		// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
		// ==============================================================================

		BOOL	Ellipse(int left, int top, int right, int bottom, int thickness, CFillData * fdOutl, BOOL AntiA = FALSE,
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Ellipse(int left, int top, int right, int bottom, CFillData * fdFill,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Ellipse(int left, int top, int right, int bottom, CFillData * fdFill, int thickness, CFillData * fdOutl,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);

		BOOL	Rectangle(int left, int top, int right, int bottom, int thickness, CFillData * fdOutl, BOOL AntiA = FALSE,
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, CFillData * fdFill,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Rectangle(int left, int top, int right, int bottom, CFillData * fdFill, int thickness, CFillData * fdOutl,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);

		BOOL	Polygon(LPPOINT pts, int nPts, int thickness, CFillData * fdOutl, BOOL AntiA = FALSE,
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, CFillData * fdFill,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	Polygon(LPPOINT pts, int nPts, CFillData * fdFill, int thickness, CFillData * fdOutl,
			BOOL AntiA = FALSE, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);

		// Note: has no effect in HWA, if thickness is greater than 1.
		BOOL  Line(int x1, int y1, int x2, int y2, int thickness, CFillData * fdOutl, BOOL AntiA,
								  BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		// Filled Primitives

		BOOL	FloodFill(int x, int y, int & left, int & top, int & right, int & bottom, COLORREF crFill, BOOL AntiA = FALSE,
			int tol = 0, BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		BOOL	FloodFill(int x, int y, COLORREF crFill, BOOL AntiA = FALSE,  int tol = 0,
							BlitMode bm = BMODE_OPAQUE, BlitOp bo = BOP_COPY, LPARAM param = 0);

		// ======================
		// Rotation
		// ======================
		// Rotate surface in radians
		BOOL	Rotate(cSurface & dest, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// radian
		BOOL	Rotate(cSurface & dest, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// degree
		BOOL	Rotate90(cSurface & dest, BOOL b270);

		// Create rotated surface
		BOOL	CreateRotatedSurface (cSurface & ps, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);
		BOOL	CreateRotatedSurface (cSurface & ps, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);

#ifdef HWABETA
		static void GetSizeOfRotatedRect (int *pWidth, int *pHeight, float angle);
#else
		static void GetSizeOfRotatedRect (int *pWidth, int *pHeight, int angle);
#endif

		// ======================
		// text
		// ======================

		// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
		int		TextOutA(LPCSTR text, ULONG dwCharCount,int x,int y,ULONG alignMode,LPRECT pClipRc, COLORREF color=0, HFONT hFnt=(HFONT)NULL, BlitMode bm=BMODE_TRANSP, BlitOp=BOP_COPY, LPARAM param=0, int AntiA=0);
		// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
		int		TextOutW(const UShortWCHAR * text, ULONG dwCharCount,int x,int y,ULONG alignMode,LPRECT pClipRc, COLORREF color=0, HFONT hFnt=(HFONT)NULL, BlitMode bm=BMODE_TRANSP, BlitOp=BOP_COPY, LPARAM param=0, int AntiA=0);

#ifdef _UNICODE
	#define TextOut TextOutW
#else
	#define TextOut TextOutA
#endif
		// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
		int		DrawTextA(LPCSTR text, ULONG dwCharCount,LPRECT pRc, ULONG dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
						  BlitMode bm=BMODE_TRANSP, BlitOp bo=BOP_COPY, LPARAM param=0, int AntiA=0,ULONG dwLeftMargin=0,ULONG dwRightMargin=0,ULONG dwTabSize=8);
		// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
		int		DrawTextW(const UShortWCHAR * text, ULONG dwCharCount,LPRECT pRc, ULONG dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
						  BlitMode bm=BMODE_TRANSP, BlitOp bo=BOP_COPY, LPARAM param=0, int AntiA=0,ULONG dwLeftMargin=0,ULONG dwRightMargin=0,ULONG dwTabSize=8);

		// ======================
		// Color / Palette functions
		// ======================
		// Is transparent
		BOOL	IsTransparent();

		// Replace color
		BOOL	ReplaceColor(COLORREF newColor, COLORREF oldColor);

		// Test collision fine entre deux surfaces
		BOOL	IsColliding(cSurface & dest, int xDest, int yDest, int xSrc, int ySrc, int rcWidth, int rcHeight);

		// Create icon
		HICON	CreateIcon(int iconWidth, int iconHeight, COLORREF transpColor, POINT *pHotSpot);

	    // Palette support
		BOOL	Indexed(void);

		BOOL	SetPalette(LPLOGPALETTE palette, SetPaletteAction action = SetPaletteAction::None);
		BOOL	SetPalette(csPalette * pCsPal, SetPaletteAction action = SetPaletteAction::None);
		BOOL	SetPalette(cSurface & src, SetPaletteAction action = SetPaletteAction::None);
		BOOL	SetPalette(HPALETTE palette, SetPaletteAction action = SetPaletteAction::None);

		void	Remap(cSurface & src);
		void	Remap(LPBYTE remapTable);

		csPalette * GetPalette();
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
		BOOL	SetAutoVSync(SetAutoVSyncFlags nAutoVSync);
#endif
		BOOL	WaitForVBlank();

		// System colors
		// COLOR_MENUBAR (30) seems to be the highest sys color index, so we're using a uint8
		static COLORREF	GetSysColor(int colorIndex);
		// TODO: YQ: Is this meant to be a callback?
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
		ULONG		CreateMask(sMask * pMask, UINT dwFlags);

		// Lost device callback
#ifdef HWABETA
		void		OnLostDevice();
		void		AddLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
		void		RemoveLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);

		// CF2.5+'s Direct3D11 surfaces: Set premultiplied alpha flag
		void		SetPremultipliedAlpha(BOOL usePMAlpha, BOOL pmedTextures);
		void		PremultiplyAlpha();		// Only for surfaces with a lockable buffer

		// CF2.5+'s Direct3D11 surfaces only.
		// Only for surfaces with a lockable buffer.
		// TODO: Which surface types have a lockable buffer?
		void		DemultiplyAlpha();

#endif

	// Friend functions
	// ----------------
		FusionAPIImport friend cSurfaceImplementation * FusionAPI GetSurfaceImplementation(cSurface &cs);
		FusionAPIImport friend void FusionAPI SetSurfaceImplementation(cSurface &cs, cSurfaceImplementation *psi);

	protected:
		static void		BuildSysColorTable();

	// Clipping
	//----------
		// TODO: YQ: What does this apply to?
		HRGN SetDrawClip(HDC hDC);
		void RestoreDrawClip(HDC hDC,HRGN hOldClipRgn);

	// Private functions
	// -----------------
	private:
		BOOL	LoadPicture (CInputFile * pFile, ULONG bitmapSize, LoadImageFlags loadFlags);
		BOOL	LoadDIB (LPBITMAPINFO pBmi, LPBYTE pBits, LoadImageFlags loadFlags);

	// Private data
	// ------------
	private:
		cSurfaceImplementation * m_actual;
		POINT					origin;

		// System colors
		static BOOL			m_bSysColorTab;
		static COLORREF		m_sysColorTab[COLOR_GRADIENTINACTIVECAPTION+1];

	protected:
		// TODO: YQ: In English? :p
		int		m_error;		// Est-ce qu'on doit toujours garder ça?
};

// maximum opacity
// TODO: YQ: Is this applicable in HWA too? Why is 100 commented out?
#define OP_MAX 128			// 100

extern "C" FusionAPIImport BOOL	WINAPI BuildRemapTable	(LPBYTE, LOGPALETTE *, LOGPALETTE *, WORD);

// Used by EditorDisplay of most Fusion exts
FusionAPIImport cSurface* FusionAPI WinGetSurface(int idWin, int surfID = 0);
