#pragma once

// Hints to the compiler that the class/function this is attached to is defined externally and imported.
// Annotating this is not required, but makes more efficient code.
#define FusionAPIImport __declspec(dllimport)

// FusionAPI; incoming or outgoing to Fusion runtime. Uses __stdcall convention.
#define FusionAPI __stdcall

// Expose this function outside the DLL with an undecorated name. Replaces the DEF file.
// Hat tip to https://stackoverflow.com/a/41910450
#define DllExportHint comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
//#define PATH_MAX MAX_PATH

// Replaces the Clang-style "__FUNCTION__ with all arguments" with MSVC-style
#define __PRETTY_FUNCTION__ __FUNCSIG__

// Preprocessor hack to turns any plain text into "plain text", with the quotes.
#define SUB_STRIFY(X) #X
#define STRIFY(X) SUB_STRIFY(X)

// If the user hasn't specified a target Windows version via _WIN32_WINNT, and is using an _xp toolset (indicated by _USING_V110_SDK71_),
// then _WIN32_WINNT will be set to Windows XP (0x0501), to target XP and above.
#if !defined(_WIN32_WINNT) && defined(_USING_V110_SDK71_)
	#define _WIN32_WINNT _WIN32_WINNT_WINXP
	#define WINVER _WIN32_WINNT_WINXP
#endif


// WIN32_LEAN_AND_MEAN excludes APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets
// That fixes WinSock v1 being included and conflicting with v2. Otherwise, we would have to include winsock2.h before windows.h.
#define WIN32_LEAN_AND_MEAN

// Use of global min()/max() macros confuses pretty much all C++ code, as it intercepts any class::max(), like std::numeric_limits<size_t>::max()
// That aside, it's bad practice.
#define NOMINMAX
#include <windows.h>

// Include std::string, stringstream, and related functions
#include <sstream>
// Include std::size_t
#include <stddef.h>

// Activate the ""s and ""sv string literals (C++17)
using namespace std::string_literals;
using namespace std::string_view_literals;

// Include TCHAR, allowing both ANSI and Unicode functions
#include <tchar.h>

// Regretfully, Fusion is old enough to have wchar_t as unsigned short, requiring a workaround when passing
// Unicode text as arguments to functions.
// As struct variables, there is no workaround needed; a pointer to X type is same size as pointer to Y type.
// For more details, read up on /Zc:wchar_t- flag
using UShortWCHAR = unsigned short;

// Define some tstring and related classes/function.
// They're not part of std, but on Windows, for Unicode indifference, this is a necessary evil;
// at least, more readable than putting them in their own namespace when they're always defined in std.
namespace std
{
	// Pulled from iosfwd
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

	// pulled from xstring
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

// Windows uses MAX_PATH, everywhere else uses PATH_MAX
// Windows does actually hard-cap to MAX_PATH chars, unless you use the Unicode path hacks (L"\\?\" prefix)
#define PATH_MAX MAX_PATH
