
#ifndef CNCY_H
#define CNCY_H

#include "PictEdDefs.h"

#ifdef __cplusplus
class cSurface;
class CImageFilterMgr;
class CSoundFilterMgr;
class CSoundManager;
class CRunApp;
class CRunFrame;
class CEditApp;
class CEditFrame;
class CMvt;
class CPropValue;
class OI;
class dllTrans;
class CValue;
#else
#define cSurface void
#define	CImageFilterMgr	void
#define CSoundFilterMgr void
#define CSoundManager void
#define CRunApp void
#define CRunFrame void
#define CEditApp void
#define CEditFrame void
#define CMvt void
#define CPropValue void
#define OI void
#define dllTrans void
struct CValue;
#endif
struct PropData;
struct RunHeader;

// Force structure alignement
#ifndef      _H2INC
#pragma pack( push, _pack_cncy_ )
#pragma pack(2)
#endif

#ifndef      _H2INC

// Object instance parent types
enum {
	PARENT_NONE,
	PARENT_FRAME,
	PARENT_FRAMEITEM,
	PARENT_QUALIFIER
};

// Proc definition
typedef void (WINAPI * UPDATEFILENAMEPROC)(LPCSTR, LPSTR);

// Old object name size, still used in events
#ifndef OINAME_SIZE
#define	OINAME_SIZE			24
#endif	// OINAME_SIZE

// Obsolete
enum	{
	MODIF_SIZE,
	MODIF_PLANE,
	MODIF_FLAGS,
	MODIF_PICTURE,
	MODIF_HFRAN,
	MODIF_BOX,
	MODIF_TEXT,
	MODIF_PREFS
};
#define	KPX_MODIF_BASE	1024

#endif	// _H2INC

// Standard objects
#define	KPX_BASE	32		// Extensions
enum	{
	OBJ_BOX,
	OBJ_BKD,
	OBJ_SPR,
	OBJ_TEXT,
	OBJ_QUEST,
	OBJ_SCORE,
	OBJ_LIVES,
	OBJ_COUNTER,
	OBJ_RTF,
	OBJ_CCA,
	NB_SYSOBJ
};

// Menu header (v2)
typedef struct MenuHdr {

	DWORD	mhHdrSize;		// == sizeof(MenuHdr)
	DWORD	mhMenuOffset;	// From start of MenuHdr
	DWORD	mhMenuSize;
	DWORD	mhAccelOffset;	// From start of MenuHdr
	DWORD	mhAccelSize;
							// Total data size = mhHdrSize + mhMenuSize + mhAccelSize 
} MenuHdr;

//////////////////////////////////////////////////////////////////////////////
// Application mini Header
//
typedef struct AppMiniHeader {

	union {
		char		gaType[4];		// "PAME"
		DWORD		gaDWType;
	};
	WORD		gaVersion;		// Version number
	WORD		gaSubVersion;	// Sub-version number
	DWORD		gaPrdVersion;	// MMF version
	DWORD		gaPrdBuild;		// MMF build number

} AppMiniHeader;

#define	RUNTIME_DWTYPE	'EMAP'
#define	RUNTIME_VERSION_MMF1		0x0300
#define	RUNTIME_VERSION_MMF15		0x0301
#define	RUNTIME_VERSION_MMF2		0x0302
#define	RUNTIME_VERSION				RUNTIME_VERSION_MMF2

//////////////////////////////////////////////////////////////////////////////
// Application full Header
//

enum {
	CTRLTYPE_MOUSE,		// not used
	CTRLTYPE_JOY1,
	CTRLTYPE_JOY2,
	CTRLTYPE_JOY3,
	CTRLTYPE_JOY4,
	CTRLTYPE_KEYBOARD,
	MAX_CTRLTYPE
};

#define MAX_PLAYER			4
#define MAX_KEY				8
#define PLAYERNAME_SIZE		100

typedef struct PlayerCtrls {

	WORD		pcCtrlType[MAX_PLAYER];				// Control type per player (0=mouse,1=joy1, 2=joy2, 3=keyb)
	WORD		pcCtrlKeys[MAX_PLAYER][MAX_KEY];	// Control keys per player

} PlayerCtrls;
typedef PlayerCtrls* fpPlayerCtrls;

typedef struct AppHeader {

	DWORD		gaSize;								// Structure size
	WORD		gaFlags;							// Flags
	WORD		gaNewFlags;							// New flags
	WORD		gaMode;								// graphic mode
	WORD		gaOtherFlags;						// Other Flags
	short		gaCxWin;							// Window x-size
	short		gaCyWin;							// Window y-size
	DWORD		gaScoreInit;						// Initial score
	DWORD		gaLivesInit;						// Initial number of lives
	PlayerCtrls	gaPlayerCtrls;
	DWORD		gaBorderColour;						// Border colour
	DWORD		gaNbFrames;							// Number of frames
	DWORD		gaFrameRate;						// Number of frames per second
	BYTE		gaMDIWindowMenu;					// Index of Window menu for MDI applications
	BYTE		gaFree[3];

} AppHeader;

// gaFlags
#define		GA_BORDERMAX		0x0001
#define		GA_NOHEADING		0x0002
#define		GA_PANIC			0x0004
#define		GA_SPEEDINDEPENDANT	0x0008
#define		GA_STRETCH			0x0010
//#define		GA_SOUND_ON			0x0020
//#define		GA_MUSIC_ON			0x0040
#define		GA_MENUHIDDEN		0x0080
#define		GA_MENUBAR			0x0100
#define		GA_MAXIMISE			0x0200
#define		GA_MIX				0x0400
#define		GA_FULLSCREENATSTART	0x0800
#define		GA_FULLSCREENSWITCH	0x1000
#define		GA_PROTECTED		0x2000
#define		GA_COPYRIGHT		0x4000
#define		GA_ONEFILE			0x8000

// gaNewFlags
#define		GANF_SAMPLESOVERFRAMES		0x0001
#define		GANF_RELOCFILES				0x0002
#define		GANF_RUNFRAME				0x0004
#define		GANF_SAMPLESEVENIFNOTFOCUS	0x0008

// New in build 92
#define		GANF_NOMINIMIZEBOX			0x0010
#define		GANF_NOMAXIMIZEBOX			0x0020
#define		GANF_NOTHICKFRAME			0x0040
#define		GANF_DONOTCENTERFRAME		0x0080

// New in build 101 (MMF 1.5)
#define		GANF_SCREENSAVER_NOAUTOSTOP	0x0100
#define		GANF_DISABLE_CLOSE			0x0200

// New in build 105
#define		GANF_HIDDENATSTART			0x0400

// New in build 110
#define		GANF_XPVISUALTHEMESUPPORT	0x0800

// New in MMF2
#define		GANF_VSYNC					0x1000
#define		GANF_RUNWHENMINIMIZED		0x2000
#define		GANF_MDI					0x4000
#define		GANF_RUNWHILERESIZING		0x8000

// Other flags
#define		GAOF_DEBUGGERSHORTCUTS		0x0001
#define		GAOF_DDRAW					0x0002
#define		GAOF_DDRAWVRAM				0x0004
#define		GAOF_OBSOLETE				0x0008
#define		GAOF_AUTOIMGFLT				0x0010
#define		GAOF_AUTOSNDFLT				0x0020
#define		GAOF_ALLINONE				0x0040
#define		GAOF_SHOWDEBUGGER			0x0080
#define		GAOF_RESERVED_1				0x0100
#define		GAOF_RESERVED_2				0x0200

//////////////////////////////////////////////////////////////////////////////
// Element of chunk Extensions
//
#ifndef      _H2INC
typedef struct ExtDesc {
	WORD		extSize;
	WORD		extIndex;
	DWORD		extMagicNumber;
} ExtDesc;
typedef ExtDesc *LPEXTDESC;

typedef struct ExtDesc2 {
	WORD		extSize;
	WORD		extIndex;
	DWORD		extMagicNumber;
	DWORD		extVersionLS;
	DWORD		extVersionMS;
} ExtDesc2;
typedef ExtDesc2 *LPEXTDESC2;
#endif

//////////////////////////////////////////////////////////////////////////////
// Movement Extensions Chunk
//
typedef struct MvtExtDesc {
	WORD		extTotalSize;
	WORD		extHdrSize;
	DWORD		extBuild;
} MvtExtDesc;
typedef MvtExtDesc *LPMVTEXTDESC;

//////////////////////////////////////////////////////////////////////////////
// Frame Header
//
typedef struct FrameHeader
{
	// Playfield
	LONG		leWidth;			// Playfield width in pixels
	LONG		leHeight;			// Playfield height in pixels
	COLORREF	leBackground;

	// Options
	DWORD		leFlags;


} FrameHeader;

// leFlags
#define		LEF_DISPLAYNAME			0x0001
#define		LEF_GRABDESKTOP			0x0002
#define		LEF_KEEPDISPLAY			0x0004
//#define		LEF_FADEIN				0x0008
//#define		LEF_FADEOUT				0x0010
#define		LEF_TOTALCOLMASK		0x0020
#define		LEF_PASSWORD			0x0040
#define		LEF_RESIZEATSTART		0x0100
#define		LEF_DONOTCENTER			0x0200
#define		LEF_FORCE_LOADONCALL	0x0400
#define		LEF_NOSURFACE			0x0800
#define		LEF_RESERVED_1			0x1000
#define		LEF_RESERVED_2			0x2000
#define		LEF_RECORDDEMO			0x4000	

//////////////////////////////////////////////////////////////////////////////
// Layers
//

#define FLOPT_XCOEF			0x0001
#define FLOPT_YCOEF			0x0002
#define FLOPT_NOSAVEBKD		0x0004
#define FLOPT_WRAP_OBSOLETE	0x0008
#define FLOPT_VISIBLE		0x0010
#define FLOPT_WRAP_HORZ		0x0020
#define FLOPT_WRAP_VERT		0x0040
#define FLOPT_REDRAW		0x000010000
#define FLOPT_TOHIDE		0x000020000
#define FLOPT_TOSHOW		0x000040000

typedef struct EditFrameLayer
{
	DWORD		dwOptions;		// Options
	float		xCoef;
	float		yCoef;
	DWORD		nBkdLOs;
	DWORD		nFirstLOIndex;

} EditFrameLayer;

//////////////////////////////////////////////////////////////////////////////
// ObjInfo/FrameItem Header
//
typedef struct ObjInfoHeader
{
	WORD	oiHandle;
	WORD	oiType;
	WORD	oiFlags;			// Memory flags
	WORD	oiReserved;			// No longer used
	DWORD	oiInkEffect;		// Ink effect
	DWORD	oiInkEffectParam;	// Ink effect param

} ObjInfoHeader;

// oiFlags
#define	OIF_LOADONCALL	0x0001
#define	OIF_DISCARDABLE	0x0002
#define	OIF_GLOBAL		0x0004
#define OIF_RESERVED_1	0x0008

//////////////////////////////////////////////////////////////////////////////
// LevObj/FrameItemInstance
//

#ifndef      _H2INC
#ifdef __cplusplus
class diskLO {
public:
#else
typedef struct diskLO {
#endif
	WORD	loHandle;			// HLO
	WORD	loOiHandle;			// HOI
	LONG	loX;				// Coords
	LONG	loY;
	WORD	loParentType;		// Parent type
	WORD	loOiParentHandle;	// HOI Parent
	WORD	loLayer;			// Layer
	WORD	loFree;				// Not used
#ifdef __cplusplus
};
#else
} diskLO;
#endif
typedef diskLO * fpDiskLO;
#endif

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon
//

// Obstacle types
enum {
	OBSTACLE_NONE,
	OBSTACLE_SOLID,
	OBSTACLE_PLATFORM,
	OBSTACLE_LADDER,
	OBSTACLE_TRANSPARENT		// for Add Backdrop
};

////////////////////////////////
// Static object - ObjectsCommon
//
#ifndef      _H2INC
typedef struct Static_OC {

	// Size
	DWORD	ocDWSize;

	// Obstacle type & collision mode
	WORD	ocObstacleType;
	WORD	ocColMode;			// Collision mode (0 = fine, 1 = box)

	LONG	ocCx;				// Size
	LONG	ocCy;

} Static_OC;
typedef Static_OC * LPStatic_OC;
#endif

///////////////////////////////////////////////////////////////
// Fill Type & shapes - Definitions
//

#ifndef      _H2INC
// Gradient
typedef struct GradientData {
	COLORREF		color1;
	COLORREF		color2;
	DWORD			vertical;
} GradientData;

// Shapes
enum {
	SHAPE_NONE=0,			// Error
	SHAPE_LINE,
	SHAPE_RECTANGLE,
	SHAPE_ELLIPSE
};

// Fill types
enum {
	FILLTYPE_NONE,
	FILLTYPE_SOLID,
	FILLTYPE_GRADIENT,
	FILLTYPE_MOTIF
};

// Line flags
#define	LINEF_INVX	0x0001
#define	LINEF_INVY	0x0002

#endif

///////////////////////////////////////////////////////////////
// Fill Type - Part of FilledShape
//
#ifndef      _H2INC

#ifdef __cplusplus
class FillType_Data {
public:
#else
typedef struct FillType_Data {
#endif
	WORD				ocFillType;
	union {
		struct {
			DWORD		ocLineFlags;		// Only for lines in non filled mode
		};
		struct {
			COLORREF	ocColor;			// Solid
		};
		struct {
			COLORREF	ocColor1;			// Gradient
			COLORREF	ocColor2;
			DWORD		ocGradientFlags;
		};
		struct {
			WORD		ocImage;			// Image
		};
	};
#ifdef __cplusplus
};
#else
} FillType_Data;
#endif

#endif

///////////////////////////////////////////////////////////////
// Filled Shape - Part of QuickBackdrop / Counter ObjectsCommon
//
#ifndef      _H2INC

#ifdef __cplusplus
class FilledShape_Data {
public:
#else
typedef struct FilledShape_Data {
#endif
	WORD			ocBorderSize;			// Border
	COLORREF		ocBorderColor;
	WORD			ocShape;				// Shape
	FillType_Data	ocFillData;				// Fill Type infos
#ifdef __cplusplus
};
#else
} FilledShape_Data;
#endif
typedef FilledShape_Data * LPFilledShape_Data;

#endif

/////////////////////////////////
// Quick backdrop - ObjectsCommon
//
#ifndef      _H2INC

typedef struct QuickBackdrop_OC {

	DWORD	ocDWSize;

	WORD	ocObstacleType;		// Obstacle type
	WORD	ocColMode;			// Collision mode (0 = fine, 1 = box)

	LONG	ocCx;				// Size
	LONG	ocCy;

	FilledShape_Data	ocFilledShape;		// Filled shape infos

} QuickBackdrop_OC;
typedef QuickBackdrop_OC * LPQuickBackdrop_OC;

#endif

/////////////////////////////////
// Backdrop - ObjectsCommon
//
#ifndef      _H2INC

typedef struct Backdrop_OC {

	DWORD	ocDWSize;

	WORD	ocObstacleType;		// Obstacle type
	WORD	ocColMode;			// Collision mode (0 = fine, 1 = box)

	LONG	ocCx;				// Size
	LONG	ocCy;

	WORD	ocImage;			// Image

} Backdrop_OC;
typedef Backdrop_OC * LPBackdrop_OC;

#endif

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon Data
//

//////////////////////////////////////////////////////////////////////////////
// ImageSet
//

#ifdef __cplusplus
class ImageSet_Data {
public:
#else
typedef	struct	ImageSet_Data {
#endif
	WORD		nbFrames;		// Number of frames
								// Followed by list of image handles (WORD[])
#ifdef __cplusplus
};
#else
} ImageSet_Data;
#endif
typedef ImageSet_Data * LPImageSet_Data;

////////////////////////////////////////
// Text - ocData
//
typedef struct otText {

	DWORD	otDWSize;
	LONG	otCx;
	LONG	otCy;
	DWORD	otNumberOfText;			// Paragraph number (> 1 if question object)
	DWORD	otStringOffset[1];		// String offsets

} otText;
typedef otText	*	fpot;
#define sizeof_ot	(sizeof(otText)-sizeof(DWORD))

typedef	struct	txString {
	WORD		tsFont;					// Font
	WORD		tsFlags;				// Flags
	COLORREF	tsColor;				// Color
	char		tsChar[1];
} txString;
typedef	txString	*	fpts;
#define	sizeof_ts	8					// (sizeof(txString)-1)

#define	TSF_LEFT		0x0000			// DT_LEFT
#define	TSF_HCENTER		0x0001			// DT_CENTER
#define	TSF_RIGHT		0x0002			// DT_RIGHT
#define	TSF_VCENTER		0x0004			// DT_VCENTER
#define	TSF_HALIGN		0x000F			// DT_LEFT | DT_RIGHT | DT_CENTER | DT_VCENTER | DT_BOTTOM

#define	TSF_CORRECT		0x0100
#define	TSF_RELIEF		0x0200

////////////////////////////////////////
// Scores, lives, counters
//
typedef struct CtAnim_Data {

	DWORD		odDWSize;
	LONG		odCx;					// Size: only lives & counters
	LONG		odCy;
	WORD		odPlayer;				// Player: only score & lives
	WORD		odDisplayType;			// CTA_xxx
	WORD		odDisplayFlags;			// BARFLAG_INVERSE
	WORD		odFont;					// Font

	// Followed by display data:
//	union {
//		ImageSet_Data		odImages;
//		FilledShape_Data	odShape;
//	};

} CtAnim_Data;
typedef CtAnim_Data * LPCtAnim_Data;

// Display types
enum	{
	CTA_HIDDEN,
	CTA_DIGITS,
	CTA_VBAR,
	CTA_HBAR,
	CTA_ANIM,
	CTA_TEXT,
};

// Display flags
#define	BARFLAG_INVERSE		0x0100

// Counters images
enum {
	COUNTER_IMAGE_0,
	COUNTER_IMAGE_1,
	COUNTER_IMAGE_2,
	COUNTER_IMAGE_3,
	COUNTER_IMAGE_4,
	COUNTER_IMAGE_5,
	COUNTER_IMAGE_6,
	COUNTER_IMAGE_7,
	COUNTER_IMAGE_8,
	COUNTER_IMAGE_9,
	COUNTER_IMAGE_SIGN_NEG,
	COUNTER_IMAGE_SIGN_PLUS,
	COUNTER_IMAGE_POINT,
	COUNTER_IMAGE_EXP,
	COUNTER_IMAGE_MAX
};
#define	V1_COUNTER_IMAGE_MAX	(COUNTER_IMAGE_SIGN_NEG+1)

////////////////////////////////////////
//
// Objet RTF - ocData
//
#ifndef      _H2INC

typedef struct ocRTF {

	DWORD		odDWSize;
	DWORD		odVersion;				// 0
	DWORD		odOptions;				// Options
	COLORREF	odBackColor;			// Background color	
	LONG		odCx;					// Size
	LONG		odCy;

} ocRTF;
typedef ocRTF * LPOCRTF;

//#define	RTFOPT_TRANSPARENT	0x0001		// Transparent
//#define	RTFOPT_VSLIDER		0x0002		// Display vertical slider if necessary
//#define	RTFOPT_HSLIDER		0x0004		// Display horizontal slider if necessary

#endif

////////////////////////////////////////
//
// Objet CCA - ocData
//
#ifndef      _H2INC

typedef struct ocCCA {

	DWORD	odDWSize;
	LONG	odCx;						// Size (ignored)
	LONG	odCy;
	WORD	odVersion;					// 0
	WORD	odNStartFrame;
	DWORD	odOptions;					// Options
	DWORD	odIconOffset;				// Icon offset
	DWORD	odFree;						// Free

} ocCCA;
typedef ocCCA * LPOCCCA;

// Options
#define	CCAF_SHARE_GLOBALVALUES		0x00000001
#define	CCAF_SHARE_LIVES			0x00000002
#define	CCAF_SHARE_SCORES			0x00000004
#define	CCAF_SHARE_WINATTRIB		0x00000008
#define	CCAF_STRETCH				0x00000010
#define	CCAF_POPUP					0x00000020
#define CCAF_CAPTION				0x00000040
#define CCAF_TOOLCAPTION			0x00000080
#define CCAF_BORDER					0x00000100
#define CCAF_WINRESIZE				0x00000200
#define CCAF_SYSMENU				0x00000400
#define CCAF_DISABLECLOSE			0x00000800
#define CCAF_MODAL					0x00001000
#define CCAF_DIALOGFRAME			0x00002000
#define	CCAF_INTERNAL				0x00004000
#define	CCAF_HIDEONCLOSE			0x00008000
#define	CCAF_CUSTOMSIZE				0x00010000
#define	CCAF_INTERNALABOUTBOX		0x00020000
#define	CCAF_CLIPSIBLINGS			0x00040000
#define	CCAF_SHARE_PLAYERCTRLS		0x00080000
#define CCAF_MDICHILD				0x00100000
#define CCAF_DOCKED					0x00200000
#define	CCAF_DOCKING_AREA			0x00C00000
#define	CCAF_DOCKED_LEFT			0x00000000
#define	CCAF_DOCKED_TOP				0x00400000
#define	CCAF_DOCKED_RIGHT			0x00800000
#define	CCAF_DOCKED_BOTTOM			0x00C00000
#define	CCAF_REOPEN					0x01000000
#define CCAF_MDIRUNEVENIFNOTACTIVE	0x02000000

#endif

////////////////////////////////////////
//
// Transition
//

#ifndef      _H2INC

// Transition header
typedef struct TransitionHdr {
	DWORD		trDllID;				// DLL id
	DWORD		trID;					// Transition ID
	DWORD		trDuration;				// Duration
	DWORD		trFlags;				// From / to color / background
	DWORD		trColor;				// Color
} TransitionHdr;

// Transition run-time data
#ifdef __cplusplus
class Transition_Data {
public:
#else
typedef struct Transition_Data {
#endif
	TransitionHdr	trHdr;
	DWORD			trDllNameOffset;
	DWORD			trParamsOffset;
	DWORD			trParamsSize;
									// Followed by DLL name & transition parameters
#ifdef __cplusplus
};
#else
} Transition_Data;
#endif
typedef Transition_Data * LPTRANSITIONDATA;

#define	TRFLAG_COLOR	0x0001

#endif

// Text alignment flags
#define	TEXT_ALIGN_LEFT		0x00000001
#define	TEXT_ALIGN_HCENTER	0x00000002
#define	TEXT_ALIGN_RIGHT	0x00000004
#define	TEXT_ALIGN_TOP		0x00000008
#define	TEXT_ALIGN_VCENTER	0x00000010
#define	TEXT_ALIGN_BOTTOM	0x00000020

// Text caps
#define	TEXT_FONT			0x00010000
#define	TEXT_COLOR			0x00020000
#define	TEXT_COLOR16		0x00040000

//////////////////////////////////////////////////////////////////////////////
//
//	Run-time Definitions
//

#ifdef	RUN_TIME

#ifndef _H2INC
typedef short *				LPSHORT;
typedef LPVOID *			LPLPVOID;
#endif

// Versions
#define	KNPV1_VERSION			0x126		// First K&P version
#define	CNCV1_VERSION			0x207		// Click & Create / The Games Factory
#define	KNP_VERSION				0x300

// Internet Versions
#define	FIRST_NETVERSION		0x001		// First vitalize version
#define	CCN_NETVERSION			0x002		// CCN applications
#define	MORECOMP_NETVERSION		0x003		// Short OIs, compression of levObjs, adpcm sounds, 
#define	CNC_NETVERSION			0x003		// Current Internet Version

// Collision mask margins
#define		COLMASK_XMARGIN			64
#define		COLMASK_YMARGIN			16

// Messages
#define		UM_KNP_GETAPPID		(WM_APP+7)	// No longer used
#define		UM_HVSCROLL			(WM_APP+8)	// No longer used

// App IDs
#define		APPID_KNC		0x3952			// No longer used
#define		APPID_KNCSA		0x9172			// No longer used

// Control IDs
#define		ID_EDIT_WIN		300				// ID of edit window

#endif	// RUN_TIME

// No longer used
#ifndef		HFRAN
#define		HFRAN	LPBYTE
#endif

// HLO
#define	HLO			WORD
#define	HLO_ERROR	((HLO)-1)
typedef	HLO *		LPHLO;

// Frame password size
#define	PASSWORD_SIZE			9

// Cross-ref tables
enum	{
	IMG_TAB,
	FONT_TAB,
	SOUND_TAB,
	MUSIC_TAB,
	MAX_TABREF
};

#ifdef RUN_TIME

// Preferences
#define	PREF_MUSICON		0x4000
#define	PREF_SOUNDON		0x8000

// Frame handles
#define	HCELL			WORD
#define	HCELL_ERROR		((WORD)-1)

// Jump flags
#define	MPJC_NOJUMP		0
#define	MPJC_DIAGO		1
#define	MPJC_BUTTON1	2
#define	MPJC_BUTTON2	3

// How to play (no longer used)
enum	{
	HTP_NO=0,
	HTP_TXT,
	HTP_WRI,
	HTP_HLP
};
#define	NB_HTPTYPE	4

#endif	// RUN_TIME

// Versions
#define MMFVERSION_MASK		0xFFFF0000
#define MMFBUILD_MASK		0x00000FFF		// MMF build
#define MMFVERFLAG_MASK		0x0000F000
#define MMFVERFLAG_HOME		0x00008000		// TGF
#define MMFVERFLAG_PRO		0x00004000		// MMF Pro
#define MMFVERFLAG_ATX		0x00002000		// Not used
#define MMFVERFLAG_PLUGIN	0x00001000		// Plugin
#define MMFVERSION_15		0x01050000		// MMF 1.5
#define MMFVERSION_20		0x02000000		// MMF 2.0
#define	MMF_CURRENTVERSION	MMFVERSION_20

// Build numbers
#define MMF_BUILD_NONAME			203
#define MMF_BUILD_MENUIMAGES		204
#define MMF_BUILD_SUBAPPICON		205
#define MMF_BUILD_WINMENUIDX		206
#define MMF_BUILD_NOTUSED			207
#define MMF_BUILD_ABOUTTEXT			208
#define MMF_BUILD_FRAMEDPOS			209
#define MMF_BUILD_TRSPCOLOR			210
#define MMF_BUILD_211				211
#define MMF_BUILD_DEMO				212
#define MMF_BUILD_PREBETA_2			213
#define MMF_BUILD_PREBETA_3			214
#define MMF_BUILD_PREBETA_4			215		// First update Jason's book
#define MMF_BUILD_ALTVALUES			216
#define MMF_BUILD_EDTCHUNKS			217
#define MMF_BUILD_COUNTERTEXT		218
#define MMF_BUILD_SDK				219		// Pre-SDK version
#define MMF_BUILD_SDK2				220		// Second SDK version
#define MMF_BUILD_MVTSDK2			221		// Second Mvt SDK version
#define MMF_BUILD_FIXCOLLISANDBEH	222
#define MMF_BUILD_PROPFILTERS		223
#define MMF_BUILD_MFXEXTS			224
#define MMF_BUILD_LASTFRAMEOFFSET	225
#define MMF_BUILD_EDUDEMOFEB06		226
#define MMF_BUILD_GLBOBJBUG			227
#define MMF_BUILD_IDTC				228
#define MMF_BUILD_229				229
#define MMF_BUILD_230				230
#define MMF_BUILD_231				231
#define	MMF_BUILD_FIXQUALIF			232
#define	MMF_BUILD_233				233
#define	MMF_BUILD_234_TGFPR			234
#define	MMF_BUILD_235				235
#define	MMF_BUILD_236				236
#define	MMF_BUILD_237				237
#define	MMF_BUILD_238				238
#define	MMF_BUILD_239_CD			239
#define	MMF_BUILD_240_DEMO			240
#define	MMF_BUILD_241_SP1			241
#define	MMF_BUILD_242_SP2			242
#define	MMF_BUILD_243_SP3			243
#define	MMF_BUILD_244_SP4			244
#define	MMF_BUILD_245_SP5			245
#define	MMF_BUILD_246_SP6			246
#define	MMF_CURRENTBUILD			MMF_BUILD_246_SP6

// MFA file format versions
#define MFA_BUILD_ALTSTR			1						// Alterable strings
#define MFA_BUILD_COUNTERTEXT		2						// Text mode in counters
#define MFA_BUILD_LASTFRAMEOFFSET	3						// Additional frame offset
#define MFA_BUILD_FIXQUALIF			4						// Fix in qualifiers + prd version
#define MFA_CURRENTBUILD			MFA_BUILD_FIXQUALIF

// Structures for picture editor
typedef struct EditSurfaceParams {

	DWORD		m_dwSize;		// sizeof(EditSurfaceParams)
	LPSTR		m_pWindowTitle;	// Picture Editor title (NULL = default title)
	cSurface*	m_pSf;			// Surface to edit
	DWORD		m_dwOptions;	// Options, see PictEdDefs.h
	DWORD		m_dwFixedWidth;	// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
	DWORD		m_dwFixedHeight;// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)
	POINT		m_hotSpot;		// Hot spot coordinates
	POINT		m_actionPoint;	// Action point coordinates

} EditSurfaceParams;
typedef EditSurfaceParams* LPEDITSURFACEPARAMS;

typedef struct EditImageParams {

	DWORD	m_dwSize;			// sizeof(EditImageParams)
	LPSTR	m_pWindowTitle;		// Picture Editor title (NULL = default title)
	DWORD	m_dwImage;			// Image to edit - note: only the LOWORD is used in this version
	DWORD	m_dwOptions;		// Options, see PictEdDefs.h
	DWORD	m_dwFixedWidth;		// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
	DWORD	m_dwFixedHeight;	// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)

} EditImageParams;
typedef EditImageParams* LPEDITIMAGEPARAMS;

// Structure for image list editor
typedef struct EditAnimationParams {

	DWORD	m_dwSize;			// sizeof(EditAnimationParams)
	LPSTR	m_pWindowTitle;		// Picture Editor title (NULL = default title)
	int		m_nImages;			// Number of images in the list
	int		m_nMaxImages;		// Maximum number of images in the list
	int		m_nStartIndex;		// Index of first image to edit in the editor
	LPWORD	m_pImages;			// Image list (one WORD per image)
	LPSTR*	m_pImageTitles;		// Image titles (can be NULL)
	DWORD	m_dwOptions;		// Options, see PictEdDefs.h
	DWORD	m_dwFixedWidth;		// Default width or fixed width (if PICTEDOPT_FIXEDIMGSIZE is used)
	DWORD	m_dwFixedHeight;	// Default height or fixed height (if PICTEDOPT_FIXEDIMGSIZE is used)

} EditAnimationParams;
typedef EditAnimationParams* LPEDITANIMATIONPARAMS;

// Global variables structure
#ifdef __cplusplus
class mv {
public:
#else
typedef	struct	mv {
#endif

	// Common to editor and runtime
	HINSTANCE			mvHInst;				// Application HINSTANCE
	npAppli				mvIdAppli;				// Application object in DLL
	npWin				mvIdMainWin;			// Main window object in DLL
	npWin				mvIdEditWin;			// Child window object in DLL
	HWND				mvHMainWin;				// Main window handle
	HWND				mvHEditWin;				// Child window handle
	HPALETTE			mvHPal256;				// 256 color palette
	WORD				mvAppMode;				// Screen mode with flags
	WORD				mvScrMode;				// Screen mode
	DWORD				mvEditDXDocToClient;	// Edit time only: top-left coordinates
	DWORD				mvEditDYDocToClient;
	CImageFilterMgr*	mvImgFilterMgr;			// Image filter manager
	CSoundFilterMgr*	mvSndFilterMgr;			// Sound filter manager
	CSoundManager*		mvSndMgr;				// Sound manager

	union {
		CEditApp*		mvEditApp;				// Current application, edit time (not used)
		CRunApp*		mvRunApp;				// Current application, runtime
	};
	union {
		CEditFrame*		mvEditFrame;
		CRunFrame*		mvRunFrame;
	};

	// Runtime
	#ifdef __cplusplus
		RunHeader*		mvRunHdr;				// Current RunHeader
	#else
		LPVOID			mvRunHdr;
	#endif
	DWORD				mvPrefs;				// Preferences (sound on/off)
	LPSTR				subType;
	BOOL				mvFullScreen;			// Full screen mode
	LPSTR				mvMainAppFileName;		// App filename
	int					mvAppListCount;
	int					mvAppListSize;
	CRunApp**			mvAppList;
	int					mvExtListCount;
	int					mvExtListSize;
	LPSTR *				mvExtList;
	int					mvNbDllTrans;
	dllTrans*			mvDllTransList;
	DWORD				mvJoyCaps[32];
	HHOOK				mvHMsgHook;
	int					mvModalLoop;
	int					mvModalSubAppCount;
	LPVOID				mvFree[5];

	// Functions
	////////////

	// Editor: Open Help file
	void				(CALLBACK * mvHelp) (LPCSTR pHelpFile, UINT nID, LPARAM lParam);

	// Editor: Get default font for object creation
	BOOL				(CALLBACK * mvGetDefaultFont) (LPLOGFONT plf, LPSTR pStyle, int cbSize);

	// Editor: Edit images and animations
	BOOL				(CALLBACK * mvEditSurface) (LPVOID edPtr, LPEDITSURFACEPARAMS pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditImage) (LPVOID edPtr, LPEDITIMAGEPARAMS pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditAnimation) (LPVOID edPtr, LPEDITANIMATIONPARAMS pParams, HWND hParent);

	// Runtime: Extension User data
	LPVOID				(CALLBACK * mvGetExtUserData) (CRunApp* pApp, HINSTANCE hInst);
	LPVOID				(CALLBACK * mvSetExtUserData) (CRunApp* pApp, HINSTANCE hInst, LPVOID pData);

	// Runtime: Register dialog box
	void				(CALLBACK * mvRegisterDialogBox) (HWND hDlg);
	void				(CALLBACK * mvUnregisterDialogBox) (HWND hDlg);

	// Runtime: Add surface as backdrop object
	void				(CALLBACK * mvAddBackdrop) (cSurface* pSf, int x, int y, DWORD dwInkEffect, DWORD dwInkEffectParam, int nObstacleType, int nLayer);

	// Runtime: Binary files
	BOOL				(CALLBACK * mvGetFile)(LPCSTR pPath, LPSTR pFilePath, DWORD dwFlags);
	void				(CALLBACK * mvReleaseFile)(LPCSTR pPath);
	HANDLE				(CALLBACK * mvOpenHFile)(LPCSTR pPath, LPDWORD pDwSize, DWORD dwFlags);
	void				(CALLBACK * mvCloseHFile)(HANDLE hf);

	// Plugin: download file
	int					(CALLBACK * mvLoadNetFile) (LPSTR pFilename);

	// Plugin: send command to Vitalize
	int					(CALLBACK * mvNetCommand) (int, LPVOID, DWORD, LPVOID, DWORD);

	// Editor & Runtime: Returns the version of MMF or of the runtime
	DWORD				(CALLBACK * mvGetVersion) ();

	// Editor & Runtime: callback function for properties or other functions
	#ifdef _cplusplus
		LRESULT			(CALLBACK * mvCallFunction) (LPVOID edPtr, int nFnc, LPARAM lParam1=0, LPARAM lParam2=0, LPARAM lParam3=0);
	#else
		LRESULT			(CALLBACK * mvCallFunction) (LPVOID edPtr, int nFnc, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);
	#endif

	// Place-holder for next versions
	LPVOID				mvAdditionalFncs[16];

#ifdef __cplusplus
};
#else
} mv;
#endif
typedef mv *LPMV;

// Callback function identifiers for mvCallFunction
enum {
	// Editor only
	EF_INSERTPROPS=1,		// Insert properties into Property window
	EF_REMOVEPROP,			// Remove property
	EF_REMOVEPROPS,			// Remove properties
	EF_REFRESHPROP,			// Refresh propery
	EF_REALLOCEDITDATA,		// Reallocate edPtr
	EF_GETPROPVALUE,		// Get object's property value
	EF_GETAPPPROPVALUE,		// Get application's property value
	EF_GETFRAMEPROPVALUE,	// Get frame's property value
	EF_SETPROPVALUE,		// Set object's property value
	EF_SETAPPPROPVALUE,		// Set application's property value
	EF_SETFRAMEPROPVALUE,	// Set frame's property value
	EF_GETPROPCHECK,		// Get object's property check state
	EF_GETAPPPROPCHECK,		// Get application's property check state
	EF_GETFRAMEPROPCHECK,	// Get frame's property check state
	EF_SETPROPCHECK,		// Set object's property check state
	EF_SETAPPPROPCHECK,		// Set application's property check state
	EF_SETFRAMEPROPCHECK,	// Set frame's property check state
	EF_INVALIDATEOBJECT,	// Refresh object in frame editor
	EF_RECALCLAYOUT,		// Recalc runtime layout (docking)

	// Editor & runtime
	EF_MALLOC=100,				// Allocate memory
	EF_CALLOC,					// Allocate memory & set it to 0
	EF_REALLOC,					// Re-allocate memory
	EF_FREE,					// Free memory
	EF_GETSOUNDMGR,				// Get sound manager
	EF_CLOSESOUNDMGR,			// Close sound manager
	EF_ENTERMODALLOOP,			// Reserved
	EF_EXITMODALLOOP,			// Reserved
};

// Callback function macros for mvCallFunction
#ifdef __cplusplus
__inline void mvInsertProps(LPMV mV, LPVOID edPtr, PropData* pProperties, UINT nInsertPropID, BOOL bAfter) \
	{ mV->mvCallFunction(edPtr, EF_INSERTPROPS, (LPARAM)pProperties, (LPARAM)nInsertPropID, (LPARAM)bAfter); }

__inline void mvRemoveProp(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ mV->mvCallFunction(edPtr, EF_REMOVEPROP, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvRemoveProps(LPMV mV, LPVOID edPtr, PropData* pProperties) \
	{ mV->mvCallFunction(edPtr, EF_REMOVEPROPS, (LPARAM)pProperties, (LPARAM)0, (LPARAM)0); }

__inline void mvRefreshProp(LPMV mV, LPVOID edPtr, UINT nPropID, BOOL bReInit) \
	{ mV->mvCallFunction(edPtr, EF_REFRESHPROP, (LPARAM)nPropID, (LPARAM)bReInit, (LPARAM)0); }

__inline LPVOID mvReAllocEditData(LPMV mV, LPVOID edPtr, DWORD dwNewSize) \
	{ return (LPVOID)mV->mvCallFunction(edPtr, EF_REALLOCEDITDATA, (LPARAM)edPtr, dwNewSize, 0); }

__inline CPropValue* mvGetPropValue(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (CPropValue*)mV->mvCallFunction(edPtr, EF_GETPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline CPropValue* mvGetAppPropValue(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (CPropValue*)mV->mvCallFunction(edPtr, EF_GETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline CPropValue* mvGetFramePropValue(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (CPropValue*)mV->mvCallFunction(edPtr, EF_GETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvSetPropValue(LPMV mV, LPVOID edPtr, UINT nPropID, CPropValue* pValue) \
	{ mV->mvCallFunction(edPtr, EF_SETPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline void mvSetAppPropValue(LPMV mV, LPVOID edPtr, UINT nPropID, CPropValue* pValue) \
	{ mV->mvCallFunction(edPtr, EF_SETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline void mvSetFramePropValue(LPMV mV, LPVOID edPtr, UINT nPropID, CPropValue* pValue) \
	{ mV->mvCallFunction(edPtr, EF_SETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

__inline UINT mvGetPropCheck(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (UINT)mV->mvCallFunction(edPtr, EF_GETPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline UINT mvGetAppPropCheck(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (UINT)mV->mvCallFunction(edPtr, EF_GETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline UINT mvGetFramePropCheck(LPMV mV, LPVOID edPtr, UINT nPropID) \
	{ return (UINT)mV->mvCallFunction(edPtr, EF_GETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

__inline void mvSetPropCheck(LPMV mV, LPVOID edPtr, UINT nPropID, UINT nCheck) \
	{ mV->mvCallFunction(edPtr, EF_SETPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvSetAppPropCheck(LPMV mV, LPVOID edPtr, UINT nPropID, UINT nCheck) \
	{ mV->mvCallFunction(edPtr, EF_SETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvSetFramePropCheck(LPMV mV, LPVOID edPtr, UINT nPropID, UINT nCheck) \
	{ mV->mvCallFunction(edPtr, EF_SETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

__inline void mvInvalidateObject(LPMV mV, LPVOID edPtr) \
	{ mV->mvCallFunction(edPtr, EF_INVALIDATEOBJECT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline LPVOID mvMalloc(LPMV mV, int nSize) \
	{ return (LPVOID)mV->mvCallFunction(NULL, EF_MALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

__inline LPVOID mvCalloc(LPMV mV, int nSize) \
	{ return (LPVOID)mV->mvCallFunction(NULL, EF_CALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

__inline LPVOID mvReAlloc(LPMV mV, LPVOID ptr, int nNewSize) \
	{ return (LPVOID)mV->mvCallFunction(NULL, EF_REALLOC, (LPARAM)ptr, (LPARAM)nNewSize, (LPARAM)0); }

__inline void mvFree(LPMV mV, LPVOID ptr) \
	{ mV->mvCallFunction(NULL, EF_FREE, (LPARAM)ptr, (LPARAM)0, (LPARAM)0); }

__inline void mvRecalcLayout(LPMV mV) \
	{ mV->mvCallFunction(NULL, EF_RECALCLAYOUT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline CSoundManager* mvGetSoundMgr(LPMV mV) \
	{ mV->mvCallFunction(NULL, EF_GETSOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

__inline void mvCloseSoundMgr(LPMV mV) \
	{ mV->mvCallFunction(NULL, EF_CLOSESOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

#endif // __cplusplus

// Options for OpenHFile
#define	OHFF_LOCALFILE		0x0001		// Vitalize mode only: don't try to download file from server

#ifdef RUN_TIME

// EnumEltProc definition
typedef	int (CALLBACK* ENUMELTPROC)(LPWORD, int, LPARAM, LPARAM);

// Extension function table
typedef	struct	tagKpxFunc {
	HINSTANCE			kpxHInst;
	LPSTR				kpxName;
	LPSTR				kpxSubType;
	LPVOID				kpxUserData;
	DWORD ( WINAPI		* kpxGetInfos) (int);
	int  ( WINAPI		* kpxLoadObject) (mv *, OI*, LPBYTE, int);
	void ( WINAPI		* kpxUnloadObject) (mv *, LPBYTE, int);
	void ( WINAPI		* kpxUpdateFileNames) (mv *, LPSTR, LPBYTE, void (WINAPI * )(LPSTR, LPSTR));
	short( WINAPI		* kpxGetRunObjectInfos) (mv *, LPBYTE);
	short( WINAPI		* kpxWindowProc) (LPBYTE, HWND, UINT, WPARAM, LPARAM);
	int	 ( WINAPI		* kpxEnumElts) (mv *, LPBYTE, ENUMELTPROC, ENUMELTPROC, LPARAM, LPARAM);
	int  ( WINAPI		* kpxLoadData) (mv *, OI*, LPBYTE, HFILE, DWORD);		// Not used
	int  ( WINAPI		* kpxUnloadData) (mv *, OI*, LPBYTE);					// Not used
	void ( WINAPI		* kpxStartApp) (mv *, CRunApp*);
	void ( WINAPI		* kpxEndApp) (mv *, CRunApp*);
	void ( WINAPI		* kpxStartFrame) (mv *, CRunApp*, int);
	void ( WINAPI		* kpxEndFrame) (mv *, CRunApp*, int);
	HGLOBAL ( WINAPI	* kpxUpdateEditStructure) (mv *, LPVOID);
#if defined(VITALIZE)
	BOOL				bValidated;
#endif
} kpxFunc;
typedef kpxFunc * fpKpxFunc;

// Movement Extension
typedef	struct	MvxFnc {
	HINSTANCE			mvxHInst;
	LPSTR				mvxFileTitle;

	CMvt* ( CALLBACK	* mvxCreateMvt) (DWORD);

	#if defined(VITALIZE)
		BOOL				bValidated;
	#endif
} MvxFnc;

// Old 16/32 bit macros
#define	ADDPTR32(p,l)	p+=l;
#define	SUBPTR32(p,l)	p-=l;
#define	INCPTR32(p)		p++;

#endif	// RUN_TIME

// Restore structure alignement
#ifndef      _H2INC 
#pragma pack( pop, _pack_cncy_ )
#endif

#ifndef __cplusplus
#undef CImageFilterMgr
#undef CSoundFilterMgr
#undef CSoundManager
#undef CRunApp
#undef CRunFrame
#undef CEditApp
#undef CEditFrame
#undef CMvt
#undef CPropValue
#endif

#endif	// CNCY_H
