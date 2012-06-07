
#ifndef _ImageFlt_h
#define _ImageFlt_h

#include "FilterMgr.h"
#include "ImgFltDefs.h"

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

class IMGFLTMGR_API CImageFilterMgr : public CFilterMgr
{
// Public
public:
	CImageFilterMgr ();
	virtual ~CImageFilterMgr();

	static CImageFilterMgr* CreateInstance();

	// Initialize
	void	Initialize(LPCSTR pFilterPath, DWORD dwFlags);
	void	Initialize(LPCWSTR pFilterPath, DWORD dwFlags);
	virtual void		Free();

	// Filter infos
	BOOL		DoesFilterSupportImages(int nIndex);
	BOOL		DoesFilterSupportAnimations(int nIndex);

	// Data
protected:
	friend class CImageFilter;
};

//////////////////////////////////////////////////////////////////////////////
//
// Image Filter
//
class IMGFLTMGR_API CImageFilter
{
public:
	// Constructeur / destructeur
	CImageFilter(CImageFilterMgr* pMgr);
    ~CImageFilter();

	// Customisation
	BOOL			UseSpecificFilterIndex(int index);
	BOOL			UseSpecificFilterID(DWORD dwID);

	// Save customization
	void			SetCompressionLevel(int nLevel=-1);		// 0 -> 100

	// Picture
	int				Open(LPCSTR fileName);
	int				Open(LPCWSTR fileName);
	int				Open(CInputFile* pf);
	int				PrepareLoading(int width, int height, int depth, LPLOGPALETTE pPal);
	int				Load(LPBYTE pData, int width, int height, int pitch, int depth=0, LPLOGPALETTE pPal=NULL, LPBYTE pAlpha=NULL, int nAlphaPitch=0);
	int				Save(LPCSTR fileName, LPBYTE pData, int nWidth, int nHeight, int nDepth, int nPitch, LPLOGPALETTE pPal, LPBYTE pAlpha=NULL, int nAlphaPitch=0);
	int				Save(LPCWSTR fileName, LPBYTE pData, int nWidth, int nHeight, int nDepth, int nPitch, LPLOGPALETTE pPal, LPBYTE pAlpha=NULL, int nAlphaPitch=0);
	void			Close();
	int				GetWidth(); 
	int				GetHeight(); 
	int				GetPitch();
	int				GetDepth();
	DWORD			GetDataSize();
	LPLOGPALETTE	GetPalette();
	BOOL			GetTransparentColor(COLORREF* pTranspColor);
	BOOL			ContainsAlphaChannel();

	// Load animation
	BOOL			IsAnimation();
	int				GetNumberOfFrames();
	int				GetCurrentFrame();
	int				GetFrameDelay(int frameIndex=-1);
	DWORD			GetAnimDuration();
	LPBYTE			GetUserInfo();
	DWORD			GetUserInfoSize();

	void			Restart();
	int				GoToImage(LPBYTE pData, int pitch, int n);
	void			GetUpdateRect(LPRECT pRc);
	int				GetLoopCount();
	int				GetLoopFrame();

	// Save animation
	int				CreateAnimation(LPCSTR fname, int width, int height, int depth, 
								int nFrames, int msFrameDuration, 
								int nLoopCount = 1, int nLoopFrame = 0,
								LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0);
	int				CreateAnimation(LPCWSTR fname, int width, int height, int depth, 
								int nFrames, int msFrameDuration, 
								int nLoopCount = 1, int nLoopFrame = 0,
								LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0);
	int				CreateAnimation(COutputFile* pfout, int width, int height, int depth, 
								int nFrames, int msFrameDuration, 
								int nLoopCount = 1, int nLoopFrame = 0,
								LPBYTE pUserInfo = NULL, DWORD dwUserInfoSize = 0);
	int				SaveAnimationFrame(LPBYTE pData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags);
	int				SaveAnimationFrame(LPBYTE pData, LPBYTE pPrevData, int width, int height, int pitch, int depth, LPLOGPALETTE pPal, LPBYTE pAlpha, LPBYTE pPrevAlpha, int nAlphaPitch, int msFrameDuration, DWORD dwFlags);
	void			AddPreviousFrameDuration(int msFrameDuration);
	DWORD			GetCurrentSaveAnimSize();

	// File handling
	void			SetProgressCallBack(PROGRESSPROC pProc);

	// Filter info
	DWORD			GetFilterID();
	LPCSTR			GetFilterNameA();
	int				GetFilterIndex();
	DWORD			GetFilterColorCaps();
	BOOL			CanSave();
	BOOL			CanSaveAnim();
	LPCWSTR			GetFilterNameW();

protected:
	int				ReadHeader();
	
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

#endif	// _ImageFlt_h
