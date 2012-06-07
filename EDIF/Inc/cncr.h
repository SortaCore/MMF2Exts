
#ifndef _cncinc_h
#define _cncinc_h

#include "cncy.h"
#include "cncf.h"

#ifdef __cplusplus
class cSurface;
class CTransition;
class CDebugger;
class CRunApp;
#ifdef HWABETA
class CEffectEx;
class CPList;
#endif
#else
#define cSurface void
#define CTransition void
#define CDebugger void
#ifdef HWABETA
#define CEffectEx void
#define CPList void
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
//
// LO (frame object instance)
//
//
// Note: mainly used at runtime

#ifdef __cplusplus
class LO {
public:
#else
typedef struct LO {
#endif

	WORD	loHandle;			// HLO
	WORD	loOiHandle;			// HOI
	int		loX;				// Coords
	int		loY;
	WORD	loParentType;		// Parent type
	WORD	loOiParentHandle;	// HOI Parent
	WORD	loLayer;			// Layer
	WORD	loType;
	npSpr	loSpr[4];			// Sprite handles for backdrop objects from layers > 1

#ifdef __cplusplus
};
#else
} LO;
#endif
typedef LO *LPLO;
typedef	LO *fpLevObj;


//////////////////////////////////////////////////////////////////////////////
//
// OI (frame object)
//

#define	OILF_OCLOADED			0x0001	//
#define	OILF_ELTLOADED			0x0002	//
#define	OILF_TOLOAD				0x0004	//
#define	OILF_TODELETE			0x0008	//
#define	OILF_CURFRAME			0x0010	//
#define	OILF_TORELOAD			0x0020	// Reload images when frame change
#define	OILF_IGNORELOADONCALL	0x0040	// Ignore load on call option

////////////////////
// ObjInfo structure
////////////////////
//
// Note: mainly used at runtime
// only some members of the oiHdr member are valid at editing time
//
#ifdef OI
#undef OI
#endif

#ifdef __cplusplus
class OI {
public:
#else
typedef struct OI {
#endif
	ObjInfoHeader	oiHdr;			// Header
	LPTSTR			oiName;			// Name
	LPOC			oiOC;			// ObjectsCommon

	DWORD			oiFileOffset;
	DWORD			oiLoadFlags;
	WORD			oiLoadCount;
	WORD			oiCount;

#ifdef HWABETA
	LPBYTE			oiExtEffect;
	CEffectEx*		oiExtEffectEx;	// Pour backdrops, faut trouver un système plus efficace (banque d'effets pour les backdrops)
#endif // HWABETA

#ifdef __cplusplus
};
#else
} OI;
#endif
typedef OI *LPOI;
typedef	OI* fpObjInfo;

// Runtime only
#ifdef RUN_TIME

///////////////////////////////////////////////
//
// Added backdrop objects
//
typedef struct bkd2 {

	WORD	loHnd;			// 0 
	WORD	oiHnd;			// 0 
	int		x;
	int		y;
	WORD	img;
	WORD	colMode;
	WORD	nLayer;
	WORD	obstacleType;
	npSpr	pSpr[4];
	DWORD	inkEffect;
	DWORD	inkEffectParam;

} bkd2;
typedef bkd2 *LPBKD2;

#define MAX_BKD2	(curFrame.m_maxObjects)

///////////////////////////////////////////////
//
// Layer at runtime
//
typedef struct RunFrameLayer
{
	// Name
	LPTSTR		pName;			// Name

	// Offset
	int			x;				// Current offset
	int			y;
	int			dx;				// Offset to apply to the next refresh
	int			dy;

	// Added backdrops
	int			m_nBkd2Count;
	LPBKD2		m_pBkd2;

	// Ladders
	int			m_nLadderMax;
	int			m_nLadders;
	LPRECT		m_pLadders;

	// Z-order max index for dynamic objects
	int			m_nZOrderMax;

	// Permanent data (EditFrameLayer)
	DWORD		dwOptions;		// Options
	float		xCoef;
	float		yCoef;
	DWORD		nBkdLOs;		// Number of backdrop objects
	DWORD		nFirstLOIndex;	// Index of first backdrop object in LO table

	// EditFrameLayerEffect
#ifdef HWABETA
	DWORD		dwEffect;
	LPARAM		dwEffectParam;	// CEffectEx si extended
#endif // HWABETA

	// Backup for restart
	DWORD		backUp_dwOptions;
	float		backUp_xCoef;
	float		backUp_yCoef;
	DWORD		backUp_nBkdLOs;
	DWORD		backUp_nFirstLOIndex;

#ifdef HWABETA
//	DWORD		backUp_dwEffect;		// A voir
//	LPARAM		backUp_dwEffectParam;
#endif // HWABETA

} RunFrameLayer;

// Object transition data
typedef struct objTransInfo {

	CTransition*	m_pTrans;				// Transition object
	cSurface*		m_SfSave;				// Background surface
	cSurface*		m_SfDisplay;			// Working surface
	cSurface*		m_Sf1;					// Source surface
	cSurface*		m_Sf2;					// Destination surface
	BOOL			m_fadeOut;				// fade type
	BOOL			m_fill1withbackground;	// source = background (opaque mode)
	BOOL			m_fill2withbackground;	// dest = background (opaque mode)
	BOOL			m_saved;				// m_SfSave has been updated
	int				m_xsave;
	int				m_ysave;
	int				m_cxsave;
	int				m_cysave;
#ifdef HWABETA
	BOOL			m_bStepDrawBlit;		// Use StepDrawBlit instead of StepDraw
#endif

} objTransInfo;
typedef objTransInfo * LPOBJTRANSINFO;

//////////////////////////////////////////////////////////////////////////////
//
// Frame at runtime
//

#define	MAX_TEMPSTRING	16
#define IPHONEOPT_JOYSTICK_FIRE1 0x0001
#define IPHONEOPT_JOYSTICK_FIRE2 0x0002
#define IPHONEOPT_JOYSTICK_LEFTHAND 0x0004
#define	IPHONEFOPT_MULTITOUCH			0x0008
#define	IPHONEFOPT_SCREENLOCKING		0x0010
#define	IPHONEFOPT_IPHONEFRAMEIAD		0x0020
#define JOYSTICK_NONE 0x0000
#define JOYSTICK_TOUCH 0x0001
#define JOYSTICK_ACCELEROMETER 0x0002
#define JOYSTICK_EXT 0x0003


#ifdef __cplusplus
class CRunFrame {
public:
#else
typedef struct CRunFrame {
#endif

	// Header
	FrameHeader		m_hdr;

	// Name
	LPTSTR			m_name;

	// Palette
	LPLOGPALETTE	m_palette;

	// Background surface
	cSurface*		m_pSurface;

	// Coordinates of top-level pixel in edit window
	int				m_leX;
	int				m_leY;
	int				m_leLastScrlX;
	int				m_leLastScrlY;

	// Layers
	int				m_nLayers;
	RunFrameLayer*	m_pLayers;

	// Virtual rectangle
	RECT			m_leVirtualRect;

	// Instances of frame objects
	int				m_loMaxIndex;
	int				m_loMaxHandle;
	LPWORD			m_lo_handle_to_index;
	LPLO			m_los;

	// Variables for enumeration functions
	int				m_loFranIndex;
	int				m_oiFranIndex;

	// Remap table or palette handle for sub-application
	LPBYTE			m_remapTable;

	// Transitions
	LPBYTE			m_fadeIn;
	LPBYTE			m_fadeOut;
	cSurface*		m_pFadeInSurface1;
	cSurface*		m_pFadeInSurface2;
	CTransition*	m_pTrans;

	// Exit code
	DWORD			m_levelQuit;

	// Events
	int				m_rhOK;				// TRUE when the events are initialized
	LPRH			m_rhPtr;			// Pointer to RunHeader
	LPEVG			m_eventPrograms;
	DWORD			m_free[MAX_EVENTPROGRAMS-1];
	LPOIL			m_oiList;
	LPVOID			m_free0;
	LPWORD			m_qualToOiList;
	LPWORD			m_qualOilPtr;		// Ne pas liberer!
	LPWORD			m_qualOilPtr2;		// Ne pas liberer!
	OINUM*			m_limitBuffer;
	LPDWORD			m_listPointers;
	LPDWORD			m_eventPointers;
	LPQLOAD			m_qualifiers;
	short			m_nQualifiers;

	short			m_nConditions[NUMBEROF_SYSTEMTYPES+OBJ_LAST];
	DWORD			m_free2[MAX_EVENTPROGRAMS];
	WORD			m_wJoystick;
	WORD			m_wIPhoneOptions;
	LPBYTE			m_swapBuffers;
	DWORD			m_objectList;
	LPBYTE			m_destroyList;
	int				m_free3;
	int				m_nPlayers;
	int				m_nPlayersReal;
	int				m_level_loop_state;
	int				m_startLeX;
	int				m_startLeY;
	short			m_maxObjects;
	short			m_maxOI;
	LPOBL			m_oblEnum;
	int				m_oblEnumCpt;
	BOOL			m_eventsBranched;
	BOOL			m_fade;
	DWORD			m_fadeTimerDelta;
	DWORD			m_fadeVblDelta;
	DWORD			m_pasteMask;

	int				m_nCurTempString;
	LPTSTR			m_pTempString[MAX_TEMPSTRING];	// not used

	// Other
	cSurface*		m_pSaveSurface;
	int				m_leEditWinWidth;
	int				m_leEditWinHeight;
	DWORD			m_dwColMaskBits;
	LPTSTR			m_demoFilePath;
	WORD			m_wRandomSeed;
	WORD			m_wFree;
	DWORD			m_dwMvtTimerBase;

#ifdef HWABETA
	LPBYTE			m_pLayerEffects;

	// Frame effect
	FrameEffect*	m_pEffect;						// Frame effect (chunk data, contains effect index & param used in blit)
	CEffectEx*		m_pEffectEx;					// Current effect
	bool			m_bFrameEffectChanged;			// Frame effect has been modified
	bool			m_bAlwaysUseSecondarySurface;	// This frame always use a secondary surface

	// Secondary surface (render target used if background or frame effects)
	cSurface*		m_pSecondarySurface;

	// List of sub-app surfaces to refresh at the end in D3D full screen mode
	CPList*			m_pSurfacedSubApps;
#endif

#ifdef __cplusplus
};
#else
} CRunFrame;
#endif
typedef CRunFrame *fpRunFrame;

////////////////////////
//
// Binary files
//
#ifdef __cplusplus
class CBinaryFile {
public:
	CBinaryFile()	{ m_path[0] = 0; m_pTempPath = NULL; m_fileSize = m_fileOffset = 0; m_tempCount = 0; }
	~CBinaryFile()	{ if ( m_pTempPath != NULL ) { _tremove(m_pTempPath); free(m_pTempPath); m_pTempPath = NULL; m_tempCount = 0; } }

public:
	TCHAR	m_path[_MAX_PATH];		// path stored in ccn file
	LPTSTR	m_pTempPath;			// path in temporary folder, if any
	DWORD	m_fileSize;				// file size
	DWORD	m_fileOffset;			// file offset in EXE/CCN file
	long	m_tempCount;			// usage count
};
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////////
//
// Application
//

#define	ARF_MENUINIT				0x0001
#define	ARF_MENUIMAGESLOADED		0x0002		// menu images have been loaded into memory
#define	ARF_INGAMELOOP				0x0004
#define ARF_PAUSEDBEFOREMODALLOOP	0x0008

#ifdef __cplusplus
class CRunApp {
public:
#else
typedef struct CRunApp {
#endif

	// Application info
	AppMiniHeader	m_miniHdr;			// Version
	AppHeader		m_hdr;				// General info
	LPTSTR			m_name;				// Name of the application
	LPTSTR			m_appFileName;		// filename (temporary file in editor mode)
	LPTSTR			m_editorFileName;	// filename of original .mfa file
	LPTSTR			m_copyright;		// copyright
	LPTSTR			m_aboutText;		// text to display in the About box

	// File infos
	LPTSTR			m_targetFileName;	// filename of original CCN/EXE file
	LPTSTR			m_tempPath;			// Temporary directory for external files
	HFILE			m_file;				// File handle
	DWORD			m_startOffset;

	// Help file
	LPTSTR			m_doc;				// Help file pathname

	// Icon
	LPBYTE			m_icon16x16x8;		// = LPBITMAPINFOHEADER
	HICON			m_hIcon16x16x8;		// Small icon for the main window

	// Menu
	HMENU			m_hRunMenu;			// Menu
	LPBYTE			m_accels;			// Accelerators
	LPTSTR			m_pMenuTexts;		// Menu texts (for ownerdraw menu)
	LPBYTE			m_pMenuImages;		// Images index used in the menu
	MenuHdr*		m_pMenu;

	// Frame offsets
	int				m_frameMaxIndex;			// Max. number of frames
	int				m_frameMaxHandle;			// Max. frame handle
	LPWORD			m_frame_handle_to_index;	// Handle -> index table
	LPDWORD			m_frameOffset;				// Frame offsets in the file

	// Frame passwords
	LPTSTR *			m_framePasswords;			// Table of frame passwords

	// Extensions
	int				m_nbKpx;					// Number of extensions
	fpKpxFunc		m_kpxTab;					// Function table 1
	fpkpdt          m_kpxDataTable;				// Function table 2

	// Movement Extensions
	int				m_nbMvx;					// Number of extensions
	MvxFnc*			m_mvxTable;					// DLL info

	// Elements
	LPTSTR			m_eltFileName[MAX_TABREF];	// Element banks
	HFILE			m_hfElt[MAX_TABREF];

	DWORD			m_eltBaseOff;
	WORD			m_nbEltOff[MAX_TABREF];			// Sizes of file offset tables
	LPDWORD			m_adTabEltOff[MAX_TABREF];		// File offsets of bank elements

	WORD			m_nbEltMemToDisk[MAX_TABREF];	// Size of elt cross-ref tables
	LPWORD			m_EltMemToDisk[MAX_TABREF];		// Element memory index -> element disk index
	LPWORD			m_EltDiskToMem[MAX_TABREF];		// Element disk index -> memory index

	WORD			m_tabNbCpt[MAX_TABREF];			// Sizes of usage count tables
	LPLONG			m_tabAdCpt[MAX_TABREF];			// Usage count tables of bank elements

	// Binary files
	#ifdef STAND_ALONE
		CTypedPList<CBinaryFile>	m_binaryFiles;	// Embeded files
	#else
		UINT		m_binaryFiles[8];
	#endif

	// Temporary images
	UINT			m_nImagesTemp;			// List of temporary images (used by Replace Color action)
	LPBYTE			m_pImagesTemp;

	// Frame objects
	int				m_oiMaxIndex;
	int				m_oiMaxHandle;
	LPWORD			m_oi_handle_to_index;
	LPOI *			m_ois;
	int				m_oiFranIndex;			// for enumerating
	int				m_oiExtFranIndex;		// for enumerating

	// Sub-application
	#ifndef __cplusplus
	#define CRunApp void
	#endif
		CRunApp*	m_pParentApp;			// Parent application
	#ifndef __cplusplus
	#undef CRunApp
	#endif
	LPVOID			m_pSubAppObject;		// LPRS
	DWORD			m_dwSubAppOptions;		// Sub-app options
	BOOL			m_bSubAppIsVisible;		// Sub-app visibility
	HICON			m_hSubAppIcon;			// Sub-app icon
	int				m_cx;					// Subapp: valid if stretch
	int				m_cy;

	// DLL infos
	npAppli			m_idAppli;				// App object in DLL
	int				m_nDepth;				// Screen depth
	cSurface*		m_protoLogScreen;		// Surface prototype

	// Edit window
	HWND			m_hEditWin;				// Edit Window handle
	npWin			m_idEditWin;			// Edit Window identifier

	// Current frame
	CRunFrame*		m_Frame;				// Pointer to current frame

	// Run-time status
	int				m_bResizeTimer;
	int				m_refTime;
	int				m_appRunningState;
	int				m_startFrame;
	int				m_nextFrame;
	int				m_nCurrentFrame;
	BOOL			m_bWakeUp;
	WORD			m_oldFlags;
	WORD			m_appRunFlags;
	BOOL			m_bPlayFromMsgProc;

	// Debugger
	CDebugger*		m_pDebugger;

	// Full Screen
	int				m_depthFullScreen;
	WINDOWPLACEMENT m_sWndPlacement;		// Window position backup
	int				m_oldCxMax;				// Window size backup
	int				m_oldCyMax;
	cSurface*		m_pFullScreenSurface;

	// Global data
	BOOL			m_bSharePlayerCtrls;	// Sub-app: TRUE if shares player controls
	BOOL			m_bShareLives;			// Sub-app: TRUE if shares lives
	BOOL			m_bShareScores;			// Sub-app: TRUE if shares scores
	BOOL			m_bShareGlobalValues;	// Sub-app: TRUE if shares global values
	BOOL			m_bShareGlobalStrings;	// Sub-app: TRUE if shares global strings

	// Players
	fpPlayerCtrls	m_pPlayerCtrls;
	LPLONG			m_pLives;
	LPLONG			m_pScores;

	// Global values (warning: not valid if sub-app and global values are shared)
	LPBYTE			m_pGlobalValuesInit;
	int				m_nGlobalValues;		// Number of global values
	CValue*			m_pGlobalValues;		// Global values
	LPBYTE			m_pFree;				// No longer used
	LPBYTE			m_pGlobalValueNames;

	// Global strings (warning: not valid if sub-app and global values are shared)
	LPBYTE			m_pGlobalStringInit;	// Default global string values
	int				m_nGlobalStrings;		// Number of global strings
	LPTSTR*			m_pGlobalString;		// Pointers to global strings
	LPBYTE			m_pGlobalStringNames;

	// Global objects
	LPBYTE			m_AdGO;					// Global objects data

	// FRANCOIS
	short			m_NConditions[NUMBEROF_SYSTEMTYPES+KPX_BASE-1];

	// External sound files
	LPTSTR			m_pExtMusicFile;
	LPTSTR			m_pExtSampleFile[32];		// External sample file per channel

	// New Build 243
	int				m_nInModalLoopCount;
	LPTSTR			m_pPlayerNames;
	DWORD			m_dwColorCache;

	// New Build 245
	LPBYTE			m_pVtz4Opt;			// not used
	DWORD			m_dwFree;			// not used

	// Application load
	LPTSTR			m_pLoadFilename;
	DWORD			m_saveVersion;
	BOOL			m_bLoading;

	// Bluray
	LPVOID			m_pBROpt;

	// Build info
	AppHeader2*		m_pHdr2;

	// Code page
#ifdef _UNICODE
	DWORD			m_dwCodePage;
	bool			m_bUnicodeAppFile;
#endif

	// Effects
#ifdef HWABETA
	LPBYTE			m_pEffects;							// effects used in the application
	cSurface*		m_pOldSecondarySurface;				// secondary surface of the last frame, used in transitions
	bool			m_bAlwaysUseSecondarySurface;		// at least one frame has a transition => always use a secondary surface in all the frames
	bool			m_bShowWindowedMenu;				// to show menu after switch from full screen to windowed mode
	int				m_nSubAppShowCount;					// to show the child window otherwise it's not displayed...
#endif // HWABETA

#ifdef __cplusplus
};
#else
} CRunApp;
#endif
typedef CRunApp*	fpRunApp;

#endif // RUN_TIME

#ifndef __cplusplus
#undef cSurface
#undef CTransition
#undef CDebugger
#endif

#endif // _cncinc_h
