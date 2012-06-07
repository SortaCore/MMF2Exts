class rRundata;
typedef rRundata * LPRRDATA;

// ---------------------
// FLAGS
// ---------------------
#define	SCTRL_SYSTEMCOLORS			1
#define	SCTRL_BORDER				2

// Flags for true event conditions
#define		EVTFLAG_CHANGED		0x0001			// trigger "Has text been changed" condition
// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

typedef struct tagRDATA
{
	#include "MagicRDATA.h"
	
	// Object's runtime data
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	HFONT			hFont;			// text font
	HBRUSH			hBrush;			// Background brush in custom colors mode
	int				nWnds;			// Number of windows (for subclassing routine)
	HWND			hWnd;			// Window handles (must follow the number of windows)
	HIMAGELIST		imgl;
	int				iconwidth;
	int				iconheight;
	// True events
	DWORD			dwLastChangedLoopNumber;	// Application loop number when the last change occured
	DWORD			dwEvtFlags;					// Flags that indicate if we should trigger a true event condition
	
} RUNDATA;
typedef	RUNDATA	* LPRDATA;

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the MFA
// You should use these with properties
#ifdef _UNICODE
#define EDITDATA EDITDATAW
#define LPEDATA LPEDATAW
#else
#define EDITDATA EDITDATAA
#define LPEDATA LPEDATAA
#endif

typedef struct tagEDATA_V3A
{
	extHeader		eHeader;

	#ifdef USE_MAGIC_PROPS
	_Properties		Properties;
	#endif

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONTA		textFont;		// text font, ASCII
	WORD			wImages[16];
	int				nImages;
	bool			opts[4];
	//Version 2
	uchar			iconW;
	uchar			iconH;
	ushort			maxIcon;
	uchar			buttonStyle;
	uchar			tabDir;
	uchar			lineMode;
	uchar			tabMode;
	//Version 3
	char			sText[1];		// text (dynamic size), ASCII

} EDITDATAA;
typedef EDITDATAA * LPEDATAA;

typedef struct tagEDATA_V3W
{
	extHeader		eHeader;

	#ifdef USE_MAGIC_PROPS
	_Properties		Properties;
	#endif

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONTW		textFont;		// text font
	WORD			wImages[16];
	int				nImages;
	bool			opts[4];
	//Version 2
	uchar			iconW;
	uchar			iconH;
	ushort			maxIcon;
	uchar			buttonStyle;
	uchar			tabDir;
	uchar			lineMode;
	uchar			tabMode;
	//Version 3 Unicode (ASCII is char)
	WCHAR			sText[1];		// text (dynamic size)

} EDITDATAW;
typedef EDITDATAW * LPEDATAW;


typedef struct tagEDATA_V2
{
	extHeader		eHeader;

	#ifdef USE_MAGIC_PROPS
	_Properties		Properties;
	#endif

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONT			textFont;		// text font
	char			sText[1];		// text (dynamic size)
	WORD			wImages[1];
	int				nImages;
	bool			opts[4];
	//Version 2
	uchar			iconW;
	uchar			iconH;
	ushort			maxIcon;
	uchar			buttonStyle;
	uchar			tabDir;
	uchar			lineMode;
	uchar			tabMode;

} OLDEDITDATA2;

typedef struct tagEDATA_V1
{
	extHeader		eHeader;

	#ifdef USE_MAGIC_PROPS
	_Properties		Properties;
	#endif

	// Object's data
	LONG			nWidth;			// width
	LONG			nHeight;		// height
	COLORREF		textColor;		// text color
	COLORREF		backColor;		// background color
	DWORD			dwAlignFlags;	// alignment flags
	DWORD			dwFlags;		// other flags
	LOGFONT			textFont;		// text font
	char			sText[1];		// text (dynamic size)
	WORD			wImages[1];
	int				nImages;
	bool			opts[4];


} OLDEDITDATA;