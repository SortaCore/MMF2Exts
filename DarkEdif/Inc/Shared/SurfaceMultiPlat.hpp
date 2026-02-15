#pragma once
#define SURFACE_MULTI_PLAT_DEFINED

namespace DarkEdif
{
	struct Size;
	class Surface;
}
#ifdef _WIN32
// If building an ANSI ext, we're expecting to be going for MMF2, so use cSurface 2.0 functions for drawing only.
// This can be overriden by defining FUSION_2_0_DRAWING_ONLY=0.
#if !defined(_UNICODE) && !defined(FUSION_2_0_DRAWING_ONLY)
#define FUSION_2_0_DRAWING_ONLY 1
#endif

// The idea is we avoid the ext dev using cSurface directly.
#define SURFACE_MULTI_PLAT_INCLUDING_WINSURF
#include "Windows/WindowsSurface.hpp"
#endif

#include "DarkEdif.hpp"
#undef SURFACE_MULTI_PLAT_INCLUDING_WINSURF
#include <functional>
#ifndef MULTIPLAT_FONT
// This header is expected for darkedif multiplat surface, if missing NOPROPS probably messed up
#error missing multiplat font header
#endif

#ifdef __APPLE__
#import "MMF2Lib/CRenderToTexture.h"
#endif

struct ForbiddenInternals;

namespace DarkEdif
{
	class Surface;
	struct Size;
	struct Point;
	// Represents a rectangle of coordinates on the screen, based on RECT struct in Windows,
	// used on all platforms by Fusion.
	struct Rect
	{
		std::int32_t left = 0;
		std::int32_t top = 0;
		std::int32_t right = 0;
		std::int32_t bottom = 0;
		Rect() = default;
		// Creates rectangle from (0,0) to the bottom right (x,y) passed. x and y must be positive.
		Rect(const std::int32_t right, const std::int32_t bottom);
		Rect(const Size& s);
		/* Creates rectangle from (x,y) to the bottom right (x + w, y + h) passed.
		 * x and y must be positive unless last param is true.
		 * @param allowNeg If true, allows x and y to be negative. */
		Rect(const std::int32_t left, const std::int32_t top,
			const std::int32_t right, const std::int32_t bottom,
			const bool allowNeg = false);
		/* Creates rectangle from (x1,y1) to the bottom right (x2, y2) passed.
		 * min(x) and min(y) must be positive unless last param is true.
		 * Either point can be top-left.
		 * @param allowNeg If true, allows x and y to be negative. */
		Rect(const Point & p1, const Point& p2,
			const bool allowNeg = false);

		/* Creates rectangle from (x,y) to the bottom right (x + w, y + h) passed.
		 * x and y must be positive unless last param is true.
		 * @param allowNeg If true, allows x and y to be negative. */
		Rect(const Point &p, const Size& s, bool allowNeg = false);

		// Gets width, 0+
		std::int32_t Width() const;
		// Gets height, 0+
		std::int32_t Height() const;
		Size GetSize() const;

		// Gets the center point of this Rect, veering top-left
		Point GetCenter() const;
		// Gets the top-left point of this Rect
		Point GetTopLeft() const;
		// Gets the bottom-right point of this Rect
		Point GetBottomRight() const;

		// Gets string representation, in form of "(x, y) to (x, y)"
		std::tstring ToCoordString() const;
		// Gets string representation, in form of "(x, y) size (width x height)"
		std::tstring ToAreaString() const;

		// Swaps internal sides if they are reversed (e.g. bottom higher than top).
		// Return true if rectangle was already correct.
		bool Normalize();

		// If internal sides are swapped, kills the entire app
		void Validate() const;
	};
	struct Point
	{
		std::int32_t x = 0;
		std::int32_t y = 0;
		Point() = default;
		/* Creates a Point from (x,y), expecting positive coords.
		 * @param allowNeg If true, allows x/y to be negative */
		Point(const std::int32_t x, const std::int32_t y, const bool allowNeg = false);
		// Convert to Size representing x by y
		explicit operator Size () const;
		// Convert to string representation "(x, y)"
		operator std::tstring () const;
		// Convert to string representation "(x, y)"
		std::tstring str() const;
	};
	struct PointF
	{
		float x = 0.f;
		float y = 0.f;
		PointF() = default;
		/* Creates a Point from (x,y), expecting positive coords.
		 * @param allowNeg If true, allows x/y to be negative */
		PointF(const float x, const float y, const bool allowNeg = false);
		// Convert to string representation "width x height"
		operator std::tstring () const;
		// Convert to string representation "width x height"
		std::tstring str() const;
		PointF(Point p);
	};
	struct Size {
		std::int32_t width = 0;
		std::int32_t height = 0;
		Size() = default;
		// Convert to Point representing (width, height)
		explicit operator Point () const;
		// Convert to string representation "width x height"
		operator std::tstring () const;
		// Convert to string representation "width x height"
		std::tstring str() const;
	};

	// Collision mask, based on sMask
	struct CollisionMask
	{
		std::int32_t	get_size() const;
		std::int32_t	get_width() const;
		std::int32_t	get_height() const;
		std::uint32_t	get_widthBytes() const;
		std::int32_t	get_xSpot() const;
		std::int32_t	get_ySpot() const;
		std::uint32_t	get_flags() const;
		// Gets a rectangle underlying the mask
		// To change size, you must recreate the mask
		const Rect &	get_rect() const;
		// Gets raw bits of the collision mask
		std::uint16_t *	get_data();

		static std::unique_ptr<DarkEdif::CollisionMask> CreateWithSize(std::size_t);
		~CollisionMask();
		// Do not call this
		CollisionMask();
	protected:
		struct sMaskMP {
			NO_DEFAULT_CTORS_OR_DTORS(sMaskMP);
			// size of this struct
			// TODO:  from size, or from data?
			std::int32_t	size;
			// width in pixels
			std::int32_t	width;
			// height in pixels
			std::int32_t	height;
			// Line width - width rounded up to nearest multiple of 16
			std::uint32_t	widthBytes;
			// x of hotspot
			std::int32_t	xSpot;
			// y of hotspot
			std::int32_t	ySpot;
			// Flags in the mask
			std::uint32_t	flags;
			// Gets a rectangle position in the mask
			Rect	rect;
			// Although it makes more sense to be int8, iOS runtime seems to be coded otherwise,
			// perhaps a leftover from 16-bit Windows.
			// The data allocated is (lineWidth * height) + 1, zero-inited
			std::uint16_t	data[];
		} * intern = nullptr;
		friend DarkEdif::Surface;
		friend ForbiddenInternals;
		// For internal use
		void* GetNativePointer();
#ifdef __ANDROID__
		global<jclass> javaClass;
		global<jobject> javaObj;
		static jmethodID cmaskCtorMeth;
		static jfieldID ptrField;
#endif
	};

	template<typename T, typename C>
	class Holder
	{
		static_assert(std::is_pointer_v<T>, "T must be pointer");

		C* clazz;
		T ptr;
		void (* deleter)(C*, T);
	public:
		const T & operator *() const {
			return ptr;
		}
		Holder(C* clazz, T ptr, void (deleter)(C*,T)) :
			clazz(clazz), ptr(ptr), deleter(deleter) { }
		Holder(Holder&&) = delete;
		Holder(Holder&) = delete;
		Holder(const Holder&&) = delete;
		Holder(const Holder&) = delete;
		~Holder() {
			deleter(clazz, ptr);
			ptr = nullptr;
		}
	};
	struct SurfaceFill {
		enum class FillType : std::uint16_t {
			None, //  FILLTYPE_NONE
			Flat, // FILLTYPE_SOLID
			Gradient, // FILLTYPE_GRADIENT
			Mosaic, // FILLTYPE_MOTIF (sometimes misspelt Mosiac)
		} fillType;
		union {
			// Only for lines in non filled mode
			struct {
				std::uint32_t	lineFlags;
			} line;
			// Solid
			struct {
				std::uint32_t	color;
			} solid;
			// Gradient
			struct {
				std::uint32_t	color1;
				std::uint32_t	color2;
				std::uint32_t	gradientFlags;
			} gradient;
			// Image ID from image bank
			struct {
				Surface * surf;
				Rect rect;
			} image;
		};
		enum class LineFlags : std::uint32_t {
			None,
			InvX,
			InvY,
			InvXAndY
		};
		static SurfaceFill None(LineFlags lineFlags = LineFlags::None);
		// Creates a solid opaque color fill
		static SurfaceFill Solid(std::uint32_t color);
		// Generates a fill of vertical or horizontal linear gradient fade from color 1 to 2
		static SurfaceFill Gradient(std::uint32_t color1, std::uint32_t color2, bool vertical);
		// Creates a repeating image from the passed area on the passed surface
		static SurfaceFill Mosaic(Surface * surf, Rect rect);
	protected:
		friend Surface;
		SurfaceFill(FillType);
	};
	struct FilledShape {
		std::uint16_t borderSize;	// Border
		std::uint32_t borderColor;
		std::uint16_t shape;		// Shape
		SurfaceFill fillData;		// Fill Type infos
	};

	constexpr inline std::uint32_t ColorRGB(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) {
		return (0xFF << 24U) | (b << 16U) | (g << 8U) | r;
	}
	constexpr inline std::uint32_t GetRGB(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) {
		return (0xFF << 24U) | (b << 16U) | (g << 8U) | r;
	};
	constexpr inline std::uint32_t GetARGB(const std::uint8_t a, const std::uint8_t r, const std::uint8_t g, std::uint8_t b) {
		return a << 24U | b << 16U | g << 8U | r;
	};

	inline SurfaceFill FillNone(const SurfaceFill::LineFlags lineFlags = SurfaceFill::LineFlags::None) {
		return SurfaceFill::None(lineFlags);
	}
	inline SurfaceFill FillSolid(const std::uint32_t color) {
		return SurfaceFill::Solid(color);
	}
	inline SurfaceFill FillColor(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b) {
		return SurfaceFill::Solid(ColorRGB(r, g, b));
	}
	inline SurfaceFill FillGradient(std::uint32_t color1, std::uint32_t color2, bool vertical) {
		return SurfaceFill::Gradient(color1, color2, vertical);
	}
	inline SurfaceFill FillMosaic(Surface* surf, Rect rect) {
		return SurfaceFill::Mosaic(surf, rect);
	}

	enum class BlitOptions {
		None = 0x0000,
		// Blit with anti-aliasing
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
	enum_class_is_a_bitmask(BlitOptions);

	class Surface
	{
		static void UnlockMain(Surface*, std::uint8_t*);
		static void UnlockAlpha(Surface*, std::uint8_t*);
		void IDGen();
		std::size_t Internal_GetSizeOfFile(FILE* fil, const TCHAR* op) const;
		std::size_t Internal_CreateMask(void* mask, std::uint32_t flags);
		std::size_t collisionMaskByteCount = 0;

		// This will be null for the Fusion frame editor window in Windows - otherwise, non-null
		// So it will be null for frame editor window itself, ext icon, etc.
		RunHeader* rhPtr = nullptr;
	public:
		friend ForbiddenInternals;
		CollisionMask * GetCollisionMask(std::uint32_t flags);
		std::unique_ptr<CollisionMask> platMask, obstacleMask;

#ifdef _WIN32
		// internal use only!
		Surface(RunHeader* rhPtr, cSurface* from, bool isFrameSurface = false);
		void Internal_WinZoneHack();
#endif
		// Blits to frame manually, with effects
		void BlitToFrameWithExtEffects(Point offset = Point{});

		// If ext display, what ext is it?
		Extension* ext = nullptr;
		// If true, reads ext to find the draw zone, and update in software rendering
		bool isExtDisplay = false, isFrameSurface = false, ownsSurface = false;
	#ifdef _WIN32
		cSurface* surf = nullptr;
	#elif __ANDROID__
		// CTextSurface, the only surface in Android
		// @remarks CTextSurface mostly delegates to CImage which is native,
		// and also an abstract Java class, so we can't create a CImage
		global<jobject> textSurface;
		global<jclass> textSurfaceClass;
		// Actual pixel storage. Note there is a HWA type.
		// Stored within textSurface.
		global<jobject> bmp;
		global<jclass> bmpClass;
	DarkEdifInternalAccessProtected:
		static jmethodID textSurfCtorMethod, textSurfUpdateTextureMethod, textSurfDrawMethod;
		static jfieldID textSurfBitmapFieldID, textSurfTextureFieldID;
		static jmethodID bitmapEraseMethod, bitmapGetWidthMethod, bitmapGetHeightMethod;
		static global<jobject> renderer;
		static bool isOpenGL2Or3;
		typedef void(* renderScaledRotImageFuncType)(JNIEnv * env, jobject renderer,
			// CImage inherits from CTexture
			jobject texture,
			jboolean resampling, float angle,
			float scaleX, float scaleY,
			int hotSpotX, int hotspotY,
			int x, int y, int w, int h,
			int shaderIndex, int inkEffect, int inkEffectParam);
		static renderScaledRotImageFuncType renderScaledRotImageFunc;
		// Geometry ops, like drawing shapes
		//global<jobject> canvas;
		//global<jclass> canvasClass;
		// android/graphics/Paint, used for modifying how something is drawn
		//global<jobject> paint;
		//global<jclass> paintClass;
	public:
	#else // apple
		// Raw pixels, size and clip rectangle
		CRenderToTexture* bmp = nullptr;
		// Data around an image, can wrap a bmp and collision masks
		//CImage* img;
	#endif
		/** Updates screen display manually. Runtime does not auto-redraw in some display modes.
		 * @param zone If NULL or not specified, the whole surface. */
		void WasAltered(Rect * zone = nullptr);

		// If true, can draw shapes into canvas; implies non-hardware surface
		bool hasGeometryCapacity = false;
		// If true, can draw text into canvas; implies non-hardware surface
		bool hasTextCapacity = false;

		mutable bool isLocked = false, isLockedAlpha = false, dcIsOpen = false;

		bool IsHWA() const { return !hasGeometryCapacity && !hasTextCapacity; }

#ifndef _WIN32
#		define MultiplySign "\xC3\x97"
		// UTF-8 math multiply symbol
		static constexpr std::string_view multSV = "\xC3\x97"sv;
#elif defined(_UNICODE)
#		define MultiplySign L"\u00D7"
		// UTF-16 math multiply symbol
		static constexpr wchar_t multSV = L'\u00D7';
#else
#		define MultiplySign "x"
		// ANSI x letter
		static constexpr char multSV = 'x';
#endif

		enum class PixelFormat
		{
			// Each pixel is 1 bit (1bpp), with rows rounded up to multiple of 8
			// @remarks D3DFMT_A1
			Monochrome = -2,
			// Each pixel is 1 bit (1bpp), with rows rounded up to multiple of 16,
			// suitable for Fusion collision masks
			// @remarks D3DFMT_A8
			Alpha = -1,
			// Format not specified
			Unspecified = 0,
			// Each pixel is 4 bytes, last alpha (OpenGL RGBA_8888)
			RGBA = 1,
			// Each pixel is 4 bytes, with one ignored (OpenGL RGBX_888)
			RGBX = 2,
			// Each pixel is 3 bytes (OpenGL RGB_888) - Android runtime
			// D3DFMT_R8G8B8
			RGB = 3,
			// Each pixel is 3 bytes, blue first - BMP 24bpp format
			BGR = 4,
			// Each pixel is 4 bytes, with first byte ignored - Windows runtime?
			// @remarks see D3DFMT_X8B8G8R8
			XBGR = 5,
			// Each pixel is 4 bytes, alpha first, red last - BMP 32bpp
			// @remarks see D3DFMT_A8B8G8R8
			ABGR = 6,
			// Each pixel is 4 bytes, alpha first, blue last - Android runtime?
			// D3DFMT_A8R8G8B8
			// @remarks I know Android runtime uses RGB byte layout (see CServices),
			//			and A must be first pixel - but whether ARGB or A+RGB...
			ARGB = 7,
		} format;

	DarkEdifInternalAccessProtected:
	#ifdef _WIN32
		// Create pure DC surface from DC
		void CreateFromDC(HDC hDC);
		// Create pure DC surface from a window
		void CreateFromWindow(HWND hWnd, bool IncludeFrame);

		// Returns the surface type of this Surface
		SurfaceType GetType();
		// Returns the driver type of this Surface
		SurfaceDriver GetDriver();
	#elif defined(__ANDROID__)

	#else // apple

	#endif

		// True if the Rect is within the bounds of this Surface
		bool CheckRectContained(const Rect& r) const;
		// True if the Point is within the bounds of this Surface
		bool CheckPointContained(const Point& p) const;

		// Unique ID for a DarkEdif surface
		static int IDCounter;
		TCHAR debugID[40];

	public:
		// Converts a RGBQUAD or COLORREF to text.
		// Assumes byte arrangement of [A,] B, G, R.
		static std::tstring ColorToString(std::uint32_t c, bool alpha = false);

		// Describes what this Surface contains; resolution, colour format,
		// alpha channel, etc.
		std::tstring Describe() const;

		// Copying and moving surfaces is explicitly disallowed, as it's unclear
		// if you want to copy the entire underlying memory, and allows edge cases
		// of copying when a buffer is locked.
		Surface(Surface&) = delete;
		Surface(const Surface&) = delete;
		Surface(Surface&&) = delete;
		Surface(const Surface&&) = delete;
		~Surface();

		bool wasaltered = false;
		bool GetAndResetAltered();

		enum class CloneType
		{
			// Copies only the prototype: driver type, text/bitmap capacities
			DriverTypeOnly,
			// Copies the driver type and internal RGB image, discarding alpha channel if present
			ColorOnly,
			// Copies the alpha channel, ignoring the RGB image
			AlphaOnly,
			// Copies both RGB and alpha channel
			Full
		};
		// @remarks Seems like Android expects only PNG or JPEG, or they misprogrammed CServices.saveImage
		// Android format only saves as PNG or JPEG at 100% quality only
		enum class ImageFileFormat {
			Unset, BMP, TGA, GIF, JPEG, PNG,
		};

		// ======================
		// Creation / Destruction
		// ======================

		/* Creates a surface with optional HWA, text or bitmap functionality, creating an image to back it.
		 * If neither text or bitmap is enabled, will allow the surface to be hardware-backed.
		 *  @param rhPtr			The runtime header, normally in ext->rhPtr variable.
		 *  @param needBitmapFuncs	Enables bitmap functions (such as drawing shapes), making a non-HWA surface
		 *  @param needTextFuncs	Enables text-drawing functions, making a non-HWA surface
		 *	@param alpha			If true, makes per-pixel variable transparency, if false, one chosen color is transparent */
		Surface(RunHeader* rhPtr, bool needBitmapFuncs, bool needTextFuncs,
			std::size_t width, std::size_t height, bool alpha);

#if _WIN32
		// Creates a surface tied to the Fusion window, allowing you to read or write directly.
		// Only used in Windows editor builds!
		static Surface CreateFromFrameEditorWindow();
#endif

		// Creates a surface tied to the main Fusion window, allowing you to read or write directly.
		static Surface& CreateFromMainWindow(RunHeader* rhPtr);

		/* Creates a surface from filepath, with optional HWA, text or bitmap functionality, creating an image to back it.
		 * If neither text or bitmap is enabled, will allow the surface to be hardware-backed.
		 *  @param ext				The Extension opening it
		 *  @param filePath			File path to open, will be de-embedded if needed.
		 *  @param needBitmapFuncs	Enables bitmap functions (such as drawing shapes), making a non-HWA surface
		 *  @param needTextFuncs	Enables text-drawing functions, making a non-HWA surface
		 *	@param hasalpha			If true, makes per-pixel variable transparency, if false, one chosen color is transparent */
		static std::unique_ptr<Surface> CreateFromFilePath(Extension * ext, std::tstring filePath, bool needBitmapFuncs,
			bool needTextFuncs, bool alpha);

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
		// Tells the surface it is the main display for this extension
		// Surface will then expect X and Y in rdPtr->get_rHo() to be accurate
		void SetAsExtensionDisplay(Extension*);
#endif // DARKEDIF_DISPLAY_SIMPLE

		// Creates a copy of this surface's entire canvas, overwriting destination surface
		bool Clone(Surface& dest, CloneType type);

		// Creates a copy of a section of this surface, overwriting destination surface
		bool CloneSection(Surface& dest, CloneType type, Rect rect);

		// ======================
		// Surface info
		// ======================

		// Gets width in pixels
		std::size_t GetWidth() const;
		// Gets height in pixels
		std::size_t GetHeight() const;
		inline Size GetSize() const {
			return Size { static_cast<int>(GetWidth()), static_cast<int>(GetHeight()) };
		}
		inline Rect GetBounds() const {
			return Rect { GetSize() };
		}

		// Number of bits per pixel; 1, 24 or 32.
		std::size_t GetDepth() const;

		bool SetTransparencyToColor(std::uint32_t color);
		void SetTransparencyToAlpha();

		// ======================
		// Raster operations
		// ======================

		// Locks the raw memory of the Surface, and returns an accessor for it.
		// Once the accessor goes out of scope, the Surface is unlocked.
		Holder<std::uint8_t*, Surface> GetPixelBytes();
		// Locks the raw alpha channel of the Surface, and returns an accessor for it.
		// Once the accessor goes out of scope, the Surface's alpha is unlocked.
		Holder<std::uint8_t*, Surface> GetAlphaBytes();

		// Width of each line of pixels in bytes, including the padding added
		// Can be negative when pixels are arranged in right-to-left
		int GetPitch() const;

		// ======================
		// LoadImage (DIB format) / SaveImage (DIB format)
		// ======================

	#ifdef _WIN32
		// Creates a Surface from a cSurface. See Create_FromMainWindow().
		static DarkEdif::Surface * FromWindowsSurface(RunHeader* rhPtr, cSurface*);

		// Loads from WinAPI HFILE opened with e.g. CreateFile(), reading a given byte count
		bool Windows_LoadImageFromHFILE(HFILE fileHandle, std::uint32_t lsize);

		// Loads from a DLL/EXE resource ID number
		bool Windows_LoadImageFromHINSTANCE(HINSTANCE hInst, int bmpID);

		/* Loads bitmap from memory, optionally reading the pixel data from elsewhere.
		 * @param pBmi The address of the bitmap header information (LPBITMAPINFO)
		 * @param pBits The address of the first pixel of the image; if null, assumes first pixel
		 *				follows the passed LPBITMAPINFO */
		bool Windows_LoadImageFromBMPMemory(LPBITMAPINFO pBmi, std::uint8_t* pBits = nullptr);

		// Saves to a WinAPI HANDLE opened with e.g. CreateFile()
		bool Windows_SaveImageToWinAPIFileHandle(HANDLE fileHandle, ImageFileFormat fmt = ImageFileFormat::BMP,
			std::size_t quality = 100) const;

		/* Saves file to BMP memory, optionally reading the LPBITMAP data itself from elsewhere.
		 * @param pBmi The address of the bitmap header information (LPBITMAPINFO)
		 * @param pBits The address of the first pixel of data; if null, assumes first pixel
		 *				follows the passed LPBITMAPINFO */
		bool Windows_SaveImageToBMPMemory(LPBITMAPINFO pBmi, std::uint8_t* pBits) const;
	#endif

		/* Creates a surface with optional HWA, text or bitmap functionality, creating an image to back it.
		 * If neither text or bitmap is enabled, will allow the surface to be hardware-backed.
		 *  @param rhPtr			The runtime header, normally in ext->rhPtr variable.
		 *  @param needBitmapFuncs	Enables bitmap functions (such as drawing shapes), making a non-HWA surface
		 *  @param needTextFuncs	Enables text-drawing functions, making a non-HWA surface
		 *	@param alpha			If true, makes per-pixel variable transparency, if false, one chosen color is transparent */
		static std::unique_ptr<Surface> CreateFromImageBankID(RunHeader* rhPtr, std::uint16_t imageID, bool needBitmapFuncs, bool needTextFuncs);

		// Loads from C FILE opened with e.g. fopen(x, "rb"). See DarkEdif::MakePathUnembeddedIfNeeded().
		DarkEdif::Surface::ImageFileFormat LoadImageFromFileHandle(FILE * fileHandle, std::size_t byteCountToRead = SIZE_MAX);

		/** Saves image to C FILE opened with e.g. fopen(x, "wb")
		 * @param fileHandle File opened with write privileges ("wb", "w+b"), or append
		 * @param fmt		 Format to write with, by default PNG
		 * @param quality	 If JPEG format, the quality to write, from 1-100 */
		bool SaveImageToFileHandle(FILE * fileHandle, ImageFileFormat fmt = ImageFileFormat::PNG, size_t quality = 100) const;

		// Loads from a file path. See DarkEdif::MakePathUnembeddedIfNeeded() to support binary elements.
		DarkEdif::Surface::ImageFileFormat LoadImageFromFilePath(const std::tstring & filename);

		/** Saves to a file path.
		 * @param filename Full file path to write to
		 * @param fmt	   Image format; attempts to read from file path. Defaults to PNG if it cannot match.
		 * @param quality  If JPEG format, the quality to write, from 1-100 */
		bool SaveImageToFilePath(const std::tstring& filename, ImageFileFormat fmt = ImageFileFormat::Unset, size_t quality = 100);

		// ======================
		// Pixel functions
		// ======================
		void SetPixel(Point pt, std::uint32_t color, bool includeAlpha = true);
		std::uint32_t GetPixel(Point pt, bool includeAlpha = true) const;

		// ======================
		// Blit functions
		// ======================

		// Copies this entire surface to destination, overwriting RGB, alpha/transparency, and size.
		bool CopyTo(Surface & dest) const;
		// Copies this entire surface to destination rectangle, clipping if necessary.
		bool CopyToPoint(Surface& dest, Point topLeft) const;

		/* Copies part of this surface to destination, overwriting RGB, and optionally alpha/transparency.
		 * @param dest		  Destination surface
		 * @param destRect	  Destination rectangle
		 * @param srcRect	  Source rectangle
		 * @param transparent If true, copies using the source transparency/alpha to determine
		 *					  whether destination is transparent. If false, copies flat RGB.
		 * @param blitFlags	  Controls anti-aliasing when merging, and whether alpha channel is copied.
		 * @param blitOp	  Ink effect to apply when merging the two RGBs; e.g. blend, add, xor.
		 *					  No effect means destination original RGB is ignored.
		 * @param blitOpParam How strongly to apply the ink effect.
		 * @remarks If includeTransparent is true, but blitOptions excludes CopyAlpha,
		 *			pixels from source that should have been transparent are changed to
		 *  		transparent colour of destination. */
		bool CopySection(Rect srcRect, Surface & dest, Rect destRect,
			bool transparent = true,
			BlitOptions blitFlags = BlitOptions::CopyAlpha,
			BlitOperation blitOp = BlitOperation::Copy,
			std::uint8_t blitOpParam = 0) const;

		bool CopyWithRotate(Surface & dest, PointF destPoint,
			Rect src, Point rotateCenter, float angle, const PointF& scale,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy,
			std::uint32_t blitOpParam = 0,
			BlitOptions blitFlags = BlitOptions::None) const;

		// Copies a rectangle from its point to the destination coordinate.
		bool CopySectionInternally(Rect src, Point destTopLeft);

		// Flip surface horizontally or vertically
		bool ReverseImage(bool x);

		// Flip rectangle horizontally
		bool ReverseImageSection(bool x, const Rect& rect);

		// Gets the rectangle produced by removing transparent space on all four sides.
		// Considers alpha of 0 if alpha channel is present, and transparent color otherwise.
		// May return empty rectangle if surface is entirely transparent - see IsFullyTransparent().
		// @remarks Called cSurface::GetMinimizeRect().
		Rect GetNonTransparentRect();

		/** Removes external transparent area to make the smallest rect (minimize),
		 *  optionally what rectangle the new image would have had on the old one.
		 * @param oldRect If given, what rectangle the new image would have had on the old one. */
		bool RemoveTransparentEdges(Rect * oldRect = nullptr);

		// ======================
		// Fill
		// ======================

		/** Fills entire image with the fill design, example: DarkEdif::FillColor(255, 0, 0) */
		bool FillImageWith(const SurfaceFill& sf);

		// Fill block
		bool FillSectionWith(Rect section, const SurfaceFill& fillData);

		// ======================
		// Geometric Primitives
		// ======================

		// 1. Simple routines : call GDI with Surface DC
		// =============================================

		bool DrawEllipse(Rect r, std::size_t thickness = 1, std::uint32_t outlineColor = 0);

		bool DrawFilledEllipse(Rect r, std::uint32_t fillColor, std::size_t thickness /*= 0*/,
			std::uint32_t crOutl /*= BLACK*/, bool Fill = true);

		bool DrawRectangle(Rect r, std::size_t outlineThickness = 1, std::uint32_t outlineColor = 0);

		bool DrawFilledRectangle(Rect, std::uint32_t fillColor, std::size_t outlineThickness = 0,
			std::uint32_t crOutl = GetRGB(0,0,0));

		bool DrawPolygon(Point* pts, std::size_t nPts, std::size_t thickness = 1, std::uint32_t outlineColor = 0);

		bool DrawFilledPolygon(Point * pts, std::size_t nPts, std::uint32_t fillColor, std::size_t thickness = 0,
			std::uint32_t crOutl = 0, bool fill = true);

		bool DrawLine(Point a, Point b, std::size_t thickness = 1, std::uint32_t outlineColor = 0);

		// Note: has no effect in HWA, if thickness is greater than 1.
		bool DrawLine(Point a, Point b, std::size_t thickness, SurfaceFill::LineFlags lf, bool antiAliasing,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
		// ==============================================================================

		bool DrawEllipseAdvanced(Rect& pos, std::size_t thickness, SurfaceFill& fdOutl, bool antiAliasing = false,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawEllipse(Rect& pos, SurfaceFill& fdFill,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawEllipse(Rect& pos, SurfaceFill& fdFill, std::size_t thickness, SurfaceFill & fdOutl,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0, bool Fill = true);

		bool DrawRectangle(Rect& pos, std::size_t thickness, SurfaceFill & fdFill, bool antiAliasing = false,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawRectangle(Rect& pos, SurfaceFill & fdFill,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawRectangle(Rect pos, SurfaceFill& fdFill, std::size_t thickness, SurfaceFill& fdOutl,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0, bool fill = true);

		bool DrawPolygon(Point * pts, std::size_t nPts, std::size_t thickness, SurfaceFill& fdOutl, bool antiAliasing = false,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawPolygon(Point* pts, std::size_t nPts, SurfaceFill& fdFill,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool DrawPolygon(Point * pts, std::size_t nPts, SurfaceFill& fdFill, std::size_t thickness, SurfaceFill& fdOutl,
			bool antiAliasing = false, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0, bool fill = true);

		// Filled Primitives

		bool FloodFill(Point pt, Rect& rect, std::uint32_t crFill, bool antiAliasing = false,
			size_t tol = 0, bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		bool FloodFill(Point pt, std::uint32_t clrFill, bool antiAliasing = false, std::size_t tol = 0,
			bool blitTransparent = false, BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0);

		// ======================
		// Rotation
		// ======================

		// Rotates by radians, storing result in a new destination.
		void RotateIntoRadian(Surface* destSurf, double radians, bool antiAliasing,
			std::uint32_t clrFill = 0L, bool transp = true);
		// Rotates by degrees, storing result in a new destination.
		void RotateIntoDegree(Surface* destSurf, int degrees, bool antiAliasing,
			std::uint32_t clrFill = 0L, bool transp = true);

		// Calculates a new size of canvas needed to hold the given size when rotated
		static Size GetRotatedRectSize(const Size &original, float angle);

		// ======================
		// Text
		// Rather than undef the native TextOut/DrawText, we'll just inherit it. User can always invoke either A/W directly.
		// antiAliasing = DWRITE_RENDERING_MODE?
		// ======================

		int TextWriteOut(std::tstring_view text, Point topLeft, std::uint32_t textAlignMode,
			Rect &clipRect, std::uint32_t textColor = 0, DarkEdif::FontInfoMultiPlat fontHandle = DarkEdif::FontInfoMultiPlat(),
			bool transparent = true, BlitOperation blitOp = BlitOperation::Copy,
			std::uint32_t param = 0, int antiAliasing = 0);

		int	TextWrite(std::tstring_view text, Rect & clipRect, std::uint32_t dtflags, std::uint32_t color = 0,
			DarkEdif::FontInfoMultiPlat hFnt = DarkEdif::FontInfoMultiPlat(), bool transparent = true,
			BlitOperation blitOp = BlitOperation::Copy, std::uint32_t param = 0,
			int antiAliasing = 0, std::size_t dwLeftMargin = 0, std::size_t dwRightMargin = 0,
			std::size_t dwTabSize = 8);

		// Is entirely transparent - applies from transparent color or alpha
		bool IsFullyTransparent() const;

		// Replace color
		bool ReplaceColor(std::uint32_t newColor, std::uint32_t oldColor, bool includeAlpha);

		// Test collision fine entre deux surfaces
		bool IsColliding(Surface & dest, std::size_t xDest, std::size_t yDest, std::size_t xSrc, std::size_t ySrc, int rcWidth, int rcHeight);

#ifdef _WIN32
		/* Creates a Windows icon handle from the image. Does not consider alpha channel.
		 * @param iconSize	  Icon size to use
		 * @param transpColor Transparent color, or -1 for whatever the underlying surface uses.
		 * @param hotSpot	  Hot spot to use, or null to not specify */
		HICON Windows_CreateIcon(Size iconSize, std::uint32_t transpColor = -1, Point * hotSpot = nullptr);
#endif

		// Sets transparent color. Does not change underlying pixels, but affects future copying.
		// Ignored if alpha channel is present - see HasAlpha().
		void SetTransparentColor(std::uint32_t rgb);
		// Gets transparent color. Will return -1 if alpha is present.
		std::uint32_t GetTransparentColor() const;

		// Alpha channel
		bool		HasAlpha() const;
		int			GetAlphaPitch() const;
		void		SetAlpha(const std::byte * alphaBytes, int alphaPitch);
		void		AttachAlpha(const std::byte * alphaBytes, int alphaPitch);
		std::byte *	DetachAlpha(int * alphaPitch);
		Holder<Surface *, Surface>	GetAlphaSurface();

	DarkEdifInternalAccessProtected:

		/* Outputs the surface to the given image format, then sends the image in chunks to the WriteFunc.
		 * For error and log, calls WrittenToDesc.
		 * On error, calls ErrorDesc to get error description as text.
		 * @param userPtr		The variable to pass to your write/desc functions
		 * @param WriteFunc		The function that is meant to write. Return false on error.
		 * @param fmt			Image format. This cannot be Unset or invalid.
		 * @param quality		Image quality. This should already be in range.
		 * @param WrittenToDesc	Function called to get log-worthy description of the output location;
		 *						return example, "file C:\bob.txt", "addr 0x55". See std::to_tstring(), _stprintf_s().
		 * @param ErrorDesc		Function called to get error code/text for the output error lines;
		 *						for example, std::to_tstring(GetLastError()).
		 * @return				Returns true if entire write succeeded. Returns false otherwise. */
		bool InternalImageSave(void* userPtr,
			bool(*WriteFunc)(void* userPtr, const char* memToWrite, std::size_t memSizeToWrite, std::uint32_t* amountWritten),
			const ImageFileFormat fmt, const std::size_t quality,
			std::tstring(*WrittenToDesc)(void* userPtr),
			std::tstring(*ErrorDesc)(void* userPtr)) const;

		/* Reads the surface from the passed userPtr and size in bytes, reading in chunks from ReadFunc.
		 * For error and log, calls ReadFromDesc.
		 * On error, calls ErrorDesc to get error description as text.
		 * @param userPtr		The variable to pass to your write/desc functions
		 * @param sizeBytes		The size of the entire file
		 * @param ReadFunc		The function that is meant to read. Return false on error.
		 * @param fmt			Image format. This cannot be Unset or invalid.
		 * @param WrittenToDesc	Function called to get log-worthy description of the output location;
		 *						return example, "file C:\bob.txt", "addr 0x55". See std::to_tstring(), _stprintf_s().
		 * @param ErrorDesc		Function called to get error code/text for the output error lines;
		 *						for example, std::to_tstring(GetLastError()).
		 * @return				Returns image file format, or ImageFileFormat::Unset if read failed. */
		ImageFileFormat InternalImageLoad(void* userPtr, std::size_t sizeBytes,
			bool(*ReadFunc)(void* userPtr, char* memToReadTo, std::size_t memSizeToRead, std::uint32_t* amountRead),
			std::tstring(*ReadFromDesc)(void* userPtr),
			std::tstring(*ErrorDesc)(void* userPtr));
	};
};

template <class _Elem = TCHAR, class _Traits>
std::basic_ostream<_Elem, _Traits>& operator<<(
	std::basic_ostream<_Elem, _Traits>& _Ostr, const DarkEdif::Size s) {
	static_assert(std::is_same_v<_Elem, TCHAR>, "Auto converter only available for std::tstringstream");
	const std::tstring s2 = s;
	return (_Ostr << s2);
}

template <class _Elem = TCHAR, class _Traits>
std::basic_ostream<_Elem, _Traits>& operator<<(
	std::basic_ostream<_Elem, _Traits>& _Ostr, const DarkEdif::Point p) {
	static_assert(std::is_same_v<_Elem, TCHAR>, "Auto converter only available for std::tstringstream");
	const std::tstring p2 = p;
	return (_Ostr << p2);
}

template <class _Elem = TCHAR, class _Traits>
std::basic_ostream<_Elem, _Traits>& operator<<(
	std::basic_ostream<_Elem, _Traits>& _Ostr, const DarkEdif::PointF p) {
	static_assert(std::is_same_v<_Elem, TCHAR>, "Auto converter only available for std::tstringstream");
	const std::tstring p2 = p;
	return (_Ostr << p2);
}
