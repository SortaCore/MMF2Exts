#ifndef ImgFltMMf_h
#define ImgFltMMf_h

// Image filters
#define IMPORT_IMAGE_USESURFACEDEPTH		0x0001
#define IMPORT_IMAGE_USESURFACEPALETTE		0x0002

DLLExport32 BOOL WINAPI ImportImage(CImageFilterMgr* pImgMgr, LPCSTR fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);
DLLExport32 BOOL WINAPI CanImportImage(CImageFilterMgr* pImgMgr, LPCSTR fileName);
DLLExport32 BOOL WINAPI ImportPalette(CImageFilterMgr* pImgMgr, LPCSTR fileName, LPLOGPALETTE pPal);
DLLExport32 BOOL WINAPI ExportImage(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, DWORD dwFilterID);
DLLExport32 BOOL WINAPI CanImportAnimation(CImageFilterMgr* pImgMgr, LPCSTR fileName, BOOL* bNeedConversion);
DLLExport32 BOOL WINAPI BeginAnimationImport(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags);
DLLExport32 BOOL WINAPI GetNextAnimationFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter);
DLLExport32 void WINAPI EndAnimationImport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter);
DLLExport32 BOOL WINAPI BeginAnimationExport(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame);
DLLExport32 BOOL WINAPI ExportNextFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter, int msFrameDuration);
DLLExport32 void WINAPI EndAnimationExport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter);

// Picture selector
#define	PICSEL_IMAGES	0x0001
#define	PICSEL_ANIMS	0x0002

DLLExport32 BOOL WINAPI ChoosePicture (LPOPENFILENAME pOFN, BOOL bOpen, CImageFilterMgr* pImgMgr, LPDWORD pDWFilterID, DWORD dwFlags);

#endif // ImgFltMMf_h
