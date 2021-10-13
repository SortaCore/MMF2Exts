#pragma once
#include "WindowsDefines.hpp"
#include "Surface.hpp"

// cSurface class
class SurfaceSafe : private cSurface
{
public:
		// ======================
		// Creation / Destruction
		// ======================

		// TODO: YQ: Prefered over NewSurface()?
		// TODO: YQ: Are copy and move constructors defined?
		SurfaceSafe();
		~SurfaceSafe();

		// TODO: YQ: Init what, exactly?
		static void InitializeSurfaces();
		static void FreeSurfaces();
		#if HWABETA
		// TODO: YQ: What is being freed?
		// HWA only
		static void FreeExternalModules();
		#endif

		// Operators

		SurfaceSafe & operator= (const SurfaceSafe & source);

		// Create surface implementation from surface prototype
		void Create (size_t width, size_t height, cSurface * prototype);

		// Create pure DC surface from DC
		void Create (HDC hDC);

		// Create pure DC surface from a window
		void Create (HWND hWnd, bool IncludeFrame);

		// TODO: YQ: What is a screen surface? What does creating one do?
		// Create screen surface (fullscreen mode)
		bool CreateScreenSurface();
		bool IsScreenSurface();

		// TODO: YQ: What makes a surface valid or invalid?
		// Valid?
		bool IsValid();

		/// <summary> Returns the surface type of this Surface. </summary>
		SurfaceType GetType();
		/// <summary> Returns the driver type of this Surface.</summary>
		SurfaceDriver GetDriver();
		// TODO: YQ: What is driverinfo? What should be passed to pInfo? Is it DD_GETDRIVERINFODATA?
		ULONG GetDriverInfo(void * pInfo);

		// Clone surface (= create with same size + Blit)
		void Clone(const cSurface & pSrcSurface, size_t newW = SIZE_MAX, size_t newH = SIZE_MAX);

		// TODO: YQ: What is "before to create"? Reusing the cSurface *?
		// YQ: Does this delete this cSurface? Is it suitable for use with Clone() only? just Create()?
		// Delete surface implementation (before to create another one)
		void Delete();

		// ======================
		// error codes
		// ======================
		// This has never returned an error in scenarios where nothing is drawn (e.g. drawing a line 2px in D3D 11)
		int GetLastError();

		// ======================
	    // Surface info
		// ======================
		// TODO: Width in pixels? What if invalid?
		size_t GetWidth() const;
		// TODO: Height in pixels? What if invalid?
		size_t GetHeight() const;

		// Number of bits per pixel. 8, 15, 16, 24 or 32 (BGRA).
		// 8 bits = 1 byte index in the color table
		// TODO: color table = pallete?
		// 15 = 16 bit per pixel; 5 bits per component, 1 bit ignored. Red mask = 0x7C00, green mask = 0x03E0, blue mask = 0x001F.
		// 16 = 16 bit per pixel; 5 bits for R/B, 6 for G. Red mask = 0xF800, green mask = 0x07E0, blue mask = 0x001F.
		// 24 = 8 bits per component, BGR layout.
		// 32 = 8 bits per component, BGRA layout; A is normally unused, but can be alpha.
		size_t GetDepth() const;

		bool GetInfo(size_t & width, size_t & height, size_t & depth) const;

		// ======================
	    // Surface coordinate management
		// ======================

		// TODO: YQ: What does Origin mean? Something relative to Hot Spot? Relative for blit operations?
		void SetOrigin(size_t x, size_t y);
		void SetOrigin(POINT c);

		void GetOrigin(POINT &pt);
		void GetOrigin(size_t & x, size_t & y);

		void OffsetOrigin(size_t dx, size_t dy);
		void OffsetOrigin(POINT delta);

		// ======================
	    // Raster operations
		// ======================

		// Locks the raw memory of the Surface, and returns the address to it. When you're done writing, use UnlockBuffer to return it.
		// TODO: YQ: Does this work for all surface types?
		// Cannot call when DC is active via GetDC()
		std::byte *	LockBuffer();

		// TODO: YQ: Why do you have to unlock with address, won't OOP make that pointless? Can you pass a different address?
		void UnlockBuffer(std::byte * spBuffer);
		// Width of each line of pixels in bytes
		size_t GetPitch() const;

		// ======================
		// Double-buffer handling
		// ======================

		// Sets this cSurface as the current one in a double-buffer... in what context? How are surfaces paired, and how do we know what the pairs target?
		void SetCurrentDevice();
		// TODO: YQ: Rendering to GetRenderTargetSurface()? Where and why do we need to call BeginRendering()?
		int BeginRendering(bool bClear, RGBAREF dwRgba);
		// TODO: YQ: Rendering to GetRenderTargetSurface()?
		int EndRendering();
		// TODO: YQ: update how?
		bool UpdateScreen();
		#ifdef HWABETA
			// TODO: YQ: update how?
			cSurface* GetRenderTargetSurface();
			void ReleaseRenderTargetSurface(cSurface* psf);
			// TODO: YQ: What does this do, how does Max affect it ?
			void Flush(bool bMax);
			// TODO: YQ: I would guess this is the Z position of the buffer, but a float makes no sense here.
			void SetZBuffer(float z2D);
		#endif

		// ======================
	    // Device context for graphic operations
		// ======================
		// Cannot call when buffer is locked via LockBuffer()
		HDC GetDC();
		void ReleaseDC(HDC dc);
		void AttachWindow(HWND hWnd);

		// ======================
	    // Clipping
		// ======================

		// TODO: YQ: WHat does clipping apply to? Blit?
		void GetClipRect(size_t & x, size_t & y, size_t & w, size_t & h);
		void SetClipRect(size_t x, size_t y, size_t w, size_t h);
		void ClearClipRect(void);

		// ======================
		// LoadImage (DIB format) / SaveImage (DIB format)
		// ======================
		#undef LoadImage

		bool LoadImage(HFILE fileHandle, ULONG lsize, LoadImageFlags loadFlags = LoadImageFlags::None);
		bool LoadImage(LPCTSTR filename, LoadImageFlags loadFlags = LoadImageFlags::None);
		bool LoadImage(HINSTANCE hInst, int bmpID, LoadImageFlags loadFlags = LoadImageFlags::None);
		bool LoadImage(LPBITMAPINFO pBmi, std::byte * pBits = nullptr, LoadImageFlags loadFlags = LoadImageFlags::None);
		bool SaveImage(HFILE fileHandle, SaveImageFlags saveFlags = SaveImageFlags::None);
		bool SaveImage(LPCTSTR filename, SaveImageFlags saveFlags = SaveImageFlags::None);
		bool SaveImage(LPBITMAPINFO pBmi, std::byte * pBits, SaveImageFlags saveFlags = SaveImageFlags::None);

		// TODO: YQ: WHat does this mean?
		ULONG GetDIBSize();

		// ======================
		// Pixel functions
		// ======================
		// Set pixel
		void SetPixel(size_t x, size_t y, COLORREF color);
		void SetPixel(size_t x, size_t y, std::uint8_t r, std::uint8_t g, std::uint8_t b);
		void SetPixel(size_t x, size_t y, std::uint8_t colorPaletteIndex);

		// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
		void SetPixelFast(size_t x, size_t y, COLORREF color);
		void SetPixelFast8(size_t x, size_t y, std::uint8_t colorPaletteIndex);

		// Get pixel
		bool GetPixel(size_t x, size_t y, COLORREF & color) const;
		bool GetPixel(size_t x, size_t y, std::uint8_t & r, std::uint8_t & g, std::uint8_t & b) const;
		bool GetPixel(size_t x, size_t y, std::uint8_t & colorPaletteIndex) const;

		// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
		COLORREF GetPixelFast(size_t x, size_t y);
		std::uint8_t GetPixelFast8(size_t x, size_t y);

		// ======================
		// Blit functions
		// ======================
		// Blit surface to surface
		bool Blit(cSurface & dest) const;

		bool Blit(cSurface & dest, size_t destX, size_t destY,
			BlitMode blitMode = BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM blitOpParam = 0,
			BlitOptions blitFlags = BlitOptions::None) const;

		// Blit rectangle to surface
		bool Blit(cSurface & dest, size_t destX, size_t destY,
			size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM blitOpParam = 0,
			BlitOptions blitFlags = BlitOptions::None) const;

		// Extended blit : can do stretch & rotate at the same time
		// Only implemented in 3D mode
#ifdef HWABETA
		bool BlitEx(cSurface & dest, float destX, float destY, float scaleX, float scaleY,
			size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight, LPPOINT pCenter, float angle,
			BlitMode blitModee = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy , LPARAM blitOpParam = 0,
			BlitOptions blitFlags = BlitOptions::None) const;
#endif
		// Scrolling
		bool Scroll(size_t destX, size_t destY, size_t srcX, size_t srcY, size_t width, size_t height);

		// Blit via callback
		bool FilterBlit(cSurface & dest, size_t destX, size_t destY,
			size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
			BlitMode bm, FILTERBLITPROC fbProc, LPARAM lUserParam) const;

		bool FilterBlit(cSurface & dest, FILTERBLITPROC fbProc,
			LPARAM lUserParam, BlitMode blitMode = BlitMode::BMODE_OPAQUE) const;

		// Matrix blit via callback
		bool MatrixFilterBlit(cSurface & dest, size_t destX, size_t destY,
			size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
			size_t mWidth, size_t mHeight, size_t mDXCenter, size_t mDYCenter,
			MATRIXFILTERBLITPROC fbProc, LPARAM lUserParam) const;

		// Stretch surface to surface
		bool Stretch(cSurface & dest, StretchFlags dwFlags = StretchFlags::None) const;

		// Stretch surface to rectangle
		bool Stretch(cSurface & dest, size_t destX, size_t destY, size_t destWidth, size_t destHeight,
			BlitMode bm /*= BMODE_OPAQUE*/, BlitOperation bo = BlitOperation::Copy, std::uint8_t inkEffectParam = 0, StretchFlags dwFlags = StretchFlags::None) const;

		// Stretch rectangle to rectangle
		bool Stretch(cSurface & dest, size_t destX, size_t destY, size_t destWidth, size_t destHeight,
			size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
			BlitMode blitMode /*= BMODE_OPAQUE*/, BlitOperation blitOp = BlitOperation::Copy, std::uint8_t inkEffectParam = 0, StretchFlags dwFlags = StretchFlags::None) const;

		// Revert surface horizontally
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		bool ReverseX();

		// Revert rectangle horizontally
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		bool ReverseX(size_t x, size_t y, size_t width, size_t height);

		// Revert surface vertically
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		bool ReverseY();

		// Revert rectangle vertically
		// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
		bool ReverseY(size_t x, size_t y, size_t width, size_t height);

		// Remove empty borders
		bool GetMinimizeRect(RECT *);
		bool Minimize();
		bool Minimize(RECT * r);

		// =============================
		// Blit from screen or memory DC to another DC
		// =============================

		static bool CaptureDC(HDC srcDC,HDC dstDC,LONG srcX,LONG srcY,LONG dstX,LONG dstY,LONG srcWidth,LONG srcHeight,LONG dstWidth,LONG dstHeight,bool bFlushMessage,bool bKeepRatio);

		// ======================
		// Fill
		// ======================

		// Fill surface
		bool Fill(COLORREF color);
		bool Fill(CFillData * fd);
		bool Fill(std::uint8_t colorPaletteIndex);
		bool Fill(std::uint8_t r, std::uint8_t g, std::uint8_t b);

		// Fill block
		bool Fill(size_t x, size_t y, size_t width, size_t height, COLORREF color);
		bool Fill(size_t x, size_t y, size_t width, size_t height, CFillData * fillData);
		bool Fill(size_t x, size_t y, size_t width, size_t height, std::uint8_t colorPaletteIndex);
		bool Fill(size_t x, size_t y, size_t width, size_t height, std::uint8_t r, std::uint8_t g, std::uint8_t b);
#ifdef HWABETA
		// TODO: YQ: dwFlags is made from what enum/flags?
		// HWA only
		bool Fill(size_t x, size_t y, size_t width, size_t height, COLORREF * pColors, ULONG dwFlags);
#endif

		// ======================
		// Geometric Primitives
		// ======================

		// 1. Simple routines : call GDI with Surface DC
		// =============================================

		bool Ellipse(int left, int top, int right, int bottom, size_t thickness = 1, COLORREF outlineColor = 0);

		bool Ellipse(int left, int top, int right, int bottom, COLORREF fillColor, size_t thickness /*= 0*/,
			COLORREF crOutl /*= BLACK*/, bool Fill = true);

		bool Rectangle(int left, int top, int right, int bottom, size_t thickness = 1, COLORREF outlineColor = 0);

		bool Rectangle(int left, int top, int right, int bottom, COLORREF fillColor, size_t thickness /*= 0*/,
			COLORREF crOutl /*= 0*/, bool fill = true);

		bool Polygon(POINT * pts, size_t nPts, size_t thickness = 1, COLORREF outlineColor = 0);

		bool Polygon(POINT * pts, size_t nPts, COLORREF fillColor, size_t thickness = 0,
			COLORREF crOutl = 0, bool fill = true);

		bool Line(size_t x1, size_t y1, size_t x2, size_t y2, size_t thickness = 1, COLORREF outlineColor = 0);

		// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
		// ==============================================================================

		bool Ellipse(int left, int top, int right, int bottom, size_t thickness, CFillData * fdOutl, bool antiAliasing = false,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Ellipse(int left, int top, int right, int bottom, CFillData * fdFill,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Ellipse(int left, int top, int right, int bottom, CFillData * fdFill, size_t thickness, CFillData * fdOutl,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0, bool Fill = true);

		bool Rectangle(int left, int top, int right, int bottom, size_t thickness, CFillData * fdFill, bool antiAliasing = false,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Rectangle(int left, int top, int right, int bottom, CFillData * fdFill,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Rectangle(int left, int top, int right, int bottom, CFillData * fdFill, size_t thickness, CFillData * fdOutl,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0, bool fill = true);

		bool Polygon(POINT * pts, size_t nPts, size_t thickness, CFillData * fdOutl, bool antiAliasing = false,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Polygon(POINT * pts, size_t nPts, CFillData * fdFill,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool Polygon(POINT * pts, size_t nPts, CFillData * fdFill, size_t thickness, CFillData * fdOutl,
			bool antiAliasing = false, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0, bool fill = true);

		// Note: has no effect in HWA, if thickness is greater than 1.
		bool Line(size_t x1, size_t y1, size_t x2, size_t y2, size_t thickness, CFillData * fdOutl, bool antiAliasing,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		// Filled Primitives

		bool FloodFill(size_t x, size_t y, int & left, int & top, int & right, int & bottom, COLORREF crFill, bool antiAliasing = false,
			size_t tol = 0, BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		bool FloodFill(size_t x, size_t y, COLORREF crFill, bool antiAliasing = false, size_t tol = 0,
			BlitMode blitMode = BlitMode::BMODE_OPAQUE, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0);

		// ======================
		// Rotation
		// ======================
		bool Rotate(cSurface & dest, double radians, bool antiAliasing, COLORREF clrFill = 0L, bool bTransp = true);
		bool Rotate(cSurface & dest, int degrees, bool antiAliasing, COLORREF clrFill = 0L, bool bTransp = true);
		bool Rotate90(cSurface & dest, bool b270);

		// Create rotated surface - resizes destSurf
		bool CreateRotatedSurface(cSurface & destSurf, double radians, bool antiAliasing, COLORREF clrFill = 0L, bool bTransp = true);
		bool CreateRotatedSurface(cSurface & destSurf, int degrees, bool antiAliasing, COLORREF clrFill = 0L, bool bTransp = true);

		static void GetSizeOfRotatedRect(int *pWidth, int *pHeight, float angle);

		// ======================
		// Text
		// Rather than undef the native TextOut/DrawText, we'll just inherit it. User can always invoke either A/W directly.
		// antiAliasing = DWRITE_RENDERING_MODE?
		// ======================

		int TextOut(std::tstring_view text, size_t x, size_t y, ULONG textAlignMode, RECT * clipRect, COLORREF textColor = 0,
			HFONT fontHandle = (HFONT)NULL, BlitMode blitMode = BlitMode::BMODE_TRANSP, BlitOperation blitOp = BlitOperation::Copy,
			LPARAM param = 0, int antiAliasing = 0);

		int	DrawText(std::tstring_view text, RECT * clipRect, ULONG dtflags, COLORREF color = 0, HFONT hFnt = (HFONT)NULL,
			BlitMode blitMode = BlitMode::BMODE_TRANSP, BlitOperation blitOp = BlitOperation::Copy, LPARAM param = 0,
			int antiAliasing = 0, size_t dwLeftMargin = 0, size_t dwRightMargin = 0, size_t dwTabSize = 8);

		// ======================
		// Color / Palette functions
		// ======================
		// Is transparent
		bool IsTransparent();

		// Replace color
		bool ReplaceColor(COLORREF newColor, COLORREF oldColor);

		// Test collision fine entre deux surfaces
		bool IsColliding(cSurface & dest, size_t xDest, size_t yDest, size_t xSrc, size_t ySrc, int rcWidth, int rcHeight);

		// Create icon
		HICON CreateIcon(size_t iconWidth, size_t iconHeight, COLORREF transpColor, POINT *pHotSpot);

	    // Palette support
		bool Indexed();

		bool SetPalette(LOGPALETTE * palette, SetPaletteAction action = SetPaletteAction::None);
		bool SetPalette(csPalette * pCsPal, SetPaletteAction action = SetPaletteAction::None);
		bool SetPalette(cSurface & src, SetPaletteAction action = SetPaletteAction::None);
		bool SetPalette(HPALETTE palette, SetPaletteAction action = SetPaletteAction::None);

		void Remap(cSurface & src);
		void Remap(std::byte * remapTable);

		csPalette * GetPalette();
		UINT GetPaletteEntries(PALETTEENTRY * paletteEntries, size_t index, size_t nbColors);

		std::uint8_t GetNearestColorIndex(COLORREF rgb);
		COLORREF GetRGB(std::uint8_t index);
		std::uint8_t GetOpaqueBlackIndex(); // 256 only

		// ======================
		// Full screen
		// ======================
		void EnumScreenModes(LPENUMSCREENMODESPROC pProc, LPVOID lParam);
		bool SetScreenMode(HWND hWnd, size_t width, size_t height, size_t depth);
		void RestoreWindowedMode(HWND hWnd);
		void CopyScreenModeInfo(cSurface* pSrc);

#ifdef HWABETA
		// TODO: YQ: Why int, not bool?
		bool SetAutoVSync(int nAutoVSync);
#endif
		bool WaitForVBlank();

		// System colors
		static COLORREF	GetSysColor(std::uint8_t sysColorIndex);
		// TODO: YQ: Is this meant to be a callback?
		static void		OnSysColorChange();

		// Transparent color
		void		 SetTransparentColor(COLORREF rgb);
		COLORREF	 GetTransparentColor();
		std::uint8_t GetTransparentColorIndex();

		// Alpha channel
		bool			HasAlpha();
		std::byte *		LockAlpha();
		void			UnlockAlpha();
		int				GetAlphaPitch();
		void			CreateAlpha();
		void			SetAlpha(std::byte * alphaBytes, int alphaPitch);
		void			AttachAlpha(std::byte * alphaBytes, int alphaPitch);
		std::byte *		DetachAlpha(int * alphaPitch);
		SurfaceSafe *	GetAlphaSurface();
		void			ReleaseAlphaSurface(cSurface* pAlphaSf);

		// Transparent monochrome mask
		// Pass NULL as mask to get size it needs to be
		// Returns size of mask
		ULONG CreateMask(CollisionMask * pMask, CreateCollisionMaskFlags createMaskFlags);

		// Lost device callback
#ifdef HWABETA
		void OnLostDevice();
		void AddLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
		void RemoveLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam);
#endif

		// Friend functions
		// ----------------
		FusionAPIImport friend cSurfaceImplementation * FusionAPI GetSurfaceImplementation(cSurface &cs);
		FusionAPIImport friend void FusionAPI SetSurfaceImplementation(cSurface &cs, cSurfaceImplementation *psi);

	// Private data
	// ------------
	private:
		cSurfaceImplementation * m_actual;
		POINT origin;
		int m_error;

		LPBYTE alphaLock;
};
