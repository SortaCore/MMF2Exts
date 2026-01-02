#pragma once

// Hints to the compiler that the class/function this is attached to is defined externally and imported.
// Annotating this is not required, but makes more efficient code.
#define FusionAPIImport __declspec(dllimport)

// FusionAPI; incoming or outgoing to Fusion runtime. Uses __stdcall convention.
#define FusionAPI __stdcall

// Expose this function outside the DLL with an undecorated name. Replaces the DEF file.
// Hat tip to https://stackoverflow.com/a/41910450
#define DllExportHint comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

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

// Targeting XP is surprisingly poorly done; you would think you can just specify v141_xp toolset and be okay,
// but the toolset eventually uses STL library versions that don't link properly.
// Noteably, std::shared_timed_mutex is linked to a kernel32.lib, which under the hood uses SRWLock, a feature
// introduced in Vista's kernel32.dll. The kernel32.dll in the OS won't have the SRWLock functions, resulting in
// the ext not loading on Fusion startup.
// You can track these missing calls down using Dependency Walker.
#if (defined(_USING_V110_SDK71_) || (defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0600) || (defined(WINVER) && WINVER < 0x0600)) && _MSC_VER > 1929
	#error Windows XP is not properly supported after VS 2019 v16.7 (toolset 14.27)
#endif

// Windows XP does not implement GetTickCount64(), and static analysis complains
// if we use GetTickCount()
#if defined(WINVER) && WINVER < 0x600 && !defined(GetTickCount64)
	#define GetTickCount64() ((uint64_t)GetTickCount())
#endif

// WIN32_LEAN_AND_MEAN excludes APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets
// That fixes WinSock v1 being included and conflicting with v2. Otherwise, we would have to include winsock2.h before windows.h.
#ifndef WIN32_LEAN_AND_MEAN
	#error WIN32_LEAN_AND_MEAN should be defined!
#endif

// Use of global min()/max() macros confuses pretty much all C++ code, as it intercepts any class::max(),
// like std::numeric_limits<size_t>::max().
// That aside, it's bad practice, as parameters are evaluated twice, meaning side effects can happen twice,
// even though the parameter is passed visibly once.
// If you get issues with GDIPlus headers, see https://stackoverflow.com/a/4914108 for calling std::min()/max()
// as min()/max().
#ifndef NOMINMAX
	#error NOMINMAX should be defined!
#endif

// ATL requires objbase and atlbase defined before windows include
#ifdef FUSION_INCLUDE_ATL
#include <objbase.h>
#include <atlbase.h>
#endif
#include <windows.h>

// Include TCHAR, allowing both ANSI and Unicode functions
#include "Strings.hpp"

// Regretfully, Fusion is old enough to have wchar_t as unsigned short, requiring a workaround when passing
// Unicode text as arguments to functions.
// As struct variables, there is no workaround needed; a pointer to X type is same size as pointer to Y type.
// For more details, read up on /Zc:wchar_t- flag
using UShortWCHAR = unsigned short;

// Not using Unicode, so TCHAR == char, or wchar_t is unsigned short
#if !defined(_UNICODE) || !defined(_NATIVE_WCHAR_T_DEFINED)
using UShortTCHAR = TCHAR;
#else // wchar_t is not unsigned short, which is modern standard.
using UShortTCHAR = unsigned short;
#endif
// _T(X), but for UShortTCHAR. Use when passing text to Fusion runtime functions.
#define _FUSIONT(x) ((const UShortTCHAR *)_T(x))

// Windows uses MAX_PATH, everywhere else uses PATH_MAX
// Windows does actually hard-cap to MAX_PATH chars, unless you use the Unicode path hacks (L"\\?\" prefix)
#define PATH_MAX MAX_PATH

using WindowHandleType = HWND;

// We hide compiler warning 4200, caused by zero-length arrays causing perhaps
// unexpected default ctor behaviour. However, despite being a warning that claims
// these are non-standard, it is a valid ISO C99 standard; called a "flexible array member",
// for docs see C11, 6.7.21/18.
// struct { short some_var; int end_array[]; } allows struct to be malloc'd with end_array
// a runtime-determined size, rather than compile-time. So some_var is a header of sorts.
// It IS non-standard to use [0] instead of [] for these members, though GCC supports it.
#pragma warning (disable: 4200)

// Indicate that Extension does not inherit, as it changes how pointers to members are defined
class __single_inheritance Extension;

// Include Direct3D headers if requested, allowing Windows display surfaces to directly mess
// with underlying Fusion Direct3D tech.
// Note that even the old Win7.1A SDK used for WinXP+ compiling has Direct3D 9 and 11 headers.
//
// The D3D pointers are ref-counted and COM-based, so to auto-free them, it is recommended
// that you store them in a CComPtr from ATL (or MFC) variants of MSVC compiler,
// which you install with Visual Studio Installer under Additional Components, then
// #include <atlbase.h>
// Alternatively, you can free manually with XX->Release(), but you will have to carefully track
// every usage, as any successful ptr->GetXX or ptr->QueryInterface() call will increment ptr's refcount.
// Lore:
// https://www.reddit.com/r/directx/comments/gjk2w9/comment/fqmjmol/
// https://stackoverflow.com/a/7212562
// 
// For XP+ compatibility, install v14.27 ATL (or MFC) variant specifically:
//   C++ v14.27 ATL for v142 build tools (x86 & x64), or
//   C++ v14.27 MFC for v142 build tools (x86 & x64) - bigger and slower.
// For later Windows, install latest:
//   C++ ATL for latest vXX build tools (x86 & x64), or
//   C++ MFC for latest vXX build tools (x86 & x64) - bigger and slower.
// Or you can use even newer tech like WGL, ymmv.
// 
// For Direct3D 11, you can enable the debug runtime using DxCpl.exe.
// For Direct3D 9, the runtime cannot be set to debug mode in Windows 10+.
// If you were to use earlier windows, there is a directx.cpl to enable it.
// 
// DirectX display, used by MMF2 only, is even less tested in DarkEdif,
// although it's noted that crashes on exit in Run App in last MMF2 HWA beta v258.2.
#ifdef FUSION_INTERNAL_ACCESS
#ifdef _DEBUG
	// Define Direct3D debug name GUID used to set name in D3D11 and earlier.
	// Alternatively, you can link to dxguid.lib.
	#if defined(FUSION_INCLUDE_ATL) && (defined(FUSION_DIRECT3D_11_INTERNALS) || defined(FUSION_DIRECT3D_9_INTERNALS) || defined(FUSION_DIRECT3D_8_INTERNALS))
	#include <initguid.h>
	#endif
	// Enables Direct3D debug info; see DarkEdif help file.
	#define D3D_DEBUG_INFO
#endif
#include <dxgi.h>

#ifdef FUSION_DIRECT3D_9_INTERNALS

// Fusion does not implement 9Ex, even in CF2.5+ on Direct3D 9 mode.
// However, Win SDK v7.1A headers have a borked 9Ex exclude guard, as the D3D_DISABLE_9EX ends
// after IDirect3DSwapChain, instead of after IDirect3DCryptoSession (last class).
// So D3D9Ex classes are still included and get upset cos they are dependent on types that
// rightly aren't defined.
// So we do this workaround to fake the 9Ex classes.
#if _USING_V110_SDK71_
#include <d3d9types.h>
#define D3D_DISABLE_9EX
extern "C"
{
	struct __declspec(uuid("FF24BEEE-DA21-4beb-98B5-D2F899F98AF9")) IDirect3DAuthenticatedChannel9;
	struct __declspec(uuid("FA0AB799-7A9C-48CA-8C5B-237E71A54434")) IDirect3DCryptoSession9;
}
#else
#define D3D_DISABLE_9EX
#endif
#include <d3d9.h>
#include <D3DCommon.h> // includes Debug Name GUID etc
#endif
#ifdef FUSION_DIRECT3D_11_INTERNALS
// Direct3D 11 headers also pull in Direct3D 10, which we don't use in Fusion,
// so we hit the D3D 10 header guards so MSVC thinks they're already defined.
// Less stuff polluting the namespace.
#define __d3d10_h__
#define __d3d10_1_h__
#define __D3D10SHADER_H__
#define __D3D10_1SHADER_H__
#define __D3D10MISC_H__
#define __D3D10EFFECT_H__
#include <d3d11.h>
#endif
#ifdef FUSION_DIRECT3D_8_INTERNALS
// On the off chance you want to use Direct3D 8 internals,
// a good start is https://github.com/edgeforce/directx8
// These have not been fleshed out in DarkEdif.
#include <d3d8.h>
#endif
#endif
