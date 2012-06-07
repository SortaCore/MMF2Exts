// Object identifier "SAM2"
#define IDENTIFIER	MAKEID(D,L,D,0)

#include <wininet.h>

// ------------------------------
// DEFINITION OF CONDITIONS CODES
// ------------------------------
#define	CND_DOWNLOADING				0
#define CND_COMPLETED				1
#define CND_ERROR					2
#define CND_ABORTED					3
#define	CND_LAST					4

// ---------------------------
// DEFINITION OF ACTIONS CODES
// ---------------------------
#define	ACT_ACTION_DOWNLOAD			0
#define	ACT_ACTION_ABORT			1
#define	ACT_ACTION_PAUSE			2
#define	ACT_ACTION_RESUME			3
#define	ACT_LAST					4

// -------------------------------
// DEFINITION OF EXPRESSIONS CODES
// -------------------------------
#define	EXP_TOTALSIZE				0
#define EXP_CURRENTPOS				1
#define EXP_CURRENTPERCENT			2
#define EXP_CURRENTSPEED			3
#define EXP_TOTALTIME				4
#define EXP_CURRENTURL				5
#define EXP_SIZE					6
#define	EXP_LAST                    7

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the CCA.
// You would use these with a dialog...

typedef struct tagEDATA_V1
{
	extHeader		eHeader;
//	short			swidth;
//	short			sheight;

} EDITDATA;
typedef EDITDATA _far *			LPEDATA;

// Object versions
#define	KCX_CURRENT_VERSION			1

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here. Also, some OEFLAGS require you to add
// structures to this structure.
#define MAX_TIMEPOS		50
typedef struct tagSlot
{
	LPSTR pURL;
	int slotNumber;
	HANDLE hFile;
	HINTERNET hOpenedURL;
	DWORD dwFileSize;
	DWORD dwTotalRead;
	LPBYTE pBuffer;
	BOOL bCompleted;
	DWORD dwInitialTime;
	DWORD dwSpeed;
	int bError;
	BOOL bAborted;
	BOOL bPause;
} InternetSlot;

#define MAX_SLOTS	4
#define DOWNLOAD_BUFFER_SIZE 16384

typedef struct tagRDATA
{
	headerObject	rHo;					// Header

	HINSTANCE hWininet;
	HINTERNET (FAR STDAPICALLTYPE * lpInternetOpen)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD);
	BOOL (FAR STDAPICALLTYPE * lpInternetCloseHandle)(HINTERNET);
	HINTERNET (FAR STDAPICALLTYPE * lpInternetOpenUrl)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD);
	BOOL (FAR STDAPICALLTYPE * lpInternetQueryDataAvailable)(HINTERNET, LPDWORD, DWORD, DWORD);
	BOOL (FAR STDAPICALLTYPE * lpInternetReadFile)(HINTERNET, LPVOID, DWORD, LPDWORD);
	DWORD (FAR STDAPICALLTYPE * lpInternetSetFilePointer)(HINTERNET, LONG, PVOID, DWORD, DWORD);
	BOOL (FAR STDAPICALLTYPE * lpHttpQueryInfo)(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD);
	HINTERNET hInternetSession;
	InternetSlot slots[MAX_SLOTS];

} RUNDATA;
typedef	RUNDATA	_far *			LPRDATA;

// Size when editing the object under level editor
// -----------------------------------------------
#define	MAX_EDITSIZE			sizeof(EDITDATA)

// Default flags
// -------------
#define	OEFLAGS      			0
#define	OEPREFS      			0

/* See OEFLAGS.txt for more info on these useful things! */


// If to handle message, specify the priority of the handling procedure
// 0= low, 255= very high. You should use 100 as normal.                                                
// --------------------------------------------------------------------
#define	WINDOWPROC_PRIORITY		100
