//----------------------------------------------//
//          Constantes et structures            //
//----------------------------------------------//

#ifndef		_cnpdll_h		// Si deja inclus, ignorer
#define		_cnpdll_h

#ifndef PI
#define     PI  ((double)3.1415926535)
#endif

#ifndef _H2INC
#ifdef WIN32
#pragma pack(push, 2)
#else
#pragma pack(2)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Include files for H2Inc
#ifdef      _H2INC

	#define NOATOM
	#define NOMETAFILE		1
	#define NOMINMAX
	#define NOMSG
	#define NOOPENFILE
	#define NOSCROLL
	#define NOSOUND			1
	#define NOSYSMETRICS
	#define NOTEXTMETRIC
	#define NODBCS
	#define NOSYSTEMPARAMSINFO
	#define NOCOMM			1
	#define NOOEMRESOURCE
	#define	NOPROFILER		1
	#define	NOMDI			1

	#define	MMNODRV			1
	#define	MMNOSOUND		1
	#define	MMNOAUX			1
	#define	MMNOTIMER
	#define	MMNOMMSYSTEM	1

	#pragma warning (disable:4010)
	#ifdef	WIN32
	# include	"win32.h"
	#endif
	#include	"empty.h"

#endif		// _H2INC

#ifdef __cplusplus
class	cSurface;
class	cSurfaceImplementation;
struct  sMask;
#else
#define	cSurface void
#define cSurfaceImplementation void
#define sMask void
#endif


// Current build
#define	MMFS2_CURRENT_BUILD		246

// 2/4 characters -> WORD/DWORD
#ifndef MAC
#define	WORDSTR(c1,c2)			((WORD)((WORD)c2<<8|(WORD)c1))
#define	DWORDSTR(c1,c2,c3,c4)	((DWORD)((DWORD)c4<<24|(DWORD)c3<<16|(DWORD)c2<<8|(DWORD)c1))
#endif

	// Definitions differentes si WIN32

// ==================
//		Win 32
// ==================
#ifdef	WIN32

	#define	_near
	#define	__near
	#define	_far
	#define	__far
	#define	_pascal	_stdcall
	#define	__pascal __stdcall

		// Export
	#define	DLLExport
	#ifdef _H2INC
		#define	DLLExport32
	#else
		#ifdef IN_DLL
			#define	DLLExport32		__declspec(dllexport)
		#else
			#define	DLLExport32		__declspec(dllimport)
		#endif
	#endif
	#define	_export
	#define	__export

	// User messages
	#define	WM_AMOA					WM_APP

#endif

	// Flags boites de dialogues (obsolete)
	// -------------------------
#define DL_MODAL            1
#define DL_CENTER_WINDOW    2
#define DL_CENTER_SCREEN    4

	// Messages propres a KNP
	// ----------------------
#define	WM_IT50			(WM_AMOA+123)
#define	WM_NET_FIRST	(WM_AMOA+174)			// v2
#define	WM_NET_LAST		(WM_AMOA+199)

#define	UM_KEYDOWN		(WM_AMOA+5)
#define	UM_PANIC		(WM_AMOA+6)

#define	IT50_WPARAMID	0x2775

	// Classes de fenetres
	// -------------------
#define PCS_DBL			0x0001
#define PCS_SNG			0x0002
#define PCS_SCR			0x0003
#define	PCS_CLASS		0x00FF

#define	PCSF_STRETCH	0x0100	// Stretch log to phys

#define	PCSF_MDIMAIN	0x1000
#define	PCSF_MDICHILD	0x2000

	// Extra-bytes
	// -----------
#define	KNP_WINDOWEXTRA	12		// (3 dwords)
#define	KWL_FPWIN		0		//   pointeur sur structure DLL fenetre
#define	KWL_USER		4		// DWORD disponible pour utilisateur

	// Objects
#ifndef _dllpriv_h
#define	Appli	void
#define	Win		void
#define	npAppli	Appli *
#define	npWin	Win *
#endif

	// Identifiants errones
	// --------------------
#define NPSPR_ERROR     0
#define HSPR_ERROR     0

enum {
	WIN_MEMERR,
	WIN_APPERR,
	WIN_CREATERR
};
enum {
	CREATELOG_MEMERR=16,
	CREATELOG_DCERR,
	CREATELOG_BMPERR
};
#define	WIN_ERROR		((npWin)32)
//#define	HWIN_ERROR		((HWIN)32)

enum {
	APPLI_MEMERR,
	APPLI_TIMERR
};
#define	APPLI_ERROR		((npAppli)32)
//#define	HAPPLI_ERROR		((HAPPLI)32)

// Modes ecran
// -----------
#define SM_DEFAULT	0x00		// Pour SetModeAppli: mode par défaut

#define SM_1		0x01		// DDB or DIB
#define SM_4		0x02		// DDB or DIB
#define SM_8		0x03		// DDB or DIB
#define SM_24		0x04		// DDB or DIB
#define SM_4p		0x05		// DDB only
#define SM_15		0x06		// DDB only

#define	SM_16		(SM_15+1)
#define	SM_32		(SM_16+1)
#define	MAX_MODE	SM_32

#define	SM_DDRAW	0x20		// Flag "Direct Draw"
#define	SM_VRAM		0x40		// Flag "Video RAM"
#define	SM_NOTDIB	(SM_DDRAW | SM_VRAM)
#define	SM_MASK		0x0F		// Mask mode

// Modes graphiques
#define GROP_NORMAL R2_COPYPEN
#define GROP_XOR    R2_XORPEN

// Modes collisions
#define CM_BOX      		0
#define CM_BITMAP   		1

// Test collisions
#define	CM_TEST_OBSTACLE	0
#define	CM_TEST_PLATFORM	1

// Masques de bits d'écriture des plans de collision
#define CM_OBSTACLE			0x0001
#define CM_PLATFORM			0x0002

// Hauteur des plateformes
#define	HEIGHT_PLATFORM		6
#define GCMF_OBSTACLE		0x0000
#define GCMF_PLATFORM		0x0001

// Flags pour GetImageSize
#define GIS_WITHMASK    0x8000
#define	GIS_EXACT		0x4000

	// AddImage flags
#define AI_WITHMASK		0x0010		// L'image originale a un masque (si son mode en a un)
#define AI_COMPARE		0x0020		// Comparer l'image aux autres
#define	AI_ACE			0x0080		// En mode >= 256 couleurs, compacter le sprite
#define	AI_FLAGS		0x00F0		// Masque flags

#define	AI_NOCOMP		0x0000
#define	AI_RLE			0x0100		// Compactage type RLE
#define	AI_RLEW			0x0200		// Compactage type RLEW
#define	AI_RLET			0x0400		// Compactage type RLET
#define	AI_LZX			0x0800		// Compactage type LZX
#define	AI_ALPHA		0x1000		// The image contains an alpha channel
#define	AI_ACECOMP		0x4000		// Compactage sprite type ACE
#define	AI_MAC			0x8000		// Flag temporaire utilise par la version MAC (sur?)
#define	AI_COMPMASK		(IF_RLE | IF_RLEW | IF_RLET)

	// imgFlags
#define	IF_NOCOMP		0x00
#define	IF_RLE			0x01		// Compactage type RLE
#define	IF_RLEW			0x02		// Compactage type RLEW
#define	IF_RLET			0x04		// Compactage type RLET
#define	IF_LZX			0x08		// Compactage type LZX
#define	IF_ALPHA		0x10		// The image contains an alpha channel
#define	IF_ACE			0x40		// Compactage sprite type ACE
#define	IF_MAC			0x80		// Flag temporaire utilise par la version MAC (sur?)
#define	IF_COMPMASK		(IF_RLE | IF_RLEW | IF_RLET)

	// PasteSprite flags
#define	PSF_HOTSPOT		0x0001		// Take hot spot into account
#define	PSF_NOTRANSP	0x0002		// Non transparent image... ignored in PasteSpriteEffect

	// Ink effects
enum {
	EFFECT_NONE=0,
	EFFECT_SEMITRANSP,
	EFFECT_INVERTED,
	EFFECT_XOR,
	EFFECT_AND,
	EFFECT_OR,
	MAX_EFFECT
};

#define	EFFECTFLAG_TRANSPARENT		0x10000000L
#define	EFFECTFLAG_ANTIALIAS		0x20000000L
#define	EFFECT_MASK					0xFFFF

	// AddSound flags
#define AS_COMPARE			0x00010000		// Compare to others
#define	AS_MASK				0xFFFF0000

	// PlaySound flags
#define	PS_NORMAL			0x0000		// Nothing special
#define	PS_UNINTER			0x0001		// Cannot be stopped by other sound except by other UNINTER_PRIO
#define	PS_UNINTER_PRIO		0x0002		// Cannot be stopped
#define	PS_LOOP				0x0010		// Loop, sLParam = loop number, 0 = continuous
#define	PS_GLOBALFOCUS		0x0020		// Sound not stopped if the application loses the focus
#define	PS_DSOUND			0x0100		// To play with Direct Sound

	// AddFont flags
#define AF_COMPARE		0x100       // Compare to others

	// WinPasteText flags
#define	WPTF_CALCRECT		0x8000
#define	WPTF_WITHPREFIX		0x4000



// =======================================================================
// Memory banks
// =======================================================================
// Public:
// -------

// Types
enum	{
	BK_IMGS,				// Image bank
	BK_SNGS,				// Sound bank
	BK_COLMASKS,			// Collision mask bank
	BK_COLMASKS_PLATFORM,	// Platform collision mask bank
	BK_IMGSFS,				// Image surfaces
	BK_FONTS,				// Font bank
	BK_MAX
};

// =======================================================================
// Images
// =======================================================================

typedef struct  Img
{
	DWORD		imgCheckSum;
	DWORD		imgCount;
	DWORD		imgSize;
	short		imgWidth;
	short		imgHeight;
	BYTE		imgFormat;
	BYTE		imgFlags;
	WORD		imgNotUsed;
	short		imgXSpot;
	short		imgYSpot;
	short		imgXAction;
	short		imgYAction;
	COLORREF	imgTrspColor;
} Img;
typedef Img* npImg;
typedef Img* fpImg;


// =======================================================================
// Sounds
// =======================================================================

typedef struct  Sound
{
	DWORD	snCheckSum;			// Checksum (du son sans l'entete)
	DWORD	snCount;			// Ref count
	DWORD	snSize;				// Taille of datas (including name)
	DWORD	snFlags;			// Type (SP_MIDI, SP_WAVE) + flags
	DWORD	snReserved;
	DWORD	snNameSize;
} Sound;
typedef Sound* fpSound;

	// Sound flags
#define SP_WAVE				0x0001               // Flags
#define SP_MIDI				0x0002
#define SNDF_TYPEMASK		0x000F
#define	SNDF_LOADONCALL		0x0010
#define	SNDF_PLAYFROMDISK	0x0020
#define	SNDF_LOADED			0x1000

// =======================================================================
// Fonts
// =======================================================================

// Structure Font
typedef struct  Font
{
	DWORD		fnCheckSum;     // Checksum (des images)
	DWORD		fnCount;        // Compteur d'utilisations
	DWORD		fnSize;         // Taille sans l'entete
	LOGFONT		fnLf;			// Header font (pour CreateFont si Windows font)
} Font;
typedef Font *npFont;
typedef Font *fpFont;

	// Font avec LOGFONT 16 bits pour version 32 bits

typedef struct tagLOGFONT16 {
	short	lfHeight;
	short	lfWidth;
	short	lfEscapement;
	short	lfOrientation;
	short	lfWeight;
	BYTE	lfItalic;
	BYTE	lfUnderline;
	BYTE	lfStrikeOut;
	BYTE	lfCharSet;
	BYTE	lfOutPrecision;
	BYTE	lfClipPrecision;
	BYTE	lfQuality;
	BYTE	lfPitchAndFamily;
	BYTE	lfFaceName[LF_FACESIZE];
} LOGFONT16;

	// Structures
	// ----------

	// Zone, box, rectangle
#ifndef _dllpriv_h
typedef struct  tagBox
{
	short	x1,y1,x2,y2;
} Box;
typedef Box *npBox;
typedef Box *fpBox;
#endif

	// Structure infos ecran
typedef struct tagSMI {
	UINT	totalBitCount;		// I.e. 16
	UINT	usedBitCount;		// I.e. 15
	UINT	mainMode;			// SM_4, SM_8, SM_etc... avec flags SM_DDRAW
	UINT	subMode;			// sous-mode
	DWORD	rMask;
	DWORD	gMask;
	DWORD	bMask;
	UINT	flags;
} screenModeInfos;

	// Objet fenetre
#define	WF_SIZE						0x0001			// Flag "resize in progress"
#define	WF_STRETCH					0x0002			// Flag "stretch log to phys"
#define	WF_NOPAINT					0x0008			// No paint
#define	WF_DONOTMERGEZONES			0x0010			// Do not merge new zones
#define	WF_IDLE						0x0020			// Idle mode (GetMessage)
#define	WF_IDLE50					0x0040			// Idle mode (GetMessage + IT50)
#define	WF_IDLEFLAGS	(WF_IDLE | WF_IDLE50)		// Idle mode flags
#define	WF_VIEWZONES				0x0080			// Flag "view zones" en mode debug
#define	WF_SCR						0x0100			// Flag "Screen saver window"
#define	WF_MDIMAIN					0x0200			// MDI Frame
#define	WF_MDICLIENT				0x0400			// MDI Client
#define	WF_MDICHILD					0x0800			// MDI Child
#define	WF_OWNDC					0x1000
#define	WF_OWNDCLOG					0x2000
#define	WF_FULLSCREENX2				0x4000
#define	WF_DONOTDESTROY				0x8000

#define	WF2_NOUPDATE				0x0001
#define WF2_DIBFULLSCREEN			0x0002
#define	WF2_MMF15					0x0004
#define WF2_VSYNC					0x0008
#define	WF2_NOBACKSURF				0x0010

	// Objet application
#define	APPF_ENDAPP					0x0001		// Internal
#define	APPF_GLOBALSOUNDS			0x0004

	// Objet sprite
#define SF_RAMBO    		0x00000001		// flag "rentre dans tout le monde"
#define SF_RECALCSURF		0x00000002		// Recalc surface (if rotation or stretch)
#define SF_PRIVATE			0x00000004		// flag privé utilisé par le runtime pour la destruction des fade
#define	SF_INACTIF			0x00000008		// flag "inactif" = reaffichage ssi intersection avec un autre
#define SF_TOHIDE			0x00000010		// flag "a cacher"
#define SF_TOKILL			0x00000020		// flag "a detruire"
#define SF_REAF				0x00000040		// flag "a reafficher"
#define	SF_HIDDEN			0x00000080		// flag "cache"
#define	SF_COLBOX			0x00000100		// flag "collisions en mode box"
#define	SF_NOSAVE			0x00000200		// flag "do not save background"
#define	SF_FILLBACK			0x00000400		// flag "fill background using a solid colour (sprAdrBack)"
#define	SF_DISABLED			0x00000800
#define	SF_REAFINT			0x00001000		// Internal
#define	SF_OWNERDRAW		0x00002000		// flag "owner draw"
#define	SF_OWNERSAVE		0x00004000		// flag "owner save"
#define	SF_FADE				0x00008000		// Private

#define SF_OBSTACLE			0x00010000		// Obstacle
#define SF_PLATFORM			0x00020000		// Platform
#define	SF_BACKGROUND		0x00080000		// Backdrop object

#define	SF_SCALE_RESAMPLE	0x00100000		// Resample when stretching
#define	SF_ROTATE_ANTIA		0x00200000		// Antialiasing for rotations
#define	SF_NOHOTSPOT		0x00400000		// No hot spot
#define SF_OWNERCOLMASK		0x00800000		// Owner-draw sprite supports collision masks

#define SF_UPDATECOLLIST	0x10000000

// SpriteCol_TestPoint / SpriteCol_TestSprite
#define SCF_OBSTACLE		0x01
#define SCF_PLATFORM		0x02
#define SCF_EVENNOCOL		0x04			// Flag: returns even sprites that haven't the SF_RAMBO flag
#define SCF_BACKGROUND		0x08			// Flag: if 0, returns active sprites, otherwise returns background sprites
#define	SCF_TESTFEET		0x10			// Test only the bottom of the sprite

#define LAYER_ALL			(-1)

// GetFirst/Last/Next/PrevSprite
#define GS_BACKGROUND	0x0001
#define GS_SAMELAYER	0x0002

// ActiveSprite
#define	AS_DEACTIVATE		0x0000			// Desactive un sprite actif
#define	AS_REDRAW			0x0001			// Reaffiche un sprite inactif
#define	AS_ACTIVATE			0x0002			// Active un sprite inactif
#define	AS_ENABLE			0x0004
#define	AS_DISABLE			0x0008
#define	AS_REDRAW_NOBKD		0x0011
#define AS_REDRAW_RECT		0x0020

#define	SSF_HIDE		0x0000			// Cacher un sprite
#define	SSF_SHOW		0x0001			// Montrer un sprite

// Ownerdraw sprites, callback function
enum	{
	SPRITE_DRAW,
	SPRITE_RESTORE,
	SPRITE_SAVE,
	SPRITE_KILL,
	SPRITE_GETCOLMASK,
};

	// Objet libre
#ifndef _dllpriv_h
typedef struct  tagObj
{
	UINT	objSize;                   // Taille de l'objet: 0 = fin objets
	UINT	objPrevSize;               // Taille objet precedent (0 = 1er objet)
	UINT	objType;                   // Type d'objet (1=vide,2=appli,3=fenetre,...)
	UINT	objPrev;                   // Adresse objet precedent de meme type (0 = first)
	UINT	objNext;                   // Adresse objet suivant de meme type (0 = last)
} Obj;
typedef Obj *npObj;
typedef Obj *fpObj;
#endif

#ifndef _H2INC
#ifdef IN_DLL
class Spr;
#endif
#endif

#define SPRCOLLISLIST

#ifdef __cplusplus
class Spr
{
public:
#else
typedef struct  Spr
{
#endif
#ifndef SPRLIST
	Obj			sprObj;
#endif
	DWORD		sprFlags;			// Flags
	WORD		sprLayer;			// Sprite plane (layer)
	short		sprAngle;			// Angle
	int			sprZOrder;			// Z-order value

	// Coordinates
	int			sprX;
	int			sprY;

	// Bounding box
	int			sprX1;
	int			sprY1;
	int			sprX2;
	int			sprY2;

	// New coordinates
	int			sprXnew;
	int			sprYnew;

	// New bounding box
	int			sprX1new;
	int			sprY1new;
	int			sprX2new;
	int			sprY2new;

	// Background bounding box
	int			sprX1z;
	int			sprY1z;
	int			sprX2z;
	int			sprY2z;

	// Scale & Angle
	float		sprScaleX;
	float		sprScaleY;

	// Temporary values for collisions
	WORD		sprTempImg;			// TODO: use DWORD later?
	short		sprTempAngle;
	float		sprTempScaleX;
	float		sprTempScaleY;

	// Image or owner-draw routine
	union	{
		struct	{
	DWORD		sprImg;             // Numero d'image
	DWORD		sprImgNew;          // Nouvelle image
				};
	LPARAM		sprRout;			// Ownerdraw callback routine
			};

	// Ink effect
	DWORD		sprEffect;			// 0=normal, 1=semi-transparent, > 16 = routine
	LPARAM		sprEffectParam;		// parametre effet (coef transparence, etc...)

	// Fill color (wipe with color mode)
	COLORREF	sprBackColor;

	// Surfaces
	cSurfaceImplementation*	sprBackSurf;	// Background surface, if no general background surface

	cSurfaceImplementation*	sprSf;			// Surface (if stretched or rotated)
	sMask*					sprColMask;		// Collision mask (if stretched or rotated)

	cSurfaceImplementation*	sprTempSf;		// Temp surface (if stretched or rotated)
	sMask*					sprTempColMask;	// Temp collision mask (if stretched or rotated)

	// User data
	LPARAM		sprExtraInfo;

	// Colliding sprites
#ifndef _H2INC
#ifdef IN_DLL
	CPTypeArray<Spr>	sprCollisList;	// liste de sprites entrant en collisions
#else
	int					sprCollisList[2];
#endif
#endif

#ifdef __cplusplus
};
#else
} Spr;
#endif
typedef Spr *npSpr;
typedef Spr *fpSpr;

	// Structure "taille d'une appli"
typedef	struct	tagAppSize {
	DWORD	asInternal;				// Place occupee dans le segment DLL
	DWORD	asLogics;				// Taille ecrans logiques
	DWORD	asColMasks;				// Taille buffers de collision avec le decor
	DWORD	asImages;				// Banque des images
	DWORD	asMasks;				// Banque des masques de collision des images
	DWORD	asSounds;				// Banque des sons
	DWORD	asFonts;				// Banque des fonts
	DWORD	asSprites;				// Buffers de sauvegarde des fonds des sprites
} appSize;
typedef	appSize *fpas;


// Structure pour WinOpenEx
typedef	struct	tagCW {
	DWORD			cwSize;
	CREATESTRUCT	cwCreateStruct;
	int				cwCxMax;
	int				cwCyMax;
	HCURSOR			cwHCursor;
	union {
		DWORD		cwBackColor;
		HBRUSH		cwHBackBrush;
	};
	int				cwClsWin;
	int				cwWinFlags;
} CREATEWIN;

typedef	struct	CREATEWINEX {
	DWORD			cwSize;
	CREATESTRUCT	cwCreateStruct;
	int				cwCxMax;
	int				cwCyMax;
	HCURSOR			cwHCursor;
	union {
		DWORD		cwBackColor;
		HBRUSH		cwHBackBrush;
	};
	int				cwClsWin;
	int				cwWinFlags;
	HWND			cwHWnd;
	LPARAM			cwNotUsed;
} CREATEWINEX;

// Structure for SaveRect
typedef	struct	saveRect {
	LPBYTE		pData;
	RECT		rc;
} saveRect;
typedef saveRect* fpSaveRect;


//------------------------------
// Prototypes des fonctions KNPS
//------------------------------

#define DialOpen(hi,id,hp,pr,a,b,c,lp) DialogBoxParam(hi,id,hp,pr,lp)

DLLExport32 DWORD	WINAPI GetDLLVersion();

	// Application
	// -----------
DLLExport32 npAppli	WINAPI InitAppli		(HINSTANCE, HPALETTE);
DLLExport32 DWORD	WINAPI SetModeAppli		(npAppli, int);
DLLExport32 int		WINAPI LockBank			(npAppli, int);
DLLExport32 int		WINAPI UnlockBank		(npAppli, int);
DLLExport32 int		WINAPI PurgeBank		(npAppli, int);
DLLExport32 void	WINAPI KillBank			(npAppli, int);
DLLExport32 int		WINAPI Bank_GetEltCount (npAppli ptApp, UINT bkNum);
DLLExport32 LPVOID	WINAPI Bank_GetEltAddr (npAppli ptApp, UINT bkNum, UINT eNum);

DLLExport32 void	WINAPI EndAppli			(npAppli);

	// Windows, dialogues
	// ------------------
DLLExport32 npWin	WINAPI WinOpenEx		(npAppli, CREATEWIN   *);
DLLExport32 npWin	WINAPI SCRWinOpen		(npAppli, HWND);
DLLExport32 void	WINAPI WCDClose			(npWin);
DLLExport32 HWND	WINAPI WinGetHandle		(npWin);
DLLExport32 HWND	WINAPI WinGetMCHandle	(npWin);
DLLExport32 HDC		WINAPI WinGetHDC		(npWin);
DLLExport32 HDC		WINAPI WinGetHDCLog		(npWin);
DLLExport32 void	WINAPI WinReleaseHDC	(npWin, HDC);
DLLExport32 void	WINAPI WinReleaseHDCLog	(npWin, HDC);
DLLExport32 void	WINAPI WinGetLogRect	(npWin, RECT   *);
DLLExport32 npWin   WINAPI WinSearch		(HWND);

DLLExport32 LRESULT CALLBACK DefMsgProc		(HWND hWin, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Menus
	// -----
DLLExport32 HMENU	WINAPI WinSetMenu      (npWin, HMENU, HACCEL);
//DLLExport32 void	WINAPI WinSetAccel     (npWin, HACCEL);

	// Souris
	// ------
DLLExport32 HCURSOR	WINAPI WinSetMouse     (npWin, HCURSOR);
DLLExport32 void	WINAPI WinStartWait    (npWin);
DLLExport32 void	WINAPI WinEndWait      (npWin);

	// Events
	// ------
//DLLExport32 WORD	WINAPI GetMsg			(npWin, fpKm);
//DLLExport32 DWORD	WINAPI GetIt50			(void);
//DLLExport32 DWORD	WINAPI RazCptVbl		(void);
//DLLExport32 DWORD	WINAPI GetCptVbl		(void);
//DLLExport32 void	WINAPI EnableIt50		(npWin, int);

	// Graphics
	// --------
DLLExport32 void	WINAPI WinClip			(npWin, int, int, int, int);
DLLExport32 void	WINAPI WinFillRect		(npWin, int, int, int, int, DWORD);
DLLExport32 void	WINAPI WinFillBr		(npWin, int, int, int, int, HBRUSH);
DLLExport32 void	WINAPI WinBox			(npWin, fpBox);
DLLExport32 void	WINAPI WinRect			(npWin, RECT   *);
DLLExport32 void	WINAPI WinLine			(npWin, POINT   *, UINT);
DLLExport32 void	WINAPI WinGrabDesktop	(npWin);

#ifndef IN_KPX
DLLExport32 int		WINAPI StartFullScreen	(npWin, int, int, int);
DLLExport32 int		WINAPI EndFullScreen	();
#endif // IN_KPX

DLLExport32 void	WINAPI WinPaper			(npWin, COLORREF);
DLLExport32 void	WINAPI WinPen			(npWin, COLORREF, int, int);
DLLExport32 void	WINAPI WinGraphMode		(npWin, int);

#define		WSP_NBSPRITES		1
#define		WSP_SETAPPFLAGS		2
#define		WSP_GETAPPFLAGS		4
#define		WSP_GETWINFLAGS2	9
#define		WSP_SETWINFLAGS2	10
DLLExport32 BOOL	WINAPI WinSetParam		(npWin, int, UINT, DWORD);

	// Dibs
	// ----
DLLExport32 DWORD	WINAPI InitDibHeader	(npAppli, int, int, int, BITMAPINFO *);
DLLExport32 void	WINAPI FillDib			(BITMAPINFO   *, COLORREF);
DLLExport32 DWORD	WINAPI ImageToDib		(npAppli, DWORD, DWORD, LPBYTE);
DLLExport32 DWORD	WINAPI DibToImage		(npAppli, fpImg, BITMAPINFOHEADER *);
DLLExport32 DWORD	WINAPI DibToImageEx		(npAppli, fpImg, BITMAPINFOHEADER *, COLORREF, DWORD);
DLLExport32 void	WINAPI RemapDib			(BITMAPINFO   *, npAppli, LPBYTE);

	// Palette
	// -------
DLLExport32 HPALETTE WINAPI	SetDefaultPalette256	(PALETTEENTRY   *);
DLLExport32 void	WINAPI SetPaletteAppli			(npAppli, HPALETTE);
DLLExport32 int		WINAPI GetPaletteAppli			(LOGPALETTE   *, int, npAppli);
DLLExport32 int		WINAPI GetNearestIndex			(npAppli, int, COLORREF);
DLLExport32 COLORREF WINAPI GetRGB					(npAppli, int, int);
DLLExport32 int		WINAPI GetOpaqueBlack			(npAppli);

	// Font
	// ----
DLLExport32 DWORD	WINAPI AddFont			(npAppli, DWORD, fpFont, LPDWORD, DWORD);
DLLExport32 DWORD	WINAPI IncFontCount		(npAppli, DWORD);
DLLExport32 long	WINAPI DelFont			(npAppli, DWORD);
DLLExport32 int		WINAPI GetFontInfos		(npAppli, DWORD, fpFont, LPDWORD);
DLLExport32 HFONT	WINAPI WinCreateFont	(npAppli, DWORD);
DLLExport32 int		WINAPI WinPasteText		(npWin, HFONT, RECT*, LPSTR, COLORREF, DWORD);

	// Fonts - fonctions 32 bits utilisant des structures 16 bits
DLLExport32	void	WINAPI LogFont16To32	( LOGFONT   * lf32, LOGFONT16   * lf16 );
DLLExport32	void	WINAPI LogFont32To16	( LOGFONT16   * lf16, LOGFONT   * lf32 );
DLLExport32	HFONT	WINAPI CreateFontIndirect16	( LOGFONT16   * lFont );

	// Images
	// ------
DLLExport32 void		WINAPI PasteSprite		(npWin, DWORD, int, int, DWORD);
DLLExport32 void		WINAPI PasteSpriteEffect (npWin, DWORD, int, int, DWORD, DWORD, LPARAM);
DLLExport32 DWORD		WINAPI AddImage			(npAppli, WORD, WORD, short, short, short, short, COLORREF, DWORD, LPVOID, LPBYTE);
DLLExport32 DWORD		WINAPI IncImageCount	(npAppli, DWORD);
DLLExport32 int			WINAPI IsImageEmpty		(npAppli, DWORD);
DLLExport32 DWORD		WINAPI GetImageBits		(npAppli, DWORD, DWORD, LPBYTE);
DLLExport32 void		WINAPI StretchImage		(npAppli, DWORD, WORD, WORD, LPBYTE);
DLLExport32 long		WINAPI DelImage			(npAppli, DWORD);
DLLExport32 sMask*		WINAPI AddMask			(npAppli ptApp, DWORD iNum, UINT nFlags);
DLLExport32 long		WINAPI DelMask			(npAppli, DWORD);
DLLExport32 int			WINAPI GetImageInfos	(npAppli, DWORD, fpImg);
DLLExport32 int			WINAPI GetImageInfoEx	(npAppli pApp, DWORD nImage, int nAngle, float fScaleX, float fScaleY, fpImg pIfo);
DLLExport32 DWORD		WINAPI GetImageSize		(WORD, WORD, WORD);
DLLExport32 sMask*		WINAPI GetImageMask		(npAppli ptApp, DWORD dwImage, UINT nFlags);
DLLExport32 sMask*		WINAPI CompleteSpriteColMask (npWin ptrWin, npSpr ptSpr, DWORD dwPSCFlags,	UINT nWidth, UINT nHeight);
DLLExport32 DWORD		WINAPI PrepareSpriteColMask (npWin ptrWin, npSpr ptSpr, DWORD newImg, int newX, int newY, int newAngle, float newScaleX, float newScaleY, RECT* prc, sMask** ppMask);

	// Sprites
	// -------
DLLExport32 npSpr	WINAPI AddSprite		(npWin ptrWin, int xSpr, int ySpr, DWORD iSpr, WORD wLayer, int nZOrder, DWORD backSpr, DWORD sFlags, LPARAM extraInfo);
DLLExport32 npSpr	WINAPI AddOwnerDrawSprite (npWin ptrWin, int x1, int y1, int x2, int y2, WORD wLayer, int nZOrder, DWORD backSpr, DWORD sFlags, LPARAM extraInfo, LPARAM sprProc);
DLLExport32 npSpr	WINAPI ModifSprite		(npWin ptrWin, npSpr ptSpr, int xSpr, int ySpr, DWORD iSpr);
DLLExport32 npSpr	WINAPI ModifSpriteEx	(npWin ptrWin, npSpr ptSpr, int xSpr, int ySpr, DWORD iSpr, float fScaleX, float fScaleY, BOOL bResample, int nAngle, BOOL bAntiA);
DLLExport32 npSpr	WINAPI ModifSpriteEffect (npWin ptrWin, npSpr ptSpr, DWORD effect, LPARAM effectParam);
DLLExport32 npSpr	WINAPI ModifOwnerDrawSprite	(npWin ptrWin, npSpr ptSprModif, int x1, int y1, int x2, int y2);
DLLExport32 void	WINAPI ShowSprite		(npWin, npSpr, int);
DLLExport32 void	WINAPI ActiveSprite		(npWin, npSpr, int);
DLLExport32 void	WINAPI DelSprites		(npWin);
DLLExport32 void	WINAPI DelSprite		(npWin, npSpr);
DLLExport32 void	WINAPI DelSpriteFast	(npWin, npSpr);
DLLExport32 LPARAM	WINAPI GetSpriteExtra	(npWin, npSpr);
DLLExport32 DWORD	WINAPI GetSpriteFlags	(npWin, npSpr);
DLLExport32 DWORD	WINAPI SetSpriteFlags	(npWin ptrWin, npSpr ptSpr, DWORD dwNewFlags);
DLLExport32 void	WINAPI SpriteClear		(npWin);
DLLExport32 void	WINAPI SpriteDraw		(npWin);
DLLExport32 void	WINAPI SpriteUpdate		(npWin);
DLLExport32 void	WINAPI ScreenUpdate		(npWin);

DLLExport32 void	WINAPI GetSpriteScale	(npWin ptrWin, npSpr ptSpr, float* pScaleX, float* pScaleY, BOOL* pResample);
DLLExport32 void	WINAPI SetSpriteScale	(npWin ptrWin, npSpr ptSpr, float fScaleX, float fScaleY, BOOL bResample);
DLLExport32 int		WINAPI GetSpriteAngle	(npWin ptrWin, npSpr ptSpr, BOOL* pAntiA);
DLLExport32 void	WINAPI SetSpriteAngle	(npWin ptrWin, npSpr ptSpr, int nAngle, BOOL bAntiA);
DLLExport32 void	WINAPI GetSpriteRect	(npWin ptrWin, npSpr ptSpr, RECT* prc);
DLLExport32 void	WINAPI MoveSpriteToFront(npWin ptrWin, npSpr pSpr);
DLLExport32 void	WINAPI MoveSpriteToBack	(npWin ptrWin, npSpr pSpr);
DLLExport32 void	WINAPI MoveSpriteBefore	(npWin ptrWin, npSpr pSprToMove, npSpr pSprDest);
DLLExport32 void	WINAPI MoveSpriteAfter	(npWin ptrWin, npSpr pSprToMove, npSpr pSprDest);
DLLExport32 BOOL	WINAPI IsSpriteBefore	(npWin ptrWin, npSpr pSpr, npSpr pSprDest);
DLLExport32 BOOL	WINAPI IsSpriteAfter	(npWin ptrWin, npSpr pSpr, npSpr pSprDest);
DLLExport32 void	WINAPI SwapSprites		(npWin ptrWin, npSpr sp1, npSpr sp2);
DLLExport32 int		WINAPI GetSpriteLayer	(npWin ptrWin, npSpr ptSpr);
DLLExport32 void	WINAPI SetSpriteLayer	(npWin ptrWin, npSpr ptSpr, int nLayer);
DLLExport32 sMask*	WINAPI GetSpriteMask	(npWin ptrWin, npSpr pSpr, UINT newImg, UINT nFlags);

DLLExport32 npSpr	WINAPI GetFirstSprite	(npWin ptrWin, int nLayer, DWORD dwFlags);
DLLExport32 npSpr	WINAPI GetNextSprite	(npWin ptrWin, npSpr pSpr, DWORD dwFlags);
DLLExport32 npSpr	WINAPI GetPrevSprite	(npWin ptrWin, npSpr pSpr, DWORD dwFlags);
DLLExport32 npSpr	WINAPI GetLastSprite	(npWin ptrWin, int nLayer, DWORD dwFlags);

	// Sauvegarde / Restitution de zones
	// ---------------------------------
DLLExport32 void	WINAPI	WinResetZones	(npWin);
DLLExport32 void	WINAPI	WinAddZone		(npWin, RECT   *);
DLLExport32 void	WINAPI	WinAddCoord		(npWin, int, int, int, int);
DLLExport32 int		WINAPI	SaveRect		(npWin, fpSaveRect, int, int, int, int);
DLLExport32 void	WINAPI	RestoreRect		(npWin, fpSaveRect);
DLLExport32 void	WINAPI	KillRect		(fpSaveRect);
DLLExport32 UINT*	WINAPI	WinGetZones		(npWin);
DLLExport32 void	WINAPI	WinEnableUpdate	(npWin, int);

	// Collisions
	// ----------
DLLExport32 DWORD	WINAPI SetSpriteColFlag			(npWin, npSpr, DWORD);				// UINT = SF_xxxx
DLLExport32 npSpr   WINAPI SpriteCol_TestPoint		(npWin, npSpr, int, int, int, DWORD);		// Entre 1 pixel et les sprites sauf un
DLLExport32 npSpr   WINAPI SpriteCol_TestRect		(npWin, npSpr, int, int, int, int, int, DWORD);		// Entre 1 rectangle et les sprites sauf un
DLLExport32 npSpr	WINAPI SpriteCol_TestSprite		(npWin ptrWin, npSpr ptSpr, DWORD newImg, int newX, int newY, int newAngle, float newScaleX, float newScaleY, int subHt, DWORD dwFlags);
DLLExport32 UINT	WINAPI SpriteCol_TestSprite_All	(npWin, npSpr, LPVOID*, DWORD, int, int, int, float, float, DWORD);
DLLExport32 int     WINAPI WinSetColMode			(npWin, WORD);						// Mode BOX ou BITMAP

DLLExport32 BOOL	WINAPI ColMask_Create			(npWin, UINT, UINT, DWORD);			// Creation bitmap masque fond
DLLExport32 BOOL	WINAPI ColMask_CreateEx			(npWin, int, int, int, int, DWORD);
DLLExport32 void    WINAPI ColMask_Kill				(npWin);							// Destruction masque fond
DLLExport32 void    WINAPI ColMask_Fill				(npWin, DWORD);						// Init masque fond
DLLExport32 int     WINAPI ColMask_FillRectangle	(npWin, int, int, int, int, DWORD);
DLLExport32 void	WINAPI ColMask_OrImage			(npWin, DWORD, int, int, DWORD);
DLLExport32 void    WINAPI ColMask_OrMask			(npWin, sMask*, int, int, DWORD, DWORD);
DLLExport32 void	WINAPI ColMask_OrPlatform		(npWin, DWORD, int, int);
DLLExport32 void	WINAPI ColMask_OrPlatformMask	(npWin, sMask*, int, int);
DLLExport32 int		WINAPI ColMask_Scroll			(npWin, int, int, RECT   *, RECT   *);
DLLExport32 void	WINAPI ColMask_SetOrigin		(npWin, int, int);
DLLExport32 void	WINAPI ColMask_ToLog			(npWin ptrWin, UINT nPlane);
DLLExport32 void	WINAPI ColMask_SetClip			(npWin, RECT *);

DLLExport32 BOOL	WINAPI ColMask_TestPoint		(npWin ptrWin, int x, int y, UINT nPlane);
DLLExport32 BOOL	WINAPI ColMask_TestSprite		(npWin ptrWin, npSpr pSpr, int newImg, int newX, int newY, int newAngle, float newScaleX, float newScaleY, int subHt, UINT nPlane);
DLLExport32 int     WINAPI ColMask_TestRect			(npWin, int, int, int, int, UINT nPlane);

DLLExport32 BOOL	WINAPI Mask_TestPoint			(LPBYTE pMask, int x, int y, int nMaskWidth, int nMaskHeight);
DLLExport32 BOOL	WINAPI Mask_TestMask			(LPBYTE pMaskBits1, LPBYTE pMaskBits2, int x1Spr1, int y1Spr1, int wSpr1, int hSpr1, int x1Spr2, int y1Spr2, int wSpr2, int hSpr2);
DLLExport32 BOOL	WINAPI Mask_TestRect			(LPBYTE pMask, int nMaskWidth, int nMaskHeight, int x, int y, int nWidth, int nHeight);

	// Hooks
	// -----
DLLExport32 HHOOK	WINAPI StartFilterHook	(HWND);
DLLExport32 int		WINAPI StopFilterHook	(HHOOK);

	// Divers
	// ------
#define WavePerio()

	// New
	// ---
DLLExport32 void	WINAPI WinSetFlags		(npWin, WORD);
DLLExport32 WORD	WINAPI WinGetFlags		(npWin);
DLLExport32 int		WINAPI WinScroll		(npWin, int, int, int, int, int, int);

DLLExport32 int		WINAPI EnumScreenModes  (screenModeInfos *ptsfo, int maxModes, UINT flags);

	// V2
	// --

#define	HCSPALETTE	UINT

DLLExport32 HCSPALETTE	WINAPI AddPalette (LOGPALETTE* pLogPal);
DLLExport32 void		WINAPI DelPalette (HCSPALETTE pCsPal);
DLLExport32 void		WINAPI SetAppCSPalette (npAppli ptrApp, HPALETTE hpal, HCSPALETTE pCsPal);
DLLExport32 HCSPALETTE	WINAPI GetAppCSPalette (npAppli ptrApp);

// Sounds

#ifdef __cplusplus
class CSoundManager;
DLLExport32 int WINAPI PlaySnd (CSoundManager* pSndMgr, npAppli ptApp, UINT sNum, HWND hWin, DWORD sFlags, DWORD sLParam, UINT nChannel);
#endif // __cplusplus

DLLExport32 void WINAPI PauseSnd (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 void WINAPI PauseSndChannel (npAppli ptApp, UINT nChannel);
DLLExport32 void WINAPI ResumeSnd (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 void WINAPI ResumeSndChannel (npAppli ptApp, UINT nChannel);
DLLExport32 void WINAPI StopSnd (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 void WINAPI StopSndChannel (npAppli ptApp, UINT nChannel);
DLLExport32 int WINAPI IsSndPlaying (npAppli ptApp, UINT nSound);
DLLExport32 int WINAPI IsSndChannelPlaying (npAppli ptApp, UINT nChannel);
DLLExport32 int WINAPI IsSndPaused (npAppli ptApp, UINT nSound);
DLLExport32 int WINAPI IsSndChannelPaused (npAppli ptApp, UINT nChannel);
DLLExport32 void WINAPI SetSndMainVolume (npAppli ptApp, UINT sType, int nVolume);
DLLExport32 void WINAPI SetSndMainPan (npAppli ptApp, UINT sType, int nPan);
DLLExport32 int WINAPI GetSndMainVolume (npAppli ptApp, UINT sType);
DLLExport32 int WINAPI GetSndMainPan (npAppli ptApp, UINT sType);
DLLExport32 void WINAPI SetSndChannelVolume (npAppli ptApp, UINT nChannel, int nVolume);
DLLExport32 void WINAPI SetSndChannelPan (npAppli ptApp, UINT nChannel, int nPan);
DLLExport32 void WINAPI SetSndVolume (npAppli ptApp, UINT sType, UINT sNum, int nVolume);
DLLExport32 void WINAPI SetSndPan (npAppli ptApp, UINT sType, UINT sNum, int nPan);
DLLExport32 int WINAPI GetSndChannelVolume (npAppli ptApp, UINT nChannel);
DLLExport32 int WINAPI GetSndChannelPan (npAppli ptApp, UINT nChannel);
DLLExport32 int WINAPI GetSndVolume (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 int WINAPI GetSndPan (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 DWORD WINAPI GetSndChannelDuration (npAppli ptApp, UINT nChannel);
DLLExport32 DWORD WINAPI GetSndChannelPosition (npAppli ptApp, UINT nChannel);
DLLExport32 void WINAPI SetSndChannelPosition (npAppli ptApp, UINT nChannel, DWORD dwPosition);
DLLExport32 DWORD WINAPI GetSndDuration (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 DWORD WINAPI GetSndPosition (npAppli ptApp, UINT sType, UINT sNum);
DLLExport32 void WINAPI SetSndPosition (npAppli ptApp, UINT sType, UINT sNum, DWORD dwPosition);
DLLExport32 UINT WINAPI FindSndFromName(npAppli ptApp, UINT sType, LPCSTR pName);
DLLExport32 int WINAPI GetSndChannel(npAppli ptApp, UINT sNum);

DLLExport32 void WINAPI LockSndChannel (npAppli ptApp, UINT nChannel, BOOL bLock);
DLLExport32 void WINAPI SetSndChannelFreq (npAppli ptApp, UINT nChannel, DWORD dwFreq);
DLLExport32 void WINAPI SetSndFreq (npAppli ptApp, UINT sType, UINT sNum, DWORD dwFreq);
DLLExport32 DWORD WINAPI GetSndChannelFreq (npAppli ptApp, UINT nChannel);
DLLExport32 DWORD WINAPI GetSndFreq (npAppli ptApp, UINT sType, UINT sNum);

DLLExport32 UINT    WINAPI AddSound			(npAppli, UINT, LPSTR, LPBYTE, DWORD);
DLLExport32 UINT	WINAPI ReplaceSound		(npAppli ptApp, UINT sFlags, UINT nSound, LPSTR fpName, LPBYTE lpData, DWORD dwDataSize);
DLLExport32 BOOL	WINAPI SetSoundFlags	(npAppli ptApp, UINT nSound, UINT sFlags);
DLLExport32 DWORD	WINAPI IncSoundCount	(npAppli, UINT);
DLLExport32 int     WINAPI GetSoundInfo		(npAppli, UINT, fpSound);
DLLExport32 int		WINAPI GetSoundName		(npAppli, UINT, LPSTR, UINT);
DLLExport32 long	WINAPI GetSoundData		(npAppli, UINT, LPBYTE);
DLLExport32 LPBYTE	WINAPI GetSoundDataPtr	(npAppli, UINT);
DLLExport32 long	WINAPI DelSound			(npAppli, UINT);

DLLExport32 int		WINAPI WaveSetChannels	(npAppli ptApp, int nChannels);

#ifdef __cplusplus
}
#endif

#ifndef _H2INC
enum {
	LOCKIMAGE_READBLITONLY,
	LOCKIMAGE_ALLREADACCESS
};

#ifdef __cplusplus
DLLExport32 BOOL WINAPI WinAttachSurface (int idWin, cSurface   * cs);
DLLExport32 void WINAPI WinDetachSurface (int idWin);

DLLExport32 BOOL WINAPI LockImageSurface (LPVOID, DWORD hImage, cSurface   &cs, int flags=LOCKIMAGE_READBLITONLY);
DLLExport32 void WINAPI UnlockImageSurface (cSurface   &cs);

// Get window surface (logical screen)
enum {
	WSURF_LOGSCREEN,
	WSURF_BACKSAVE
};
DLLExport32 cSurface   * WINAPI WinGetSurface (int idWin, int surfID=WSURF_LOGSCREEN);
#endif	// __cplusplus
#endif	// !defined(_H2INC)

#ifndef _H2INC
#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif
#endif

#endif	// _cnpdll_h
