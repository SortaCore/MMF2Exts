class rRundata;
typedef rRundata * LPRRDATA;

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

struct InternetSlot
{
	string pURL;
	HANDLE hFile;
	HINTERNET hOpenedURL;
	unsigned long long FileSize, TotalRead;
	LPBYTE *pBuffer;
	unsigned int InitialTime, Speed;
	bool bCompleted, bError, bAborted, bPause;
	InternetSlot()
	{
		hFile = NULL;
		FileSize = 0ULL;
		TotalRead = 0ULL;
		pBuffer = NULL;
		InitialTime = 0;
		Speed = 0;
		bCompleted = false;
		bError = false;
		bAborted = false;
		bPause = false;
	}

};

typedef struct tagRDATA
{
	#include "MagicRDATA.h"
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
typedef	RUNDATA	* LPRDATA;

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the MFA
// You should use these with properties

typedef struct tagEDATA_V1
{
	extHeader		eHeader;
//	short			swidth;
//	short			sheight;

} EDITDATA;
typedef EDITDATA * LPEDATA;
