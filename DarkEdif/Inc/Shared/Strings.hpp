#pragma once
// Includes TCHAR, allowing both ANSI and Unicode functions.
// On non-Windows, defines _tcsXXX macros as their ANSI equivalents, or UTF-8 where possible.
// If the POSIX equivalent returns differently to the Windows safe versions, then the new define is
// marked as unusuable.
// On Android, the native Java string type is UTF-16LE.
// On iOS, the native NSString can be made from any internal encoding.

// Windows has TCHAR natively
#if _WIN32
	#include <tchar.h>

// Other platforms don't, and shouldn't be defined as using Unicode as they use it implicitly.
#elif defined(_UNICODE)
	// You shouldn't be using Unicode during Android/iOS builds, even for TCHAR.
	// Unicode UTF-8 is used by default for text (without _UNICODE), so defining _UNICODE has no point.
	// The wchar_t type is UTF-32 on both Android and iOS, and so is rarely used.
	// TCHAR is automatically defined to UTF-8, a.k.a char, so for Unicode/non-Unicode Windows being ported, Android will work ok too.
	#error _UNICODE should not be defined
#else // _UNICODE not defined, on non-Windows, as expected
	// String literal wrapper for Unicode compatiblity
	#define _T(x) x

	// Variable types for Unicode compatibility
	#define TCHAR char
	#define LPTSTR TCHAR *
	#define LPCTSTR const TCHAR *

	// TCHAR to char function equivalents
	#define _tcsdup(a) strdup(a)
	#define _tcslen(a) strlen(a)
	#define _tcsnlen(a,b) strnlen(a,b)
	#define _tcscpy(a,b) strcpy(a,b)
	#define _tcscat(a,b) strcat(a,b)
	#define _tcscmp(a,b) strcmp(a,b)
	#define _tcschr(a,b) strchr(a,b)
	#define _tcsrchr(a,b) strrchr(a,b)
	#define _totlower tolower
	#define _totupper toupper
	#define _ttof(f) atof(f)
	#define _ttoi(i) atoi(i)
	#define _tcserror(a) strerror(a)
	#define _tcsftime(a,b,c,d) strftime(a,b,c,d)

	#define _tcscpy_s(a,b,c) strcpy(a,c)

	#define MessageBox MessageBoxA

	#define _tcsicmp(a,b) strcasecmp(a,b)
	#define _tcsnicmp(a,b,c) strncasecmp(a,b,c)

	// Less safe TCHAR equivalents

	// Expects you to specify the 2nd parameter as size (std::size is preferred)
	#define _vstprintf_s(a,b,c,d) vsprintf(a,c,d)

	// Expects you to specify the 2nd parameter as size (std::size is preferred)
	#define _vsntprintf_s(a,b,c,d,e) vsnprintf(a,(c == (size_t)-1 ? b : std::min(b,c)),d,e)

	// Expects you to specify the 2nd parameter as size (std::size is preferred)
	#define _stprintf_s(a,b,c,...) sprintf(a, c, __VA_ARGS__)

	// Expects you to specify the 2nd parameter as size (std::size is preferred)
	#define _tcscpy_s(a,b,c) strcpy(a,c)

#endif // non-Windows


// Define some tstring and related classes/function.
// They're not part of std, but on Windows, for Unicode indifference, this is a necessary evil;
// at least, more readable than putting them in their own namespace when they're always defined in std.
namespace std
{
	// Pulled from iosfwd header
	using tios = basic_ios<TCHAR, char_traits<TCHAR>>;
	using tstreambuf = basic_streambuf<TCHAR, char_traits<TCHAR>>;
	using tistream = basic_istream<TCHAR, char_traits<TCHAR>>;
	using tostream = basic_ostream<TCHAR, char_traits<TCHAR>>;
	using tiostream = basic_iostream<TCHAR, char_traits<TCHAR>>;
	using tstringbuf = basic_stringbuf<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tistringstream = basic_istringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tostringstream = basic_ostringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tstringstream = basic_stringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tfilebuf = basic_filebuf<TCHAR, char_traits<TCHAR>>;
	using tifstream = basic_ifstream<TCHAR, char_traits<TCHAR>>;
	using tofstream = basic_ofstream<TCHAR, char_traits<TCHAR>>;
	using tfstream = basic_fstream<TCHAR, char_traits<TCHAR>>;
	#if _HAS_CXX20
		using tsyncbuf = basic_syncbuf<TCHAR>;
		using tosyncstream = basic_osyncstream<TCHAR>;
	#endif // _HAS_CXX20

	// pulled from xstring header
	using tstring = basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tstring_view = basic_string_view<TCHAR>;

	#ifndef _UNICODE
		template<typename... Args>
		inline auto to_tstring(Args &&... args) -> decltype(std::to_string(std::forward<Args>(args)...)) {
			return std::to_string(std::forward<Args>(args)...);
		}
	#else
		template<typename... Args>
		inline auto to_tstring(Args &&... args) -> decltype(std::to_wstring(std::forward<Args>(args)...)) {
			return std::to_wstring(std::forward<Args>(args)...);
		}
	#endif
}
