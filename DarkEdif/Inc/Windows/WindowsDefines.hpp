#pragma once

#ifndef __INTELLISENSE__
// Hints to the compiler that the class/function this is attached to is defined externally and imported.
// Annotating this is not required, but makes more efficient code.
#define FusionAPIImport __declspec(dllimport)
#define EXDEF /* no op */
#else
// Lie to IntelliSense that the dllimported func is defined locally, so it stops complaining about missing definition
#define FusionAPIImport /* no op */
// Make IntelliSense see the function as defined, but contents are assumed unreachable code
#define EXDEF { __assume(0); }
#endif

// EXORD indicates this is implemented by MMFS2 lib by ordinal number.
// In MMF2 base, some ANSI functions were implemented with no suffix, then in Unicode add-on implemented with A suffix.
// This creates no link-time errors, as ordinal indexes are used instead, so ANSI exts will call the suffixed function.
// However, if you want compatibility, it's best to call by ordinal number, e.g. GetProcAddress(_T("mmfs2.dll"), MAKEINTRESOURCEA(ord)).
// For this case, EXORD2 provides MMF1 and MMF2 ordinal indexes.
// EXORD provides MMF2 ordinal index.
// EXORDs are also provided for functions that are present in all MMF2 versions,
// for the crazies who want to try creating a MMF1+ extension.
//
// When ordinals are limited to requiring HWA, Unicode, CF2.5 or a specific build,
// then XXX_REQUIRED(ord) is used instead of EXORD, e.g. MMF2_UNICODE_OR_CF25_REQUIRED(123)
// means if you build for MMF2 Unicode/CF2.5, it's safe as-is.
// MMF2 base will not find that ordinal and have a link time error, so the editor won't show the object in Create New Object window.
// If you wanted both - fallback behaviours - then you target the base one, and you will get warnings for using the function directly,
// with an ordinal number to call instead.
#define MMF2_ORD(mmf2) /* no op */
#define MMF_ORD(mmf1, mmf2) /* no op */

// MMF2 CD Release was build 239.0f, and had ordinals 1-825.
// MMF2 CD -> latest beta build 258.2 adds ordinals 826-830, ordinals related to sound channels:
// GetSndChannelFreq, LockSndChannel, SetSndChannelFreq, SetSndFreq, GetSndFreq
//
// In MMF2 HWA, during standard display mode, angles are ints; during HWA mode they are floats;
// whereas in CF2.5, they're always floats even in Standard.
// HWA and Unicode are separate in MMF2.
// MMF2 HWA DLC adds ordinals 831-1018.
// MMF2 Unicode DLC adds 1019-1082, and the HWA range 831-1018 is not available in Unicode.
// A lot of ANSI text-based functions are renamed to have an A suffix and a W counterpart,
// but the LIB always uses ordinal index, so this doesn't matter.
// Vitalize! 4 is basically MMF2 Unicode, as its dll defines and skips the same ordinal ranges.
// 
// * CF2.5 has HWA and Unicode always available.
// CF2.5 starts with build 280.27 (Dec 2013), containing ordinals 1-1096.
// CF2.5 build 292.2 (Apr 2019) adds 1097-1138 ordinals: fxFill added to cSurface, cSurfaceImplementation and CFillData;
// MappedFont functions, sprite functions, zip and unzip,
// cSurface premultiplied alpha, cSurfaceImplementation parts and fxFill,
// CImageFilterMgr get/set autopremultiplied, CEffectEx_ExInitialize.
// CF2.5 build 292.27 (Oct 2020) adds ords 1139-1153:
// CFile, CInputFile, and one function to cSurface.

// This section gates functions, asking the ext dev to upgrade to a better suited func when using older ones,
// and warning when using too-new ones.
// For example, using 32-bit file size functions when 64-bit are available, non-HWA drawing when HWA is available,
// ANSI when Unicode is available.
// It also creates compiler warning when using a function introduced too late for the target version.
// Note some function declarations are hidden by preprocessor gates anyway.

#ifndef FUSION_TARGET_BUILD
#if defined(_UNICODE) && defined(HWABETA)
// Latest CF2.5
#define FUSION_TARGET_BUILD 292026
#else
// Last MMF2 beta
#define FUSION_TARGET_BUILD 258002
#endif
#endif
// Sanity check
#if FUSION_TARGET_BUILD > 280027 && (!defined(HWABETA) || (!defined(_UNICODE) && !defined(ALLOW_ANSI_EXT_IN_UNICODE_RUNTIME)))
#error Incorrect Fusion target build or project configuration; if targeting CF2.5, _UNICODE and HWABETA defines are expected.
#endif

#define FUSION_FUNC_NOT_AVAILABLE(ord, x) [[deprecated("The function is not available in your target build, requires " x ". Use runtime linking to ordinal " ord " instead.")]]
#ifndef DARKEDIF_INTERNAL_INCLUDE
#define FusionAPISwitch(ord, func, build) [[deprecated("This function is better suited by " #func ", which is " #build ". Ordinal " #ord ".")]]
#else
#define FusionAPISwitch(ord, func, build) /* no op */
#endif

// Target ANSI: best we can use here is MMF2 base or MMF2 HWA
#ifndef _UNICODE
	#define MMF2_UNICODE_OR_CF25_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "MMF2 Unicode or CF2.5")
	#define CF25_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5")
	#define CF25_292_02_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.2 or later")
	#define CF25_292_27_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.27 or later")
	#define PreferFusionFunc_Unicode(ord, func) /* Replacement func requires Unicode */
	#define PreferFusionFunc_CF25(ord, func) /* Replacement func requires CF2.5 */
	#define PreferFusionFunc_CF25_296_02(ord, func) /* Replacement func requires CF2.5 292.2 */
	#define PreferFusionFunc_CF25_296_27(ord, func) /* Replacement func requires CF2.5 292.27 */
	// MMF2 HWA, no Unicode
	#ifdef HWABETA
		#define MMF2_HWA_OR_CF25_REQUIRED(ord) /* OK */
		#define PreferFusionFunc_HWA(ord, func) FusionAPISwitch(ord, func, "MMF2 HWA and CF2.5")
	#else // Base MMF2
		#define MMF2_HWA_OR_CF25_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "MMF2 HWA or CF2.5")
		#define PreferFusionFunc_HWA(ord, func) /* Replacement func requires HWA */
	#endif
#else // Unicode defined
	#define MMF2_UNICODE_OR_CF25_REQUIRED(ord) /* OK */
	#define PreferFusionFunc_Unicode(ord, func) FusionAPISwitch(ord, func, "MMF2 Unicode and CF2.5")
	#ifndef HWABETA  // Unicode, no HWA: Unicode MMF2
		#define MMF2_HWA_OR_CF25_REQUIRED(ord)  FUSION_FUNC_NOT_AVAILABLE(#ord, "MMF2 HWA or CF2.5")
		#define CF25_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5")
		#define CF25_292_02_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.2 or later")
		#define CF25_292_27_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.27 or later")
	#else // HWA + Unicode: MMF2 is now incompatible, CF2.5 required
		#define MMF2_HWA_OR_CF25_REQUIRED(ord) /* OK */
		#define CF25_REQUIRED(ord) /* OK */
		#define PreferFusionFunc_HWA(ord, func) FusionAPISwitch(ord, func, "MMF2 HWA and CF2.5")
		#define PreferFusionFunc_CF25(ord, func) FusionAPISwitch(ord, func, "CF2.5")
		#if FUSION_TARGET_BUILD >= 296027
			#define CF25_292_27_REQUIRED(ord) /* OK */
			#define CF25_292_02_REQUIRED(ord) /* OK */
			#define PreferFusionFunc_CF25_296_02(ord, func) FusionAPISwitch(ord, func, "CF2.5 build 292.2+")
			#define PreferFusionFunc_CF25_296_27(ord, func) FusionAPISwitch(ord, func, "CF2.5 build 292.27+")
		#else
			#define CF25_292_27_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.27 or later")
			#define PreferFusionFunc_CF25_296_27(ord, func) /* Replacement func requires CF2.5 292.27+ */
			#if FUSION_TARGET_BUILD >= 296002
				#define CF25_292_02_REQUIRED(ord) /* OK */
				#define PreferFusionFunc_CF25_296_02(ord, func) FusionAPISwitch(ord, func, "CF2.5 build 292.2+")
			#else // Fusion target build is after MMF2, prior to CF 296.2, assume CF2.5 initial release
				#define CF25_292_02_REQUIRED(ord) FUSION_FUNC_NOT_AVAILABLE(#ord, "CF2.5 build 292.2 or later")
				#define PreferFusionFunc_CF25_296_02(ord, func) /* Replacement func requires CF2.5 292.2+ */
			#endif
		#endif
	#endif
#endif

// Windows 64-bit exporter is surely soon(tm)
#if !defined(FusionSDKWin64Compat) && defined(_WIN64)
#define FusionSDKWin64Compat
#endif
#ifdef FusionSDKWin64Compat
// Requires the use of 64-bit replacement function
#define Fusion64APISwitch(repl) __declspec(deprecated, repl)
#else
// When enabled, requires the use of 64-bit replacement function
#define Fusion64APISwitch(repl) /* no op */
#endif

// Fusion runtime doesn't host two variants of its internals for ANSI and Unicode extensions;
// a string pointer is based on what runtime is. Unicode runtime = Unicode strings.
// If you have an ANSI extension loaded by Unicode runtime, it won't correctly read the Wide
// Fusion internal strings, and the compiler will not suggest why.
// Most of the time this will result in a single character being read.
//
// You can test for Unicode runtime using mvIsUnicodeVersion(), and LOSSILY convert with
// DarkEdif::WideToANSI(), but it will not be able to convert all characters.
// DarkEdif::WideToUTF8() is better, but most char-functions on Windows are ANSI, not UTF8.
// If you don't understand this error: build Unicode or don't read Fusion internals.
#if defined(DARKEDIF_INTERNAL_INCLUDE) || defined(_UNICODE) || \
		!defined(ALLOW_ANSI_EXT_IN_UNICODE_RUNTIME) || defined(DARKEDIF_SUPPRESS_ANSI_INTERNALS_WARNING)
	#define FusionANSIWarning /* no op */
#else // non-Unicode and warning alive
	#define FusionANSIWarning [[deprecated("This string is not ANSI. It may be wide if the Fusion runtime is Unicode (CF2.5 or MMF2 + Unicode add-on).")]]
#endif

// FusionAPI; incoming or outgoing to Fusion runtime. Uses __stdcall convention.
#define FusionAPI __stdcall

// Expose this function outside the DLL with an undecorated name. Replaces the DEF file.
// Hat tip to https://stackoverflow.com/a/41910450
#define DllExportHint comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

// Replaces the Clang-style "__FUNCTION__ with all arguments" with MSVC-style
#define __PRETTY_FUNCTION__ __FUNCSIG__

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

template<auto MemFn> struct OrdinalCall;

template<typename R, typename C, typename... Args, R(__thiscall C::* MemFn)(Args...)>
struct OrdinalCall<MemFn>
{
	using Fn = R(__thiscall*)(C*, Args...);

	static R Call(C* self, uint16_t ordinal, Args... args)
	{
		auto fn = reinterpret_cast<Fn>(
			GetProcAddress(
				GetModuleHandle(TEXT("mmfs2.dll")),
				MAKEINTRESOURCEA(ordinal)));
		assert(fn);
		return fn(self, std::forward<Args>(args)...);
	}
};

#define WRAP_METHOD(origClassName, returnType, funcName, ordinalNum, paramTypesAndNames, paramTypes, ...) \
inline returnType funcName paramTypesAndNames \
{ \
	return (returnType)OrdinalCall<static_cast<returnType (origClassName::*) paramTypes>(&origClassName::funcName)>::Call( \
		reinterpret_cast<origClassName*>(this), ordinalNum, __VA_ARGS__); \
}


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
#ifdef FUSION_DDRAW_INTERNALS
// The IIDs aren't correctly defined for some reason
#if defined(FUSION_INCLUDE_ATL) && !defined(INITGUID)
#include <initguid.h>
#endif // GUID defines

// Fusion invokes DirectDraw (DirectX) v1-2. The backend can be v7, but Fusion doesn't directly use the advanced features.
// In theory, there's not much performance hit as what Fusion requests doesn't implicitly require slower methods.
// You should consider DirectDraw only partially implemented on Fusion's part.
// Note that no created surface will have DDraw features from tests. Only the frame surface itself.
// DirectX / DirectX + VRAM modes were later removed in CF2.5.
#include <ddraw.h>
#endif // FUSION_DDRAW_INTERNALS

#endif
