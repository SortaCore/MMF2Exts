#pragma once
#ifndef _WIN32
#error Including header in wrong platform, try SurfaceMultiPlat.hpp
#endif
#if !defined(FUSION_INTERNAL_ACCESS) && !defined(SURFACE_MULTI_PLAT_INCLUDING_WINSURF)
#error To use the Windows-only cSurface, you must define FUSION_INTERNAL_ACCESS
#endif

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
#include "Palet.hpp"
#include <CommDlg.h>
#pragma comment(lib, "gdi32.lib")

class cSurface;
class cSurfaceImplementation;
class CFillData;
class CInputFile;

typedef	COLORREF (CALLBACK * FILTERBLITPROC)(COLORREF pixelDest, COLORREF pixelSource, ULONG userParam);
typedef	COLORREF (CALLBACK * MATRIXFILTERBLITPROC)(COLORREF * pixelDest, COLORREF * pixelSrc, ULONG userParam);

// Display mode
typedef	struct DisplayMode {
	int mcx;
	int mcy;
	int mDepth;
//	int mNearestBitCount;
//	int mDriver;
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
//typedef LPSURFACE cSurface *;

// Blit modes
// @remarks This is locked to this name due to cSurface's DLL function signatures
enum BlitMode {
	BMODE_OPAQUE,
	BMODE_TRANSP,
	BMODE_MAX
};

// Blit options
// @remarks This is an unnamed enum in original SDK
enum class _Enum_is_bitflag_ WinSurf_BlitOptions : ULONG {
	None = 0x0000,
	// Anti-aliasing
	AntiAliasing = 0x0001,
	// Copy alpha channel to destination alpha channel instead of applying it
	CopyAlpha = 0x0002,
#ifdef HWABETA
	// TODO: YQ: What is a safe source?
	// HWA only
	SafeSource = 0x0010,
	// TODO: YQ: Tile as in mosaic? Any margins?
	// HWA only
	Tile = 0x0020
#endif // HWABETA
};
enum_class_is_a_bitmask(WinSurf_BlitOptions);

// Blit operations
// @remarks This is locked to this name due to cSurface's DLL function signatures
enum BlitOp {
	// Use BlitOperation instead
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
// @remarks This is locked to this name, due to cSurface's DLL function signatures,
//			but strangely does not seem to be used.
enum class SurfaceCap
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
// @remarks This is locked to this name due to cSurface's DLL function signatures
enum class SetPaletteAction
{
	// Just update palette
	None,
	// remap current surface pixels to new palette
	RemapSurface,
	Max
};
// Surface types
// @remarks This is an unnamed enum in original SDK
enum class SurfaceType : int
{
	// Buffer only
	Memory,
	// Buffer + Device context (i.e. DIBSection, DDRAW surface, etc...
	Memory_DeviceContext,
	// Buffer + permanent device context (i.e. DIBDC)
	Memory_PermanentDeviceContext,
	// Surface Direct Draw in system memory. Do not create.
	DirectDraw_SystemMemory,
	// Screen surface in HWA mode. Used by frame surface (rhIdEditWin). Do not create.
	HWA_Screen,
	// Render target texture in HWA mode. CF2.5, 32bpp only.
	HWA_RenderTargetTexture,
	// HWA texture created in video memory, unmanaged (lost when device is lost). CF2.5, 32bpp only.
	HWA_UnmanagedTexture,
	// HWA texture created in video memory, managed (automatically reloaded when the device is lost). CF2.5, 32bpp only.
	HWA_ManagedTexture,
	// HWA mosaic texture. CF2.5, prototype is always DIB driver. May be created in CPU memory.
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
	// Not used in MMF2; WinGDI will be switched to DIB.
	WinGDI,

	// Microsoft DirectDraw, an old tech, part of DirectX; since Vista, is now a translation layer to Direct3D.
	// Used in MMF2 DirectX and DirectX+VRAM display type. However, cannot be prototyped consistently. Do not create.
	DirectDraw,

	// Bitmap introduced in Windows 3.1 and still going strong.
	// Not used in MMF2; Bitmap will be switched to DIB.
	Bitmap,

	// 3DFX Glide API. Deprecated.
	// The first popular Windows hardware-accelerated 3D tech, predating even Direct3D. Based on a subset of OpenGL v1.1
	// Eventually superceded by Direct3D and OpenGL.
	// TODO: YQ: Is this a ST_HWATEXTURE-compatible driver? Implied by
	// https://github.com/clickteam-plugin/Surface/blob/19968f04b67b58b1aaf3585f8558126df14e6fa5/Runtime.cpp#L138
	// Not used in MMF2; WinGDI will be switched to DIB.
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
// @remarks This is a series of defines in original SDK
enum class _Enum_is_bitflag_ SetAutoVSyncFlags : int
{
	VSync = 0x0001,
	ResetDevice = 0x0002,
	BackBuffers = 0x0004,
	// YQ: Why is this the same?!
	BackBufferShift = 0x0004,
};
enum_class_is_a_bitmask(SetAutoVSyncFlags);

// LoadImage() function flags.
// @remarks This is locked to this name due to cSurface's DLL function signatures
enum class _Enum_is_bitflag_ LIFlags : int
{
	None = 0x0000,
	Remap = 0x0001,
	ChangeSurfaceDepth = 0x0002,
	// Do not normalize palette
	DoNotNormalizePalette = 0x0004
};
enum_class_is_a_bitmask(LIFlags);

// SaveImage() function flags.
// @remarks This is locked to this name due to cSurface's DLL function signatures
enum class _Enum_is_bitflag_ SIFlags : int
{
	None = 0x0000,
	OnlyHeader = 0x0001,
	SaveAlpha = 0x0002
};
enum_class_is_a_bitmask(SIFlags);

// TODO: YQ: When is this used? What does LBF_ mean?
// @remarks This is an unnamed enum in original SDK
enum {
	LBF_DONOTCHANGESURFACEDEPTH = 0x0001
};

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
	// Assumes surface is locked with LockImageSurface?
	SafeSource		= 0x0010,
	// Repeats the surface blit in a tiled fashion, speedy; for 3DFX+ driver surfaces
	Tile			= 0x0020
#endif
};
enum_class_is_a_bitmask(StretchFlags);


// Transparent monochrome mask for collisions
// You should not use this; use DarkEdif::CollisionMask
struct sMask
{
	DarkEdifInternalAccessProtected:
	// TODO: YQ: Is this size of CollisionMask (sMask) in bytes?
	int mkSize;
	// Width in pixels
	int mkWidth;
	// Height in pixels
	int mkHeight;
	// TODO: YQ: Is this width in bytes... that wouldn't make much sense
	UINT	mkWidthBytes;
	// TODO: YQ: X/Y based on Hot Spot, Action Point, or something else?
	int mkXSpot;
	int mkYSpot;
	// TODO: YQ: MKFlags? Is this SCMF_XX
	ULONG	mkFlags;
	// TODO: YQ: What is this relative to?
	RECT	mkRect;
};

enum class CreateCollisionMaskFlags : std::uint16_t {
	Full,
	Platform
};

// A dummy class indicating you cannot read this without defining the right Direct3D headers and FUSION_INTERNAL_ACCESS.
// If you have, then use a different variable in the union.
// @remarks Although COM uses a long chain of inheritance, it's single-inheritance so this pointer should suffice.
struct FusionD3DDummy {};

#if defined(FUSION_INTERNAL_ACCESS)
#pragma pack (push, 1)

// Semi-opaque struct used in D3DSURFINFO; only expert Fusion ext devs should use this.
// D3D11 only?
struct FusionD3D11Texture final
{
	union {
#ifdef __d3d11_h__
		// Contains a ID3D11Texture2D; if grabbing, note GetResource increments the ref count,
		// so ideally store in an atlbase.h CComPtr smart pointer, or call Release().
		// CComPtr<ID3D11Texture2D> resTextD3D11;
		// d3d11ShaderResourceView->GetResource((ID3D11Resource**)&resourceTextureD3D11);
		ID3D11ShaderResourceView* D3D11ShaderResourceView;
#endif
		FusionD3DDummy * D3DGenericTexture;
	};
	std::uint32_t unknown1[3];	// set to 1, 0, 0
	std::uint32_t width, height;
	std::uint32_t flags;		// Set to 0xF0
	std::uint32_t unknown2[2];	// set to 0, 0
	// Struct may continue further, but D3D ended here

	NO_DEFAULT_CTORS_OR_DTORS(FusionD3D11Texture);
};
union FusionD3DTexture {
	// D3D11 has a sub-struct
	FusionD3D11Texture* D3D11TextureHolder;
#ifdef _D3D9_H_
	IDirect3DTexture9* D3D9Texture;
#endif
#ifdef _D3D8_H_
	IDirect3DTexture8* D3D8Texture;
#endif
	FusionD3DDummy * D3D8Or9Texture;
};

union FusionD3DDevice {
#ifdef __d3d11_h__
	// Direct3D 11 was introduced in Windows 7, and as of 2025 is the latest supported by Windows Fusion.
	ID3D11Device* D3D11Device;
#endif
#ifdef _D3D9_H_
	// Direct3D9 device. 9 supports Windows 98+, although support was dropped to Win 2000+ in 9c.
	// @remarks In later Windows, variants of 9 are usually wrapped into later Direct3D.
	// 9Ex was introduced in Vista+, but is not used in Fusion Direct3D 9 runtime.
	// Direct3D 11 tech can also be created with feature level limit to 9, making a ID3D11 interface,
	// but only using 9 features. Fusion does not use this either.
	// You cannot debug Direct3D 9 or 9Ex apps on Windows 10 and later.
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3d9-createdevice
	IDirect3DDevice9* D3D9Device;
#endif // _D3D9_H_
#ifdef _D3D8_H_
	IDirect3DDevice8* D3D8Device;
#endif
	FusionD3DDummy* D3DGenericDevice;
};
// Two types: IDirect3D9 or ID3D11DeviceContext.
union FusionD3DDevCtxOrTech {
#ifdef __d3d11_h__
	ID3D11DeviceContext* D3D11DevCtx;
#endif
	// Direct3D 8-9 don't have a DeviceContext class like D3D11,
	// but Fusion returns the IDirect3D9 in the place of the device context in GetDriverInfo().
	// This is not D3D9Ex. See Direct3D9 notes in FusionD3DDevice.
#ifdef _D3D9_H_
	IDirect3D9* D3D9;
#endif
#ifdef _D3D8_H_
	IDirect3D8* D3D8;
#endif
	// ID3DDeviceContext or IDirect3D9 fallback ptr
	FusionD3DDummy* D3DGenericDeviceCtxOrTech;
};

#pragma pack (pop)
#else // Ext dev didn't want fusion internals, so use opaque types
typedef FusionD3DDummy FusionD3DTexture, FusionD3DDevice, FusionD3DDevCtxOrTech;
#endif // Fusion internals

// Direct3D 8-11 info returned by cSurface::GetDriverInfo(). Not all variables will be set.
struct FusionD3DSurfDriverInfo final
{
	// Size of this struct in bytes, varies per D3D level.
	// It is recommended you set this to whatever GetDriverInfo(NULL) returns.
	DWORD surfInfoSize;

	// Set to 8, 9, 11.
	int D3DVersion;

	// D3D9: D3D9, Device, Texture.
	// D3D11: Faux-Context, Device, Texture.

	// D3D or device context ptr; e.g. D3D9 a IDirectEx9 *, in D3D11 a ID3D11DeviceContext *.
	// Set for frame surfaces.
	FusionD3DDevCtxOrTech D3DContextOrTech;

	// Device pointer, e.g. LPDIRECT3DDEVICE9, ID3D11Device *.
	// @remarks Does not report as a live object in D3D11 Debug on end of app, as it's already deleted.
	FusionD3DDevice D3DDevice;
	

	// @brief A direct pointer to a D3D texture in D3D8-9, but a pointer-to-pointer in D3D11.
	//		  Only inited with cSurface if HWA_RenderTargetTexture type (which is D3D11 only),
	//		  otherwise null and set on first blit. Not set for frame surface.
	// @remarks
	// For D3D9, it is a IDirect3DTexture9 *.
	// 
	// For D3D11, set to a ID3D11ShaderResourceView **. Get a ID3D11Texture2D * by calling GetResource.
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-cd3d11_texture2d_desc
	// 
	// CComPtr<ID3D11Texture2D> texture;
	// (*((ID3D11ShaderResourceView **)x.m_ppD3DTexture))->GetResource((ID3D11Resource**)&texture);
	FusionD3DTexture D3DTexture;

	template<typename T = void>
	inline T * GetTexturePtr() {
		// Deref twice for D3D11, once for others
		if (D3DVersion == 11)
			return (T*)D3DTexture.D3D11TextureHolder->D3DGenericTexture;
		return (T*)D3DTexture.D3D8Or9Texture;
	}

	// PS max level - may come up as Shader Model.
	// This is capped to min of max supported by D3D version, and max supported by GPU.
	// D3D8 is limited to PS 1.4, and is set here as 0xFFFF0104.
	// D3D9 is limited to PS 3.0, and is set here as 0xFFFF0300.
	// D3D11 is limited to PS 5.1, but Fusion sets this to 0x5.
	int		PixelShaderVersion;
	// VS max level (often matches pixel shader version).
	// Fusion does not set this for D3D8 or 9, but for 11 it sets it to 0x5.
	int		VertexShaderVersion;
	// Max texture size (a power of 2, e.g. 16384)
	int		MaxTextureWidth;
	int		MaxTextureHeight;
	// [D3D11 only] Not set for frame
	LPVOID	ppD3D11RenderTargetTexture;
	// [D3D11 only] Not set for frame
	LPVOID	ppD3D11RenderTargetView;
	// [D3D11 only] Set for frame?
	LPVOID	txtContext;
};

// Mode for LockImageSurface
// TODO: Is this a bitflag?
enum class LockImageSurfaceMode : int
{
	// Reading the pixels only
	// TODO: Does this include alpha?
	ReadBlitOnly,

	// TODO: What does this include?
	AllReadAccess,

	// Can be used with SurfaceDriver::_3DFX and later
	// TODO: Is this required outright, or does it speed things, etc for HWA? What if used for software?
	HWACompatible
};
// Call to populate a cSurface from an image bank ID. Warning: invalid ID will cause a crash.
FusionAPIImport BOOL FusionAPI LockImageSurface(void* idApp, DWORD hImage, cSurface& cs, int flags = (int)LockImageSurfaceMode::ReadBlitOnly);
FusionAPIImport void FusionAPI UnlockImageSurface(cSurface& cs);

// Allocate/Free surface
// TODO: YQ: new cSurface, or NewSurface()?
FusionAPIImport cSurface * FusionAPI NewSurface();
FusionAPIImport void FusionAPI DeleteSurface(cSurface * pSurf);

// Get surface prototype. You should NOT free the cSurface proto returned!
// TODO: YQ: If there is an error, where is it placed?
FusionAPIImport BOOL FusionAPI GetSurfacePrototype(cSurface ** proto, int depth, int st, int drv);

// DIB
static_assert(sizeof(ULONG) == 4, "32-bit long expected");
FusionAPIImport ULONG FusionAPI GetDIBHeaderSize(int depth);
FusionAPIImport ULONG FusionAPI GetDIBWidthBytes(int width, int depth);
FusionAPIImport ULONG FusionAPI GetDIBSize(int width, int height, int depth);
FusionAPIImport std::uint8_t * FusionAPI GetDIBBitmap(LPBITMAPINFO pBmi);

// cSurface class
class FusionAPIImport cSurface
{
public:
	// ======================
	// Creation / Destruction
	// ======================

	// TODO: YQ: Prefered over NewSurface()?
	// TODO: YQ: Are copy and move constructors defined?

	// Creates a cSurface container, without creating its internal surface - see Create, Delete
	cSurface();
	~cSurface();

	// Operators
	cSurface& operator=(const cSurface& source);

	// TODO: YQ: Init what, exactly?
	static void InitializeSurfaces();
	static void FreeSurfaces();
#if HWABETA
	// TODO: YQ: What is being freed?
	// HWA only
	static void FreeExternalModules();
#endif

	// Create surface implementation from surface prototype
	void Create(int width, int height, cSurface * prototype);

	// Create pure DC surface from DC
	void Create(HDC hDC);

	// Create pure DC surface from a window
	void Create(HWND hWnd, BOOL IncludeFrame);

	// TODO: YQ: What is a screen surface? What does creating one do?
	// Create screen surface(fullscreen mode)
	BOOL CreateScreenSurface();
	BOOL IsScreenSurface();

	// TODO: YQ: What makes a surface valid or invalid? Create() fail?
	// Valid?
	BOOL IsValid();

	// Returns ST_XX enum, cast to SurfaceType enum
	int GetType();
	// Returns SD_XX enum, cast to SurfaceDriver enum
	int GetDriver();
	// Set of driver info data. See D3DSURFINFO struct. Call with NULL to get the expected struct size to pass as pInfo.
	// @remarks This is only going to return content if Driver is not DIB, possibly 3DFX+ only.
	//			3DFX or DirectDraw does not return D3DSURFINFO struct.
	//			3DFX has not been tested, as MMF2 does not expose this display mode.
	//			DirectDraw (MMF2 DirectX mode) returns an as-yet unknown struct of 28 bytes.
	ULONG GetDriverInfo(void * pInfo);

	// Clone surface(= create with same size + Blit)
	void Clone(const cSurface & pSrcSurface, int newW = -1, int newH = -1);

	// TODO: YQ: What is "before to create"? Reusing the cSurface *?
	// YQ: Does this delete this cSurface? Is it suitable for use with Clone() only? just Create()?
	// Delete surface implementation(before to create another one)
	void Delete();

	// ======================
	// error codes
	// ======================
	// This has never returned an error in scenarios where nothing is drawn (e.g. drawing a line 2px in D3D 11)
	int GetLastError(void);

	// ======================
	// Surface info
	// ======================
	// TODO: Width in pixels? What if invalid?
	int GetWidth(void) const;
	// TODO: Height in pixels? What if invalid?
	int GetHeight(void) const;

	// Number of bits per pixel. 8, 15, 16, 24 or 32(BGRA).
	// 8 bits = 1 byte index in the color table
	// TODO: color table = pallete?
	// 15 = 16 bit per pixel; 5 bits per component, 1 bit ignored. Red mask = 0x7C00, green mask = 0x03E0, blue mask = 0x001F.
	// 16 = 16 bit per pixel; 5 bits for R/B, 6 for G. Red mask = 0xF800, green mask = 0x07E0, blue mask = 0x001F.
	// 24 = 8 bits per component, BGR layout.
	// 32 = 8 bits per component, BGRA layout; A is normally unused, but can be alpha.
	int GetDepth(void) const;
	BOOL GetInfo(int & width, int & height, int & depth) const;

	// ======================
	// Surface coordinate management
	// ======================

	// TODO: YQ: What does Origin mean? Something relative to Hot Spot? Relative for blit operations?
	void SetOrigin(int x, int y);
	void SetOrigin(POINT c);

	void GetOrigin(POINT &pt);
	void GetOrigin(int & x, int & y);

	void OffsetOrigin(int dx, int dy);
	void OffsetOrigin(POINT delta);

	// ======================
	// Raster operations
	// ======================

	// Locks the raw memory of the Surface, and returns the address to it. When you're done writing, use UnlockBuffer to return it.
	// Does not work for any HWA surface type; you can only blit into it from another surface.
	LPBYTE	LockBuffer();

	// TODO: YQ: Why do you have to unlock with address, won't OOP make that pointless? Can you pass a different address?
	void UnlockBuffer(LPBYTE spBuffer);

	// Image stride, aka pitch, is the number of bytes for one row of pixels in image, including memory alignment padding.
	// If image is upside down in RAM compared to memory (usual for BMP format), pitch will be negative.
	// https://learn.microsoft.com/en-us/windows/win32/medfound/image-stride
	int GetPitch() const;

	// ======================
	// Double-buffer handling
	// ======================

	// Sets this cSurface as the current one in a double-buffer... in what context? How are surfaces paired, and how do we know what the pairs target?
	void SetCurrentDevice();
	// TODO: YQ: Rendering to GetRenderTargetSurface()? Where and why do we need to call BeginRendering()?
	// Implemented in Direct3D 9 and 11 on CF2.5+. Not available in Direct3D 8.
	int  BeginRendering(BOOL clear, RGBAREF rgba);
	// TODO: YQ: Rendering to GetRenderTargetSurface()?
	int  EndRendering();
	// TODO: YQ: update how?
	BOOL UpdateScreen();

	#ifdef HWABETA
		// TODO: YQ: update how?
		// This seems entirely useless on HWA non-frame surfaces.
		cSurface* GetRenderTargetSurface();
		void	  ReleaseRenderTargetSurface(cSurface* psf);
		// TODO: YQ: What does this do, how does Max affect it ?
		void	  Flush(BOOL bMax);
		// TODO: YQ: I would guess this is the Z position of the buffer, but a float makes no sense here.
		void	  SetZBuffer(float z2D);
	#endif

	// ======================
	// Device context for graphic operations
	// ======================

	// Returns a HDC. Ref-counted, you must release the DC with ReleaseDC() once done.
	HDC	 GetDC(void);
	void ReleaseDC(HDC dc);
	void AttachWindow(HWND hWnd);

	// ======================
	// Clipping
	// ======================

	// TODO: YQ: WHat does clipping apply to? Blit?
	void GetClipRect(int & x, int & y, int & w, int & h);
	void SetClipRect(int x, int y, int w, int h);
	void ClearClipRect(void);

	// ======================
	// LoadImage (DIB format) / SaveImage (DIB format)
	// ======================

	// Defined in Win32 header, will mess with these func names if defined here
	#undef LoadImage

	BOOL LoadImageA(HFILE hf, ULONG lsize, LIFlags loadFlags = LIFlags::None);
	// See the _FUSIONT macro for filename!
	BOOL LoadImageA(LPCSTR fileName, LIFlags loadFlags = LIFlags::None);
	BOOL LoadImageA(HINSTANCE hInst, int bmpID, LIFlags loadFlags = LIFlags::None);
	BOOL LoadImageA(LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LIFlags loadFlags = LIFlags::None);

	BOOL LoadImageW(HFILE hf, ULONG lsize, LIFlags loadFlags = LIFlags::None);
	// See the _FUSIONT macro for filename!
	BOOL LoadImageW(const UShortWCHAR * fileName, LIFlags loadFlags = LIFlags::None);
	BOOL LoadImageW(HINSTANCE hInst, int bmpID, LIFlags loadFlags = LIFlags::None);
	BOOL LoadImageW(LPBITMAPINFO pBmi, LPBYTE pBits = NULL, LIFlags loadFlags = LIFlags::None);

	// Redefine it back
#ifdef _UNICODE
	#define LoadImage LoadImageW
#else
	#define LoadImage LoadImageA
#endif

	BOOL SaveImage(HFILE hf, SIFlags saveFlags = SIFlags::None);
	// See the _FUSIONT macro for filename!
	BOOL SaveImage(LPCSTR fileName, SIFlags saveFlags = SIFlags::None);
	// See the _FUSIONT macro for filename!
	BOOL SaveImage(const UShortWCHAR * fileName, SIFlags saveFlags = SIFlags::None);
	BOOL SaveImage(LPBITMAPINFO pBmi, LPBYTE pBits, SIFlags saveFlags = SIFlags::None);

	// TODO: YQ: WHat does this mean?
	ULONG GetDIBSize();

	// ======================
	// Pixel functions
	// ======================
	// Set pixel
	void SetPixel(int x, int y, COLORREF c);
	void SetPixel(int x, int y, BYTE R, BYTE G, BYTE B);
	void SetPixel(int x, int y, int index);

	// Faster: assume clipping is done, the origin is at(0,0) and the surface is locked
	void SetPixelFast(int x, int y, COLORREF c);
	void SetPixelFast8(int x, int y, int index);

	// Get pixel
	BOOL GetPixel(int x, int y, COLORREF & c) const;
	BOOL GetPixel(int x, int y, BYTE & R, BYTE & G, BYTE & B) const;
	BOOL GetPixel(int x, int y, int & index) const;

	// Faster: assume clipping is done, the origin is at(0,0) and the surface is locked
	COLORREF GetPixelFast(int x, int y);
	int 	 GetPixelFast8(int x, int y);

	// ======================
	// Blit functions
	// ======================

	// Blit surface to surface. Works with HWA destination.
	BOOL Blit(cSurface & dest) const;

	// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
	// TODO: does it apply to this func?
	BOOL Blit(cSurface & dest, int destX, int destY,
			  BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM effectParam = 0,
			  /* WinSurf_BlitOptions */ ULONG dwBlitFlags = 0) const;

	// Blit rectangle to surface
	// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
	// TODO: does it apply to this func?
	BOOL Blit(cSurface & dest, int destX, int destY,
			  int srcX, int srcY, int srcWidth, int srcHeight,
			  BlitMode bm /*= BlitMode::BMODE_OPAQUE*/, BlitOp bo = BlitOp::BOP_COPY, LPARAM effectParam = 0,
			  /* WinSurf_BlitOptions */ ULONG dwBlitFlags = 0) const;

#ifdef HWABETA
	// Extended blit : can do stretch & rotate at the same time
	// Only implemented in 3D mode
	// In CF2.5 : HIBYTE(dwBlitFlags) is alpha blend coefficient
	// TODO: does it apply to this func?
	BOOL BlitEx(cSurface & dest, float dX, float dY, float fScaleX, float fScaleY,
				int sX, int sY, int sW, int sH, LPPOINT pCenter, float fAngle,
				BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM effectParam = 0,
				/* WinSurf_BlitOptions */ ULONG dwFlags = 0) const;
#endif
	// Scrolling
	BOOL Scroll(int xDest, int yDest, int xSrc, int ySrc, int width, int height);

	// Blit via callback
	BOOL FilterBlit(cSurface & dest, int destX, int destY,
					int srcX, int srcY, int srcWidth, int srcHeight,
					BlitMode bm, FILTERBLITPROC fbProc, LPARAM lUserParam) const;

	BOOL FilterBlit(cSurface & dest, FILTERBLITPROC fbProc,
					LPARAM lUserParam, BlitMode bm = BlitMode::BMODE_OPAQUE) const;

	// Matrix blit via callback
	BOOL MatrixFilterBlit(cSurface & dest, int destX, int destY,
						  int srcX, int srcY, int srcWidth, int srcHeight,
						  int mWidth, int mHeight, int mDXCenter, int mDYCenter,
						  MATRIXFILTERBLITPROC fbProc, LPARAM lUserParam) const;

	// Stretch surface to surface
	BOOL Stretch(cSurface & dest, ULONG dwFlags = 0) const;

	// Stretch surface to rectangle
	BOOL Stretch(cSurface & dest, int destX, int destY, int destWidth, int destHeight,
				 BlitMode bm /*= BlitMode::BMODE_OPAQUE*/, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0, ULONG dwFlags = 0) const;

	// Stretch rectangle to rectangle
	BOOL Stretch(cSurface & dest, int destX, int destY, int destWidth, int destHeight,
				 int srcX, int srcY, int srcWidth, int srcHeight,
				 BlitMode bm /*= BlitMode::BMODE_OPAQUE*/, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0, ULONG dwFlags = 0) const;

	// Revert surface horizontally
	// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
	BOOL ReverseX();

	// Revert rectangle horizontally
	// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
	BOOL ReverseX(int x, int y, int width, int height);

	// Revert surface vertically
	// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
	BOOL ReverseY();

	// Revert rectangle vertically
	// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
	BOOL ReverseY(int x, int y, int width, int height);

	// Remove empty borders
	BOOL GetMinimizeRect(RECT *);
	BOOL Minimize();
	BOOL Minimize(RECT * r);

	// =============================
	// Blit from screen or memory DC
	// =============================

	static BOOL CaptureDC(HDC srcDC, HDC dstDC, LONG srcX, LONG srcY,
						  LONG dstX, LONG dstY, LONG srcWidth, LONG srcHeight,
						  LONG dstWidth, LONG dstHeight, BOOL bFlushMessage, BOOL bKeepRatio);

	// ======================
	// Fill
	// ======================

	// Fill surface
	BOOL Fill(COLORREF c);
	BOOL Fill(CFillData * fd);
	BOOL Fill(int index);
	BOOL Fill(int R, int G, int B);

	// Fill block
	BOOL Fill(int x, int y, int w, int h, COLORREF c);
	BOOL Fill(int x, int y, int w, int h, CFillData * fd);
	BOOL Fill(int x, int y, int w, int h, int index);
	BOOL Fill(int x, int y, int w, int h, int R, int G, int B);
#ifdef HWABETA
	// TODO: YQ: dwFlags is made from what enum/flags?
	// HWA only
	BOOL Fill(int x, int y, int w, int h, COLORREF* pColors, ULONG dwFlags);

	// Fill with blit mode and RGBA coef
	// TODO: This may be CF2.5+ only, or D3D11 surfaces only
	BOOL fxFill(int x, int y, int w, int h, COLORREF c, BlitMode bm, COLORREF rgbaCoef);
	BOOL fxFill(int x, int y, int w, int h, COLORREF* pColors, ULONG dwFlags, BlitMode bm, COLORREF rgbaCoef);
#endif

	// ======================
	// Geometric Primitives
	// ======================

	// 1. Simple routines : call GDI with Surface DC
	// =============================================

	BOOL Ellipse(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

	BOOL Ellipse(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/,
				 COLORREF crOutl /* = BLACK */, BOOL Fill = TRUE);

	BOOL Rectangle(int left, int top, int right, int bottom, int thickness = 1, COLORREF crOutl = 0);

	BOOL Rectangle(int left, int top, int right, int bottom, COLORREF crFill, int thickness /*= 0*/,
				   COLORREF crOutl /* = 0 */, BOOL bFill = TRUE);

	BOOL Polygon(LPPOINT pts, int nPts, int thickness = 1, COLORREF crOutl = 0);

	BOOL Polygon(LPPOINT pts, int nPts, COLORREF crFill, int thickness = 0,
				 COLORREF crOutl = 0, BOOL bFill = TRUE);

	BOOL Line(int x1, int y1, int x2, int y2, int thickness = 1, COLORREF crOutl = 0);

	// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
	// ==============================================================================

	BOOL Ellipse(int left, int top, int right, int bottom, int thickness, CFillData * fdOutl,
				 BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY,
				 LPARAM param = 0);

	BOOL Ellipse(int left, int top, int right, int bottom, CFillData * fdFill,
				 BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY,
				 LPARAM param = 0);

	BOOL Ellipse(int left, int top, int right, int bottom, CFillData * fdFill, int thickness, CFillData * fdOutl,
				 BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY,
				 LPARAM param = 0, BOOL Fill = TRUE);

	BOOL Rectangle(int left, int top, int right, int bottom, int thickness, CFillData * fdOutl, BOOL AntiA = FALSE,
				   BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	BOOL Rectangle(int left, int top, int right, int bottom, CFillData * fdFill,
				   BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	BOOL Rectangle(int left, int top, int right, int bottom, CFillData * fdFill, int thickness, CFillData * fdOutl,
				   BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);

	BOOL Polygon(LPPOINT pts, int nPts, int thickness, CFillData * fdOutl, BOOL AntiA = FALSE,
				 BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	BOOL Polygon(LPPOINT pts, int nPts, CFillData * fdFill, BOOL AntiA = FALSE,
				 BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	BOOL Polygon(LPPOINT pts, int nPts, CFillData * fdFill, int thickness, CFillData * fdOutl,
				 BOOL AntiA = FALSE, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0, BOOL Fill = TRUE);

	// Note: has no effect in HWA, if thickness is greater than 1.
	BOOL  Line(int x1, int y1, int x2, int y2, int thickness, CFillData * fdOutl, BOOL AntiA,
			   BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	// Filled Primitives

	BOOL FloodFill(int x, int y, int & left, int & top, int & right, int & bottom, COLORREF crFill, BOOL AntiA = FALSE,
				   int tol = 0, BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	BOOL FloodFill(int x, int y, COLORREF crFill, BOOL AntiA = FALSE,  int tol = 0,
				   BlitMode bm = BlitMode::BMODE_OPAQUE, BlitOp bo = BlitOp::BOP_COPY, LPARAM param = 0);

	// ======================
	// Rotation
	// ======================
	// Rotate surface in radians
	BOOL Rotate(cSurface & dest, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// radian
	BOOL Rotate(cSurface & dest, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);		// degree
	BOOL Rotate90(cSurface & dest, BOOL b270);

	// Create rotated surface
	BOOL CreateRotatedSurface(cSurface & ps, double a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);
	BOOL CreateRotatedSurface(cSurface & ps, int a, BOOL bAA, COLORREF clrFill = 0L, BOOL bTransp=TRUE);

#ifdef HWABETA
	static void GetSizeOfRotatedRect(int *pWidth, int *pHeight, float angle);
#else
	static void GetSizeOfRotatedRect(int *pWidth, int *pHeight, int angle);
#endif

	// ======================
	// Text
	// ======================

	// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
	int TextOutA(LPCSTR text, ULONG dwCharCount,int x,int y,ULONG alignMode,
				 LPRECT pClipRc, COLORREF color = 0, HFONT hFnt =(HFONT)NULL,
				 BlitMode bm = BlitMode::BMODE_TRANSP, BlitOp bop = BlitOp::BOP_COPY,
				 LPARAM param = 0, int AntiA = 0);
	// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
	int TextOutW(const UShortWCHAR * text, ULONG dwCharCount,int x,int y,ULONG alignMode,
				 LPRECT pClipRc, COLORREF color = 0, HFONT hFnt =(HFONT)NULL,
				 BlitMode bm = BlitMode::BMODE_TRANSP, BlitOp bop = BlitOp::BOP_COPY,
				 LPARAM param = 0, int AntiA = 0);

#ifdef _UNICODE
#define TextOut TextOutW
#else
#define TextOut TextOutA
#endif
	// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
	int DrawTextA(LPCSTR text, ULONG dwCharCount,LPRECT pRc, ULONG dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
				  BlitMode bm=BlitMode::BMODE_TRANSP, BlitOp bo=BlitOp::BOP_COPY, LPARAM param=0, int AntiA=0,ULONG dwLeftMargin=0,ULONG dwRightMargin=0,ULONG dwTabSize=8);
	// See the _FUSIONT macro for filename! If failing, check your surface type has a DC.
	int DrawTextW(const UShortWCHAR * text, ULONG dwCharCount,LPRECT pRc, ULONG dtflags, COLORREF color=0, HFONT hFnt=(HFONT)NULL,
				  BlitMode bm=BlitMode::BMODE_TRANSP, BlitOp bo=BlitOp::BOP_COPY, LPARAM param=0, int AntiA=0,ULONG dwLeftMargin=0,ULONG dwRightMargin=0,ULONG dwTabSize=8);

	// ======================
	// Color / Palette functions
	// ======================
	// Is transparent
	BOOL IsTransparent();

	// Replace color
	BOOL ReplaceColor(COLORREF newColor, COLORREF oldColor);

	// Test collision fine entre deux surfaces
	BOOL IsColliding(cSurface & dest, int xDest, int yDest, int xSrc, int ySrc, int rcWidth, int rcHeight);

	// Create icon
	HICON CreateIcon(int iconWidth, int iconHeight, COLORREF transpColor, POINT *pHotSpot);

	// Palette support
	BOOL Indexed();

	BOOL SetPalette(LPLOGPALETTE palette, SetPaletteAction action = SetPaletteAction::None);
	BOOL SetPalette(csPalette * pCsPal, SetPaletteAction action = SetPaletteAction::None);
	BOOL SetPalette(cSurface & src, SetPaletteAction action = SetPaletteAction::None);
	BOOL SetPalette(HPALETTE palette, SetPaletteAction action = SetPaletteAction::None);

	void Remap(cSurface & src);
	void Remap(LPBYTE remapTable);

	csPalette * GetPalette();
	UINT		GetPaletteEntries(LPPALETTEENTRY paletteEntry, int index, int nbColors);

	int 		GetNearestColorIndex(COLORREF rgb);
	COLORREF	GetRGB(int index);
	int 		GetOpaqueBlackIndex();

	// ======================
	// Full screen
	// ======================
	void EnumScreenModes(LPENUMSCREENMODESPROC pProc, LPVOID lParam);
	BOOL SetScreenMode(HWND hWnd, int width, int height, int depth);
	void RestoreWindowedMode(HWND hWnd);
	void CopyScreenModeInfo(cSurface* pSrc);

#ifdef HWABETA
	BOOL SetAutoVSync(SetAutoVSyncFlags nAutoVSync);
#endif
	BOOL WaitForVBlank();

	// System colors
	// COLOR_MENUBAR(30) seems to be the highest sys color index, so we're using a uint8
	static COLORREF	GetSysColor(int colorIndex);
	// TODO: YQ: Is this meant to be a callback?
	static void		OnSysColorChange();

	// Transparent color, used if alpha is not present. See HasAlpha(). Defaults to black, RGB(0, 0, 0).
	void 		SetTransparentColor(COLORREF rgb);
	COLORREF	GetTransparentColor();
	int 		GetTransparentColorIndex();

	// Alpha channel
	BOOL		HasAlpha();
	LPBYTE		LockAlpha();
	void		UnlockAlpha();

	// Gets image stride for alpha channel. See GetPitch() for a related info.
	// Alpha is always 8bpp.
	int			GetAlphaPitch();
	// Warning: makes a completely transparent alpha channel by default
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
	void 	OnLostDevice();
	void 	AddLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
	void 	RemoveLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);

	// CF2.5+'s Direct3D11 surfaces: Set premultiplied alpha flag
	void 	SetPremultipliedAlpha(BOOL usePMAlpha, BOOL pmedTextures);
	void 	PremultiplyAlpha();		// Only for surfaces with a lockable buffer

	// CF2.5+'s Direct3D11 surfaces only.
	// Only for surfaces with a lockable buffer.
	// TODO: Which surface types have a lockable buffer?
	void 	DemultiplyAlpha();

#endif

	// Friend functions
	// ----------------

	// Useless function for reading m_actual
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
	BOOL LoadPicture(CInputFile * pFile, ULONG bitmapSize, LIFlags loadFlags);
	BOOL LoadDIB(LPBITMAPINFO pBmi, LPBYTE pBits, LIFlags loadFlags);

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
	int m_error;		// Est-ce qu'on doit toujours garder ça?
};

// maximum opacity
// TODO: YQ: Is this applicable in HWA too? Why is 100 commented out?
#define OP_MAX 128			// 100

extern "C" FusionAPIImport BOOL	WINAPI BuildRemapTable	(LPBYTE, LOGPALETTE *, LOGPALETTE *, WORD);

// Used by EditorDisplay of most Fusion exts
FusionAPIImport cSurface* FusionAPI WinGetSurface(int idWin, int surfID = 0);

// Required struct name due to Cnpdll.h GetImageInfos() func
// Corresponds to image bank entry?
struct Img final
{
	// Img struct flags; bitmask
	enum class Flags : BYTE {
		// For bit ops
		None	= 0,
		// Lossless image compression using Run-Length Encoding
		// Replaces repeating bytes with a single value + repeat count
		RLE		= 0b1,
		// Lossless image compression using Run-Length Encoding, word-aligned
		// Replaces repeating words (16-bit values) with a single value + repeat count
		RLEW	= 0b10,
		// Lossless image compression using Run-Length Encoding, transposed
		// Image is rotated 90 and repeating bytes? words? are replaced with a single value + repeat count
		RLET	= 0b100,
		// CF2.5+ only: Lossless image compression using Lempel-Ziv Extended
		// CF2.5+ always compresses all images
		LZX		= 0b1000,
		// Image has alpha channel; if not set, image uses transparent color
		Alpha	= 0b10000,
		// ?
		ACE		= 0b100000,
		// ?
		Mac		= 0b1000000,
		// CF2.5+ only: image is stored as RGBA, not RGBX or RGB + A
		// See ImageBankFormat::Normal_32bpp_8bpc
		RGBA	= 0b10000000
	};

	// Checksum algorithm; not sure what is used or what it's run on
	DWORD		imgCheckSum;
	// Probably ref count
	DWORD		imgCount;
	// Byte count, including both color and alpha, not including this header
	DWORD		imgSize;
	// Why is this signed?
	short		imgWidth;
	// Why is this signed?
	short		imgHeight;
	// Format, may affect flags
	BYTE		imgFormat;
	// Flags, including alpha and compression
	Flags		imgFlags;
	WORD		imgNotUsed;
	// XY coord of this image hot spot (display offset relative to object's hoX/hoY)
	short		imgXSpot, imgYSpot;
	// X coord of this image hot spot (display offset relative to object's hoX/hoY)
	short		imgXAction;
	short		imgYAction;
	// Image transparent color (ignored if imgFlags say it's alpha)
	COLORREF	imgTrspColor;
};
enum_class_is_a_bitmask(Img::Flags);

extern "C" {
	/** Returns number of elements in the Fusion bank. Returns 0+.
	 * @param appli mv->mvAppli or rhPtr->rhAppli.
	 * @param bkNum Bank number: see BK_XX enum.
	 * @return Number of elements in bank, 0+. */
	FusionAPIImport int FusionAPI Bank_GetEltCount(void * appli, UINT bkNum);

	/** Returns the image ID's info to the given struct.
	 * @param appli mv->mvAppli or rhPtr->rhAppli.
	 * @param imgID Image ID number (1-65535).
	 * @param imgPtr Output image info struct.
	 * @return 1 if successful. 0 if failed. */
	FusionAPIImport int FusionAPI GetImageInfos(void* appli, DWORD imgID, Img* imgPtr);
}
