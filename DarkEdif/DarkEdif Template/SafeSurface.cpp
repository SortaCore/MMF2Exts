#include "Surface.hpp"
#include "SafeSurface.hpp"

SurfaceSafe::SurfaceSafe() : cSurface() {

}
SurfaceSafe::~SurfaceSafe() {}

void SurfaceSafe::InitializeSurfaces() {
	cSurface::InitializeSurfaces();
}
void SurfaceSafe::FreeSurfaces() {
	cSurface::FreeSurfaces();
}
void SurfaceSafe::FreeExternalModules()
{
	#if HWABETA
		cSurface::FreeExternalModules();
	#else
		throw std::runtime_error("Not usable in non-HWA");
	#endif
}
SurfaceSafe & SurfaceSafe::operator= (const SurfaceSafe & source)
{
	*dynamic_cast<cSurface *>(this) = dynamic_cast<const cSurface &>(source);
	return *this;
};
void SurfaceSafe::Create (size_t width, size_t height, cSurface * prototype) {
	// TODO: Check if already created, report error
	// TODO: Check if width/height is 0 or absurdly large, report error
	// TODO: Check if prototype is non-null and valid, report error
	dynamic_cast<cSurface*>(this)->Create((int)width, (int)height, prototype);
}
void SurfaceSafe::Create(HDC hDC) {
	// TODO: Check if already created, report error
	// TODO: Check if HDC is valid, report error
	dynamic_cast<cSurface*>(this)->Create(hDC);
}
void SurfaceSafe::Create (HWND hWnd, bool includeFrame) {
	// TODO: Check if already created, report error
	// TODO: Check if HWND is valid, report error
	dynamic_cast<cSurface*>(this)->Create(hWnd, includeFrame);
}
bool SurfaceSafe::CreateScreenSurface() {
	return dynamic_cast<cSurface*>(this)->CreateScreenSurface() != FALSE;
}
bool SurfaceSafe::IsScreenSurface() {
	return dynamic_cast<cSurface*>(this)->IsScreenSurface() != FALSE;
}
bool SurfaceSafe::IsValid() {
	return dynamic_cast<cSurface*>(this)->IsValid() != FALSE;
}
SurfaceType SurfaceSafe::GetType() {
	// TOOD: What if invalid - not created yet?
	return (SurfaceType)dynamic_cast<cSurface*>(this)->GetType();
}
SurfaceDriver SurfaceSafe::GetDriver() {
	// TOOD: What if invalid - not created yet?
	return (SurfaceDriver)dynamic_cast<cSurface*>(this)->GetDriver();
}
ULONG SurfaceSafe::GetDriverInfo(void * pInfo) {
	// TOOD: What if invalid - not created yet?
	return dynamic_cast<cSurface*>(this)->GetDriverInfo(pInfo);
}
void SurfaceSafe::Clone (const cSurface & pSrcSurface, size_t newW /* = MAX_SIZE_T */, size_t newH /* = MAX_SIZE_T */) {
	dynamic_cast<cSurface*>(this)->Clone(pSrcSurface, (int)newW, (int)newH);
}
void SurfaceSafe::Delete() {
	// TODO: Check if not created, report error?
	dynamic_cast<cSurface*>(this)->Delete();
}
int SurfaceSafe::GetLastError() {
	// TODO: Check the return type matches SURFACEERROR enum, that the enum is complete, and that we don't want to add more error types.
	return dynamic_cast<cSurface*>(this)->GetLastError();
}
size_t SurfaceSafe::GetWidth() const {
	return (size_t)dynamic_cast<const cSurface*>(this)->GetWidth();
}
size_t SurfaceSafe::GetHeight() const {
	return (size_t)dynamic_cast<const cSurface*>(this)->GetHeight();
}
size_t SurfaceSafe::GetDepth() const{
	return (size_t)dynamic_cast<const cSurface*>(this)->GetDepth();
}

bool SurfaceSafe::GetInfo(size_t & width, size_t & height, size_t & depth) const {
	// TODO: What if invalid?
	int w, h, d;
	BOOL b = dynamic_cast<const cSurface*>(this)->GetInfo(w, h, d);
	width = (size_t)w;
	height = (size_t)h;
	depth = (size_t)d;
	return b != FALSE;
}

// ======================
// Surface coordinate management
// ======================
void SurfaceSafe::SetOrigin(size_t x, size_t y) {
	// TODO: Check new origin is within bounds
	// TODO: Check surface is valid
	dynamic_cast<cSurface*>(this)->SetOrigin((int)x, (int)y);
}
void SurfaceSafe::SetOrigin(POINT c) {
	// TODO: Check new origin is within bounds (not negative too)
	// TODO: Check surface is valid
	dynamic_cast<cSurface*>(this)->SetOrigin(c);
}

void SurfaceSafe::GetOrigin(POINT &pt) {
	// TODO: Check new origin is within bounds (not negative too)
	// TODO: Check surface is valid
	dynamic_cast<cSurface*>(this)->GetOrigin(pt);
}
void SurfaceSafe::GetOrigin(size_t & x, size_t & y) {
	// TODO: Check surface is valid
	int x2, y2;
	dynamic_cast<cSurface*>(this)->GetOrigin(x2, y2);
	x = (size_t)x2;
	y = (size_t)y2;
}

void SurfaceSafe::OffsetOrigin(size_t dx, size_t dy) {
	// TODO: Check new origin is within bounds
	// TODO: Check surface is valid
	dynamic_cast<cSurface*>(this)->OffsetOrigin((int)dx, (int)dy);
}
void SurfaceSafe::OffsetOrigin(POINT delta) {
	// TODO: Check new origin is within bounds
	// TODO: Check surface is valid
	dynamic_cast<cSurface*>(this)->OffsetOrigin(delta);
}

// ======================
// Raster operations
// ======================

std::byte * SurfaceSafe::LockBuffer() {
	// TODO: Check surface is valid
	// TODO: Check surface is unlocked
	return (std::byte *)dynamic_cast<cSurface*>(this)->LockBuffer();
}
void SurfaceSafe::UnlockBuffer(std::byte * spBuffer) {
	// TODO: Check surface is valid
	// TODO: Check surface is locked
	return dynamic_cast<cSurface*>(this)->UnlockBuffer((LPBYTE)spBuffer);
}
size_t SurfaceSafe::GetPitch() const {
	// TODO: Check surface is valid
	// TODO: What does negative pitch mean? should we ABS it?
	return (size_t)std::abs(dynamic_cast<const cSurface*>(this)->GetPitch());
}

// ======================
// Double-buffer handling
// ======================

void SurfaceSafe::SetCurrentDevice() {
	// TODO: Check surface is valid
	return dynamic_cast<cSurface*>(this)->SetCurrentDevice();
}
int	 SurfaceSafe::BeginRendering(bool clear, RGBAREF rgba) {
	// TODO: Check surface is valid
	// TODO: Check rgba is not fully transparent?
	// TODO: Check rendering has not already begun
	return dynamic_cast<cSurface*>(this)->BeginRendering(clear, rgba);
}
int	 SurfaceSafe::EndRendering() {
	// TODO: Check surface is valid
	// TODO: Check rgba is not fully transparent?
	// TODO: Check rendering has begun
	return dynamic_cast<cSurface*>(this)->EndRendering();
}
bool SurfaceSafe::UpdateScreen() {
	return dynamic_cast<cSurface*>(this)->UpdateScreen();
}

cSurface* SurfaceSafe::GetRenderTargetSurface() {
#ifdef HWABETA
	// TODO: Check surface is valid
	// TODO: Check surface is HWA
	return dynamic_cast<cSurface*>(this)->GetRenderTargetSurface();
#else
	throw std::runtime_error("Can't use this function in non-HWA.");
#endif
}
void SurfaceSafe::ReleaseRenderTargetSurface(cSurface* psf) {
#ifdef HWABETA
	// TODO: Check surface is valid
	// TODO: Check surface is HWA
	return dynamic_cast<cSurface*>(this)->ReleaseRenderTargetSurface(psf);
#else
	throw std::runtime_error("Can't use this function in non-HWA.");
#endif
}
void SurfaceSafe::Flush(bool max) {
#ifdef HWABETA
	// TODO: Check surface is valid
	// TODO: Check surface is HWA
	return dynamic_cast<cSurface*>(this)->Flush(max);
#else
	throw std::runtime_error("Can't use this function in non-HWA.");
#endif
}
void SurfaceSafe::SetZBuffer(float z2D) {
#ifdef HWABETA
	// TODO: Check surface is valid
	// TODO: Check surface is HWA
	// TODO: Is there a way to check z2D?
	return dynamic_cast<cSurface*>(this)->SetZBuffer(z2D);
#else
	throw std::runtime_error("Can't use this function in non-HWA.");
#endif
}
HDC	 SurfaceSafe::GetDC() {
	// TODO: Check surface is valid
	// TODO: Check surface has a DC
	return dynamic_cast<cSurface*>(this)->GetDC();
}
void SurfaceSafe::ReleaseDC(HDC dc) {
	// TODO: Check surface is valid
	// TODO: Check surface has a DC
	return dynamic_cast<cSurface*>(this)->ReleaseDC(dc);
}
void SurfaceSafe::AttachWindow (HWND hWnd) {
	// TODO: Does surface need to be valid? Does it require a specific driver/surface type?
	return dynamic_cast<cSurface*>(this)->AttachWindow(hWnd);
}

void SurfaceSafe::GetClipRect(size_t & x, size_t & y, size_t & w, size_t & h) {
	// TODO: Check surface is valid
	// TODO: Check surface has a clip?
	int x2, y2, w2, h2;
	dynamic_cast<cSurface*>(this)->GetClipRect(x2,y2,w2,h2);
	x = (size_t)x2;
	y = (size_t)y2;
	w = (size_t)w2;
	h = (size_t)h2;
}
void SurfaceSafe::SetClipRect(size_t x, size_t y, size_t w, size_t h) {
	// TODO: Check surface is valid
	// TODO: Check clip is within bounds
	dynamic_cast<cSurface*>(this)->SetClipRect((int)x, (int)y, (int)w, (int)h);
}
void SurfaceSafe::ClearClipRect() {
	// TODO: Check surface is valid
	// TODO: Check surface has a clip?
	dynamic_cast<cSurface*>(this)->ClearClipRect();
}
#undef LoadImage

bool SurfaceSafe::LoadImage(HFILE fileHandle, ULONG lsize, LoadImageFlags loadFlags /* = LoadImageFlags::None */) {
	// TODO: Check surface is valid?
	// TODO: Check hFile is valid, and is readable
	return dynamic_cast<cSurface*>(this)->LoadImageW(fileHandle, lsize, loadFlags) != FALSE;
}
bool SurfaceSafe::LoadImage(LPCTSTR filename, LoadImageFlags loadFlags /* = LoadImageFlags::None */) {
	// TODO: Check surface is valid?
	// TODO: Check filename is non-null, exists and is readable
#ifdef _UNICODE
	return dynamic_cast<cSurface*>(this)->LoadImageW((const UShortWCHAR *)filename, loadFlags) != FALSE;
#else
	return dynamic_cast<cSurface*>(this)->LoadImageA(filename, loadFlags) != FALSE;
#endif
}
bool SurfaceSafe::LoadImage(HINSTANCE hInst, int bmpID, LoadImageFlags loadFlags /* = LoadImageFlags::None */) {
	// TODO: Check surface is valid?
	// TODO: Check hInst is valid?
	// TODO: Check bmpID is valid
	return dynamic_cast<cSurface*>(this)->LoadImageW(hInst, bmpID, loadFlags) != FALSE;
}
bool SurfaceSafe::LoadImage(LPBITMAPINFO pBmi, std::byte * pBits /* = nullptr */, LoadImageFlags loadFlags /* = LoadImageFlags::None */) {
	// TODO: Check surface is valid?
	// TODO: Check pBmi is valid?
	// TODO: Check pBits is valid, if set
	return dynamic_cast<cSurface*>(this)->LoadImageW(pBmi, (LPBYTE)pBits, loadFlags) != FALSE;
}
bool SurfaceSafe::SaveImage(HFILE fileHandle, SaveImageFlags saveFlags /* = SaveImageFlags::None */) {
	// TODO: Check surface is valid
	// TODO: Check fileHandle is valid and writeable
	return dynamic_cast<cSurface*>(this)->SaveImage(fileHandle, saveFlags) != FALSE;
}
bool SurfaceSafe::SaveImage(LPCTSTR filename, SaveImageFlags saveFlags /* = SaveImageFlags::None */) {
	// TODO: Check surface is valid
	// TODO: Check filename is valid and writeable
#ifdef _UNICODE
	return dynamic_cast<cSurface*>(this)->SaveImage((const UShortWCHAR *)filename, saveFlags) != FALSE;
#else
	return dynamic_cast<cSurface*>(this)->SaveImage(filename, saveFlags) != FALSE;
#endif
}
bool SurfaceSafe::SaveImage(LPBITMAPINFO pBmi, std::byte * pBits, SaveImageFlags saveFlags /* = SaveImageFlags::None */) {
	// TODO: Check surface is valid
	// TODO: Check bitmap is valid?
	return dynamic_cast<cSurface*>(this)->SaveImage(pBmi, (LPBYTE)pBits, saveFlags) != FALSE;
}

ULONG SurfaceSafe::GetDIBSize() {
	// TODO: Check surface is valid?
	return dynamic_cast<cSurface*>(this)->GetDIBSize();
}

// ======================
// Pixel functions
// ======================
// Set pixel
void SurfaceSafe::SetPixel(size_t x, size_t y, COLORREF color) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is not a 256-color palette?
	return dynamic_cast<cSurface*>(this)->SetPixel((int)x, (int)y, color);
}
void SurfaceSafe::SetPixel(size_t x, size_t y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is not a 256-color palette?
	return dynamic_cast<cSurface*>(this)->SetPixel((int)x, (int)y, r, g, b);
}
void SurfaceSafe::SetPixel(size_t x, size_t y, std::uint8_t colorPaletteIndex) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is a 256-color palette
	return dynamic_cast<cSurface*>(this)->SetPixel((int)x, (int)y, colorPaletteIndex);
}

// Faster: assume clipping is done, the origin is at (0,0) {} and the surface is locked
void SurfaceSafe::SetPixelFast(size_t x, size_t y, COLORREF color) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is not a 256-color palette?
	return dynamic_cast<cSurface*>(this)->SetPixelFast((int)x, (int)y, color);
}
void SurfaceSafe::SetPixelFast8(size_t x, size_t y, std::uint8_t colorPaletteIndex) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is not a 256-color palette?
	return dynamic_cast<cSurface*>(this)->SetPixelFast8((int)x, (int)y, (int)colorPaletteIndex);
}

bool SurfaceSafe::GetPixel(size_t x, size_t y, COLORREF & color) const {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	return dynamic_cast<const cSurface*>(this)->GetPixel((int)x, (int)y, color);
}
bool SurfaceSafe::GetPixel(size_t x, size_t y, std::uint8_t & r, std::uint8_t & g, std::uint8_t & b) const {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is not a 256-color palette?
	return dynamic_cast<const cSurface*>(this)->GetPixel((int)x, (int)y, r, g, b);
}
bool SurfaceSafe::GetPixel(size_t x, size_t y, std::uint8_t & colorPaletteIndex) const {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is a 256-color palette
	int idx;
	bool b = dynamic_cast<const cSurface*>(this)->GetPixel((int)x, (int)y, idx);
	colorPaletteIndex = (std::uint8_t)idx;
	return b;
}

// Faster: assume clipping is done, the origin is at (0,0) and the surface is locked
COLORREF SurfaceSafe::GetPixelFast(size_t x, size_t y) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	return dynamic_cast<cSurface*>(this)->GetPixelFast((int)x, (int)y);
}
std::uint8_t SurfaceSafe::GetPixelFast8(size_t x, size_t y) {
	// TODO: Check surface is valid
	// TODO: Check x/y is within bounds
	// TODO: Check surface is a 256-color palette
	return dynamic_cast<cSurface*>(this)->GetPixelFast8((int)x, (int)y);
}

// ======================
// Blit functions
// ======================
// Blit surface to surface
bool SurfaceSafe::Blit(cSurface & destSurf) const {
	// TODO: Check surface is valid
	// TODO: Check surface is big enough to write to
	// TODO: Check clip rect?
	return dynamic_cast<const cSurface*>(this)->Blit(destSurf);
}

bool SurfaceSafe::Blit(cSurface & destSurf, size_t destX, size_t destY,
	BlitMode blitMode, BlitOperation blitOp, LPARAM blitOpParam,
	BlitOptions blitFlags /* = BlitOptions::None */) const {
	// TODO: Check surface is valid
	// TODO: Check surface is big enough to write to
	// TODO: Check clip rect?
	return dynamic_cast<const cSurface*>(this)->Blit(destSurf, (int)destX, (int)destY, blitMode, (BlitOp)blitOp, blitOpParam, (ULONG)blitFlags);
}

// Blit rectangle to surface
bool SurfaceSafe::Blit(cSurface & destSurf, size_t destX, size_t destY,
	size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
	BlitMode blitMode /* = BlitMode::Opaque */, BlitOperation blitOp, LPARAM blitOpParam,
	BlitOptions blitFlags /* = BlitOptions::None */) const {
	// TODO: Check surface is valid
	// TODO: Check surface is big enough to write to
	// TODO: Check clip rect?
	return dynamic_cast<const cSurface*>(this)->Blit(destSurf, (int)destX, (int)destY, (int)srcX, (int)srcY, (int)srcWidth, (int)srcHeight,
		blitMode, (BlitOp)blitOp, blitOpParam, (ULONG)blitFlags);
}

// Extended blit : can do stretch & rotate at the same time
// Only implemented in 3D mode

bool SurfaceSafe::BlitEx(cSurface & destSurf, float destX, float destY, float scaleX, float scaleY,
	size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight, LPPOINT pCenter, float angle,
	BlitMode blitMode /* = BlitMode::BMODE_OPAQUE */, BlitOperation blitOp /* = BlitOperation::Copy */, LPARAM blitOpParam /* = 0 */,
	BlitOptions blitFlags /*= BlitOptions::None */) const {
	// TODO: Check surface is valid
	// TODO: Check surface is big enough to write to
	// TODO: Check clip rect?
#ifdef HWABETA
	return dynamic_cast<const cSurface *>(this)->BlitEx(destSurf, destX, destY, scaleX, scaleY, (int)srcX, (int)srcY, (int)srcWidth, (int)srcHeight,
		pCenter, angle, blitMode, (BlitOp)blitOp, blitOpParam, (ULONG)blitFlags);
#else
	throw std::runtime_error("Not HWA");
#endif
}
// Scrolling
bool SurfaceSafe::Scroll(size_t destX, size_t destY, size_t srcX, size_t srcY, size_t width, size_t height) {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check clip rect?
	return dynamic_cast<cSurface *>(this)->Scroll((int)destX, (int)destY, (int)srcX, (int)srcY, (int)width, (int)height) != FALSE;
}

// Blit via callback
bool SurfaceSafe::FilterBlit(cSurface & destSurf, size_t destX, size_t destY,
	size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
	BlitMode blitMode, FILTERBLITPROC fbProc, LPARAM lUserParam) const {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check filter function
	// TODO: Check clip rect?

	// I could switch this to using a regular cdecl function, but since it's invoked by WinAPI, it'll be extra overhead
	return dynamic_cast<const cSurface *>(this)->FilterBlit(destSurf, (int)destX, (int)destY, (int)srcX, (int)srcY, (int)srcWidth, (int)srcHeight, blitMode, fbProc, lUserParam) != FALSE;
}

bool SurfaceSafe::FilterBlit (cSurface & destSurf, FILTERBLITPROC fbProc,
	LPARAM lUserParam, BlitMode blitMode) const {
	// TODO: Check surface is valid
	// TODO: Check destination is in bounds
	// TODO: Check filter function
	// TODO: Check clip rect?
	return dynamic_cast<const cSurface *>(this)->FilterBlit(destSurf, fbProc, lUserParam, blitMode) != FALSE;
}

// Matrix blit via callback
bool SurfaceSafe::MatrixFilterBlit(cSurface & destSurf, size_t destX, size_t destY,
	size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
	size_t mWidth, size_t mHeight, size_t mDXCenter, size_t mDYCenter,
	MATRIXFILTERBLITPROC fbProc, LPARAM lUserParam) const {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check filter function
	// TODO: Check clip rect?
	return dynamic_cast<const cSurface *>(this)->MatrixFilterBlit(destSurf, (int)destX, (int)destY, (int)srcX, (int)srcY, (int)srcWidth,
		(int)srcHeight, (int)mWidth, (int)mHeight, (int)mDXCenter, (int)mDYCenter, fbProc, lUserParam) != FALSE;
}

// Stretch surface to surface
bool SurfaceSafe::Stretch(cSurface & destSurf, StretchFlags strFlags) const {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check stretch flags are valid?
	return dynamic_cast<const cSurface *>(this)->Stretch(destSurf, (ULONG)strFlags) != FALSE;
}

// Stretch surface to rectangle
bool SurfaceSafe::Stretch(cSurface & destSurf, size_t destX, size_t destY, size_t destWidth, size_t destHeight,
	BlitMode blitMode, BlitOperation blitOp, std::uint8_t inkEffectParam, StretchFlags strFlags) const {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check stretch flags are valid?
	return dynamic_cast<const cSurface *>(this)->Stretch(destSurf, (int)destX, (int)destY, (int)destWidth, (int)destHeight, blitMode, (BlitOp)blitOp, inkEffectParam, (ULONG)strFlags) != FALSE;
}

// Stretch rectangle to rectangle
bool SurfaceSafe::Stretch(cSurface & destSurf, size_t destX, size_t destY, size_t destWidth, size_t destHeight,
	size_t srcX, size_t srcY, size_t srcWidth, size_t srcHeight,
	BlitMode blitMode, BlitOperation blitOp, std::uint8_t inkEffectParam, StretchFlags strFlags) const {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	// TODO: Check destination is in bounds
	// TODO: Check stretch flags are valid?
	return dynamic_cast<const cSurface *>(this)->Stretch(destSurf, (int)destX, (int)destY, (int)destWidth, (int)destHeight,
		(int)srcX, (int)srcY, (int)srcWidth, (int)srcHeight, blitMode, (BlitOp)blitOp, inkEffectParam, (ULONG)strFlags) != FALSE;
}

// Revert surface horizontally
// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
bool SurfaceSafe::ReverseX() {
	// TODO: Check surface is valid
	return dynamic_cast<cSurface *>(this)->ReverseX() != FALSE;
}

// Revert rectangle horizontally
// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
bool SurfaceSafe::ReverseX(size_t x, size_t y, size_t width, size_t height) {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	return dynamic_cast<cSurface *>(this)->ReverseX((int)x, (int)y, (int)width, (int)height) != FALSE;
}

// Revert surface vertically
// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
bool SurfaceSafe::ReverseY() {
	// TODO: Check surface is valid
	return dynamic_cast<cSurface *>(this)->ReverseY() != FALSE;
}

// Revert rectangle vertically
// Note: Slow for HWA, as it implicitly copies to software memory, and flips there
bool SurfaceSafe::ReverseY(size_t x, size_t y, size_t width, size_t height) {
	// TODO: Check surface is valid
	// TODO: Check source is in bounds
	return dynamic_cast<cSurface *>(this)->ReverseY((int)x, (int)y, (int)width, (int)height) != FALSE;
}

// Remove empty borders
bool SurfaceSafe::GetMinimizeRect(RECT *rect) {
	// TODO: Check surface is valid
	// TODO: Check rect is in bounds
	return dynamic_cast<cSurface *>(this)->GetMinimizeRect(rect) != FALSE;
}
bool SurfaceSafe::Minimize() {
	// TODO: Check surface is valid
	// TODO: Check rect is in bounds
	return dynamic_cast<cSurface *>(this)->Minimize() != FALSE;
}
bool SurfaceSafe::Minimize(RECT * rect) {
	// TODO: Check surface is valid
	// TODO: Check rect is in bounds
	return dynamic_cast<cSurface *>(this)->Minimize(rect) != FALSE;
}

// =============================
// Blit from screen or memory DC to another DC
// =============================

bool SurfaceSafe::CaptureDC(HDC srcDC, HDC dstDC, LONG srcX, LONG srcY, LONG dstX, LONG dstY,
	LONG srcWidth, LONG srcHeight, LONG dstWidth, LONG dstHeight, bool flushMessage, bool keepRatio) {
	// TODO: Check surface is valid
	// TODO: Check rect is in bounds
	return cSurface::CaptureDC(srcDC, dstDC, srcX, srcY, dstX, dstY, srcWidth, srcHeight, dstWidth, dstHeight, flushMessage, keepRatio) != FALSE;
}

// ======================
// Fill
// ======================

// Fill surface
bool SurfaceSafe::Fill(COLORREF color) {
	// TODO: Check surface is valid
	// TODO: Check COLORREF is valid
	return dynamic_cast<cSurface *>(this)->Fill(color) != FALSE;
}
bool SurfaceSafe::Fill(CFillData * fillData) {
	// TODO: Check surface is valid
	// TODO: Check fillData is valid
	return dynamic_cast<cSurface *>(this)->Fill(fillData) != FALSE;
}
bool SurfaceSafe::Fill(std::uint8_t colorPaletteIndex) {
	// TODO: Check surface is valid
	// TODO: Check colorPaletteIndex is valid
	return dynamic_cast<cSurface *>(this)->Fill((int)colorPaletteIndex) != FALSE;
}
bool SurfaceSafe::Fill(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	// TODO: Check surface is valid
	return dynamic_cast<cSurface *>(this)->Fill(r, g, b) != FALSE;
}

// Fill block
bool SurfaceSafe::Fill(size_t x, size_t y, size_t width, size_t height, COLORREF color) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check COLORREF is valid
	return dynamic_cast<cSurface *>(this)->Fill((int)x, (int)y, (int)width, (int)height, color) != FALSE;
}
bool SurfaceSafe::Fill(size_t x, size_t y, size_t width, size_t height, CFillData * fillData) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check fillData is valid
	return dynamic_cast<cSurface *>(this)->Fill((int)x, (int)y, (int)width, (int)height, fillData) != FALSE;
}
bool SurfaceSafe::Fill(size_t x, size_t y, size_t width, size_t height, std::uint8_t colorPaletteIndex) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check colorPaletteIndex is valid
	return dynamic_cast<cSurface *>(this)->Fill((int)x, (int)y, (int)width, (int)height, (int)colorPaletteIndex) != FALSE;
}
bool SurfaceSafe::Fill(size_t x, size_t y, size_t width, size_t height, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	return dynamic_cast<cSurface *>(this)->Fill((int)x, (int)y, (int)width, (int)height, (int)r, (int)g, (int)b) != FALSE;
}

bool SurfaceSafe::Fill(size_t x, size_t y, size_t width, size_t height, COLORREF* pColors, ULONG dwFlags) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check pColors is valid
	// TODO: Check dwFlags - might be ExtFloodFill() flags under the hood
#ifdef HWABETA
	return dynamic_cast<cSurface *>(this)->Fill((int)x, (int)y, (int)width, (int)height, pColors, dwFlags) != FALSE;
#else
	throw std::runtime_error("Not HWA");
#endif
}

// ======================
// Geometric Primitives
// ======================

// 1. Simple routines : call GDI with Surface DC
// =============================================

bool SurfaceSafe::Ellipse(int left, int top, int right, int bottom, size_t thickness, COLORREF outlineColor) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Ellipse(left, top, right, bottom, thickness, outlineColor) != FALSE;
}

bool SurfaceSafe::Ellipse(int left, int top, int right, int bottom, COLORREF fillColor, size_t thickness,
	COLORREF outlineColor, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check thickness is valid
	// TODO: Check crFill/outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Ellipse(left, top, right, bottom, fillColor, thickness, outlineColor, fill) != FALSE;
}

bool SurfaceSafe::Rectangle(int left, int top, int right, int bottom, size_t thickness, COLORREF outlineColor) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Rectangle(left, top, right, bottom, thickness, outlineColor) != FALSE;
}

bool SurfaceSafe::Rectangle(int left, int top, int right, int bottom, COLORREF fillColor, size_t thickness,
	COLORREF crOutl, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check params are in bounds
	// TODO: Check thickness is valid
	// TODO: Check fillColor/outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Rectangle(left, top, right, bottom, fillColor, thickness, crOutl, fill) != FALSE;
}

bool SurfaceSafe::Polygon(POINT * pts, size_t nPts, size_t thickness, COLORREF outlineColor) {
	// TODO: Check surface is valid
	// TODO: Check pts is valid and are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Polygon(pts, nPts, thickness, outlineColor) != FALSE;
}

bool SurfaceSafe::Polygon(POINT * pts, size_t nPts, COLORREF fillColor, size_t thickness,
	COLORREF outlineColor, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check pts are in bounds
	// TODO: Check nPts is valid
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Polygon(pts, nPts, fillColor, thickness, outlineColor, fill) != FALSE;
}

bool SurfaceSafe::Line(size_t x1, size_t y1, size_t x2, size_t y2, size_t thickness, COLORREF outlineColor) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	return dynamic_cast<cSurface *>(this)->Line(x1, y1, x2, y2, (int)thickness, outlineColor) != FALSE;
}

// 2. More complex but slower (variable opacity, anti-alias, custom filling, ...) {}
// ==============================================================================

bool SurfaceSafe::Ellipse(int left, int top, int right, int bottom, size_t thickness, CFillData * fillData, bool antiAliasing,
	BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineColor is valid
	// TODO: Check fillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Ellipse(left, top, right, bottom, (int)thickness, fillData, antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Ellipse(int left, int top, int right, int bottom, CFillData * fillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check outlineColor is valid
	// TODO: Check fillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Ellipse(left, top, right, bottom, fillData, antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Ellipse(int left, int top, int right, int bottom, CFillData * fillData, size_t thickness, CFillData * outlineFillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check fillData/outlineFillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Ellipse(left, top, right, bottom, fillData, thickness, outlineFillData, antiAliasing, blitMode, (BlitOp)blitOp, param, fill) != FALSE;
}

bool SurfaceSafe::Rectangle(int left, int top, int right, int bottom, size_t thickness, CFillData * outlineFillData, bool antiAliasing,
	BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineFillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Rectangle(left, top, right, bottom, (int)thickness, outlineFillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Rectangle(int left, int top, int right, int bottom, CFillData * fillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check fillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Rectangle(left, top, right, bottom, fillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Rectangle(int left, int top, int right, int bottom, CFillData * fillData, size_t thickness, CFillData * outlineFillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check fillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Rectangle(left, top, right, bottom, fillData, (int)thickness,
		outlineFillData, antiAliasing, blitMode, (BlitOp)blitOp, param, fill) != FALSE;
}

bool SurfaceSafe::Polygon(POINT * pts, size_t nPts, size_t thickness, CFillData * outlineFillData, bool antiAliasing,
	BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check pts are in bounds
	// TODO: Check nPts is valid
	// TODO: Check thickness is valid
	// TODO: Check outlineFillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Polygon(pts, nPts, (int)thickness, outlineFillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Polygon(POINT * pts, size_t nPts, CFillData * fillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check pts are in bounds
	// TODO: Check nPts is valid
	// TODO: Check fillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Polygon(pts, nPts, fillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::Polygon(POINT * pts, size_t nPts, CFillData * fillData, size_t thickness, CFillData * outlineFillData,
	bool antiAliasing, BlitMode blitMode, BlitOperation blitOp, LPARAM param, bool fill) {
	// TODO: Check surface is valid
	// TODO: Check pts are in bounds
	// TODO: Check nPts is valid
	// TODO: Check thickness is valid
	// TODO: Check fillData/outlineFillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Polygon(pts, nPts, fillData, (int)thickness, outlineFillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param, fill) != FALSE;
}

// Note: has no effect in HWA, if thickness is greater than 1.
bool SurfaceSafe::Line(size_t x1, size_t y1, size_t x2, size_t y2, size_t thickness, CFillData * outlineFillData, bool antiAliasing,
	BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check outlineFillData is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->Line(x1, y1, x2, y2, (int)thickness, outlineFillData,
		antiAliasing, blitMode, (BlitOp)blitOp, param) != FALSE;
}

// Filled Primitives

bool SurfaceSafe::FloodFill(size_t x, size_t y, int & left, int & top, int & right, int & bottom, COLORREF fillColor, bool antiAliasing,
	size_t tolerance, BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check fillColor is valid
	// TODO: Check tolerance is valid
	// TODO: Check blitMode/blitOp/param is valid
	return dynamic_cast<cSurface *>(this)->FloodFill(x, y, left, top, right, bottom, fillColor,
		antiAliasing, (int)tolerance, blitMode, (BlitOp)blitOp, param) != FALSE;
}

bool SurfaceSafe::FloodFill(size_t x, size_t y, COLORREF fillColor, bool antiAliasing, size_t tolerance,
	BlitMode blitMode, BlitOperation blitOp, LPARAM param) {
	// TODO: Check surface is valid
	// TODO: Check coords are in bounds
	// TODO: Check thickness is valid
	// TODO: Check fillColor is valid
	// TODO: Check tolerance is valid
	// TODO: Check blitMode/blitOp/param is valid

	return dynamic_cast<cSurface *>(this)->FloodFill(x, y, fillColor,
		antiAliasing, (int)tolerance, blitMode, (BlitOp)blitOp, param) != FALSE;
}

// ======================
// Rotation
// ======================
// Rotate surface in radians
bool SurfaceSafe::Rotate(cSurface & destSurf, double radians, bool antiAliasing, COLORREF fillColor, bool transparent) {
	// TODO: Check surface is valid
	// TODO: Check destSurf is valid, and in bounds
	// TODO: Check fillColor is valid
	return dynamic_cast<cSurface *>(this)->Rotate(destSurf, radians, antiAliasing, fillColor, transparent) != FALSE;
}
bool SurfaceSafe::Rotate(cSurface & destSurf, int degrees, bool antiAliasing, COLORREF fillColor, bool transparent) {
	// TODO: Check surface is valid
	// TODO: Check destSurf is valid, and in bounds
	// TODO: Check fillColor is valid
	return dynamic_cast<cSurface *>(this)->Rotate(destSurf, degrees, antiAliasing, fillColor, transparent) != FALSE;
}
bool SurfaceSafe::Rotate90(cSurface & destSurf, bool b270) {
	// TODO: Check surface is valid
	// TODO: Check destSurf is valid, and in bounds
	return dynamic_cast<cSurface *>(this)->Rotate90(destSurf, b270) != FALSE;
}

// Create rotated surface
bool SurfaceSafe::CreateRotatedSurface(cSurface & destSurf, double radians, bool antiAliasing, COLORREF fillColor, bool transparent) {
	// TODO: Check surface is valid
	// TODO: Check destSurf is valid, and in bounds
	// TODO: Check fillColor is valid
	return dynamic_cast<cSurface *>(this)->CreateRotatedSurface(destSurf, radians, antiAliasing, fillColor, transparent) != FALSE;

}
bool SurfaceSafe::CreateRotatedSurface(cSurface & destSurf, int degrees, bool antiAliasing, COLORREF fillColor, bool transparent) {
	// TODO: Check surface is valid
	// TODO: Check destSurf is valid, and in bounds
	// TODO: Check fillColor is valid
	return dynamic_cast<cSurface *>(this)->CreateRotatedSurface(destSurf, degrees, antiAliasing, fillColor, transparent) != FALSE;
}

void SurfaceSafe::GetSizeOfRotatedRect(int *pWidth, int *pHeight, float angle) {
#ifdef HWABETA
	cSurface::GetSizeOfRotatedRect(pWidth, pHeight, angle);
#else
	cSurface::GetSizeOfRotatedRect(pWidth, pHeight, (int)angle);
#endif
}

// ======================
// text
// ======================

int SurfaceSafe::TextOut(std::tstring_view text, size_t x, size_t y, ULONG textAlignMode, LPRECT clipRect,
	COLORREF textColor, HFONT fontHandle, BlitMode blitMode /* = BlitMode::BMODE_TRANSP */,
	BlitOperation blitOp /* = BlitOperation::Copy */, LPARAM param /* = 0*/, int antiAliasing /* = 0 */) {
	// TODO: Check surface is valid
	// TODO: Check x/y is in bounds
	// TODO: Check textAlignMode is valid
	// TODO: Check clipRect is in bounds
	// TODO: Check textColor is valid
	// TODO: Check fontHandle is valid - null uses system font, but might be an invalid handle
#ifdef _UNICODE
	return dynamic_cast<cSurface *>(this)->TextOutW((const UShortWCHAR *)text.data(), text.size(), x, y, textAlignMode, clipRect, textColor, fontHandle, blitMode, (BlitOp)blitOp, param, antiAliasing);
#else
	return dynamic_cast<cSurface *>(this)->TextOutA(text.data(), text.size(), x, y, textAlignMode, clipRect, textColor, fontHandle, blitMode, (BlitOp)blitOp, param, antiAliasing);
#endif
}
int SurfaceSafe::DrawText(std::tstring_view text, LPRECT clipRect, ULONG dtFlags, COLORREF textColor, HFONT fontHandle,
	BlitMode blitMode /* = BlitMode::BMODE_TRANSP */,
	BlitOperation blitOp /* = BlitOperation::Copy */, LPARAM param /* = 0*/, int antiAliasing /* = 0 */,
	size_t leftMargin /* = 0 */, size_t rightMargin /* = 0 */, size_t tabSize /* = 8 */) {
	// TODO: Check surface is valid
	// TODO: Check x/y is in bounds
	// TODO: Check textAlignMode is valid
	// TODO: Check clipRect is in bounds
	// TODO: Check dtFlags is valid (DT_XXX flags, e.g. DT_LEFT)
	// TODO: Check textColor is valid
	// TODO: Check fontHandle is valid - null uses system font, but might be an invalid handle
	
#ifdef _UNICODE
	return dynamic_cast<cSurface *>(this)->DrawTextW((const UShortWCHAR *)text.data(), text.size(), clipRect, dtFlags, textColor, fontHandle, blitMode, (BlitOp)blitOp, param, antiAliasing, leftMargin, rightMargin, tabSize);
#else
	return dynamic_cast<cSurface *>(this)->DrawTextA(text.data(), text.size(), clipRect, dtFlags, textColor, fontHandle, blitMode, (BlitOp)blitOp, param, antiAliasing, leftMargin, rightMargin, tabSize);
#endif

}

// ======================
// Color / Palette functions
// ======================
// Is transparent
bool SurfaceSafe::IsTransparent() {
	// TODO: Check surface is valid
	return dynamic_cast<cSurface *>(this)->IsTransparent() != FALSE;
}

// Replace color
bool SurfaceSafe::ReplaceColor(COLORREF newColor, COLORREF oldColor) {
	// TODO: Check surface is valid
	// TODO: Check newColor/oldColor are valid
	return dynamic_cast<cSurface *>(this)->ReplaceColor(newColor, oldColor) != FALSE;
}

// Test fine collision between two surfaces
bool SurfaceSafe::IsColliding(cSurface & destSurf, size_t destX, size_t destY, size_t srcX, size_t srcY, int rcWidth, int rcHeight) {
	// TODO: Check surface is valid
	// TODO: Check src coords are in bounds 
	// TODO: Check dest coords are in bounds
	// TODO: Check rcWidth/rcHeight is valid? Not sure what they're for
	// TODO: Check both surface have collision masks?
	return dynamic_cast<cSurface *>(this)->IsColliding(destSurf, destX, destY, srcX, srcY, rcWidth, rcHeight) != FALSE;
}

// Create icon
HICON SurfaceSafe::CreateIcon(size_t iconWidth, size_t iconHeight, COLORREF transpColor, POINT * hotSpotPoint) {
	// TODO: Check surface is valid (and driver type?)
	// TODO: Check iconWidth/Height are in bounds - note this function stretches
	// TODO: Check transpColor is valid - this function ignores the cSurface transparent color
	// TODO: Check hotSpotPoint is in bounds
	return dynamic_cast<cSurface *>(this)->CreateIcon((int)iconWidth, (int)iconHeight, transpColor, hotSpotPoint);
}

// Palette support
bool SurfaceSafe::Indexed() {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth?
	return dynamic_cast<cSurface *>(this)->Indexed() != FALSE;
}

bool SurfaceSafe::SetPalette(LOGPALETTE * palette, SetPaletteAction setPaletteAction /* = SetPaletteAction::None */) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	// TODO: Check palette is valid
	// TODO: Check setPaletteAction is valid
	return dynamic_cast<cSurface *>(this)->SetPalette(palette, setPaletteAction) != FALSE;
}
bool SurfaceSafe::SetPalette(csPalette * pCsPal, SetPaletteAction setPaletteAction /* = SetPaletteAction::None */) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	// TODO: Check pCsPal is valid
	// TODO: Check setPaletteAction is valid
	return dynamic_cast<cSurface *>(this)->SetPalette(pCsPal, setPaletteAction) != FALSE;
}
bool SurfaceSafe::SetPalette(cSurface & src, SetPaletteAction setPaletteAction /* = SetPaletteAction::None */) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	// TODO: Check src surface is valid
	// TODO: Check setPaletteAction is valid
	return dynamic_cast<cSurface *>(this)->SetPalette(src, setPaletteAction) != FALSE;
}
bool SurfaceSafe::SetPalette(HPALETTE palette, SetPaletteAction setPaletteAction /* = SetPaletteAction::None */) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	// TODO: Check palette is valid
	// TODO: Check setPaletteAction is valid
	return dynamic_cast<cSurface *>(this)->SetPalette(palette, setPaletteAction) != FALSE;
}

void SurfaceSafe::Remap(cSurface & src) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	// TODO: Check src surface is valid and has palette
	dynamic_cast<cSurface *>(this)->Remap(src);
}
void SurfaceSafe::Remap(std::byte * remapTable) {
	// TODO: Check surface is valid
	// TODO: Check driver type and depth supports palette?
	dynamic_cast<cSurface *>(this)->Remap((LPBYTE)remapTable);
}

csPalette * SurfaceSafe::GetPalette() {
	// TODO: Check surface is valid and has palette?
	return dynamic_cast<cSurface *>(this)->GetPalette();
}
UINT SurfaceSafe::GetPaletteEntries(PALETTEENTRY * paletteEntries, size_t index, size_t nbColors) {
	// TODO: Check surface is valid and has palette?
	// TODO: Check palette entries requested are within bounds
	return dynamic_cast<cSurface *>(this)->GetPaletteEntries(paletteEntries, (int)index, (int)nbColors);
}

std::uint8_t SurfaceSafe::GetNearestColorIndex(COLORREF rgb) {
	// TODO: Check surface is valid and has palette?
	return dynamic_cast<cSurface *>(this)->GetNearestColorIndex(rgb);
}
COLORREF SurfaceSafe::GetRGB(std::uint8_t index) {
	// TODO: Check surface is valid and has palette?
	// TODO: Check palette entry requested is within bounds
	return dynamic_cast<cSurface *>(this)->GetRGB(index);
}

std::uint8_t SurfaceSafe::GetOpaqueBlackIndex() {
	// TODO: Check surface is valid and has palette?
	// TODO: Check driver type/surface depth? I think this is 256 colors only.
	return dynamic_cast<cSurface *>(this)->GetOpaqueBlackIndex();
}

// ======================
// Full screen
// ======================
void SurfaceSafe::EnumScreenModes(LPENUMSCREENMODESPROC pProc, LPVOID lParam) {
	// TODO: Check surface is valid?
	// TODO: Check driver type?
	// TODO: Check pProc is valid
	dynamic_cast<cSurface *>(this)->EnumScreenModes(pProc, lParam);
}
bool SurfaceSafe::SetScreenMode(HWND hWnd, size_t width, size_t height, size_t depth) {
	// TODO: Check surface is valid?
	// TODO: Check driver type?
	// TODO: Check window handle is valid
	// TODO: Check width/height is valid
	// TODO: Check depth is valid
	return dynamic_cast<cSurface *>(this)->SetScreenMode(hWnd, (int)width, (int)height, (int)depth) != FALSE;
}
void SurfaceSafe::RestoreWindowedMode(HWND hWnd) {
	// TODO: Check surface is valid?
	// TODO: Check driver type?
	// TODO: Check window handle is valid
	// TODO: Check windowed mode was stored? we're restoring it, not sure if that implies storing, or if it's "not maximize, not minimize" restoring that Windows uses
	dynamic_cast<cSurface *>(this)->RestoreWindowedMode(hWnd);
}
void SurfaceSafe::CopyScreenModeInfo(cSurface* pSrc) {
	// TODO: Check surface is valid
	// TODO: Check driver type?
	// TODO: Check pSrc is valid
	dynamic_cast<cSurface *>(this)->CopyScreenModeInfo(pSrc);
}

// TODO: YQ: Why int, not bool?
bool SurfaceSafe::SetAutoVSync(int nAutoVSync) {
#ifdef HWABETA
	// TODO: Check surface is valid
	// TODO: Check driver type?
	// TODO: Check nAutoVSync is valid?
	return dynamic_cast<cSurface *>(this)->SetAutoVSync(nAutoVSync) != FALSE;
#else
	throw std::runtime_error("Not HWA");
#endif
}
bool SurfaceSafe::WaitForVBlank() {
	// TODO: Check surface is valid
	// TODO: Check driver type?
	return dynamic_cast<cSurface *>(this)->WaitForVBlank() != FALSE;
}

// System colors
COLORREF SurfaceSafe::GetSysColor(std::uint8_t sysColorIndex) {
	// COLOR_MENUBAR (30) seems to be the highest sys color index, so we're using a uint8
	return cSurface::GetSysColor((int)sysColorIndex);
}
// TODO: YQ: Is this meant to be a callback?
void SurfaceSafe::OnSysColorChange() {
	cSurface::OnSysColorChange();
}

// Transparent color
void SurfaceSafe::SetTransparentColor(COLORREF rgb) {
	// TODO: Check surface is valid
	// TODO: Check no alpha channel?
	// TODO: Check rgb is valid?
	dynamic_cast<cSurface *>(this)->SetTransparentColor(rgb);
}
COLORREF SurfaceSafe::GetTransparentColor() {
	// TODO: Check surface is valid
	// TODO: Check no alpha channel?
	// TODO: Check rgb is valid?
	return dynamic_cast<cSurface *>(this)->GetTransparentColor();
}
std::uint8_t SurfaceSafe::GetTransparentColorIndex() {
	// TODO: Check surface is valid
	// TODO: Check has transparent color, and palatte
	// TODO: Check no alpha channel?
	// All signs point to palette being limited to 256 entries, so I'm using a uint8.
	return (std::uint8_t)dynamic_cast<cSurface *>(this)->GetTransparentColorIndex();
}

// Alpha channel
bool SurfaceSafe::HasAlpha() {
	// TODO: Check surface is valid?
	return (std::uint8_t)dynamic_cast<cSurface *>(this)->HasAlpha();
}
std::byte * SurfaceSafe::LockAlpha() {
	// TODO: Check surface is valid
	// TODO: Check surface has alpha?
	// TODO: Check alpha not already locked
	// TODO: Check color surface is not locked?
	alphaLock = dynamic_cast<cSurface *>(this)->LockAlpha();
	return (std::byte *)alphaLock;
}
void SurfaceSafe::UnlockAlpha() {
	// TODO: Check surface is valid
	// TODO: Check alpha locked
	dynamic_cast<cSurface *>(this)->UnlockAlpha();
	alphaLock = nullptr;
}
int SurfaceSafe::GetAlphaPitch() {
	// TODO: Check surface is valid
	// TODO: Check surface has alpha?
	return dynamic_cast<cSurface *>(this)->GetAlphaPitch();
}
void SurfaceSafe::CreateAlpha() {
	// TODO: Check surface is valid
	// TODO: Check surface driver/type can have alpha?
	// TODO: Check alpha not already created?
	return dynamic_cast<cSurface *>(this)->CreateAlpha();
}
void SurfaceSafe::SetAlpha(std::byte * alphaBytes, int alphaPitch) {
	// TODO: Check surface is valid
	// TODO: Check surface driver/type can have alpha?
	// TODO: Check alpha not already created?
	return dynamic_cast<cSurface *>(this)->SetAlpha((LPBYTE)alphaBytes, alphaPitch);
}
void SurfaceSafe::AttachAlpha(std::byte * alphaBytes, int alphaPitch) {
	// TODO: Check surface is valid
	// TODO: Check surface driver/type can have alpha?
	// TODO: Check alpha not already created?
	return dynamic_cast<cSurface *>(this)->SetAlpha((LPBYTE)alphaBytes, alphaPitch);

}
std::byte * SurfaceSafe::DetachAlpha(int * alphaPitch) {
	// TODO: Check surface is valid
	// TODO: Check alpha is attached
	return (std::byte *)dynamic_cast<cSurface *>(this)->DetachAlpha((LPLONG)alphaPitch);
}
SurfaceSafe * SurfaceSafe::GetAlphaSurface() {
	// TODO: Check surface is valid
	// TODO: Check surface has alpha
	// TODO: Check surface does not have released alpha surface already
	SurfaceSafe * ss = new SurfaceSafe();
	dynamic_cast<cSurface &>(*ss) = *dynamic_cast<cSurface *>(this)->GetAlphaSurface();
	return ss;
}
void SurfaceSafe::ReleaseAlphaSurface(cSurface* alphaSurf) {
	// TODO: Check surface is valid
	// TODO: Check surface has this released alpha surface
	dynamic_cast<cSurface *>(this)->ReleaseAlphaSurface(alphaSurf);
}

// Transparent monochrome mask
ULONG SurfaceSafe::CreateMask(CollisionMask * pMask, CreateCollisionMaskFlags createMaskFlags) {
	// TODO: Check surface is valid
	// TODO: Check pMask is valid
	// TODO: Check createMaskFlags is valid
	return dynamic_cast<cSurface *>(this)->CreateMask(pMask, (UINT)createMaskFlags);
}

// Lost device callback
void SurfaceSafe::OnLostDevice() {
	#ifdef HWABETA
		dynamic_cast<cSurface *>(this)->OnLostDevice();
	#else
		throw std::runtime_error("Not HWA");
	#endif
}
void SurfaceSafe::AddLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam) {
	#ifdef HWABETA
		// TODO: Check pCallback is valid
		dynamic_cast<cSurface *>(this)->AddLostDeviceCallBack(pCallback, lUserParam);
	#else
		throw std::runtime_error("Not HWA");
	#endif
}
void SurfaceSafe::RemoveLostDeviceCallBack(LOSTDEVICECALLBACKPROC pCallback, LPARAM lUserParam) {
	#ifdef HWABETA
		// TODO: Check pCallback is valid
		dynamic_cast<cSurface *>(this)->RemoveLostDeviceCallBack(pCallback, lUserParam);
	#else
		throw std::runtime_error("Not HWA");
	#endif
}
