#pragma once
#include "AllPlatformDefines.hpp"

// Different returns, there is no one-size-fits-all.
// On Windows, memcpy_s returns errno, so 0 (NO_ERROR) indicates success.
// On non-Windows, have to use memcpy, which returns void *, so non-0 (non-NULL) indicates success.
#define memcpy_s(a,b,c,d) varying_return_types_memcpy(a, c, d)

// Remove some safety
#define _strdup(a) strdup(a)

// POSIX naming
#define stricmp(a,b) strcasecmp(a,b)
#define strnicmp(a,b,c) strncasecmp(a,b,c)
#define _stricmp(a,b) strcasecmp(a,b)
#define _strnicmp(a,b,c) strncasecmp(a,b,c)

// Expects you to specify the 2nd parameter as size (std::size is preferred)
#define strcat_s(a,b,c) strcat(a,c)
// Expects you to specify the 2nd parameter as size (std::size is preferred)
#define sprintf_s(a,b, ...) sprintf(a, __VA_ARGS__)
// Expects you to specify the 2nd parameter as size (std::size is preferred)
#define strcpy_s(a,b,c) strcpy(a, c)

// Expects you to specify the 2nd parameter as size (std::size is preferred)
#define fread_s(a,b,c,d,e) fread(a,c,d,e)

// Checks CRT memory on Windows only
#define _CrtCheckMemory() /* no op */

// Internal function for reading heap memory size. Bad practice to use this!
// msize() may return the size of memory BLOCK allocated, not the requested size.
#define _msize(a) malloc_usable_size(a)

#define SUBSTRIFY(X) #X
#define STRIFY(X) #X

#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO)
// Hide in namespace for iOS, which cannot share global functions without conflict
namespace DarkEdif {
	void OutputDebugStringAInternal(const char* debugString);
}
#define OutputDebugStringA(x) DarkEdif::OutputDebugStringAInternal(x)
#else
	#define OutputDebugStringA(x) (void)0
#endif

using WindowHandleType = void*;

namespace DarkEdif {
	int MessageBoxA(WindowHandleType hwnd, const TCHAR* caption, const TCHAR* text, int iconAndButtons);

	// Translates to std::this_thread::sleep_for(), or yield(), depending on parameter.
	[[deprecated("Use std::this_thread::sleep_for() or yield()")]]
	void Sleep(unsigned int milliseconds);
}

// MessageBox button selected
#define IDOK 0
#define IDCANCEL 1

// MessageBox buttons to show
#define MB_OK 0
#define MB_YESNO 0
#define MB_YESNOCANCEL 0

// MessageBox default button is first one
#define MB_DEFBUTTON1 0

// MessageBox icon choice
#define MB_ICONERROR 1
#define MB_ICONWARNING 2
#define MB_ICONINFORMATION 3
// MessageBox must be top-most
#define MB_TOPMOST 0

#include <unistd.h> // for readlink()
#include <cmath> // for std::ceil and co
