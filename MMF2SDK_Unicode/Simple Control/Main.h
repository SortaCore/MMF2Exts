
// Object identifier "SAMC"

#pragma message ("************************ WARNING ****************************")
#pragma message ("***** Do not forget to change the IDENTIFIER in Main.h! *****")
#pragma message ("*************************************************************")

#define IDENTIFIER	MAKEID(S,A,M,C)		// REQUIRED: you MUST replace the letters in the MAKEID macro by others
										// and then remove the #pragma message above. If you do not do this, MMF2
										// could confuse your object with another in the event editor.

// ------------------------------
// DEFINITION OF CONDITIONS CODES
// ------------------------------
#define	CND_ISEMPTY					0
#define	CND_CHANGED					1
#define	CND_LAST					2

// ---------------------------
// DEFINITION OF ACTIONS CODES
// ---------------------------
#define	ACT_SETTEXT					0
#define ACT_SETINT					1
#define ACT_SETFLOAT				2
#define	ACT_LAST					3

// -------------------------------
// DEFINITION OF EXPRESSIONS CODES
// -------------------------------
#define	EXP_GETTEXT					0
#define	EXP_GETINT					1
#define	EXP_GETFLOAT				2
#define	EXP_LAST					3

// ---------------------
// FLAGS
// ---------------------
#define	SCTRL_SYSTEMCOLORS			1
#define	SCTRL_BORDER				2

// ---------------------
// OBJECT DATA STRUCTURE
// ---------------------
// Used at edit time and saved in the MFA/CCN/EXE files

typedef struct tagEDATAA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONTA		textFont;		// text font
	char			sText[1];		// text (dynamic size)

} EDITDATAA;
typedef EDITDATAA *			LPEDATAA;

typedef struct tagEDATAW
{
	// Header - required
	extHeader		eHeader;

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONTW		textFont;		// text font
	WCHAR			sText[1];		// text (dynamic size)

} EDITDATAW;
typedef EDITDATAW *			LPEDATAW;

#ifdef _UNICODE
#define EDITDATA	EDITDATAW
#define LPEDATA		LPEDATAW
#else
#define EDITDATA	EDITDATAA
#define LPEDATA		LPEDATAA
#endif

// Object versions
#define	KCX_CURRENT_VERSION			1

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// Used at runtime. Initialize it in the CreateRunObject function.
// Free any allocated memory or object in the DestroyRunObject function.
//
// Note: if you store C++ objects in this structure and do not store
// them as pointers, you must call yourself their constructor in the
// CreateRunObject function and their destructor in the DestroyRunObject
// function. As the RUNDATA structure is a simple C structure and not a C++ object.

typedef struct tagRDATA
{
	// Main header - required
	headerObject	rHo;			// Main header - required

	// Optional headers - depend on the OEFLAGS value, see documentation and examples for more info
	rCom			rc;				// Common structure for movements & animations
	rMvt			rm;				// Movements
	rVal			rv;				// Alterable values

	// Object's runtime data
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	HFONT			hFont;			// text font
	HBRUSH			hBrush;			// Background brush in custom colors mode

	int				nWnds;			// Number of windows (for subclassing routine)
	HWND			hWnd;			// Window handles (must follow the number of windows)

	// True events
	DWORD			dwLastChangedLoopNumber;	// Application loop number when the last change occured
	DWORD			dwEvtFlags;					// Flags that indicate if we should trigger a true event condition

} RUNDATA;
typedef	RUNDATA	*			LPRDATA;

// Flags for true event conditions
#define		EVTFLAG_CHANGED		0x0001			// trigger "Has text been changed" condition

// Size when editing the object under level editor
// -----------------------------------------------
#define	MAX_EDITSIZE			sizeof(EDITDATA)

// Default flags
// -------------
#define	OEFLAGS	(OEFLAG_WINDOWPROC | OEFLAG_VALUES | OEFLAG_MOVEMENTS | OEFLAG_TEXT)
#define OEPREFS	0


// If to handle message, specify the priority of the handling procedure
// 0= low, 255= very high. You should use 100 as normal.
// --------------------------------------------------------------------
#define	WINDOWPROC_PRIORITY		100
