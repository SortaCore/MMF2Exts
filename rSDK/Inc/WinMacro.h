
	// Definitions differentes si WIN32
#ifndef _WinMacro_h
#define _WinMacro_h

// ==================
//		Win 32
// ==================
#ifdef	WIN32

	// Fonctions C
#ifndef _fmalloc
#define _fmemcpy(a, b, c)	memcpy(a, b, c)
#define	_fmemccpy(a,b,c,d)	_memccpy(a,b,c,d);
#define _fmemmove(a, b, c)	memmove(a, b, c)
#define _fmemcmp(a, b, c)	memcmp(a, b, c)
#define _fmemset(a, b, c)	memset(a, b, c)
#define _fstrcpy(a, b)		strcpy(a, b)
#define _fstrncpy(a, b, c)	strncpy(a, b, c)
#define _fstrcat(a, b)		strcat(a, b)
#define _fstrncat(a, b, c)	strncat(a, b, c)
#define _fstrcmp(a, b)		strcmp(a, b)
#define _fstricmp(a, b)		_stricmp(a, b)
#define _fstrnicmp(a, b, c)	_strnicmp(a, b, c)
#define	_fstrchr(a, b)		strchr(a, b)
#define	_fstrrchr(a, b)		strrchr(a, b)
#define	_fstrlen(a)			strlen(a)
#define	_fstrlwr(a)			_strlwr(a)
#define	_fstrstr(a, b)		strstr(a, b)
#define	_fcalloc(a, b)		calloc(a, b)
#define	_ffree(a)			free(a)
#define	_fmalloc(a)			malloc(a)
#define	_fmsize(a)			_msize(a)
#define	_frealloc(a,b)		realloc(a,b)

	// Fonctions Windows
#define	hmemcpy(d,s,l)		memcpy(d,s,l)
#endif

	// Fonctions diverses
#define	Make_Ptr32(p,o)		(p+o)
#define	Add_Ptr32(p,o)		(p+o)
#define	Offset_Ptr32(s,p)	(long)(p-s)

	// Macros
#define	FP_OFF(p)	((UINT)p)
#define	FP_SEG(p)	((UINT)p)
#define	MK_FP(s,o)	o

	// Constantes
#define	READ		OF_READ
#define	WRITE		OF_WRITE
#define	READ_WRITE	OF_READWRITE

	// WM_COMMAND parameters
#define	wmCommandID				(int)LOWORD(wParam)
#define	wmCommandHCtl			(HWND)lParam
#define	wmCommandNotif			(int)HIWORD(wParam)

	// WM_PARENTNOTIFY parameters
#define	wmParentNotifyFlags		LOWORD(wParam)
#define	wmParentNotifyChildID	HIWORD(wParam)
#define	wmParentNotifyX			LOWORD(lParam)
#define	wmParentNotifyY			HIWORD(lParam)
#define	wmParentNotifyHChild	(HWND)lParam

	// WM_HSCROLL & WM_VSCROLL parameters
#define	wmScrollCode			LOWORD(wParam)
#define	wmScrollPos				HIWORD(wParam)
#define	wmScrollHCtl			(HWND)lParam

	// WM_MENUSELECT parameters
#define	wmMenuSelectIDItem		LOWORD(wParam)
#define	wmMenuSelectFlags		HIWORD(wParam)
#define	wmMenuSelectHMenu		(HMENU)lParam

	// WM_MDIACTIVATE parameters
#define	wmMDIActivateDeact		(HWND)wParam
#define	wmMDIActivateAct		(HWND)lParam

	// WM_ACTIVATE parameters
#define	wmActivateFActive		LOWORD(wParam)
#define	wmActivateFMinimized	HIWORD(wParam)
#define	wmActivateHwnd			(HWND)lParam

#else

	// WM_COMMAND parameters
#define	wmCommandID				(int)wParam
#define	wmCommandHCtl			(HWND)LOWORD(lParam)
#define	wmCommandNotif			(int)HIWORD(lParam)

	// WM_PARENTNOTIFY parameters
#define	wmParentNotifyFlags		wParam
#define	wmParentNotifyChildID	HIWORD(lParam)
#define	wmParentNotifyX			LOWORD(lParam)
#define	wmParentNotifyY			HIWORD(lParam)
#define	wmParentNotifyHChild	(HWND)LOWORD(lParam)

	// WM_HSCROLL & WM_VSCROLL parameters
#define	wmScrollCode			wParam
#define	wmScrollPos				LOWORD(lParam)
#define	wmScrollHCtl			(HWND)HIWORD(lParam)

	// WM_MENUSELECT parameters
#define	wmMenuSelectIDItem		wParam
#define	wmMenuSelectFlags		LOWORD(lParam)
#define	wmMenuSelectHMenu		(HMENU)HIWORD(lParam)

	// WM_MDIACTIVATE parameters
#define	wmMDIActivateDeact		(HWND)HIWORD(lParam)
#define	wmMDIActivateAct		(HWND)LOWORD(lParam)

	// WM_ACTIVATE parameters
#define	wmActivateFActive		wParam
#define	wmActivateFMinimized	HIWORD(lParam)
#define	wmActivateHwnd			(HWND)LOWORD(lParam)

#endif

#endif	// _WinMacro_h
