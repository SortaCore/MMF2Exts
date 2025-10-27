#include "DarkEdif.hpp" // Should include SurfaceMultiPlat.hpp
#include "Extension.hpp"
#ifdef _WIN32
	#include "ImageFlt.hpp"
	extern "C" void FusionAPI WinAddZone(void*, RECT*);
#else // not Windows
	#include <sys/stat.h>
	#define _stat stat
	#define _fileno(x) fileno(x)
	#define _fstat(a,b) fstat(a,b)
	typedef std::uint32_t COLORREF;
	#ifdef __ANDROID__
		// static definition
		jmethodID DarkEdif::Surface::textSurfCtorMethod, DarkEdif::Surface::textSurfUpdateTextureMethod,
			DarkEdif::Surface::textSurfDrawMethod;
		jfieldID DarkEdif::Surface::textSurfBitmapFieldID, DarkEdif::Surface::textSurfTextureFieldID;
		jmethodID DarkEdif::Surface::bitmapEraseMethod, DarkEdif::Surface::bitmapGetWidthMethod,
			DarkEdif::Surface::bitmapGetHeightMethod;
		global<jobject> DarkEdif::Surface::renderer;
		bool DarkEdif::Surface::isOpenGL2Or3;
		DarkEdif::Surface::renderScaledRotImageFuncType DarkEdif::Surface::renderScaledRotImageFunc;
		extern thread_local JNIEnv* threadEnv;
		// Skip past Java with direct C calls
		// ESxRenderer does OpenGL 2 and 3, not 1: see SurfaceView.setRenderer()
		extern "C" void Java_OpenGL_ESxRenderer_renderScaledRotatedImage(JNIEnv * env, jobject renderer,
			// CImage inherits from CTexture
			jobject texture,
			jboolean resampling, float angle,
			float scaleX, float scaleY,
			int hotSpotX, int hotspotY,
			int x, int y, int w, int h,
			int shaderIndex, int inkEffect, int inkEffectParam);
		extern "C" void Java_OpenGL_ES1Renderer_renderScaledRotatedImage(JNIEnv * env, jobject renderer,
			// CImage inherits from CTexture
			jobject texture,
			jboolean resampling, float angle,
			float scaleX, float scaleY,
			int hotSpotX, int hotspotY,
			int x, int y, int w, int h,
			int shaderIndex, int inkEffect, int inkEffectParam);
	#else // apple
		#ifndef __INTELLISENSE__
			#import "MMF2Lib/CBitmap.h"
			#import "MMF2Lib/CImage.h"
			#import "MMF2Lib/CRenderToTexture.h"
			#import "MMF2Lib/CRenderer.h"
		#else // IntelliSense
			class CImage {};
			class CBitmap {};
			class CRenderToTexture {};
		#endif // not IntelliSense
	#endif // iOS/Mac
#endif // not Windows

DarkEdif::Rect::Rect(const std::int32_t right, const std::int32_t bottom) :
	right(right), bottom(bottom)
{
	Validate(); // in case right or bottom are negative
}
DarkEdif::Rect::Rect(const std::int32_t left, const std::int32_t top,
	const std::int32_t right, const std::int32_t bottom, const bool allowNeg /* = false */) :
	left(left), top(top), right(right), bottom(bottom)
{
	if (!allowNeg && (left < 0 || top < 0 || right < 0 || bottom < 0))
		LOGF(_T("Invalid Point coordinates passed to Rect ctor; passed points (%i, %i) and (%i, %i), but negatives not allowed.\n"), left, top, right, bottom);

	Validate();
}

/* Creates rectangle from (x1,y1) to the bottom right (x2, y2) passed.
 * min(x) and min(y) must be positive unless last param is true.
 * Either point can be top-left.
 * @param allowNeg If true, allows x and y to be negative. */
DarkEdif::Rect::Rect(const Point & p1, const Point& p2, const bool allowNeg /* = false */)
	: left(std::min(p1.x, p2.x)), top(std::min(p1.y, p2.y)),
	right(std::max(p1.x, p2.x)), bottom(std::max(p1.y, p2.y))
{
	if (!allowNeg && (left < 0 || top < 0))
		LOGF(_T("Invalid Point coordinates passed to Rect ctor; passed points %s and %s, but negatives not allowed.\n"), p1.str().c_str(), p2.str().c_str());
}
bool DarkEdif::Rect::Normalize() {
	if (left < right && top < bottom)
		return true;
	int temp;
	if (left > right)
	{
		temp = left;
		left = right;
		right = left;
	}
	if (top > bottom)
	{
		temp = top;
		top = bottom;
		bottom = temp;
	}
	return false;
}
void DarkEdif::Rect::Validate() const
{
	if (left < right && top < bottom)
		return;
	Rect corrected = *this;
	if (corrected.Normalize())
		LOGF(_T("Incorrect Normalize shorthand\n")); // should only be false due to if above
	else
	{
		LOGF(_T("Invalid rectangle was found: rect %s should have been rect %s.\n"),
			ToCoordString().c_str(), corrected.ToCoordString().c_str());
	}
}
DarkEdif::Size DarkEdif::Rect::GetSize() const {
	return Size{ (int)Width(), (int)Height() };
}
DarkEdif::Rect::Rect(const Size& s) : Rect(s.width, s.height) {

}
DarkEdif::Rect::Rect(const Point& p, const Size& s, bool allowNeg /* = false */) :
	left(p.x), top(p.y), right(p.x + s.width), bottom(p.y + s.height)
{
	if (!allowNeg && (p.x < 0 || p.y < 0))
		LOGF(_T("Invalid Point coordinates passed to Rect ctor; passed point %s, but negatives not allowed.\n"), p.str().c_str());
	Validate();
}

DarkEdif::Point::Point(const std::int32_t x, const std::int32_t y, const bool allowNeg) :
	x(x), y(y)
{
	if (!allowNeg && (x < 0 || y < 0))
		LOGF(_T("Invalid Point coordinates; passed %s, but negatives not allowed.\n"), str().c_str());
}
DarkEdif::Point::operator DarkEdif::Size () const {
	return Size { x, y };
}
DarkEdif::Point::operator std::tstring () const {
	TCHAR str[40];
	if (0 >= _stprintf_s(str, std::size(str), _T("(%d, %d)"), x, y))
	{
		LOGE(_T("Invalid size print - error %d.\n"), errno);
		return _T("<size print error>"s);
	}
	return std::tstring(str);
}
std::tstring DarkEdif::Point::str() const {
	return (std::tstring)*this;
}
DarkEdif::PointF::PointF(const float x, const float y, const bool allowNeg) :
	x(x), y(y)
{
	if (!allowNeg && (x < 0.f || y < 0.f))
		LOGF(_T("Invalid Point coordinates; passed %s, but negatives not allowed.\n"), str().c_str());
}
DarkEdif::PointF::operator std::tstring() const {
	TCHAR str[40];
	if (0 >= _stprintf_s(str, std::size(str), _T("(%f, %f)"), x, y))
	{
		LOGE(_T("Invalid size print - error %d.\n"), errno);
		return _T("<size print error>"s);
	}
	return std::tstring(str);
}
std::tstring DarkEdif::PointF::str() const {
	return (std::tstring)*this;
}
DarkEdif::PointF::PointF(Point p) : x((float)p.x), y((float)p.y) {

}

DarkEdif::Size::operator DarkEdif::Point () const {
	return Point { width, height };
}
DarkEdif::Size::operator std::tstring () const {
	TCHAR str[40];
	if (0 >= _stprintf_s(str, std::size(str), _T("%u") MultiplySign "%u",
		(std::uint32_t)width, (std::uint32_t)height))
	{
		LOGE(_T("Invalid size print - error %d.\n"), errno);
		return _T("<size print error>"s);
	}
	return std::tstring(str);
}
std::tstring DarkEdif::Size::str() const {
	return (std::tstring)*this;
}

std::int32_t DarkEdif::Rect::Width() const {
	Validate();
	return right - left;
}
std::int32_t DarkEdif::Rect::Height() const {
	Validate();
	return bottom - top;
}
DarkEdif::Point DarkEdif::Rect::GetCenter() const {
	Validate();
	return Point { (left + right) / 2, (top + bottom) / 2, true };
}
DarkEdif::Point DarkEdif::Rect::GetTopLeft() const {
	Validate();
	return Point { left, top, true };
}
DarkEdif::Point DarkEdif::Rect::GetBottomRight() const {
	Validate();
	return Point { right, bottom, true };
}
std::tstring DarkEdif::Rect::ToCoordString() const {
	Validate();
	TCHAR buff[40];
	if (0 >= _stprintf_s(buff, std::size(buff), _T("(%d, %d) to (%d, %d)"), left, top, right, bottom))
	{
		LOGE(_T("Invalid rectangle coord print - error %d.\n"), errno);
		return _T("<rect coord print error>"s);
	}
	return std::tstring(buff);
}
std::tstring DarkEdif::Rect::ToAreaString() const {
	Validate();
	TCHAR bob[40];
	if (0 >= _stprintf_s(bob, std::size(bob), _T("(%d, %d) size (%d, %d)"), left, top, Width(), Height()))
	{
		LOGE(_T("Invalid rectangle area print - error %d.\n"), errno);
		return _T("<rect area print error>"s);
	}
	return std::tstring(bob);
}

#ifdef __ANDROID__
// static definitions
jmethodID DarkEdif::CollisionMask::cmaskCtorMeth = 0;
jfieldID DarkEdif::CollisionMask::ptrField = 0;

DarkEdif::CollisionMask::CollisionMask()
	: javaClass(threadEnv->FindClass("Sprites/CMask"), "CMask class inside CollisionMask")
{
	JNIExceptionCheck();
	if (cmaskCtorMeth == 0)
	{
		cmaskCtorMeth = threadEnv->GetMethodID(javaClass, "<init>", "()V");
		JNIExceptionCheck();
		ptrField = threadEnv->GetFieldID(javaClass, "ptr", "J");
		JNIExceptionCheck();
	}
	jobject newObj = threadEnv->NewObject(javaClass, cmaskCtorMeth);
	JNIExceptionCheck();

	javaObj.name = "CMask object inside CollisionMask";
	javaObj.ref = threadEnv->NewGlobalRef(newObj);
	JNIExceptionCheck();
	intern = (sMaskMP *)(long)threadEnv->GetLongField(javaObj, ptrField);
	JNIExceptionCheck();
}
void* DarkEdif::CollisionMask::GetNativePointer() {
	return javaObj.ref;
}
#else
DarkEdif::CollisionMask::CollisionMask() {}
void* DarkEdif::CollisionMask::GetNativePointer() {
	return intern;
}
#endif

std::unique_ptr<DarkEdif::CollisionMask> DarkEdif::CollisionMask::CreateWithSize(const std::size_t sizeBytes)
{
	assert(sizeBytes != 0);
	std::unique_ptr<CollisionMask> mask = std::make_unique<CollisionMask>();
	mask->intern = (sMaskMP*)calloc(sizeBytes, 1);
	if (!mask->intern)
		mask.reset();
	return mask;
}
DarkEdif::CollisionMask::~CollisionMask() {
	free(intern); // safe to call on null
	intern = nullptr;
}
std::int32_t DarkEdif::CollisionMask::get_size() const {
	assert(intern != NULL);
	return intern->size;
}
std::int32_t DarkEdif::CollisionMask::get_width() const {
	assert(intern != NULL);
	return intern->height;
}
std::int32_t DarkEdif::CollisionMask::get_height() const {
	assert(intern != NULL);
	return intern->size;
}
std::uint32_t DarkEdif::CollisionMask::get_widthBytes() const {
	assert(intern != NULL);
	return intern->widthBytes;
}
std::int32_t DarkEdif::CollisionMask::get_xSpot() const {
	assert(intern != NULL);
	return intern->xSpot;
}
std::int32_t DarkEdif::CollisionMask::get_ySpot() const {
	assert(intern != NULL);
	return intern->ySpot;
}
std::uint32_t DarkEdif::CollisionMask::get_flags() const {
	assert(intern != NULL);
	return intern->flags;
}
// Gets a rectangle underlying the mask
const DarkEdif::Rect & DarkEdif::CollisionMask::get_rect() const {
	assert(intern != NULL);
	return intern->rect;
}
// Gets raw bits of the collision mask
std::uint16_t* DarkEdif::CollisionMask::get_data() {
	assert(intern != NULL);
	return intern->data;
}
//bool GenerateFrom(Extension* ext, Surface& img);

DarkEdif::SurfaceFill::SurfaceFill(DarkEdif::SurfaceFill::FillType f) :
	fillType(f), solid()
{
	// static
}

DarkEdif::SurfaceFill DarkEdif::SurfaceFill::None(DarkEdif::SurfaceFill::LineFlags lineFlags /* = None */)
{
	DarkEdif::SurfaceFill setting(FillType::None);
	setting.line.lineFlags = (std::uint32_t)lineFlags;
	return setting;
}
DarkEdif::SurfaceFill DarkEdif::SurfaceFill::Solid(std::uint32_t color)
{
	DarkEdif::SurfaceFill setting(FillType::Flat);
	setting.solid.color = color;
	// No alpha channel? Assume user derped
	if ((color & 0xFF000000) == 0)
		color |= 0xFF000000;
	return setting;
}
DarkEdif::SurfaceFill DarkEdif::SurfaceFill::Gradient(std::uint32_t color1, std::uint32_t color2, bool vertical)
{
	DarkEdif::SurfaceFill setting(FillType::Gradient);
	setting.gradient.color1 = color1;
	setting.gradient.color2 = color2;
	setting.gradient.gradientFlags = vertical ? 1 : 0; // only one Surface object seems to use
	return setting;
}
DarkEdif::SurfaceFill DarkEdif::SurfaceFill::Mosaic(Surface* surf, Rect rect)
{
	DarkEdif::SurfaceFill setting(FillType::Gradient);
	setting.image.surf = surf;
	setting.image.rect = rect;
	return setting;
}

#ifdef _WIN32
static DarkEdif::Surface* mainWindow = nullptr;
static D3DSURFINFO drivInfo = {};
#endif


void DarkEdif::Surface::UnlockMain(DarkEdif::Surface* surf, std::uint8_t*)
{
	if (surf->isLocked)
		LOGF(_T("Double unlocking main surface %p.\n"), surf);
	surf->isLocked = false;
}
void DarkEdif::Surface::UnlockAlpha(DarkEdif::Surface* surf, std::uint8_t*)
{
	if (surf->isLockedAlpha)
		LOGF(_T("Double unlocking alpha surface at %p.\n"), surf);
	surf->isLockedAlpha = false;
}

DarkEdif::Holder<std::uint8_t*, DarkEdif::Surface> DarkEdif::Surface::GetPixelBytes()
{
	if (isLocked)
		LOGF(_T("Using LockBuffer when already locked.\n"));
#ifdef _WIN32
	std::uint8_t* buff = surf->LockBuffer();
#else
	std::uint8_t* buff = nullptr;
	LOGF(_T("Cannot lock buffer; not implemented on this platform.\n"));
#endif
	if (!buff)
		LOGF(_T("%sCouldn't lock buffer.\n"), debugID); // noreturn, save me
	return Holder<std::uint8_t*, DarkEdif::Surface>(this, buff, &Surface::UnlockMain);
}
DarkEdif::Holder<std::uint8_t*, DarkEdif::Surface> DarkEdif::Surface::GetAlphaBytes()
{
	if (isLockedAlpha)
		LOGF(_T("%sUsing GetAlphaBytes when already locked.\n"), debugID);
#ifdef _WIN32
	std::uint8_t* buff = surf->LockAlpha();
#else
	LOGF(_T("%sCannot get alpha bytes; not implemented on this platform.\n"), debugID);
	std::uint8_t* buff = nullptr;
#endif
	if (!buff)
		LOGF(_T("%sCouldn't lock alpha buffer.\n"), debugID); // noreturn, save me
	return Holder<std::uint8_t*, DarkEdif::Surface>(this, buff, &Surface::UnlockAlpha);
}
int DarkEdif::Surface::GetPitch() const {
#ifdef _WIN32
	return surf->GetPitch();
#else
	LOGF(_T("%sCannot get pitch; not implemented on this platform.\n"), debugID);
	return INT32_MIN;
#endif
}

static DarkEdif::Surface::ImageFileFormat FileExtensionToImageFormat(const std::tstring fileName)
{
	const std::size_t dotAt = fileName.rfind(_T('.'));
	if (dotAt == std::tstring::npos || dotAt == fileName.size() - 1)
	{
		LOGE(_T("DarkEdif::Surface error: No dot in image filename \"%s\", cannot guess image file format.\n"), fileName.c_str());
		return DarkEdif::Surface::ImageFileFormat::Unset;
	}
	if (dotAt == fileName.size() - 1)
	{
		LOGE(_T("DarkEdif::Surface error: File extension is empty (dot at end) in image filename \"%s\", cannot guess image file format.\n"), fileName.c_str());
		return DarkEdif::Surface::ImageFileFormat::Unset;
	}
	// Check the . wasn't inside a folder name, e.g. "C:/ThisIs.Dumb/file", so we have no ext
	std::size_t slashPos = fileName.rfind(_T('/'));
	if (slashPos == std::tstring::npos)
		slashPos = fileName.rfind(_T('\\'));
	if (slashPos != std::tstring::npos && slashPos > dotAt)
	{
		LOGE(_T("DarkEdif::Surface error: Last dot in image filename %s was in a folder name, cannot guess format.\n"), fileName.c_str());
		return DarkEdif::Surface::ImageFileFormat::Unset;
	}
	std::tstring ext = fileName.substr(dotAt + 1);
#if defined(_UNICODE) && defined(_WIN32)
	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](wchar_t c) { return towlower(c); });
#else
	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](unsigned char c) { return std::tolower(c); });
#endif
	DarkEdif::Surface::ImageFileFormat fmt = fmt = DarkEdif::Surface::ImageFileFormat::Unset;
	if (ext == _T("png"sv))
		fmt = DarkEdif::Surface::ImageFileFormat::PNG;
	else if (ext == _T("bmp"sv))
		fmt = DarkEdif::Surface::ImageFileFormat::BMP;
	else if (ext == _T("gif"sv))
		fmt = DarkEdif::Surface::ImageFileFormat::GIF;
	else if (ext == _T("jpeg"sv) || ext == _T("jpg"sv))
		fmt = DarkEdif::Surface::ImageFileFormat::JPEG;
	else if (ext == _T("tga"sv))
		fmt = DarkEdif::Surface::ImageFileFormat::TGA;
	else
	{
		LOGE(_T("DarkEdif::Surface error: Image filename \"%s\" extension \"%s\" couldn't be "
			"interpreted as an image format. Pass image format explicitly.\n"), fileName.c_str(), ext.c_str());
	}

	return fmt;
}

#ifdef _WIN32
// Loads from WinAPI HFILE opened with e.g. CreateFile(), reading a given byte count
bool DarkEdif::Surface::Windows_LoadImageFromHFILE(const HFILE fileHandle, const std::uint32_t lsize) {
	const bool ret = surf->LoadImage(fileHandle, (ULONG)lsize) != FALSE;
	if (!ret)
	{
		LOGE(_T("%sFailed to load image from HFILE %p, of passed size %u bytes; possible error code is %u.\n"),
			debugID, fileHandle, lsize, GetLastError());
		return false;
	}
	LOGV(_T("%sLoaded image from HFILE %p of passed size %u bytes OK.\n"),
		debugID, fileHandle, lsize);
	return true;
}

// Loads from a DLL/EXE resource ID number
bool DarkEdif::Surface::Windows_LoadImageFromHINSTANCE(const HINSTANCE hInst, const int bmpID) {
	const bool ret = surf->LoadImage(hInst, bmpID) != FALSE;
	if (!ret)
	{
		LOGE(_T("%sFailed to load image resource ID %i from HINSTNACE %p; possible error code is %u.\n"),
			debugID, bmpID, hInst, GetLastError());
		return false;
	}
	LOGV(_T("%sLoaded image ID %i from HINSTANCE %p OK.\n"),
		debugID, bmpID, hInst);
	return true;
}
static bool FindFilter(CImageFilterMgr* pImgMgr, DWORD& filter, const std::tstring& fileName,
	DarkEdif::Surface::ImageFileFormat fmt)
{
	std::tstring ext;
	if (fmt == DarkEdif::Surface::ImageFileFormat::Unset)
	{
		fmt = FileExtensionToImageFormat(fileName);
		if (fmt == DarkEdif::Surface::ImageFileFormat::Unset)
			return false;
	}

	// Get filter by extension
	if (fmt == DarkEdif::Surface::ImageFileFormat::PNG)
		ext = _T("png"sv);
	else if (fmt == DarkEdif::Surface::ImageFileFormat::BMP)
		ext = _T("bmp"sv);
	else if (fmt == DarkEdif::Surface::ImageFileFormat::GIF)
		ext = _T("gif"sv);
	else if (fmt == DarkEdif::Surface::ImageFileFormat::TGA)
		ext = _T("tga"sv);
	else if (fmt == DarkEdif::Surface::ImageFileFormat::JPEG)
		ext = _T("jpg"sv);
	else
		LOGF(_T("Invalid format ID %d passed to Surface::FindFilter.\n"), (int)fmt);

	// It should be a real image ext, but did the user not include the image filter?
	filter = 0;

#if defined(_WIN32) && !defined(_UNICODE)
	if (!mvIsUnicodeVersion(Edif::SDK->mV))
	{
		CImageFilterMgrANSI20* pImgMgrEvil20 = (CImageFilterMgrANSI20*)Edif::SDK->mV->ImgFilterMgr;
		for (int i = 0; i < pImgMgrEvil20->GetFilterCount(); ++i)
		{
			const TCHAR** exts = pImgMgrEvil20->GetFilterExts(i);
			if (exts == 0)
				break; // at end
			for (int j = 0; exts[j]; ++j)
			{
				if (!_tcsicmp(exts[j], ext.c_str()))
				{
					filter = pImgMgrEvil20->GetFilterID(i);
					return true;
				}
			}
		}
	}
	else
#endif
	for (int i = 0; i < pImgMgr->GetFilterCount(); ++i)
	{
		const TCHAR** exts = pImgMgr->GetFilterExts(i);
		if (exts == 0)
			break; // at end
		for (int j = 0; exts[j]; ++j)
		{
			if (!_tcsicmp(exts[j], ext.c_str()))
			{
				filter = pImgMgr->GetFilterID(i);
				return true;
			}
		}
	}
	return false;
}

/* Loads bitmap from memory, optionally reading the pixel data from elsewhere.
 * @param pBmi The address of the bitmap header information (LPBITMAPINFO)
 * @param pBits The address of the first pixel of the image; if null, assumes first pixel
 *				follows the passed LPBITMAPINFO */
bool DarkEdif::Surface::Windows_LoadImageFromBMPMemory(const LPBITMAPINFO pBmi, std::uint8_t* const pBits /* = nullptr */) {
	const bool ret = surf->LoadImage(pBmi, pBits) != FALSE;
	if (!ret)
	{
		LOGE(_T("%sFailed to load image from bmp memory %p (pBits = %p); possible error code is %u.\n"),
			debugID, pBmi, pBits, GetLastError());
		return false;
	}
	LOGV(_T("%sLoaded image from bmp memory %p OK.\n"), debugID, pBmi);
	return true;
}

bool DarkEdif::Surface::Windows_SaveImageToWinAPIFileHandle(const HANDLE fileHandle,
	const DarkEdif::Surface::ImageFileFormat fmt /* = ImageFileFormat::BMP */,
	const std::size_t quality /* = 100 */) const
{
	return InternalImageSave((void *)fileHandle,
		[](void * userPtr, const char* mem, std::size_t count, std::uint32_t * amountWritten) {
			return WriteFile((HANDLE)userPtr, mem, count, (LPDWORD)amountWritten, NULL) != FALSE;
		},
		fmt, quality,
		[](void * userPtr) {
			TCHAR desc[40];
			_stprintf_s(desc, std::size(desc), _T("HANDLE %p"), userPtr);
			return std::tstring(desc);
		},
		[](void*) {
			return std::to_tstring(GetLastError());
		}
	);
};

/* Saves file to BMP memory, optionally reading the LPBITMAP data itself from elsewhere.
 * @param pBmi The address of the bitmap header information (LPBITMAPINFO)
 * @param pBits The address of the first pixel of data; if null, assumes first pixel
 *				follows the passed LPBITMAPINFO */
bool DarkEdif::Surface::Windows_SaveImageToBMPMemory(LPBITMAPINFO pBmi, std::uint8_t* pBits) const
{
	return surf->SaveImage(pBmi, pBits) != FALSE;
}

#endif // _WIN32

std::unique_ptr<DarkEdif::Surface> DarkEdif::Surface::CreateFromImageBankID(RunHeader * rhPtr, std::uint16_t imgID, bool needBitmapFuncs, bool needTextFuncs)
{
#ifdef _WIN32
	cSurface fake;
	if (!LockImageSurface(rhPtr->rhIdAppli, imgID, fake,
		(int)(!needBitmapFuncs && !needTextFuncs ? LockImageSurfaceMode::HWACompatible : LockImageSurfaceMode::ReadBlitOnly)))
	{
		return LOGE(_T("DarkEdif::Surface error: Couldn't lock image surface to load image ID %hu.\n"), imgID), nullptr;
	}
	// TODO: Does this do alpha? should it?
	auto newSurf = std::make_unique<Surface>(rhPtr, needBitmapFuncs, needTextFuncs, fake.GetWidth(), fake.GetHeight(), fake.HasAlpha() != FALSE);
	if (!newSurf || fake.Blit(*newSurf->surf) == FALSE)
	{
		UnlockImageSurface(fake);
		return LOGE(_T("DarkEdif::Surface error: Couldn't blit image ID %hu from bank to new surface.\n"), imgID), nullptr;
	}
	UnlockImageSurface(fake);
#else
	auto newSurf = std::make_unique<Surface>(rhPtr, needBitmapFuncs, needTextFuncs, -1, -1, true);
	LOGF(_T("DarkEdif::Surface error: Cannot load image from image bank ID, not implemented on this platform\n"));
#endif
	return newSurf;
}

bool DarkEdif::Surface::InternalImageSave(void* userPtr,
	bool(*WriteFunc)(void* userPtr, const char* memToWrite, std::size_t memSizeToWrite, std::uint32_t* amountWritten),
	const ImageFileFormat fmt, const std::size_t quality,
	std::tstring(*WrittenToDesc)(void* userPtr),
	std::tstring(*ErrorDesc)(void* userPtr)) const
{
	if (quality != 100 && fmt != ImageFileFormat::JPEG)
		return LOGE(_T("%sFailed to save image to HANDLE %p with format ID %d, quality %zu; format does not support variable quality.\n"),
			debugID, WrittenToDesc(userPtr).c_str(), (int)fmt, quality), false;

	if (quality == 0 || quality > 100)
		return LOGE(_T("%sFailed to save image to HANDLE %p with format ID %d, quality %zu; quality is invalid.\n"),
			debugID, WrittenToDesc(userPtr).c_str(), (int)fmt, quality), false;

	// Attempt to find by ext, which should be done before this
	if (fmt == ImageFileFormat::Unset)
		return LOGE(_T("%sFailed to save image to HANDLE %p with format Unset; format must be specified.\n"),
			debugID, WrittenToDesc(userPtr).c_str()), false;

#ifdef _WIN32
	DWORD filter;
	if (!FindFilter(Edif::SDK->mV->ImgFilterMgr, filter, _T(""s), fmt)) {
		return LOGE(_T("%sFailed to save image to %s %p with format %d; image filter not included in app.\n"),
			debugID, WrittenToDesc(userPtr).c_str(), format), false;
	}

	TCHAR szTempFileName[MAX_PATH + 2], lpTempPathBuffer[MAX_PATH + 2];
	if (GetTempPath(MAX_PATH + 1, lpTempPathBuffer) == 0) {
		return LOGE(_T("%sFailed to save image to %s; non-standard image filter necessitated a temp "
			"file which could not be created (path lookup error %u).\n"),
			debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
	}
	HANDLE fake;
	while (true)
	{
		// This creates an empty file with this name
		if (GetTempFileName(lpTempPathBuffer, _T("DKE"), 0, szTempFileName) == 0) {
			return LOGE(_T("%sFailed to save image to %s; non-standard image filter necessitated a temp "
				"file which could not be created (path 2 lookup error %u).\n"),
				debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
		}
		// Could go for FILE_FLAG_SEQUENTIAL_SCAN, but I'm unsure it'll not be worse for strange reasons
		fake = CreateFile(szTempFileName, GENERIC_READ | GENERIC_WRITE,
			0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (fake == INVALID_HANDLE_VALUE) {
			// File already exists, retry
			if (GetLastError() == ERROR_FILE_EXISTS)
				continue;
			return LOGE(_T("%sFailed to save image to %s; non-standard image filter necessitated a temp "
				"file which could not be created (createfile error %u).\n"),
				debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
		}
		break;
	}

	if (!surf->SaveImage((HFILE)fake)) {
		CloseHandle(fake);
		return LOGE(_T("%sFailed to save image to %s; non-standard image filter necessitated a temp "
			"file which could not be created (SaveImage error, possibly %u).\n"),
			debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
	}

	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(fake, &fileSize) == FALSE || fileSize.HighPart > 0) {
		CloseHandle(fake);
		return LOGE(_T("%sFailed to save image to %s; temp file size couldn't be read (error %u).\n"),
			debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
	}
	// Reset back to start to read back
	if (SetFilePointer(fake, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		CloseHandle(fake);
		return LOGE(_T("%sFailed to save image to %s; non-standard image filter necessitated a temp "
			"file which failed to seek (SetFilePointer error, %u).\n"),
			debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
	}
	char buff[10 * 1024];
	for (std::uint32_t remaining = fileSize.LowPart, read, written; remaining > 0; remaining -= read)
	{
		if (ReadFile(fake, buff, std::min((std::uint32_t)std::size(buff), remaining), (LPDWORD)&read, NULL) == FALSE) {
			CloseHandle(fake);
			return LOGE(_T("%sFailed to save image to %s; failed to read temp file back (ReadFile error %u).\n"),
				debugID, WrittenToDesc(userPtr).c_str(), GetLastError()), false;
		}

		if (WriteFunc(userPtr, buff, read, &written) == FALSE || written != read) {
			CloseHandle(fake);
			return LOGE(_T("%sFailed to save image to %s; failed to write, error: %s.\n"),
				debugID, WrittenToDesc(userPtr).c_str(), ErrorDesc(userPtr).c_str()), false;
		}
	}
	// If this fails it may indicate it failed to write disk cache - but we're not caring if it's on disk.
	if (!CloseHandle(fake))
		LOGW(_T("%sTemporary file did not close nicely, error %u. Ignoring.\n"), debugID, GetLastError());
#else
	// FILE * fake = std::tmpfile();
	// surf->Save to fake, then read back, sending it off to WriteFunc
	LOGF(_T("%sSaving image not implemented on this platform.\n"), debugID);
	// fclose(fake);
#endif

	LOGV(_T("%sSaved image to %s OK.\n"), debugID, WrittenToDesc(userPtr).c_str());
	return true;
}


std::size_t DarkEdif::Surface::Internal_GetSizeOfFile(FILE* fileHandle, const TCHAR * op) const
{
	// Someone read a chunk of the file then told us the remainder was all an image,
	// presumably not intended, but if someone does have a weird format they can force
	// by getting remaining file size and passing it explicitly
	const int res = (int)ftell(fileHandle);
	if (res == -1)
		return LOGE(_T("%sCouldn't %s; position-read error %d: %s.\n"), debugID, op, errno, _tcserror(errno)), SIZE_MAX;
	else if (res > 0)
		return LOGE(_T("%sCouldn't %s; not at start of file, but unexpectedly %d bytes into it.\n"), debugID, op, res), SIZE_MAX;

	// fseek to end produces undefined behaviour, see POSIX standard

	// This doesn't allow >2GiB files, but no sane image is that size
	struct _stat sizeMan;
	if (_fstat(_fileno(fileHandle), &sizeMan) == -1)
		return LOGE(_T("%sCouldn't %s; size-read error %d: %s.\n"), debugID, op, errno, _tcserror(errno)), SIZE_MAX;
	if ((std::uint64_t)sizeMan.st_size >= INT32_MAX)
		return LOGE(_T("%sCouldn't %s; file is bigger than 2GiB.\n"), debugID, op), SIZE_MAX;
	return (std::size_t)sizeMan.st_size;
}
DarkEdif::Surface::ImageFileFormat DarkEdif::Surface::LoadImageFromFileHandle(FILE* fileHandle, std::size_t size /* = SIZE_MAX */)
{
	assert(fileHandle);
	if (size == SIZE_MAX)
	{
		TCHAR desc[40];
		_stprintf_s(desc, std::size(desc), _T("load image from handle %p"), fileHandle);
		size = Internal_GetSizeOfFile(fileHandle, desc);
		if (size == SIZE_MAX)
			return ImageFileFormat::Unset; // error already reported
	}
	return InternalImageLoad(fileHandle, size,
		[](void* userPtr, char* mem, std::size_t count, std::uint32_t* amountRead) {
			*amountRead = (std::uint32_t)fread(mem, 1, count, (FILE *)userPtr);
			return *amountRead == count;
		},
		[](void* userPtr) {
			TCHAR desc[40];
			_stprintf_s(desc, std::size(desc), _T("FILE %p"), userPtr);
			return std::tstring(desc);
		},
		[](void*) {
			return std::to_tstring(errno);
		}
	);
}

bool DarkEdif::Surface::SaveImageToFileHandle(FILE* fileHandle,
	ImageFileFormat fmt /* = ImageFileFormat::PNG */, std::size_t quality /* = 100 */) const
{
	return InternalImageSave(fileHandle,
		[](void* userPtr, const char* mem, std::size_t count, std::uint32_t* amountWritten) {
			*amountWritten = (std::uint32_t)fwrite(mem, 1, count, (FILE *)userPtr);
			return *amountWritten == count;
		},
		fmt, quality,
		[](void* userPtr) {
			TCHAR desc[40];
			_stprintf_s(desc, std::size(desc), _T("FILE %p"), userPtr);
			return std::tstring(desc);
		},
		[](void*) {
			TCHAR errStr[256] = {}, errStr2[256];
#ifdef _WIN32
			_tcserror_s(errStr, std::size(errStr), errno);
#else
			strcpy(errStr, strerror(errno));
#endif
			_stprintf_s(errStr2, std::size(errStr2), _T("%d: %s"), errno, errStr);
			return std::tstring(errStr2);
		}
	);
}

bool DarkEdif::Surface::SaveImageToFilePath(const std::tstring& file, ImageFileFormat fmt /* = ImageFileFormat::Unset*/, size_t quality /* = 100 */)
{
	if (fmt == ImageFileFormat::Unset)
	{
		fmt = FileExtensionToImageFormat(file);
		if (fmt == ImageFileFormat::Unset)
			return false;
	}

#ifdef _WIN32
	DWORD filter = -1;
	const auto pImgMgr = Edif::SDK->mV->ImgFilterMgr;
	if (!FindFilter(Edif::SDK->mV->ImgFilterMgr, filter, file, fmt)) {
		LOGE(_T("%sFailed to save image to file \"%s\" with format %d; image filter %s.\n"),
			debugID, file.c_str(), (int)fmt,
			filter == -1 ? _T("not recognised from file format") : _T("not included in app"));
		return false;
	}

	// TODO: Does this actually change quality for ExportImage?
	// If not, how do we save with a filter?!
	CImageFilter* pFilter = NULL;
	CImageFilter pFilter2(pImgMgr);
	pFilter = &pFilter2;
	pFilter->UseSpecificFilterID(filter);
	pFilter->SetCompressionLevel(101 - (int)quality);

	int ifErr;
	{
		int depth = surf->GetDepth();
		std::unique_ptr<Surface> fake;
		if (depth == 32 && (fmt != ImageFileFormat::PNG && fmt != ImageFileFormat::GIF))
		{
			depth = 24;
			fake = std::make_unique<Surface>(rhPtr, true, false, GetWidth(), GetHeight(), false);
			if (surf->Blit(*fake->surf) == FALSE) {
				return LOGE(_T("%sFailed to save image to file \"%s\" with format %d; copy to non-transparent surface was necessary and failed.\n"),
					debugID, file.c_str(), (int)fmt), false;
			}
			LOGI(_T("Note: will save 32-bit image as 24-bit file, as image file format %d does not support transparency.\n"), (int)fmt);
		}
		if (depth == 32)
		{
			auto buffHolder = GetPixelBytes();
			auto buffAlphaHolder = GetAlphaBytes();

			ifErr = pFilter->Save((UShortTCHAR*) file.c_str(),
				(LPBYTE)*buffHolder, surf->GetWidth(), surf->GetHeight(), depth,
				surf->GetPitch(), (LPLOGPALETTE)surf->GetPalette(),
				(LPBYTE)*buffAlphaHolder, surf->GetAlphaPitch());
		}
		else
		{
			auto buffHolder = (fake ? &*fake : this)->GetPixelBytes();
			auto name = pFilter->GetFilterNameW();
			ifErr = pFilter->Save((UShortTCHAR*)file.c_str(),
				(LPBYTE)*buffHolder, surf->GetWidth(), surf->GetHeight(), depth,
				(fake ? &*fake : this)->surf->GetPitch(), (LPLOGPALETTE)surf->GetPalette());
		}
	}
	// TODO: is pFilter meant to be deleted? CRT suggests no leak
	pFilter->SetCompressionLevel(-1); // in case old object

	if (ifErr != IF_OK) {
		return LOGE(_T("%sFailed to save image to file \"%s\" with format %d; image error %d.\n"),
			debugID, file.c_str(), (int)fmt, ifErr), false;
	}
	return true;
#else
	FILE* fil = fopen(file.c_str(), "wb");
	int err = errno;
	if (fil == NULL)
	{
		TCHAR * errStr = strerror(err);
		return LOGE(_T("%sFailed to write to image file \"%s\", got error %d: %s.\n"),
			debugID, file.c_str(), err, errStr), false;
	}

	const bool ret = SaveImageToFileHandle(fil, fmt, quality);
	if (fclose(fil))
	{
		err = errno;
		TCHAR* errStr = strerror(err);
		return LOGE(_T("%sFailed to write to image file \"%s\", got error %d: %s.\n"),
			debugID, file.c_str(), err, errStr), false;
	}
	return ret;
#endif
}

DarkEdif::Surface::ImageFileFormat DarkEdif::Surface::LoadImageFromFilePath(const std::tstring & file)
{
#ifdef _WIN32
	FILE* fil;
	int err = _tfopen_s(&fil, file.c_str(), _T("rb"));
#else
	FILE* fil = fopen(file.c_str(), "rb");
	int err = errno;
#endif

	if (fil == NULL)
	{
#ifdef _WIN32
		TCHAR errStr[512] = {};
		_tcserror_s(errStr, std::size(errStr), err);
#else
		TCHAR* errStr = strerror(err);
#endif
		return LOGE(_T("%sFailed to read image file \"%s\", got error %d: %s.\n"),
			debugID, file.c_str(), err, errStr), ImageFileFormat::Unset;
	}

	ImageFileFormat fmt = LoadImageFromFileHandle(fil);
	fclose(fil);
	return fmt;
}

DarkEdif::Surface::ImageFileFormat DarkEdif::Surface::InternalImageLoad(void* userPtr, std::size_t sizeBytes,
	bool(*ReadFunc)(void* userPtr, char* memToRead, std::size_t memSizeToRead, std::uint32_t* amountRead),
	std::tstring(*ReadFromDesc)(void* userPtr),
	std::tstring(*ErrorDesc)(void* userPtr))
{
	assert(userPtr);
	// Attempt to find by ext, then default to BMP
	//if (fmt == ImageFileFormat::Unset)
	//	return LOGE(_T("%sFailed to save image to HANDLE %p with format Unset; format must be specified.\n"),
	//		debugID, WrittenToDesc(userPtr).c_str()), false;

	enum
	{
		UseSurfaceDepth = 1,
		// seems to get confused if there is no palette
		UseSurfacePalette = 2,
		// Unicode MMF2 and later only
		FirstPixelTransparent = 4,
		// Unicode MMF2 and later only
		Opaque = 8,
	};

#ifdef _WIN32
	auto mem = CInputMemFile::NewInstance();
	mem->Create(sizeBytes);
	char *buff = (char *)mem->GetMemBuffer();
#else
	std::unique_ptr<char[]> buff2 = std::make_unique<char[]>(sizeBytes);
	char * buff = buff2.get();
#endif
	std::uint32_t lastRead;
	for (std::size_t remaining = sizeBytes; remaining > 0; remaining -= lastRead)
	{
		if (!ReadFunc(userPtr, buff, std::min<std::size_t>(10U * 1024U, remaining), &lastRead)) {
			return LOGE(_T("%sFailed to load image from %s; failed to read at %zu bytes remaining (read error %s).\n"),
				debugID, ReadFromDesc(userPtr).c_str(), sizeBytes - remaining, ErrorDesc(userPtr).c_str()), ImageFileFormat::Unset;
		}
		buff += lastRead;
	}
	ImageFileFormat fmt = ImageFileFormat::Unset;

#ifdef _WIN32
	CImageFilterMgr* pImgMgr = Edif::SDK->mV->ImgFilterMgr;
	DWORD filter;

	// Note this will replace transparent color and alpha channel
	if (!ImportImageFromInputFile(pImgMgr, mem, surf, &filter, UseSurfaceDepth | UseSurfacePalette))
	{
		mem->Delete();
		return LOGE(_T("%sFailed to load image from %s; Fusion image manager reported error %d.\n"),
			debugID, ReadFromDesc(userPtr).c_str(), surf->GetLastError()), ImageFileFormat::Unset;
	}
	mem->Delete();
	if (filter == ' PMB') // 'BMP '
		fmt = ImageFileFormat::BMP;
	else if (filter == ' GNP') // 'PNG '
		fmt = ImageFileFormat::PNG;
	else if (filter == 'GEPJ') // 'JPEG'
		fmt = ImageFileFormat::JPEG;
	//else if (filter == ' MNA') // 'ANM '
		//fmt = ImageFileFormat::ANM;
	else if (filter == ' AGT')
		fmt = ImageFileFormat::TGA;
	else if (filter == ' FIG') // GIF
		fmt = ImageFileFormat::GIF;
	else
	{
		return LOGE(_T("%sLoaded image from %s but failed to recognise format %u.\n"),
			debugID, ReadFromDesc(userPtr).c_str(), filter), ImageFileFormat::Unset;
	}
#else
	return LOGE(_T("%sLoading image from %s failed, not implemented on this platform!\n"),
		debugID, ReadFromDesc(userPtr).c_str()), ImageFileFormat::Unset;
	// ?
#endif

	LOGV(_T("%sLoaded image from %s OK.\n"), debugID, ReadFromDesc(userPtr).c_str());
	return fmt;
}

void DarkEdif::Surface::IDGen() {
	if (_stprintf_s(debugID, std::size(debugID), _T("Surf %d (0x%p): "), IDCounter++, this) <= 0)
		LOGF(_T("%sFailed to create Surface debug ID, error %d.\n"), debugID, errno);
}
#ifdef _WIN32
DarkEdif::Surface::Surface(RunHeader * rhPtr, cSurface* surf, bool isFrameSurface /* = false */) :
	rhPtr(rhPtr), surf(surf), isFrameSurface(isFrameSurface)
{
	IDGen();

	// sanity checks
	if (!rhPtr && DarkEdif::RunMode == DarkEdif::MFXRunMode::BuiltEXE)
		LOGF(_T("Null rhPtr passed to Surface ctor, aborting.\n"));

	if (!surf || !surf->IsValid())
		LOGF(_T("%sUnexpected surface invalidity for %p\n"), debugID, surf);

	if (isFrameSurface)
	{
		const SurfaceType surfType = (SurfaceType)surf->GetType();
		if ((int)surfType < 0 || surfType >= SurfaceType::Max)
			LOGF(_T("%sUnexpected surface type %d.\n"), debugID, (int)surfType);
		// Memory-backed buffer (CPU based) is needed to do most geometric drawing
		hasGeometryCapacity = surfType <= SurfaceType::Memory_PermanentDeviceContext;
		// A device context (DC) is needed to draw text
		hasTextCapacity = hasGeometryCapacity && surfType >= SurfaceType::Memory_DeviceContext;

		// Can't do GetPixel on HWA surface
		//if (IsHWA())
		{
			format = PixelFormat::RGB;
			LOGI(_T("%sNote: created frame surface, describing as: \"%s\".\n"), debugID, Describe().c_str());
			return;
		}
	}

	std::uint32_t depth = surf->GetDepth();
	if (depth == 1)
		format = PixelFormat::Monochrome;
	else if (depth < 24)
		LOGF(_T("%sUnexpected depth %u. Cannot detect color format.\n"), debugID, depth);
	else
	{
		Point topLeft; Size size;
		surf->GetClipRect(topLeft.x, topLeft.y, size.width, size.height);

		COLORREF orig, cur;
		if (!surf->GetPixel(topLeft.x, topLeft.y, orig))
			LOGE(_T("%sCouldn't read any pixels in surface.\n"), debugID);
		surf->SetPixel(topLeft.x, topLeft.y, 0xFF, 0x7F, 0x13);
		if (!surf->GetPixel(topLeft.x, topLeft.y, cur))
			LOGE(_T("%sCouldn't read edited pixel in surface.\n"), debugID);
		surf->SetPixel(topLeft.x, topLeft.y, orig);

		if ((cur & 0xFFFFFF) == 0xFF7F13)
			format = depth == 32 ? PixelFormat::XBGR : PixelFormat::BGR;
		else if ((cur & 0xFFFFFF) == 0x137FFF)
		{
			if (depth == 24)
				format = PixelFormat::RGB;
			else
				format = HasAlpha() ? PixelFormat::RGBA : PixelFormat::RGBX;
		}
		else
		{
			LOGE(_T("%sUnexpected pixel color: 4 bytes were %x, %x, %x, %x.\n"), debugID,
				(std::uint32_t)((cur & 0xFF000000U) >> 24U), (std::uint32_t)((cur & 0xFF0000U) >> 16U),
				(std::uint32_t)((cur & 0xFF00U) >> 8U), (std::uint32_t)(cur & 0xFFU));
		}
	}

	// Note: in HWA mode, this won't run
	if (isFrameSurface)
		LOGI(_T("%sNote: created frame surface, describing as: \"%s\".\n"), debugID, Describe().c_str());
	else
		LOGI(_T("%sCreated surface, describing as: \"%s\".\n"), debugID, Describe().c_str());
}
extern "C" FusionAPIImport void* FusionAPI ModifSpriteEffect(void* ptrWin, Sprite * ptSpr, DWORD effect, LPARAM effectParam);
#endif
std::size_t DarkEdif::Surface::Internal_CreateMask(void* mask, const std::uint32_t flags)
{
#ifdef _WIN32
	return surf->CreateMask((sMask *)mask, flags);
#else
	LOGF(_T("%sCannot create mask; not implemented on this platform.\n"), debugID);
	return SIZE_T_MAX;
#endif
}

DarkEdif::CollisionMask * DarkEdif::Surface::GetCollisionMask(const std::uint32_t flags)
{
	static constexpr int SCMF_PLATFORM = 1;

	if ((flags & SCMF_PLATFORM) != flags)
		LOGE(_T("Unexpected flags supplied: expected 0 or 1, got %u.\n"), flags);

	auto& mask = (flags & SCMF_PLATFORM) != 0 ? platMask : obstacleMask;
	if (!mask)
	{
		if (collisionMaskByteCount == 0)
		{
			collisionMaskByteCount = Internal_CreateMask(nullptr, flags);
			if (collisionMaskByteCount == 0)
				LOGF(_T("%sCould not create mask with flags %u.\n"), debugID, flags);
		}
#ifdef _DEBUG
		else // collision mask already calculated, was it not reset last time image was resized?
		{
			const std::size_t idiotCheck = Internal_CreateMask(nullptr, flags);
			LOGF(_T("%sMask expired! Size of mask was %zu, now %zu, but size was not updated.\n"), debugID, collisionMaskByteCount, idiotCheck);
		}
#endif

		mask = CollisionMask::CreateWithSize(collisionMaskByteCount);
		if (!mask)
			LOGE(_T("%sOut of memory! Couldn't allocate %zu bytes.\n"), debugID, collisionMaskByteCount);
		const std::size_t resMaskSize = Internal_CreateMask(mask->intern, flags);
		if (resMaskSize != collisionMaskByteCount)
			LOGF(_T("%sMask couldn't be created! Expected return %zu, got %zu.\n"), debugID, collisionMaskByteCount, resMaskSize);
	}
	return mask.get();
}

void DarkEdif::Surface::WasAltered(Rect* zone /* = nullptr */)
{
	wasaltered = true;
	if (!isFrameSurface || !ext)
		return;
#ifdef _WIN32
	std::unique_ptr<Rect> localZone;
	if (!zone)
	{
		if constexpr (true)
		{
			// If scroll dependent, account for scroll offset in WindowX/Y
			const bool scrollDependent = (ext->rdPtr->rHo.hoOEFlags & OEFLAGS::SCROLLING_INDEPENDENT) == OEFLAGS::NONE;

			// rHo XY appears newer than rHo Rect. Rect is still updated after, though,
			// which is what GetZoneInfos is meant to do.

			localZone = std::make_unique<Rect>(
				Point(ext->rdPtr->rHo.hoX - (scrollDependent ? ext->rhPtr->rhWindowX : 0) - ext->rdPtr->rHo.hoImgXSpot,
					ext->rdPtr->rHo.hoY - (scrollDependent ? ext->rhPtr->rhWindowY : 0) - ext->rdPtr->rHo.hoImgYSpot, true),
				GetSize(), true);
			zone = localZone.get();

		}
		else
			zone = (Rect*)&ext->rdPtr->rHo.hoRect;
		int i = memcmp(zone, &ext->rdPtr->rHo.hoRect, sizeof(RECT));
		if (i != 0)
			LOGI(_T("Got a mismatch of display: Rect is %s, X/Y is %s.\n"),
				((Point&)ext->rdPtr->rHo.hoRect).str().c_str(),
				((Point&)*zone).str().c_str());
	}
	// Software display mode does not redraw on screen automatically.
	if (ext->Runtime.GetAppDisplayMode() < SurfaceDriver::Direct3D8)
	{
	//	WinAddZone(Edif::SDK->mV->IdEditWin, (RECT*)zone);
	//	WinAddZone(Edif::SDK->mV->IdEditWin, &ext->rdPtr->rHo.Rect);
	}
#endif
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE

	//WinAddZone(Edif::SDK->mV->IdEditWin, &ext->rdPtr->rHo.Rect);
	//ext->Runtime.Redisplay();

	// if roc exists, update it
	if constexpr ((Extension::OEFLAGS & (OEFLAGS::MOVEMENTS | OEFLAGS::ANIMATIONS | OEFLAGS::SPRITES)) != OEFLAGS::NONE)
	{
		//wasaltered = true;
		// ext->rdPtr->get_roc()->set_changed(true);
		auto pSp = ext->rdPtr->get_ros();
		if (pSp)
		{
		//	ModifSpriteEffect(Edif::SDK->mV->IdEditWin, ext->rdPtr->roc.rcSprite, (DWORD)pSp->Effect, pSp->EffectParam);
		}
		//	MsgBox::Info(_T("!!!"), _T("got rcSprite"));
		//ext->Runtime.Rehandle();
	}
#endif
}

bool DarkEdif::Surface::GetAndResetAltered()
{
	if (!wasaltered)
		return false;
	LOGV(_T("GetAndResetAltered() called with true: should call Display now.\n"));
	wasaltered = false;
#ifdef __ANDROID__
	threadEnv->CallVoidMethod(textSurface, textSurfUpdateTextureMethod);
	JNIExceptionCheck();
#endif
	return true;
}


#if DARKEDIF_DISPLAY_TYPE > DARKEDIF_DISPLAY_ANIMATIONS && defined(_WIN32)
void DarkEdif::Surface::Internal_WinZoneHack()
{
	static std::unique_ptr<Rect> lastZone;
	// Software display mode does not redraw on screen automatically.
	if (ext->Runtime.GetAppDisplayMode() < SurfaceDriver::Direct3D8)
	{
		WinAddZone(Edif::SDK->mV->IdEditWin, &ext->rdPtr->rHo.hoRect);
#if 0
		// If scroll dependent, account for scroll offset in WindowX/Y
		const bool scrollDependent = (ext->rdPtr->rHo.hoOEFlags & OEFLAGS::SCROLLING_INDEPENDENT) == OEFLAGS::NONE;
		Rect curZone(
			Point(
				ext->rdPtr->rHo.hoX - (scrollDependent ? ext->rhPtr->rhWindowX : 0) - ext->rdPtr->rHo.hoImgXSpot,
				ext->rdPtr->rHo.hoY - (scrollDependent ? ext->rhPtr->rhWindowY : 0) - ext->rdPtr->rHo.hoImgYSpot, true),
			GetSize(), true);

		WinAddZone(Edif::SDK->mV->IdEditWin, (RECT*)&curZone);
		//if (lastZone)
		//	WinAddZone(Edif::SDK->mV->IdEditWin, (RECT*)lastZone.get());
		//lastZone = std::make_unique<Rect>(curZone);
#endif
	}
	if (surf->IsTransparent())
		LOGW(_T("Your ext display surface is entirely transparent!\n"));
}
#endif // _WIN32

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
void DarkEdif::Surface::SetAsExtensionDisplay(Extension* ext)
{
	if (this->ext)
		LOGF(_T("%sAlready have an extension.\n"), debugID);
	if ((ext->surf != nullptr && ext->surf != this) || (ext->Runtime.surf && ext->Runtime.surf.get() != this))
	{
		LOGW(_T("%sWARNING: Surface was set as ext display, but ext display is currently %s.\n"),
			debugID, ext->surf->Describe().c_str());
	}
	this->ext = ext;
	if (!hasGeometryCapacity && !hasTextCapacity)
		LOGW(_T("%sWARNING: Surface was set as ext display, but is a hardware surface.\n"), debugID);

	// RECT and ImgHeight/Width is not automatically updated
	// TODO: in Windows, or everywhere?
	const Size imgSize = GetSize();
	ext->rdPtr->get_rHo()->SetSize(imgSize.width, imgSize.height);
}
void DarkEdif::Surface::BlitToFrameWithExtEffects(Point pt /* = Point {}*/)
{
	assert(ext);
	if (!ext->rdPtr->get_roc())
		LOGF(_T("%sUnexpected call to BlitToFrameWithExtEffects(); not an ext that has effects.\n"), debugID);

	HeaderObject* const hoPtr = ext->rdPtr->get_rHo();
	pt.x += hoPtr->get_X();
	pt.y += hoPtr->get_Y();
#ifdef _WIN32
	// Windows does not move with WindowX or WindowY correctly?
	pt.x -= ext->rhPtr->rhWindowX - ext->rdPtr->rHo.hoImgXSpot;
	pt.y -= ext->rhPtr->rhWindowY - ext->rdPtr->rHo.hoImgYSpot;
	int blitMode = 0, blitOptions = 0;
#endif

	int shaderIndex = -1, inkEffectParam = 0;
	BlitOperation blitOp = BlitOperation::Blend;
	const Size srcSize = GetSize(); // every plat takes a dest scale
	RunSpriteFlag rsFlags = RunSpriteFlag::None;
	float xScale = 1.f, yScale = 1.f;
	float angle = 0.f;
	Point hotSpot;
	if constexpr ((Extension::OEFLAGS & OEFLAGS::SPRITES) != OEFLAGS::NONE)
	{
		RunSprite* const rs = ext->rdPtr->get_ros();
		shaderIndex = rs->get_EffectShader();
		blitOp = static_cast<BlitOperation>(rs->get_Effect() & BlitOperation::EffectMask);
		inkEffectParam = rs->get_EffectParam();

		rCom* roc = ext->rdPtr->get_roc();
		angle = roc->GetAngle();
		xScale = roc->get_scaleX();
		yScale = roc->get_scaleY();
		rsFlags = rs->get_Flags();
		HeaderObject* const hoPtr = ext->rdPtr->get_rHo();
		hotSpot = { hoPtr->get_ImgXSpot(), hoPtr->get_ImgYSpot() };

#ifdef _WIN32
		if ((rs->get_Effect() & BlitOperation::Transparent) != BlitOperation::None)
			blitMode = (int)BlitMode::BMODE_TRANSP;
		if ((angle != 0.f || xScale != 1.f || yScale != 1.f) ||
			(rsFlags & RunSpriteFlag::Rotate_Antialias) != RunSpriteFlag::None)
		{
			blitOptions = (int)WinSurf_BlitOptions::AntiAliasing;
		}
#endif
		// TODO: Fusion should auto-wrap this value: if it doesn't we might have to
		if (angle < 0.f || angle > 360.f)
			LOGF(_T("Unexpected angle %f.\n"), (double)angle);

		// Checks for infinity, NaN scale
		if (!isfinite(xScale) || !isfinite(yScale))
			LOGF(_T("Unexpected x/y scale: %f by %f.\n"), static_cast<double>(xScale), static_cast<double>(yScale));

		// If set too low, this can be reset to 0; in this case we'll warn and prevent drawing
		if (xScale == 0.f || yScale == 0.f)
		{
			LOGW(_T("Zero scale (scale %f by %f). Object will not be drawn.\n"), static_cast<double>(xScale), static_cast<double>(yScale));
			return;
		}
	}

#ifdef _WIN32
	cSurface * frameSurf = WinGetSurface((int)ext->rhPtr->rhIdEditWin);
	// Windows does not have a hotspot param, merge it into pt
	pt.x += hotSpot.x;
	pt.y += hotSpot.y;

	// Software display mode does not redraw on screen automatically.
	if (ext->Runtime.GetAppDisplayMode() < SurfaceDriver::Direct3D8)
	{
		Rect localZone(pt, srcSize);
		WinAddZone(Edif::SDK->mV->IdEditWin, (RECT*)&localZone);
		//ext->rdPtr->roc.rcChanged = true;
	}
	// If not a HWA-supporting ext SDK, or display mode is non-HWA:
	// use software Blit() call instead of HWA-runtime BlitEx
#if !FUSION_2_0_DRAWING_ONLY
	if ((SurfaceDriver)frameSurf->GetDriver() < SurfaceDriver::Direct3D8)
#endif
	{
		surf->Blit(*frameSurf,
			// Dest position
			pt.x, pt.y,
			// Source rect
			0, 0, srcSize.width, srcSize.height,
			// Blit mode: opaque or transparent
			(BlitMode)blitMode);
	}
#if !FUSION_2_0_DRAWING_ONLY
	else
	{
		Point center = Rect(pt, srcSize).GetCenter();
		center.x += hotSpot.x;
		center.y += hotSpot.y;

		surf->BlitEx(*frameSurf,
			// Dest position
			static_cast<float>(pt.x),
			static_cast<float>(pt.y),
			// Scale
			xScale, yScale,
			// Source rect
			0, 0, srcSize.width, srcSize.height,
			// Rotation center point + angle
			(POINT *)&center, angle,
			// Blit mode: opaque or transparent
			(BlitMode)blitMode,
			// Blit operation: old style shader, new shader, transparent, etc
			(BlitOp)blitOp,
			// Alpha + RGB blend coefficient
			inkEffectParam,
			// Blit flags: WinSurf_BlitOptions
			static_cast<ULONG>(blitOptions)
		);
	}
#endif // Non-HWA
#elif defined(__ANDROID__)

	// Right now the ctor for user-drawn surface inits this stuff - potentially,
	// if the user does manual display mode, creates a frame surface then tries to use it,
	// these won't have been defined. That's obscure behaviour as they should make a user
	// surface to draw from first, but jic.
	if (!textSurfCtorMethod)
		LOGF(_T("First surface did not init the required Android tech.\n"));

	// TODO: See GetAndResetAltered(), we don't need to update if no edits
	if (!wasaltered)
		LOGV(_T("%sImage not altered, not syncing to texture wrapper.\n"), debugID);
	else
	{
		threadEnv->CallVoidMethod(textSurface, textSurfUpdateTextureMethod);
		JNIExceptionCheck();
		wasaltered = false;
		LOGV(_T("%sImage was altered, synced to texture wrapper.\n"), debugID);
	}
	jobject texture = threadEnv->GetObjectField(textSurface, textSurfTextureFieldID);
	JNIExceptionCheck();

	LOGV(_T("%sRender scaled rotate image start.\n"), debugID);
	// C func call, woo
	renderScaledRotImageFunc(threadEnv,
		renderer, texture, (rsFlags & RunSpriteFlag::Scale_Resample) != RunSpriteFlag::None,
		angle, xScale, yScale, hotSpot.x, hotSpot.y,
		pt.x, pt.y, srcSize.width, srcSize.height, shaderIndex, (int)blitOp, inkEffectParam);
	LOGV(_T("%sRender scaled rotate image end.\n"), debugID);
#else // apple
	CRenderer* crenderer = CRenderer::getRenderer();
	// void renderScaledRotatedImage(CTexture* image, float angle, float sX, float sY,
	// int hX, int hY, int x, int y, int w, int h, int shaderIndex = -1,
	// int inkEffect = 0, int inkEffectParam = -1);
	(void)rsFlags; // TODO: How do we set antialiasing/resampling during this resize on iOS/Mac?
	crenderer->renderScaledRotatedImage(bmp, angle, xScale, yScale, hotSpot.x, hotSpot.y,
		pt.x, pt.y, srcSize.width, srcSize.height, shaderIndex, (int)blitOp, inkEffectParam);
#endif
}

#endif // DARKEDIF_DISPLAY_SIMPLE

#ifdef _WIN32
// Create pure DC surface from DC
void DarkEdif::Surface::CreateFromDC(HDC hDC)
{
	if (!hasTextCapacity)
		return LOGE(_T("Cannot create Surface from DC, text capacity not present.\n"));
	if (isLocked)
		return LOGE(_T("Cannot create Surface from DC, source has locked buffer.\n"));
	if (isLockedAlpha)
		return LOGE(_T("Cannot create Surface from DC, source has locked alpha buffer.\n"));
	if (dcIsOpen)
		return LOGE(_T("Cannot create Surface from DC, source has open DC.\n"));

	// TODO: error details?
	surf->Create(hDC);

	if (surf->IsValid())
		return LOGE(_T("Cannot create Surface from window, unknown error.\n"));
}
// Create pure DC surface from a window
void DarkEdif::Surface::CreateFromWindow(HWND hWnd, bool IncludeFrame)
{
	if (!hasTextCapacity)
		return LOGE(_T("Cannot create Surface from window, text capacity not present.\n"));
	if (isLocked)
		return LOGE(_T("Cannot create Surface from window, source has locked buffer.\n"));
	if (isLockedAlpha)
		return LOGE(_T("Cannot create Surface from window, source has locked alpha buffer.\n"));
	if (dcIsOpen)
		return LOGE(_T("Cannot create Surface from window, source has open DC.\n"));

	if (surf->IsValid())
	{
		LOGV(_T("Deleting original surface to make from window, at %p.\n"), surf);
		surf->Delete();
	}

	// TODO: error details?
	surf->Create(hWnd, IncludeFrame);

	if (surf->IsValid())
		return LOGE(_T("Cannot create from window, unknown error.\n"));
}
// Returns the surface type of this Surface
SurfaceType DarkEdif::Surface::GetType()
{
	const SurfaceType st = (SurfaceType)surf->GetType();
	if (st >= SurfaceType::Max || (int)st < 0)
		LOGF(_T("Unexpected surface type %d.\n"), (int)st);
	return st;
}
// Returns the driver type of this Surface
SurfaceDriver DarkEdif::Surface::GetDriver()
{
	const SurfaceDriver sd = (SurfaceDriver)surf->GetDriver();
	if (sd >= SurfaceDriver::Max || (int)sd < 0)
		LOGF(_T("Unexpected surface driver %d.\n"), (int)sd);
	return sd;
}

DarkEdif::Surface DarkEdif::Surface::CreateFromFrameEditorWindow()
{
#if EditorBuild
	if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor)
	{
		cSurface* newSurf = WinGetSurface((int)Edif::SDK->mV->IdEditWin);
		if (!newSurf || !newSurf->IsValid())
			LOGF(_T("Cannot create Surface from main window, unknown error.\n")); // noreturn, save me
		return Surface(nullptr, newSurf, true);
	}
#endif
	LOGF(_T("Cannot create Surface from editor window, not in editor.\n"));
}
#endif

DarkEdif::Surface& DarkEdif::Surface::CreateFromMainWindow(RunHeader* rhPtr)
{
#ifdef _WIN32
	if (!mainWindow)
	{
		cSurface* newSurf = WinGetSurface((int)Edif::SDK->mV->IdEditWin);
		if (!newSurf || !newSurf->IsValid())
			LOGF(_T("Cannot create Surface from main window, unknown error.\n")); // noreturn, save me
		mainWindow = new Surface(rhPtr, newSurf, true);

		SurfaceDriver driv = mainWindow->GetDriver();
		// Standard display mode
		if (driv >= SurfaceDriver::DIB && driv < SurfaceDriver::Direct3D9)
		{
			// Set this so it's not still 0
			drivInfo.m_lSize = -1;
			drivInfo.m_nD3DVersion = 1 + (int)driv;
			// This is mostly limited by RAM, but Windows standard display is very slow,
			// so we'll set a smaller limit of 4096 for sanity.
			drivInfo.m_dwMaxTextureWidth = drivInfo.m_dwMaxTextureHeight = 4096;
		}
		// Note D3D9 enum value is before D3D8
		else if (driv >= SurfaceDriver::Direct3D9)
		{
			DWORD size = newSurf->GetDriverInfo(NULL);
			if (size > sizeof(drivInfo))
				LOGF(_T("Unexpected driver info size %u.\n"), size);
			drivInfo.m_lSize = size;
			// DDHAL_GETDRIVERINFODATA
			newSurf->GetDriverInfo(&drivInfo);
			LOGI(_T("Got Direct3D driver info: size %u (expected %u). D3D version %i. Context ptr: 0x%p. ")
				"Device ptr: 0x%p. Texture ptr: 0x%p. Pixel Shader version: %i. Vertex Shader version: %i. "
				"Max Texture Width: %i. Max Texture Height: %i. Render Target Texture: 0x%p. Render Target View: 0x%p. "
				"Render Target Context: 0x%p.\n",
				drivInfo.m_lSize, size, drivInfo.m_nD3DVersion, drivInfo.m_pD3DContext,
				drivInfo.m_pD3DDevice, drivInfo.m_ppD3DTexture, drivInfo.m_dwPixelShaderVersion, drivInfo.m_dwVertexShaderVersion,
				drivInfo.m_dwMaxTextureWidth, drivInfo.m_dwMaxTextureHeight, drivInfo.m_ppD3D11RenderTargetTexture, drivInfo.m_ppD3D11RenderTargetView,
				drivInfo.m_txtContext
			);
		}
		else
			LOGF(_T("Unexpected frame window driver %i.\n"), (int)driv);
	}
	return *mainWindow;
#else
	LOGF(_T("Not implemented on this platform\n"));
	return Surface(rhPtr, false, false, 0, 0, false);
#endif
}
std::unique_ptr<DarkEdif::Surface> DarkEdif::Surface::CreateFromFilePath(Extension * ext,
	std::tstring filePath, bool needBitmapFuncs, bool needTextFuncs, bool alpha)
{
	assert(ext);
	const auto filePath2 = DarkEdif::MakePathUnembeddedIfNeeded(ext, filePath);
	if (filePath2[0] == _T('>'))
		return LOGE(_T("Couldn't make surface from path \"%s\", error: %s"), filePath.c_str(), &filePath2[1]), nullptr;

	auto surf = std::make_unique<Surface>(ext->rhPtr, needBitmapFuncs, needTextFuncs, 1, 1, alpha);
	if (ImageFileFormat::Unset != surf->LoadImageFromFilePath(filePath2))
		return surf;

	LOGE(_T("Couldn't init surface from path \"%s\", load failed"), filePath2.c_str());
	return nullptr;
}

#ifdef _WIN32
DarkEdif::Surface * DarkEdif::Surface::FromWindowsSurface(RunHeader * const rhPtr, cSurface * surf)
{
	Surface * ret = new Surface(rhPtr, true,  false, surf->GetWidth(), surf->GetHeight(), surf->HasAlpha());
	if (surf->HasAlpha())
	{
	//	ret->surf->CreateAlpha();
	}
	else if (surf->IsTransparent())
	{
		LOGI(_T("Creating surf with transp color: %s\n"), ColorToString(surf->GetTransparentColor()).c_str());
		ret->surf->SetTransparentColor(surf->GetTransparentColor());
	}

	if (surf->Blit(*ret->surf,0,0) == FALSE)
		LOGF(_T("Cannot create Surface from Windows surface, unknown error.\n"));;

	return ret;
}
#endif

bool DarkEdif::Surface::CheckRectContained(const Rect& r) const {
	r.Validate();
	return r.left >= 0 && r.top >= 0 && (std::size_t)r.right < GetWidth() && (std::size_t)r.bottom < GetHeight();
}
bool DarkEdif::Surface::CheckPointContained(const Point & pt) const {
	return pt.x >= 0 && pt.y >= 0 && pt.x < (std::int32_t)GetWidth() && pt.y < (std::int32_t)GetHeight();
}
int DarkEdif::Surface::IDCounter = 0;
DarkEdif::Surface::Surface(RunHeader* const rhPtr, bool needBitmapFuncs, bool needTextFuncs,
	std::size_t width, std::size_t height, bool alpha) : rhPtr(rhPtr)
{
	IDGen();

	// sanity checks
	if (!rhPtr && DarkEdif::RunMode == DarkEdif::MFXRunMode::BuiltEXE)
		LOGF(_T("Null rhPtr passed to Surface ctor, aborting.\n"));

	if (width == 0 || width > 65000 || height == 0 || height > 65000)
		LOGF(_T("Image being allocated of invalid size %s, aborting.\n"), Size{ (int) width, (int)height }.str().c_str());

#ifdef _WIN32
	// 24-bit depth even for alpha, as Fusion does not use alpha channel in an ARGB surface,
	// but creates two surfaces, RGB and A.
	// Direct3D docs refer to this as B8G8R8 format.
	int depth = 24;

	SurfaceType st;
	SurfaceDriver sd = SurfaceDriver::Bitmap;
	if (needTextFuncs)
		st = SurfaceType::Memory_DeviceContext;
	else if (needBitmapFuncs)
		st = SurfaceType::Memory;
	else
	{
		st = SurfaceType::HWA_ManagedTexture;
		cSurface* main = WinGetSurface((int)Edif::SDK->mV->IdEditWin);
		sd = (SurfaceDriver)main->GetDriver();

		// Although Fusion doesn't use the alpha of 32-bit ARGB surfaces,
		// 24-bit depth surfaces cannot be made in Direct3D, so we must upgrade.
		// Direct3D docs refer to this as X8B8G8R8 format.

		// Note D3D9 enum value is before D3D8
		if (sd >= SurfaceDriver::Direct3D9)
			depth = 32;
		// Standard display mode, DirectDraw, does not do HWA textures
		else if (sd == SurfaceDriver::DIB)
			st = SurfaceType::Memory;

		// If set, check texture size, if not, warn we can't
		if (!drivInfo.m_dwMaxTextureWidth)
		{
			LOGW(_T("Warning: Creating a HWA surface of size %s. Unsure what max D3D texture size is.\n"),
				Size { (int)width, (int)height }.str().c_str());
		}
		else if (width > (std::size_t)drivInfo.m_dwMaxTextureWidth || height > (std::size_t)drivInfo.m_dwMaxTextureHeight)
		{
			LOGF(_T("Creating a surface too large. Your GPU is limited to %s, attempting to allocate %s. Lower the graphics quality.\n"),
				Size { drivInfo.m_dwMaxTextureWidth, drivInfo.m_dwMaxTextureHeight }.str().c_str(), Size { (int)width, (int)height }.str().c_str());
		}
	}

	cSurface* proto;
	if (GetSurfacePrototype(&proto, depth, (int)st, (int)sd) == FALSE)
		LOGF(_T("Couldn't get surface prototype (%d, %d, %d).\n"), depth, (int)st, (int)sd);
	surf = new cSurface();
	surf->Create(width, height, proto);
	if (!surf->IsValid())
		LOGF(_T("Couldn't create surface prototype (%d, %d, %d), size (%zu x %zu).\n"), depth, (int)st, (int)sd, width, height);

	if (!alpha)
		surf->SetTransparentColor(RGB(255, 0, 255)); // magneta by default
	else
	{
		// Creates either an 8-bit array, or Direct3D ii format surface. Alpha, 8 bits per pixel.
		surf->CreateAlpha();
		cSurface * alpha = surf->GetAlphaSurface();
		if (!alpha || !alpha->Fill((COLORREF)0xFFFFFFFF))
			LOGF(_T("Couldn't fill alpha to opaque.\n"));
		surf->ReleaseAlphaSurface(alpha);
	}

	hasGeometryCapacity = needBitmapFuncs;
	hasTextCapacity = needTextFuncs;

	ownsSurface = true;
	if (depth == 24)
		format = PixelFormat::BGR;
	else if (depth == 32)
		format = PixelFormat::XBGR;
	else
		LOGF(_T("Unrecognized pixel depth %i\n"), depth);
	// TODO: We can support unmanaged textures and save RAM, if we implement
	// the lost device callback and reload the image. For now, let's leave it.
#elif defined(__ANDROID__)
	textSurfaceClass = global(threadEnv->FindClass("OpenGL/CTextSurface"), "CTextSurface class inside DarkEdif::Surface");
	bmpClass = global(threadEnv->FindClass("android/graphics/Bitmap"), "Android graphics Bitmap class inside DarkEdif::Surface");

	if (!textSurfCtorMethod)
	{
		textSurfCtorMethod = threadEnv->GetMethodID(textSurfaceClass, "<init>", "(LApplication/CRunApp;II)V");
		JNIExceptionCheck();
		textSurfUpdateTextureMethod = threadEnv->GetMethodID(textSurfaceClass, "updateTexture", "()V");
		JNIExceptionCheck();
		textSurfDrawMethod = threadEnv->GetMethodID(textSurfaceClass, "draw", "(IIIII)V");
		JNIExceptionCheck();
		textSurfBitmapFieldID = threadEnv->GetFieldID(textSurfaceClass, "textBitmap", "Landroid/graphics/Bitmap;");
		JNIExceptionCheck();
		textSurfTextureFieldID = threadEnv->GetFieldID(textSurfaceClass, "textTexture", "LOpenGL/CTextSurface$CTextTexture;");
		JNIExceptionCheck();
		bitmapEraseMethod = threadEnv->GetMethodID(bmpClass, "eraseColor", "(I)V");
		JNIExceptionCheck();
		bitmapGetWidthMethod = threadEnv->GetMethodID(bmpClass, "getWidth", "()I");
		JNIExceptionCheck();
		bitmapGetHeightMethod = threadEnv->GetMethodID(bmpClass, "getHeight", "()I");
		JNIExceptionCheck();
		// TODO: I don't think SurfaceView ever changes the inst, but it's worth confirming
		// if app switch away + back, or new Fusion frame can kill it
		const jclass glRendClass = threadEnv->FindClass("OpenGL/GLRenderer");
		JNIExceptionCheck();
		const jfieldID rendField = threadEnv->GetStaticFieldID(glRendClass, "inst", "LOpenGL/GLRenderer;");
		JNIExceptionCheck();
		jobject rend = threadEnv->GetStaticObjectField(glRendClass, rendField);
		JNIExceptionCheck();
		renderer = global(rend, "GLRenderer static inst");
		const jclass surfViewClass = threadEnv->FindClass("Runtime/SurfaceView");
		JNIExceptionCheck();
		const jfieldID verFieldID = threadEnv->GetStaticFieldID(surfViewClass, "ES", "I");
		JNIExceptionCheck();
		const jint rendESLevel = threadEnv->GetStaticIntField(surfViewClass, verFieldID);
		JNIExceptionCheck();
		isOpenGL2Or3 = rendESLevel >= 2 && rendESLevel <= 3;
		if (rendESLevel < 1 || rendESLevel > 3)
			LOGF(_T("Unexpected OpenGL level %d.\n"), rendESLevel);
		renderScaledRotImageFunc = isOpenGL2Or3 ?
			&Java_OpenGL_ESxRenderer_renderScaledRotatedImage :
			&Java_OpenGL_ES1Renderer_renderScaledRotatedImage;
	}

	jobject textSurf = threadEnv->NewObject(textSurfaceClass, textSurfCtorMethod,
		rhPtr->get_App()->me.ref, width, height);
	JNIExceptionCheck();
	textSurface = global(textSurf, "CTextSurface obj inside DarkEdif::Surface");

	jobject bitmap = threadEnv->GetObjectField(textSurface, textSurfBitmapFieldID);
	JNIExceptionCheck();
	bmp = global(bitmap, "Android Bitmap object inside CTextSurface inside DarkEdif::Surface");

	format = PixelFormat::ABGR;
	hasGeometryCapacity = needBitmapFuncs;
	hasTextCapacity = needTextFuncs;
#else // apple
	//img = [CImage alloc];
	//[img initWithWidth: (int)width andHeight : (int)height] ;

	// Create a CRenderToTexture
	int depth = 32;// alpha ? 32 : 24;
	bmp = [CRenderToTexture alloc];
	[bmp initWithWidth:(int)width andHeight:(int)height andRunApp:(CRunApp *)rhPtr->get_App()
		 andSwapMode:false andPOTMode:false andDepth:depth
	];
	//img = (CImage *)bmp;

	hasGeometryCapacity = needBitmapFuncs;
	hasTextCapacity = needTextFuncs;
	// While Fusion allocates a CGBitmap with flags that say to ignore the alpha, it
	// uses the bytes as if an alpha is stored there.
	// Colors are swapRGB()'d from BGR to RGB layout.
	if (depth == 32)
		format = PixelFormat::ABGR;
	else
		format = PixelFormat::BGR;
#endif

	LOGI(_T("%sCreated a surface, describing as \"%s\".\n"), debugID, Describe().c_str());
}
bool DarkEdif::Surface::Clone(Surface& dest, CloneType type)
{
	if (&dest == this)
		return LOGF(_T("Cannot clone to self.\n")), false;

	LOGV(_T("Cloning surface from %p (%s) to %p (%s).\n"), this, Describe().c_str(), &dest, dest.Describe().c_str());
	if (dest.hasGeometryCapacity != hasGeometryCapacity)
		return LOGE(_T("Cannot clone surface, different geometry capacities.\n")), false;
	if (dest.hasTextCapacity != hasTextCapacity)
		return LOGE(_T("Cannot clone surface, different text capacities.\n")), false;
	if (dest.isLocked)
		return LOGE(_T("Cannot clone surface, destination has locked buffer.\n")), false;
	if (isLocked)
		return LOGE(_T("Cannot clone surface, source has locked buffer.\n")), false;
	if (dest.isLockedAlpha)
		return LOGE(_T("Cannot clone surface, destination has locked alpha buffer.\n")), false;
	if (isLockedAlpha)
		return LOGE(_T("Cannot clone surface, source has locked alpha buffer.\n")), false;
	if (dest.dcIsOpen)
		return LOGE(_T("Cannot clone surface, destination has open DC.\n")), false;
	if (dcIsOpen)
		return LOGE(_T("Cannot clone surface, source has open DC.\n")), false;

	if (type == CloneType::DriverTypeOnly)
	{
#ifdef _WIN32
		cSurface* proto = nullptr;
		if (GetSurfacePrototype(&proto, surf->GetDepth(), surf->GetType(), surf->GetDriver()) == FALSE)
			return LOGE(_T("%sCannot clone surface, prototype lookup failed for %s.\n"), debugID, Describe().c_str()), false;
		if (dest.surf->IsValid())
			dest.surf->Delete();
		dest.surf->Create(1, 1, proto);
#else
		LOGF(_T("Not implemented on this platform.\n"));
#endif
		return true;
	}
	if (type == CloneType::AlphaOnly)
	{
		if (!HasAlpha())
			return LOGE(_T("%sCannot clone surface alpha channel, original surface has no alpha channel (orig: %s)."),
				debugID, Describe().c_str()), false;
#ifdef _WIN32
		dest.surf->Blit(*dest.surf, 0, 0, BlitMode::BMODE_TRANSP,
			(BlitOp)BlitOperation::BlendDontReplaceColor);
#else
		LOGF(_T("Cannot clone surface, not implemented on this platform\n"));
#endif
	}

#ifdef _WIN32
	if (surf->Blit(*dest.surf) == FALSE)
		return LOGE(_T("%sCloning surface failed, strangely.\n"), debugID), false;
#else
	LOGF(_T("Cannot clone surface, not implemented on this platform\n"));
#endif

	return true;
}
bool DarkEdif::Surface::CloneSection(Surface& dest, CloneType type, Rect rect)
{
	if (&dest == this)
		return LOGF(_T("Cannot clone to self.\n")), false;

	LOGF(_T("Cannot clone surface, not implemented on this platform\n"));
	return false;
}

static bool NoAlpha(const TCHAR* const func, const std::uint32_t color)
{
	if ((color >> 24) != 0U && (color >> 24) != 0xFFU)
		return LOGE(_T("%s: Passing a colour with an alpha channel.\n"), func), false;
	return true;
}

std::tstring DarkEdif::Surface::Describe() const {

	static const TCHAR* const formatStr[] = {
		// -2, -1
		_T("Monochrome (1bpp)"), _T("Alpha (1bpp)"),
		// 0
		_T("Unset (?bpp)"),
		_T("RGBA (32bpp)"), _T("RGBX (32bpp)"), _T("RGB (24bpp)"),
		_T("BGR (24bpp)"), _T("XBGR (32bpp)"),  _T("BGRA (32bpp)"), _T("ABGR (32bpp)") };
	const auto RoundUpToNearestMultipleOf = [](std::size_t from, std::size_t multi) {
		return (std::size_t)((((int)from) + ((int)multi) - 1) & (-(int)multi));
	};
	std::size_t sizeBytes = GetWidth();
	if (format == PixelFormat::Monochrome)
		sizeBytes = RoundUpToNearestMultipleOf(sizeBytes, 8u) * GetHeight();
	else if (format == PixelFormat::Alpha)
		sizeBytes = RoundUpToNearestMultipleOf(sizeBytes, 16u) * GetHeight();
	else if (format == PixelFormat::Unspecified)
	{
		if (sizeBytes != 0)
			LOGF(_T("Unexpected Surface state, format should be set here\n"));
	}
	else if (format == PixelFormat::RGB || format == PixelFormat::BGR)
		sizeBytes *= GetHeight() * 3;
	else if (format == PixelFormat::XBGR || format == PixelFormat::RGBX || format == PixelFormat::RGBA ||
			format == PixelFormat::ABGR || format == PixelFormat::RGBA)
		sizeBytes *= GetHeight() * 4;
	else
		return LOGF(_T("Unexpected Surface state, format should be set here\n")), std::tstring();

	std::tstringstream result;
	result << _T("Surface at native "sv) << std::hex;
#ifdef _WIN32
	result << _T("cSurface 0x"sv) << (void*)surf;
#elif defined(__ANDROID__)
	result << _T("Java BMP ref 0x"sv) << (void *)textSurface.ref;
#else // apple
	result << _T("CBitmap 0x"sv) << (void*)bmp; // << _T(", CImage 0x") << (void*)img;
#endif
	result << _T(". Image ("sv) << GetSize()
		<< _T("), "sv) << std::dec << sizeBytes << _T(" bytes, "sv);
	if (format == PixelFormat::Alpha)
		result << _T("alpha."sv);
	else
	{
		result << _T("alpha: "sv) << (HasAlpha() ? _T("YES"sv) : _T("NO"sv))
			<< _T("; bitmap: "sv) << (hasGeometryCapacity ? _T("YES"sv) : _T("NO"sv))
			<< _T(", text: "sv) << (hasTextCapacity ? _T("YES"sv) : _T("NO"sv))
			<< _T('.');
	}
	static_assert((int)PixelFormat::Monochrome == -2);
	result << _T(' ') << formatStr[((std::size_t)format) + 2]; // Monochrome is -2
	if (HasAlpha() && (format == PixelFormat::XBGR || format == PixelFormat::RGBX))
		result << _T("+A (8bpp)"sv);
	return result.str();
}

std::size_t DarkEdif::Surface::GetWidth() const {
#ifdef _WIN32
	return (std::size_t)surf->GetWidth();
#elif defined (__ANDROID__)
	const jint width = threadEnv->CallIntMethod(bmp, bitmapGetWidthMethod);
	JNIExceptionCheck();
	return width;
#else // apple
	return bmp->width;
#endif
}
std::size_t DarkEdif::Surface::GetHeight() const {
#ifdef _WIN32
	return (std::size_t)surf->GetHeight();
#elif defined (__ANDROID__)
	const jint height = threadEnv->CallIntMethod(bmp, bitmapGetHeightMethod);
	JNIExceptionCheck();
	return height;
#else // apple
	return bmp->height;
#endif
}
std::size_t DarkEdif::Surface::GetDepth() const {
#ifdef _WIN32
	return (std::size_t)surf->GetDepth();
#elif defined (__ANDROID__)
	// TODO: we could check bmp's Bitmap.Config?
	return 32;
#else // apple
	// TODO: Confirm depth is not facaded
	return 32;
#endif
}
bool DarkEdif::Surface::SetTransparencyToColor(std::uint32_t color) {
	NoAlpha(_T(__FUNCTION__), color);
	if (HasAlpha())
		LOGE(_T("Setting transparency to color will be ignored, already has an alpha channel.\n"));
#ifdef _WIN32
	surf->SetTransparentColor(color);
	return !HasAlpha() && surf->GetTransparentColor() == color;
#else
	LOGF(_T("Cannot set transparency, not implemented on this platform\n"));
	return false;
#endif
}
void DarkEdif::Surface::SetTransparencyToAlpha() {
	if (HasAlpha())
		return LOGW(_T("Setting transparency to alpha, but already alpha.\n"));
#ifdef _WIN32
	if (surf->IsTransparent())
		LOGI(_T("Creating transparency based on existing transparent color %s.\n"),
			ColorToString(surf->GetTransparentColor()).c_str());
#else
	LOGF(_T("Cannot set transparency to alpha; not implemented on this platform\n"));
	if (false)
		;
#endif
	else
		LOGI(_T("Creating opaque alpha channel.\n"));
#ifdef _WIN32
	surf->CreateAlpha();
#else
	LOGF(_T("Cannot set transparency to alpha; not implemented on this platform\n"));
#endif
}
bool DarkEdif::Surface::HasAlpha() const {
#ifdef _WIN32
	return surf->HasAlpha() != FALSE;
#elif defined (__ANDROID__)
	// TODO: Look up alpha presence by Bitmap.Config? It CAN be erased.
	return true;
#else // apple
	// TODO: Look up alpha presence by CGContext color format?
	return true;
#endif
}
int DarkEdif::Surface::GetAlphaPitch() const
{
	throw std::runtime_error("Not implemented");
}
void DarkEdif::Surface::SetAlpha(const std::byte* alphaBytes, int alphaPitch)
{
	throw std::runtime_error("Not implemented");
}
void DarkEdif::Surface::AttachAlpha(const std::byte* alphaBytes, int alphaPitch)
{
	throw std::runtime_error("Not implemented");
}
std::byte* DarkEdif::Surface::DetachAlpha(int* alphaPitch)
{
	throw std::runtime_error("Not implemented");
}
DarkEdif::Holder<DarkEdif::Surface *, DarkEdif::Surface> DarkEdif::Surface::GetAlphaSurface()
{
#ifdef _WIN32
	Surface * s = new Surface(rhPtr, surf->GetAlphaSurface());
	s->format = PixelFormat::Alpha;
	return Holder<Surface*, Surface>(s, this,
		[](Surface* orig, Surface* alpha) {
		orig->surf->ReleaseAlphaSurface(alpha->surf);
		delete alpha;
	});
#else
	LOGF(_T("Cannot get alpha surface; not implemented on this platform\n"));
	return Holder<Surface*, Surface>(nullptr, nullptr,
		[](Surface* orig, Surface* alpha) {
#ifdef _WIN32
			orig->surf->ReleaseAlphaSurface(alpha->surf);
#else
			LOGF(_T("Cannot release alpha surface; not implemented on this platform\n"));
#endif
			delete alpha;
	});
#endif
}

std::tstring DarkEdif::Surface::ColorToString(std::uint32_t c, bool alpha /* = false*/) {
	TCHAR color[40];
	int s;
	if (alpha)
		s = _stprintf_s(color, std::size(color), _T("RGB(R=%d,G=%d,B=%d)"),
			(int)((c & 0xFF0000) >> 16), (int)((c & 0xFF00) >> 8), (int)(c & 0xFF));
	else
	{
		if ((c >> 24) != 0 && (c >> 24) != 0xFF)
			LOGW(_T("Printing color string for %x, which includes alpha.\n"), c);
		s = _stprintf_s(color, std::size(color), _T("ARGB(%d,%d,%d,%d)"),
			(int)((c & 0xFF000000U) >> 24U), (int)((c & 0xFF0000) >> 16), (int)((c & 0xFF00) >> 8), (int)(c & 0xFF));
	}
	if (s <= 0)
	{
		LOGE(_T("Failed to color print, error %d.\n"), errno);
		return _T("<error print color>"s);
	}
	return std::tstring(color);
}

DarkEdif::Surface::~Surface()
{
#if defined(_WIN32) && defined(_DEBUG)
	if (*(std::uint8_t*)&isLocked == 0xDD)
		LOGF(_T("Invalid surface 0x%p, already freed.\n"), this);
#endif
	if (isLocked)
		LOGF(_T("%sDestroying surface at %s, but has locked buffer.\n"), debugID, Describe().c_str());
	if (isLockedAlpha)
		LOGF(_T("%sDestroying surface at %s, but has locked alpha buffer.\n"), debugID, Describe().c_str());
	if (dcIsOpen)
		LOGF(_T("%sDestroying surface at %s, but has open DC.\n"), debugID, Describe().c_str());

#ifdef _WIN32
	if (ownsSurface)
	{
		surf->Delete();
		delete surf; // DeleteSurface(surf);
	}
	LOGV(_T("%sDealloced surface.\n"), debugID);
	surf = nullptr;
#elif defined(__ANDROID__)
	// Should auto-free the global refs and delete the object by GC
#else
	[bmp release];
	bmp = nullptr;
#endif
}
bool DarkEdif::Surface::CopySection(Rect srcRect, Surface& dest, Rect destRect,
	bool transparent /* = true */,
	BlitOptions blitFlags /* = BlitOptions::CopyAlpha */,
	BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint8_t blitOpParam /* = 0 */) const
{
	// TODO: Does this work if source and/or dest are locked? DC open?

	if (blitOpParam != 0 && blitOp == BlitOperation::Copy)
		LOGW(_T("Passed blit param %d but blit operation won't use it.\n"), (int)blitOpParam);

	if (CheckRectContained(srcRect))
		LOGW(_T("Source rectangle %s not within surface range %s.\n"), srcRect.ToAreaString().c_str(),
			Rect { GetSize() }.ToAreaString().c_str());

	// TODO: I'm not entirely sure what BlitMode transparent + no copy alpha does, or what it does if
	// an alpha is or isn't present. Does it use source transparent settings, do destination, use source
	//to interpret and then convert the transparency on destination without modifying alpha?

	// TODO: If non-HWA, convert blitOpParam to 0-128 range?

	// TODO: there is a Blit function with exact same functionality; I'll have to check if there's any
	// subtle functional difference, like limitations on surface type
#ifdef _WIN32
	const bool b = surf->Stretch(*dest.surf, destRect.left, destRect.top, destRect.Width(), destRect.Height(),
		srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(),
		transparent ? BlitMode::BMODE_TRANSP : BlitMode::BMODE_OPAQUE,
		(BlitOp)blitOp, blitOpParam, (ULONG)blitFlags) != false;
	if (b)
		dest.WasAltered(&destRect);
	return b;
#else
	LOGF(_T("Cannot copy section of surface; not implemented on this platform.\n"));
	return false;
#endif
}


void DarkEdif::Surface::SetPixel(Point pt, std::uint32_t color, bool includeAlpha /* = true */)
{
	// Might be non geo capacity
	if (!hasGeometryCapacity)
		LOGE(_T("%sSetting pixel in a non-software surface?\n"), debugID);
#ifdef _WIN32
	surf->SetPixel(pt.x, pt.y, (COLORREF)color);
	WasAltered(new Rect(pt, Size{ 1, 1 }));
	if (includeAlpha && !surf->HasAlpha())
#else
	LOGF(_T("Cannot set pixel; not implemented on this platform\n"));
	if (false)
#endif
		LOGE(_T("%sInclude alpha is set, and no alpha channel in the surface.\n"), debugID);
	// TODO: This likely does not set alpha, do we have to do a whole lock alpha, get the alpha byte, swap it out?
}
std::uint32_t DarkEdif::Surface::GetPixel(Point pt, bool includeAlpha /* = true */) const
{
	if (!CheckPointContained(pt))
		return LOGE(_T("%sGetting pixel at %s, which is outside the surface bounds of %s.\n"),
			debugID, pt.str().c_str(), GetBounds().ToCoordString().c_str()), -1;
	// Might be non geo capacity
	if (!hasGeometryCapacity)
		LOGE(_T("%sGetting pixel in a non-software surface?\n"), debugID);
	std::uint32_t color;
#ifdef _WIN32
	if (!surf->GetPixel(pt.x, pt.y, (COLORREF &)color))
		return LOGE(_T("%sGetting pixel at %s failed.\n"), debugID, pt.str().c_str()), -1;
	if (includeAlpha && !surf->HasAlpha())
	{
		color |= 0xFF000000; // TODO: Confirm alpha is 0xFF = full opaque
		LOGE(_T("%sGetting pixel error; include alpha is set, and no alpha channel in the surface. %s.\n"),
			debugID, Describe().c_str());
	}
#else
	color = 0;
	LOGF(_T("%sCannot get pixel; not implemented on this platform\n"), debugID);
#endif
	// TODO: This likely does not get alpha, do we have to do a whole lock alpha, get the alpha byte?
	return color;
}

// Copies this entire surface to destination, overwriting RGB, alpha/transparency, and size.
bool DarkEdif::Surface::CopyTo(Surface& dest) const
{
	if (&dest == this)
		return LOGF(_T("%sCannot copy to self.\n"), debugID), false;

	throw std::runtime_error("Not implemented");
	dest.WasAltered();
}
// Copies this entire surface to destination rectangle, clipping if necessary.
bool DarkEdif::Surface::CopyToPoint(Surface& dest, const Point topLeft) const
{
	if (&dest == this)
		return LOGF(_T("%sCannot copy to self.\n"), debugID), false;

	const Rect bounds = GetBounds();
	return CopyWithRotate(dest, PointF(topLeft), bounds, bounds.GetCenter(), 0.f, PointF { 1.f, 1.f }, HasAlpha());
}

bool DarkEdif::Surface::CopyWithRotate(Surface& dest, PointF destPoint,
	const Rect src, Point rotateCenter, const float angle, const PointF & scale,
	const bool blitTransparent /* = false */, const BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint32_t blitOpParam /* = 0 */, const BlitOptions blitFlags /* = BlitOptions::None */) const
{
	if (&dest == this)
		LOGF(_T("Self blit is forbidden.\n"));

	bool ret = false;
#ifdef _WIN32

#if !FUSION_2_0_DRAWING_ONLY
	// Not HWA: BlitEx not supported
	if (dest.IsHWA())
	{
		const SurfaceType surfType = (SurfaceType)dest.surf->GetType();

		// HWA rotation point is not offset against the dest point,
		// so dest point ends up not being top-left as in Blit(),
		// ... unless rotation center is (0, 0), not the intended
		// center of the drawn image.
		destPoint.x += rotateCenter.x;
		destPoint.y += rotateCenter.y;

		// Alpha channel must exist on destination before you blit to it
		if (surf->HasAlpha() && !dest.HasAlpha())
		{
			LOGW(_T("%sDestination surface %s does not have alpha, but copying alpha to it. Creating an alpha on destination.\n"),
				debugID, dest.Describe().c_str());
			dest.surf->CreateAlpha();
			if (!dest.HasAlpha())
				LOGF(_T("%sFailed to create alpha channel on destination surface.\n"), debugID);
		}

		// BlitEx to HWA
		ret = surf->BlitEx(*dest.surf, destPoint.x, destPoint.y, scale.x, scale.y, src.left, src.top, src.Width(), src.Height(),
			(POINT*)&rotateCenter, angle, blitTransparent ? BlitMode::BMODE_TRANSP : BlitMode::BMODE_OPAQUE,
			(BlitOp)blitOp, blitOpParam, (ULONG)blitFlags) != FALSE;
	}
	else // not HWA
#endif // !FUSION_2_0_DRAWING_ONLY
	{
		// TODO: Check float equality with epsilon?
		// e.g. A == B is (A - B < -FLT_EPSILON)
		// May also need FLT_MIN

		// Software blit does not support angle/scaling, get mad
		// Surface Object may have a way?
		if (angle != 0.f || scale.x != 1.f || scale.y != 1.f)
			return LOGE(_T("Can't blit with angle != 0 or scale != 1.0, destination surface is not HWA and BlitEx is not supported.\n")), false;

		ret = surf->Blit(*dest.surf, (int)destPoint.x, (int)destPoint.y,
			(BlitMode)blitTransparent, (BlitOp)blitOp, blitOpParam, (ULONG)blitFlags) != FALSE;
	}
#else
	throw std::runtime_error("Not implemented");
#endif
	// TODO: calculate rotated rectangle size?
	if (ret)
		dest.WasAltered();
	else // Runtime doesn't provide error info
		LOGE(_T("Blit from %s to %s failed.\n"), Describe().c_str(), dest.Describe().c_str());
	return ret;
}

// Copies a rectangle from its point to the destination coordinate.
bool DarkEdif::Surface::CopySectionInternally(Rect src, Point destTopLeft)
{
	if (!CheckRectContained(src))
		LOGE(_T("!\n"));
	throw std::runtime_error("Not implemented");
	Rect dstRect(destTopLeft, src.GetSize());
	WasAltered(&dstRect);
}

bool DarkEdif::Surface::ReverseImage(bool x)
{
	if (!hasGeometryCapacity && !hasTextCapacity)
		LOGW(_T("%sWarning: Reversing HWA image involves a software level duplicate.\n"), debugID);

#ifdef _WIN32
	if ((x && surf->ReverseX() != FALSE) || (!x && surf->ReverseY() != FALSE))
		return LOGE(_T("%sReversing image %s failed.\n"), debugID, x ? _T("X") : _T("Y")), false;
#else
	LOGF(_T("%sCannot reverse image; not implemented on this platform\n"), debugID);
#endif
	WasAltered();
	return true;
}

bool DarkEdif::Surface::ReverseImageSection(bool x, const Rect& rect)
{
	if (!CheckRectContained(rect))
	{
		return LOGE(_T("%sReversing section of image %s failed; rectangle %s outside bounds of %s.\n"),
			debugID, x ? _T("X") : _T("Y"), rect.ToCoordString().c_str(), GetBounds().ToCoordString().c_str()), false;
	}
	if (!hasGeometryCapacity && !hasTextCapacity)
		LOGW(_T("%sWarning: Reversing HWA image involves a software level duplicate.\n"), debugID);

#ifdef _WIN32
	if ((x && surf->ReverseX(rect.left, rect.top, rect.Width(), rect.Height()) != FALSE) ||
		(!x && surf->ReverseY(rect.left, rect.top, rect.Width(), rect.Height()) != FALSE))
	{
		return LOGE(_T("%sReversing image %s failed.\n"), debugID, x ? _T("X") : _T("Y")), false;
	}
#else
	LOGF(_T("Cannot reverse image; not implemented on this platform\n"));
#endif
	WasAltered();
	return true;
}

DarkEdif::Rect DarkEdif::Surface::GetNonTransparentRect()
{
	Rect rect;
#ifdef _WIN32
	// The only thing that can fail is the entire surface is fully transparent
	if (!surf->GetMinimizeRect((RECT*)&rect))
		rect = { 0, 0, 0, 0 }; // TODO: Is this necessary?
#else
	LOGF(_T("%sCannot get minimized rect; not implemented on this platform\n"), debugID);
#endif
	return rect;
}

bool DarkEdif::Surface::RemoveTransparentEdges(Rect * oldRect /* = nullptr */)
{
#ifdef _WIN32
	// The only thing that can fail is the entire surface is fully transparent
	return surf->Minimize((RECT*)&oldRect) != FALSE;
#else
	LOGF(_T("%sCannot remove transparent images; not implemented on this platform\n"), debugID);
	return false;
#endif
}

bool DarkEdif::Surface::FillImageWith(const SurfaceFill& sf)
{
	if (sf.fillType == SurfaceFill::FillType::Flat)
	{
#ifdef _WIN32
		if (surf->Fill((COLORREF)sf.solid.color) == FALSE)
			return LOGE(_T("%sCould not fill with color %s.\n"), debugID, ColorToString((int)sf.solid.color, true).c_str()), false;
		if (surf->HasAlpha())
		{
			const std::uint8_t alphaByte = sf.solid.color >> 24;
			// A color with full transparency is very likely incorrect
			if (!alphaByte && sf.solid.color != 0)
				LOGF(_T("Unexpected fully-transparent color passed to SurfaceFill.\n"));
			const std::size_t height = surf->GetHeight();
			LONG alphaPitch;
			LPBYTE alphaBytes = surf->DetachAlpha(&alphaPitch);
			memset(alphaBytes, alphaByte, std::abs(alphaPitch) * height);
			surf->AttachAlpha(alphaBytes, alphaPitch);
		}
		else
			NoAlpha(_T(__FUNCTION__), sf.solid.color);
#elif defined(__ANDROID__)
		// Overwrites alpha as well
		threadEnv->CallVoidMethod(bmp, bitmapEraseMethod, sf.solid.color);
		JNIExceptionCheck();
#else // apple
		// TODO: Alpha?
		[bmp fillWithColor: sf.solid.color];
#endif
		WasAltered();
		return true;
	}
	else
		LOGE(_T("%sFill type %i not implemented.\n"), debugID, (int)sf.fillType);
	return false;
}

bool DarkEdif::Surface::FillSectionWith(Rect section, const SurfaceFill& fillData)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&section);
}

bool DarkEdif::Surface::DrawEllipse(Rect r, std::size_t thickness /* = 1 */,
	std::uint32_t outlineColor /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&r);
}

bool DarkEdif::Surface::DrawFilledEllipse(Rect r, std::uint32_t fillColor,
	std::size_t thickness /* = 0 */,
	std::uint32_t crOutl /* = BLACK */, bool Fill /* = true */)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&r);
}

bool DarkEdif::Surface::DrawRectangle(Rect r, std::size_t outlineThickness /* = 1 */,
	std::uint32_t outlineColor /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&r);
}

bool DarkEdif::Surface::DrawFilledRectangle(Rect r, std::uint32_t fillColor,
	std::size_t outlineThickness /* = 0 */,
	std::uint32_t crOutl /* = GetRGB(0, 0, 0) */)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&r);
}

bool DarkEdif::Surface::DrawPolygon(Point* pts, std::size_t nPts,
	std::size_t thickness /* = 1 */, std::uint32_t outlineColor /* = 0 */)
{
	throw std::runtime_error("Not implemented");

	// TODO: Calculate min(pts), max(pts), pass as bounds here
	// This may only be necessary on Windows
	// WasAltered(&r);
}

bool DarkEdif::Surface::DrawFilledPolygon(Point* pts, std::size_t nPts,
	std::uint32_t fillColor, std::size_t thickness /* = 0 */,
	std::uint32_t crOutl /* = 0 */, bool fill /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Calculate min(pts), max(pts), pass as bounds here
	// This may only be necessary on Windows
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// WasAltered(&r);
}

bool DarkEdif::Surface::DrawLine(Point a, Point b,
	std::size_t thickness /* = 1 */, std::uint32_t outlineColor /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// TODO: May have to check a and b order
	Rect dstRect(a, b);
	WasAltered(&dstRect);
}

// Note: has no effect in HWA, if thickness is greater than 1.
bool DarkEdif::Surface::DrawLine(Point a, Point b, std::size_t thickness, SurfaceFill::LineFlags lf, bool antiAliasing,
	bool blitTransparent /* = false */, BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// TODO: May have to check a and b order
	Rect dstRect(a, b);
	WasAltered(&dstRect);
}

// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...)
// ==============================================================================

bool DarkEdif::Surface::DrawEllipseAdvanced(Rect& pos, std::size_t thickness, SurfaceFill& fdOutl,
	bool antiAliasing /* = false */, bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawEllipse(Rect& pos, SurfaceFill& fdFill,
	bool antiAliasing /* = false */, bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawEllipse(Rect& pos, SurfaceFill& fdFill,
	std::size_t thickness, SurfaceFill & fdOut, bool antiAliasing /* = false */,
	bool blitTransparent /* = false */, BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint32_t param /* = 0 */, bool Fill /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawRectangle(Rect& pos, std::size_t thickness, SurfaceFill & fill,
	bool antiAliasing /* = false */, bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawRectangle(Rect& pos, SurfaceFill& fill,
	bool antiAliasing /* = false */, bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawRectangle(Rect pos, SurfaceFill& fill, std::size_t thickness,
	SurfaceFill& outl, bool antiAliasing /* = false */,
	bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */,
	bool dofill /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	WasAltered(&pos);
}

bool DarkEdif::Surface::DrawPolygon(Point* pts, std::size_t nPts, std::size_t thickness,
	SurfaceFill& outl, bool antiAliasing /* = false */,
	bool blitTransparent /* = false */, BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Calculate min(pts), max(pts), pass as bounds here
	// This may only be necessary on Windows
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// WasAltered(&r);
}

bool DarkEdif::Surface::DrawPolygon(Point* pts, std::size_t nPts, SurfaceFill& fill,
	bool antiAliasing /* = false */, bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Calculate min(pts), max(pts), pass as bounds here
	// This may only be necessary on Windows
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// WasAltered(&r);
}

bool DarkEdif::Surface::DrawPolygon(Point* pts, std::size_t nPts, SurfaceFill& fill,
	std::size_t thickness, SurfaceFill& outl, bool antiAliasing /* = false */,
	bool blitTransparent /* = false */, BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint32_t param /* = 0 */, bool dofill /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Calculate min(pts), max(pts), pass as bounds here
	// This may only be necessary on Windows
	// TODO: Thick line may make the affected area larger than points, due to aliasing
	// WasAltered(&r);
}

// Filled Primitives

bool DarkEdif::Surface::FloodFill(Point pt, Rect& rect, std::uint32_t fill,
	bool antiAliasing /* = false */, std::size_t tol /* = 0 */,
	bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	WasAltered(&rect);
}

bool DarkEdif::Surface::FloodFill(Point pt, std::uint32_t clrFill,
	bool antiAliasing /* = false */, std::size_t tol /* = 0 */,
	bool blitTransparent /* = false */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	WasAltered();
}

// Rotates by radians, storing result in a new destination.
void DarkEdif::Surface::RotateIntoRadian(Surface* destSurf, double radians,
	bool antiAliasing, std::uint32_t clrFill /* = 0L */, bool transp /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Do we need to call this?
	//destSurf->WasAltered();
}
// Rotates by degrees, storing result in a new destination.
void DarkEdif::Surface::RotateIntoDegree(Surface* destSurf, int degrees, bool antiAliasing,
	std::uint32_t clrFill /* = 0L */, bool transp /* = true */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Do we need to call this?
	//destSurf->WasAltered();
}

// Calculates a new size of canvas needed to hold the given size when rotated
DarkEdif::Size DarkEdif::Surface::GetRotatedRectSize(const Size &orig, float angle)
{
	Size s = orig;
#ifdef _WIN32
	cSurface::GetSizeOfRotatedRect(&s.width, &s.height, angle);
#else
	LOGF(_T("DarkEdif::Surface error: Cannot get rotated rect; not implemented on this platform\n"));
#endif
	return s;
}

int DarkEdif::Surface::TextWriteOut(std::tstring_view text, Point topLeft, std::uint32_t textAlignMode,
	Rect& clipRect, std::uint32_t textColor /* = 0 */, DarkEdif::FontInfoMultiPlat fontHandle /* = null */,
	bool blitTransparent /* = true */, BlitOperation blitOp /* = BlitOperation::Copy */,
	std::uint32_t param /* = 0 */, int antiAliasing /* = 0 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Confirm anti-aliasing doesn't ignore clip
	WasAltered(&clipRect);
}

int	DarkEdif::Surface::TextWrite(std::tstring_view text, Rect& clipRect, std::uint32_t dtflags,
	std::uint32_t color /* = 0 */, DarkEdif::FontInfoMultiPlat hFnt /* = null */, bool blitTransparent /* = true */,
	BlitOperation blitOp /* = BlitOperation::Copy */, std::uint32_t param /* = 0 */, int antiAliasing /* = 0 */,
	std::size_t dwLeftMargin /* = 0 */, std::size_t dwRightMargin /* = 0 */, std::size_t dwTabSize /* = 8 */)
{
	throw std::runtime_error("Not implemented");
	// TODO: Confirm anti-aliasing doesn't ignore clip
	WasAltered(&clipRect);
}

bool DarkEdif::Surface::IsFullyTransparent() const {
#ifdef _WIN32
	return surf->IsTransparent() != FALSE;
#else
	LOGF(_T("Cannot check fully transparent; not implemented on this platform\n"));
	return false;
#endif
}

// Replace color
bool DarkEdif::Surface::ReplaceColor(std::uint32_t newColor, std::uint32_t oldColor, bool includeAlpha)
{
#ifdef _WIN32
	if (!surf->ReplaceColor(newColor, oldColor))
#else
	LOGF(_T("Cannot replace color; not implemented on this platform\n"));
	if (true)
#endif
	{
		LOGE(_T("%sError replacing color %s with color %s.\n"),
			debugID, ColorToString(oldColor, includeAlpha).c_str(), ColorToString(newColor, includeAlpha).c_str());
		return false;
	}
	WasAltered();
	return true;
}

// Test collision fine entre deux surfaces
bool DarkEdif::Surface::IsColliding(Surface& dest, std::size_t xDest, std::size_t yDest,
	std::size_t xSrc, std::size_t ySrc, int rcWidth, int rcHeight)
{
	if (&dest == this)
		return LOGF(_T("Cannot check colliding with self.\n")), false;

	throw std::runtime_error("Not implemented");
}

#ifdef _WIN32
// Creates a Windows icon handle from the image
HICON DarkEdif::Surface::Windows_CreateIcon(Size iconSize, std::uint32_t transpColor /* = -1 */, Point * pHotSpot /* = nullptr */)
{
	if (transpColor == -1)
		transpColor = surf->GetTransparentColor();
	else if (HasAlpha())
		LOGW(_T("%sGetting surface transparent color when alpha is present - color may be ignored.\n"), debugID);
	if (pHotSpot && !CheckPointContained(*pHotSpot))
		LOGW(_T("%sCreating icon warning: hot spot position is outside the surface area.\n"), debugID);
	return surf->CreateIcon(iconSize.width, iconSize.height, transpColor,
		pHotSpot && pHotSpot->x == 0 && pHotSpot->y == 0 ? nullptr : (POINT *)pHotSpot);
}
#endif

// Sets transparent color. Does not change underlying pixels, but affects future copying.
// Ignored if alpha channel is present - see HasAlpha().
void DarkEdif::Surface::SetTransparentColor(std::uint32_t rgb)
{
	NoAlpha(_T(__FUNCTION__), rgb);
#ifdef _WIN32
	surf->SetTransparentColor(rgb);
	WasAltered();
#else
	LOGF(_T("%sCannot set transparent color; not implemented on this platform\n"), debugID);
#endif
}
// Gets transparent color. Will return -1 if alpha is present.
std::uint32_t DarkEdif::Surface::GetTransparentColor() const
{
	if (HasAlpha())
		LOGW(_T("%sGetting surface transparent color when alpha is present - color may be ignored.\n"), debugID);

#ifdef _WIN32
	return surf->GetTransparentColor();
#else
	LOGF(_T("%sCannot get transparent color; not implemented on this platform\n"), debugID);
	return 0;
#endif
}
