#ifndef ImgFltMMf_h
#define ImgFltMMf_h

// Image filters
#define IMPORT_IMAGE_USESURFACEDEPTH		0x0001
#define IMPORT_IMAGE_USESURFACEPALETTE		0x0002
#define IMPORT_IMAGE_FIRSTPIXELTRANSP		0x0004
#define IMPORT_IMAGE_OPAQUE					0x0008

DLLExport32 BOOL WINAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI GetNextAnimationFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter);
DLLExport32 void WINAPI EndAnimationImport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter);
DLLExport32 BOOL WINAPI ExportNextFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter, int msFrameDuration);
DLLExport32 void WINAPI EndAnimationExport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter);

#ifdef _UNICODE

DLLExport32 BOOL WINAPI ImportImageA(CImageFilterMgr* pImgMgr, LPCSTR fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI CanImportImageA(CImageFilterMgr* pImgMgr, LPCSTR fileName);
DLLExport32 BOOL WINAPI ImportPaletteA(CImageFilterMgr* pImgMgr, LPCSTR fileName, LPLOGPALETTE pPal);
DLLExport32 BOOL WINAPI ExportImageA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, DWORD dwFilterID);
DLLExport32 BOOL WINAPI CanImportAnimationA(CImageFilterMgr* pImgMgr, LPCSTR fileName, BOOL* bNeedConversion);
DLLExport32 BOOL WINAPI BeginAnimationImportA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags);
DLLExport32 BOOL WINAPI BeginAnimationExportA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame);

DLLExport32 BOOL WINAPI ImportImageW(CImageFilterMgr* pImgMgr, LPCWSTR fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI CanImportImageW(CImageFilterMgr* pImgMgr, LPCWSTR fileName);
DLLExport32 BOOL WINAPI ImportPaletteW(CImageFilterMgr* pImgMgr, LPCWSTR fileName, LPLOGPALETTE pPal);
DLLExport32 BOOL WINAPI ExportImageW(CImageFilterMgr* pImgMgr, LPCWSTR pFileName, cSurface* psf, DWORD dwFilterID);
DLLExport32 BOOL WINAPI CanImportAnimationW(CImageFilterMgr* pImgMgr, LPCWSTR fileName, BOOL* bNeedConversion);
DLLExport32 BOOL WINAPI BeginAnimationImportW(CImageFilterMgr* pImgMgr, LPCWSTR pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags);
DLLExport32 BOOL WINAPI BeginAnimationExportW(CImageFilterMgr* pImgMgr, LPCWSTR pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame);

#define ImportImage ImportImageW
#define CanImportImage CanImportImageW
#define ImportPalette ImportPaletteW
#define ExportImage ExportImageW
#define CanImportAnimation CanImportAnimationW
#define BeginAnimationImport BeginAnimationImportW
#define BeginAnimationExport BeginAnimationExportW

#else

DLLExport32 BOOL WINAPI ImportImage(CImageFilterMgr* pImgMgr, LPCSTR fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI CanImportImage(CImageFilterMgr* pImgMgr, LPCSTR fileName);
DLLExport32 BOOL WINAPI ImportPalette(CImageFilterMgr* pImgMgr, LPCSTR fileName, LPLOGPALETTE pPal);
DLLExport32 BOOL WINAPI ExportImage(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, DWORD dwFilterID);
DLLExport32 BOOL WINAPI CanImportAnimation(CImageFilterMgr* pImgMgr, LPCSTR fileName, BOOL* bNeedConversion);
DLLExport32 BOOL WINAPI BeginAnimationImport(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags);
DLLExport32 BOOL WINAPI BeginAnimationExport(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame);

#define ImportImageA ImportImage
#define CanImportImageA CanImportImage
#define ImportPaletteA ImportPalette
#define ExportImageA ExportImage
#define CanImportAnimationA CanImportAnimation
#define BeginAnimationImportA BeginAnimationImport
#define BeginAnimationExportA BeginAnimationExport

#endif

// Picture selector
#define	PICSEL_IMAGES	0x0001
#define	PICSEL_ANIMS	0x0002

DLLExport32 BOOL WINAPI ChoosePictureA (LPOPENFILENAMEA pOFN, BOOL bOpen, CImageFilterMgr* pImgMgr, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI ChoosePictureW (LPOPENFILENAMEW pOFN, BOOL bOpen, CImageFilterMgr* pImgMgr, LPDWORD pDWFilterID, DWORD dwFlags);

#ifdef _UNICODE
#define ChoosePicture ChoosePictureW
#else
#define ChoosePicture ChoosePictureA
#endif

#endif // ImgFltMMf_h
