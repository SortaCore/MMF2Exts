#pragma once
#ifndef _ImageFlt_h
#define _ImageFlt_h

#include "FilterMgr.hpp"

//////////////////////////////////////
// Forwards

class CImageFilter;
class CImageFilterMgr;
class CFilterImpl;
class CInputFile;
class COutputFile;
class CFilterImpl;

//////////////////////////////////////
// Error codes

enum {
	IF_OK=0,
	IF_NOTENOUGHMEM,
	IF_CANNOTOPENFILE,
	IF_CANNOTCREATEFILE,
	IF_CANNOTWRITEFILE,
	IF_UNKNOWNFORMAT,
	IF_UNSUPPORTEDFORMAT,
	IF_NOFILESPECIFIED,
	IF_NOTSUPPORTED,
	IF_NOTOPEN,
	IF_UNSUPPORTEDDEPTH,
	IF_INVALIDFILE,
	IF_LESSTHAN256COLOR,
	IF_INCORRECTDIMENSIONS,
	IF_BADPARAMETER,
	IF_UNINITIALIZEDFILTER,
	IF_INVALIDPALETTE,
	IF_ALREADYOPEN,
	IF_ENDOFFILE,
	IF_NOTANIMATIONFILTER,
	IF_UNKNOWNERROR,
};

// Filter color caps
#define	FLTCOLORCAPS_1	0x0001
#define	FLTCOLORCAPS_4	0x0002
#define	FLTCOLORCAPS_8	0x0004
#define	FLTCOLORCAPS_15	0x0008
#define	FLTCOLORCAPS_16	0x0010
#define	FLTCOLORCAPS_24	0x0020
#define	FLTCOLORCAPS_32	0x0040

// Filter type
#define FLTTYPE_IMAGES		0x0001
#define FLTTYPE_ANIMATIONS	0x0002

// Compression
#define	COMPRESSION_DEFAULT	(-1)
#define	COMPRESSION_MIN		0
#define	COMPRESSION_MAX		100

// SaveAnimation flags
#define	SAVEANIMFLAG_SEPARATEFRAMES		0x00000001

// Progress proc
typedef BOOL (CALLBACK * PROGRESSPROC) (int p);


//////////////////////////////////////////////////////////////////////////////
//
// Image Filter Manager
//

class FusionAPIImport CImageFilterMgr : public CFilterMgr
{
// Public
public:
	MMF2_ORD(243) CImageFilterMgr () EXDEF;
	MMF2_ORD(295) CImageFilterMgr(const CImageFilterMgr &) EXDEF;
	virtual MMF2_ORD(273) ~CImageFilterMgr() EXDEF;

	static MMF2_ORD(794) CImageFilterMgr* CreateInstance() EXDEF;

	// Initialize
	PreferFusionFunc_Unicode(1053, "CImageFilterMgr::Initialize") MMF2_ORD(591) void Initialize(LPCSTR pFilterPath, DWORD dwFlags) EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1053) void Initialize(LPCWSTR pFilterPath, DWORD dwFlags) EXDEF;
	virtual MMF2_ORD(456) void Free() EXDEF;

	// Filter infos
	MMF2_ORD(408) BOOL DoesFilterSupportImages(int nIndex) EXDEF;
	MMF2_ORD(407) BOOL DoesFilterSupportAnimations(int nIndex) EXDEF;

	// Image premultiplication, CF2.5 292.02 and laterz
	CF25_292_02_REQUIRED(1128) BOOL SetAutoPremultiplyImages(BOOL bPremultiply = TRUE) EXDEF;
	CF25_292_02_REQUIRED(1127) BOOL GetAutoPremultiplyImages() EXDEF;

	// Data
protected:
	// Image premultiplication, CF2.5 292.02 and later
	BOOL m_bAutoPremultiplyImages;
};

//////////////////////////////////////////////////////////////////////////////
//
// Image Filter
//
class FusionAPIImport CImageFilter
{
public:
	// Constructeur / destructeur
	MMF2_ORD(241) CImageFilter(CImageFilterMgr* pMgr) EXDEF;
	MMF2_ORD(272) ~CImageFilter() EXDEF;

	// Customisation
	MMF2_ORD(770) BOOL UseSpecificFilterIndex(int index) EXDEF;
	MMF2_ORD(769) BOOL UseSpecificFilterID(DWORD dwID) EXDEF;

	// Save customization
	MMF2_ORD(728) void SetCompressionLevel(int nLevel=-1) EXDEF;		// 0 -> 100

	// Picture
	PreferFusionFunc_Unicode(1064, "CImageFilter::Open") MMF2_ORD(646) int Open(LPCSTR fileName) EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1064) int Open(const UShortWCHAR* fileName) EXDEF;
	MMF2_ORD(645) int Open(CInputFile* pf) EXDEF;
	MMF2_ORD(655) int PrepareLoading(int width, int height, int depth, LPLOGPALETTE pPal) EXDEF;
	MMF2_ORD(613) int Load(LPBYTE pData, int width, int height, int pitch, int depth=0, LPLOGPALETTE pPal=NULL, LPBYTE pAlpha=NULL, int nAlphaPitch=0) EXDEF;
	PreferFusionFunc_Unicode(1065, "CImageFilter::Save") MMF2_ORD(705) int Save(LPCSTR fileName, LPBYTE pData, int nWidth, int nHeight, int nDepth, int nPitch, LPLOGPALETTE pPal, LPBYTE pAlpha=NULL, int nAlphaPitch=0) EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1065) int Save(const UShortWCHAR* fileName, LPBYTE pData, int nWidth, int nHeight, int nDepth, int nPitch, LPLOGPALETTE pPal, LPBYTE pAlpha=NULL, int nAlphaPitch=0) EXDEF;
	MMF2_ORD(356) void Close() EXDEF;
	MMF2_ORD(584) int GetWidth() EXDEF;
	MMF2_ORD(519) int GetHeight() EXDEF;
	MMF2_ORD(550) int GetPitch() EXDEF;
	MMF2_ORD(486) int GetDepth() EXDEF;
	MMF2_ORD(483) DWORD GetDataSize() EXDEF;
	MMF2_ORD(545) LPLOGPALETTE GetPalette() EXDEF;
	MMF2_ORD(816) BOOL GetTransparentColor(COLORREF* pTranspColor) EXDEF;
	MMF2_ORD(359) BOOL ContainsAlphaChannel() EXDEF;

	// Load animation
	MMF2_ORD(601) BOOL IsAnimation() EXDEF;
	MMF2_ORD(539) int GetNumberOfFrames() EXDEF;
	MMF2_ORD(480) int GetCurrentFrame() EXDEF;
	MMF2_ORD(515) int GetFrameDelay(int frameIndex=-1) EXDEF;
	MMF2_ORD(464) DWORD GetAnimDuration() EXDEF;
	MMF2_ORD(578) LPBYTE GetUserInfo() EXDEF;
	MMF2_ORD(580) DWORD GetUserInfoSize() EXDEF;

	MMF2_ORD(693) void Restart() EXDEF;
	MMF2_ORD(586) int GoToImage(LPBYTE pData, int pitch, int n) EXDEF;
	MMF2_ORD(575) void GetUpdateRect(LPRECT pRc) EXDEF;
	MMF2_ORD(529) int GetLoopCount() EXDEF;
	MMF2_ORD(532) int GetLoopFrame() EXDEF;

	// Save animation
	PreferFusionFunc_Unicode(1027, "CImageFilter::CreateAnimation") MMF2_ORD(378) int CreateAnimation(LPCSTR fname, int width, int height, int depth, int nFrames, int msFrameDuration, int nLoopCount = 1, int nLoopFrame = 0, LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0) EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1027) int CreateAnimation(const UShortWCHAR * fname, int width, int height, int depth, int nFrames, int msFrameDuration, int nLoopCount = 1, int nLoopFrame = 0, LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0) EXDEF;
	MMF2_ORD(377) int CreateAnimation(COutputFile* pfout, int width, int height, int depth, int nFrames, int msFrameDuration, int nLoopCount = 1, int nLoopFrame = 0, LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0) EXDEF;
	MMF2_ORD(708) int SaveAnimationFrame(LPBYTE pData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags) EXDEF;
	MMF2_ORD(707) int SaveAnimationFrame(LPBYTE pData, LPBYTE pPrevData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, LPBYTE pPrevAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags) EXDEF;
	MMF2_ORD(332) void AddPreviousFrameDuration(int msFrameDuration) EXDEF;
	MMF2_ORD(481) DWORD GetCurrentSaveAnimSize() EXDEF;

	// File handling
	MMF2_ORD(749) void SetProgressCallBack(PROGRESSPROC pProc) EXDEF;

	// Filter info
	MMF2_ORD(507) DWORD GetFilterID() EXDEF;
	PreferFusionFunc_Unicode(1047, "CImageFilter::GetFilterNameW") MMF2_ORD(512) LPCSTR GetFilterNameA() EXDEF;
	MMF2_ORD(509) int GetFilterIndex() EXDEF;
	MMF2_ORD(501) DWORD GetFilterColorCaps() EXDEF;
	MMF2_ORD(348) BOOL CanSave() EXDEF;
	MMF2_ORD(350) BOOL CanSaveAnim() EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1047) const UShortWCHAR * GetFilterNameW() EXDEF;

protected:
	MMF2_ORD(664) int ReadHeader() EXDEF;

	// Data
protected:

	// Image filter manager
	CImageFilterMgr* m_pMgr;

	// Input
	CInputFile*		m_pfin;						// Input file
	BOOL			m_bAutoDeletePfin;
	BOOL			m_bPrepared;

	// Output
	COutputFile*	m_pfout;					// Output file
	BOOL			m_bAutoDeletePfout;
	LPBYTE			m_prevBuf;

	// Progress callback
	PROGRESSPROC	m_pProgressProc;

	// Filter implementation
	CFilterImpl*	m_pFilterImpl;
};
typedef	CImageFilter * LPIMAGEFILTER;

class FusionAPIImport CImageFilterANSI20
{
public:
	// Constructeur / destructeur
	MMF2_ORD(241) CImageFilterANSI20(CImageFilterMgr* pMgr) EXDEF;
	MMF2_ORD(272) ~CImageFilterANSI20() EXDEF;

	// Customisation
	MMF2_ORD(770) BOOL UseSpecificFilterIndex(int index) EXDEF;
	MMF2_ORD(769) BOOL UseSpecificFilterID(DWORD dwID) EXDEF;

	// Save customization
	MMF2_ORD(728) void SetCompressionLevel(int nLevel = -1) EXDEF;		// 0 -> 100

	// Picture
	MMF2_ORD(646) int Open(LPCSTR fileName) EXDEF;
	MMF2_ORD(645) int Open(CInputFile* pf) EXDEF;
	MMF2_ORD(655) int PrepareLoading(int width, int height, int depth, LPLOGPALETTE pPal) EXDEF;
	MMF2_ORD(613) int Load(LPBYTE pData, int width, int height, int pitch, int depth = 0, LPLOGPALETTE pPal = NULL, LPBYTE pAlpha = NULL, int nAlphaPitch = 0) EXDEF;
	MMF2_ORD(705) int Save(LPCSTR fileName, LPBYTE pData, int nWidth, int nHeight, int nDepth, int nPitch, LPLOGPALETTE pPal, LPBYTE pAlpha = NULL, int nAlphaPitch = 0) EXDEF;
	MMF2_ORD(356) void Close() EXDEF;
	MMF2_ORD(584) int GetWidth() EXDEF;
	MMF2_ORD(519) int GetHeight() EXDEF;
	MMF2_ORD(550) int GetPitch() EXDEF;
	MMF2_ORD(486) int GetDepth() EXDEF;
	MMF2_ORD(483) DWORD GetDataSize() EXDEF;
	MMF2_ORD(545) LPLOGPALETTE GetPalette() EXDEF;
	MMF2_ORD(816) BOOL GetTransparentColor(COLORREF* pTranspColor) EXDEF;
	MMF2_ORD(359) BOOL ContainsAlphaChannel() EXDEF;

	// Load animation
	MMF2_ORD(601) BOOL IsAnimation() EXDEF;
	MMF2_ORD(539) int GetNumberOfFrames() EXDEF;
	MMF2_ORD(480) int GetCurrentFrame() EXDEF;
	MMF2_ORD(515) int GetFrameDelay(int frameIndex = -1) EXDEF;
	MMF2_ORD(464) DWORD GetAnimDuration() EXDEF;
	MMF2_ORD(578) LPBYTE GetUserInfo() EXDEF;
	MMF2_ORD(580) DWORD GetUserInfoSize() EXDEF;

	MMF2_ORD(693) void Restart() EXDEF;
	MMF2_ORD(586) int GoToImage(LPBYTE pData, int pitch, int n) EXDEF;
	MMF2_ORD(575) void GetUpdateRect(LPRECT pRc) EXDEF;
	MMF2_ORD(529) int GetLoopCount() EXDEF;
	MMF2_ORD(532) int GetLoopFrame() EXDEF;

	// Save animation
	MMF2_ORD(378) int CreateAnimation(LPCSTR fname, int width, int height, int depth, int nFrames, int msFrameDuration, int nLoopCount = 1, int nLoopFrame = 0, LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0) EXDEF;
	MMF2_ORD(377) int CreateAnimation(COutputFile* pfout, int width, int height, int depth, int nFrames, int msFrameDuration, int nLoopCount = 1, int nLoopFrame = 0, LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0) EXDEF;
	MMF2_ORD(708) int SaveAnimationFrame(LPBYTE pData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags) EXDEF;
	MMF2_ORD(707) int SaveAnimationFrame(LPBYTE pData, LPBYTE pPrevData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, LPBYTE pPrevAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags) EXDEF;
	MMF2_ORD(332) void AddPreviousFrameDuration(int msFrameDuration) EXDEF;
	MMF2_ORD(481) DWORD GetCurrentSaveAnimSize() EXDEF;

	// File handling
	MMF2_ORD(749) void SetProgressCallBack(PROGRESSPROC pProc) EXDEF;

	// Filter info
	MMF2_ORD(507) DWORD GetFilterID() EXDEF;
	MMF2_ORD(512) LPCSTR GetFilterNameA() EXDEF;
	MMF2_ORD(509) int GetFilterIndex() EXDEF;
	MMF2_ORD(501) DWORD GetFilterColorCaps() EXDEF;
	MMF2_ORD(348) BOOL CanSave() EXDEF;
	MMF2_ORD(350) BOOL CanSaveAnim() EXDEF;
	MMF2_UNICODE_OR_CF25_REQUIRED(1047) LPCWSTR GetFilterNameW() EXDEF;

protected:
	MMF2_ORD(664) int ReadHeader() EXDEF;

	// Data
protected:

	// Image filter manager
	CImageFilterMgr* m_pMgr;

	// Input
	CInputFile* m_pfin;						// Input file
	BOOL			m_bAutoDeletePfin;
	BOOL			m_bPrepared;

	// Output
	COutputFile* m_pfout;					// Output file
	BOOL			m_bAutoDeletePfout;
	LPBYTE			m_prevBuf;

	// Progress callback
	PROGRESSPROC	m_pProgressProc;

	// Filter implementation
	CFilterImpl* m_pFilterImpl;
};

FusionAPIImport PreferFusionFunc_Unicode(1050, "ImportImageW") MMF2_ORD(136) BOOL FusionAPI ImportImageA(CImageFilterMgr* pImgMgr, LPCSTR fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags) EXDEF;
FusionAPIImport MMF2_ORD(137) BOOL FusionAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1022, "CanImportImageW") MMF2_ORD(138) BOOL FusionAPI CanImportImageA(CImageFilterMgr* pImgMgr, LPCSTR fileName) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1051, "ImportPaletteW") MMF2_ORD(139) BOOL FusionAPI ImportPaletteA(CImageFilterMgr* pImgMgr, LPCSTR fileName, LPLOGPALETTE pPal) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1030, "ExportImageW") MMF2_ORD(140) BOOL FusionAPI ExportImageA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, DWORD dwFilterID) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1021, "CanImportAnimationW") MMF2_ORD(141) BOOL FusionAPI CanImportAnimationA(CImageFilterMgr* pImgMgr, LPCSTR fileName, BOOL* bNeedConversion) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1020, "BeginAnimationImportW") MMF2_ORD(142) BOOL FusionAPI BeginAnimationImportA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags) EXDEF;
FusionAPIImport MMF2_ORD(143) BOOL FusionAPI GetNextAnimationFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter) EXDEF;
FusionAPIImport MMF2_ORD(144) void FusionAPI EndAnimationImport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter) EXDEF;
FusionAPIImport PreferFusionFunc_Unicode(1019, "BeginAnimationExportW") MMF2_ORD(145) BOOL FusionAPI BeginAnimationExportA(CImageFilterMgr* pImgMgr, LPCSTR pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame) EXDEF;
FusionAPIImport MMF2_ORD(146) BOOL FusionAPI ExportNextFrame(CImageFilterMgr* pImgMgr, cSurface* psf, CImageFilter* pFilter, int msFrameDuration) EXDEF;
FusionAPIImport MMF2_ORD(147) void FusionAPI EndAnimationExport(CImageFilterMgr* pImgMgr, CImageFilter* pFilter) EXDEF;

FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1050) BOOL FusionAPI ImportImageW(CImageFilterMgr* pImgMgr, const UShortWCHAR* fileName, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1022) BOOL FusionAPI CanImportImageW(CImageFilterMgr* pImgMgr, const UShortWCHAR* fileName) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1051) BOOL FusionAPI ImportPaletteW(CImageFilterMgr* pImgMgr, const UShortWCHAR* fileName, LPLOGPALETTE pPal) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1030) BOOL FusionAPI ExportImageW(CImageFilterMgr* pImgMgr, const UShortWCHAR* pFileName, cSurface* psf, DWORD dwFilterID) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1021) BOOL FusionAPI CanImportAnimationW(CImageFilterMgr* pImgMgr, const UShortWCHAR* fileName, BOOL* bNeedConversion) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1020) BOOL FusionAPI BeginAnimationImportW(CImageFilterMgr* pImgMgr, const UShortWCHAR* pFileName, cSurface* psf, CImageFilter* pFilter, DWORD dwFlags) EXDEF;
FusionAPIImport MMF2_UNICODE_OR_CF25_REQUIRED(1019) BOOL FusionAPI BeginAnimationExportW(CImageFilterMgr* pImgMgr, const UShortWCHAR* pFileName, cSurface* psf, CImageFilter* pFilter, int nFrames, int msFrameDuration, int nLoopCount, int nLoopFrame) EXDEF;

#ifdef _UNICODE
#define ImportImage ImportImageW
#define CanImportImage CanImportImageW
#define ImportPalette ImportPaletteW
#define ExportImage ExportImageW
#define CanImportAnimation CanImportAnimationW
#define BeginAnimationImport BeginAnimationImportW
#define BeginAnimationExport BeginAnimationExportW
#define BeginAnimationExport BeginAnimationExportW
#else
#define ImportImage ImportImageA
#define CanImportImage CanImportImageA
#define ImportPalette ImportPaletteA
#define ExportImage ExportImageA
#define CanImportAnimation CanImportAnimationA
#define BeginAnimationImport BeginAnimationImportA
#define BeginAnimationExport BeginAnimationExportA
#endif

#endif	// _ImageFlt_h
