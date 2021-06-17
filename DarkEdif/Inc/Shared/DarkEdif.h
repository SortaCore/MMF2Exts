#pragma once
#include "Edif.h"
#ifdef _WIN32
#include "Resource.h"
#else
// These figures don't matter in Android/iOS anyway, a different resources format is used
#define IDR_EDIF_ICON 101
#define IDR_EDIF_JSON 102
#endif

#include <algorithm>
#include <sstream>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <atomic>


// Stops Visual Studio complaining it cannot generate copy functions because of the 0-sized array
#pragma warning (disable:4200)

// Struct contains information about a/c/e
struct ACEInfo {
	union BothParams {
		// Actions, conditions
		Params p;
		// Expressions
		ExpParams ep;
	};
	// Original type of BothParams is short, so this must be too.
	static_assert(sizeof(BothParams) == 2U, "Union failed to be a good size.");

	union BothFlags {
		// Conditions: indicates whether condition is negatable
		EVFLAGS ev;
		// Expressions: indicates return type
		ExpReturnType ef;
	};
	// Original type of BothFlags is short.
	static_assert(sizeof(BothFlags) == 2U, "Union failed to be a good size.");


	short		FloatFlags,	 // Float flags (if bit n is true, parameter n returns a float)
				ID;			 // ID, non-Edifified; used to be called 'code'
	BothFlags	Flags;		 // For conds: EVFLAGS_ALWAYS, etc; whether condition is negatable
							 // For exps: EXPFLAG_DOUBLE, etc; return type
							 // For acts: unused, always 0
	short		NumOfParams; // Number of parameters this A/C/E is called with (Parameters[n])
	BothParams	Parameter[]; // Parameter information (max sizeof(FloatFlags)*8 params)

	ACEInfo(short _ID, BothFlags _Flags, short _NumOfParams) :
			FloatFlags(0), ID(_ID),
			Flags(_Flags), NumOfParams(_NumOfParams)
	{
		// No code needed. Leave Parameters[n] construction to parent struct.
	}

	// Simple function to return the part of the struct that is expected by MMF2
	inline void * MMFPtr (void) { return &ID; }
};

bool CreateNewActionInfo();
bool CreateNewConditionInfo();
bool CreateNewExpressionInfo();

#ifndef NOPROPS
char * PropIndex(EDITDATA * edPtr, unsigned int ID, unsigned int * size);

#if EditorBuild
void InitialisePropertiesFromJSON(mv *, EDITDATA *);
Prop * GetProperty(EDITDATA *, size_t);
void PropChangeChkbox(EDITDATA * edPtr, unsigned int PropID, const bool newValue);
void PropChange(mv * mV, EDITDATA * &edPtr, unsigned int PropID, const void * newData, size_t newSize);
#endif // EditorBuild
#endif // NOPROPS

std::tstring ANSIToTString(const std::string_view);
std::string ANSIToUTF8(const std::string_view);
std::wstring ANSIToWide(const std::string_view);
std::string UTF8ToANSI(const std::string_view, bool * const allValidChars = nullptr);
std::tstring UTF8ToTString(const std::string_view, bool * const allValidChars = nullptr);
std::wstring UTF8ToWide(const std::string_view);
std::string WideToANSI(const std::wstring_view, bool * const allValidChars = nullptr);
std::tstring WideToTString(const std::wstring_view, bool * const allCharsValid = nullptr);
std::string WideToUTF8(const std::wstring_view);
std::string TStringToANSI(const std::tstring_view, bool * const allValidChars = nullptr);
std::string TStringToUTF8(const std::tstring_view);
std::wstring TStringToWide(const std::tstring_view);

namespace DarkEdif {

	// SDK version and changes are documented in repo/DarkEdif/#MFAs and documentation/DarkEdif changelog.md
	static const int SDKVersion = 10;
#if EditorBuild

	/// <summary> Gets DarkEdif.ini setting. Returns empty if file missing or key not in file.
	///			  Will generate a languages file if absent. </summary>
	std::string GetIniSetting(const char * key);

	namespace SDKUpdater
	{
		/// <summary> Starts an update check in async. Will ignore second runs. </summary>
		void StartUpdateCheck();

		enum class ExtUpdateType
		{
			// Ext update check is in progress in the background
			CheckInProgress,
			// Ext update check is disabled
			CheckDisabled,
			// Check returned no update
			None,
			// Some error occurred in connection or parsing
			ConnectionError,
			// The updater page reported an error for the ext dev (usually only returned in Debug/Debug Unicode builds)
			ExtDevError,
			// Check returned an DarkEdif SDK update (usually only returned in Debug/Debug Unicode builds)
			SDKUpdate,
			// Minor ext update, will change ext icon but not cause message box
			Minor,
			// Major ext update, will change ext icon, and cause message box once per Fusion session
			Major
		};
		/// <summary> Checks if update is needed. Returns type if so. Optionally returns update log. </summary>
		ExtUpdateType ReadUpdateStatus(std::string * logData);

		/// <summary> Updates ::SDK->Icon to draw on it; optionally displays a message box. </summary>
		void RunUpdateNotifs(mv * mV, EDITDATA * edPtr);
	}

#endif // EditorBuild

	struct FusionDebuggerAdmin;
	struct FusionDebugger
	{
		friend FusionDebuggerAdmin;
#if EditorBuild
		class DebugItem {
			friend FusionDebugger;
		protected:
			bool DoesUserSuppliedNameMatch(const char *const testAgainst) const {
				return userSuppliedName &&
					(userSuppliedName == testAgainst || !strcmp(userSuppliedName, testAgainst));
			}
			const bool isInt;
			int cachedInt;
			std::tstring cachedText;

			size_t refreshMS;
			size_t nextRefreshTime;
			const char *userSuppliedName;

			int (*intReadFromExt)(Extension *const ext);
			bool (*intStoreDataToExt)(Extension *const ext, int newValue);
			void (*textReadFromExt)(Extension *const ext, std::tstring &writeTo);
			bool (*textStoreDataToExt)(Extension *const ext, std::tstring &newValue);

			DebugItem(decltype(intReadFromExt) reader, decltype(intStoreDataToExt) editor,
				size_t refreshMS, const char *userSuppliedName) :
				isInt(true), refreshMS(refreshMS), userSuppliedName(userSuppliedName),
				cachedInt(-1), intReadFromExt(reader), intStoreDataToExt(editor),
				textReadFromExt(NULL), textStoreDataToExt(NULL)
			{
				cachedText.resize(30);
				_itot_s(cachedInt, cachedText.data(), cachedText.size(), 10);
				nextRefreshTime = refreshMS ? GetTickCount() + refreshMS : -1;
			}
			DebugItem(decltype(textReadFromExt) reader, decltype(textStoreDataToExt) editor,
				size_t refreshMS, const char *userSuppliedName) :
				isInt(false), refreshMS(refreshMS), userSuppliedName(userSuppliedName),
				cachedInt(-1), intReadFromExt(NULL), intStoreDataToExt(NULL),
				textReadFromExt(reader), textStoreDataToExt(editor)
			{
				cachedText.reserve(256);
				nextRefreshTime = refreshMS ? GetTickCount() + refreshMS : -1;
			}

			// Run when user has finished editing.
			void EditDone(Extension *ext, const TCHAR *newText, size_t newTextSize) {
				if (isInt ? (bool)intStoreDataToExt : (bool)textStoreDataToExt)
					throw std::exception("Not an editable property.");
				cachedText = std::tstring_view(newText, newTextSize);
			}
		};

	protected:
		class DebugItem;
		Extension *const ext;
		std::vector<DebugItem> debugItems;
		std::vector<std::uint16_t> debugItemIDs;
		void StartEditForItemID(int debugItemID);
		std::uint16_t * GetDebugTree();
		void GetDebugItemFromCacheOrExt(TCHAR *writeTo, int debugItemID);
#endif // EditorBuild

	public:

		/// <summary> Adds textual property to Fusion debugger display. </summary>
		/// <param name="getLatestFromExt"> Pointer to function to read the current text from your code. Null if it never changes. </param>
		/// <param name="saveUserInputToExt"> Pointer to function to run if user submits a new value via Fusion debugger.
		///									  Null if you want it uneditable. Return true if edit was accepted by your ext. </param>
		/// <param name="initialText"> Initial text to have in this item. Null not allowed. </param>
		/// <param name="refreshMS"> Milliseconds before reader() should be called again to update the cached text. </param>
		void AddItemToDebugger(
			// Supply NULL if it will not ever change.
			void (*getLatestFromExt)(Extension *const ext, std::tstring &writeTo),
			// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not accepted.
			bool (*saveUserInputToExt)(Extension *const ext, std::tstring &newValue),
			// Supply 0 if no caching should be used, otherwise will re-call reader().
			size_t refreshMS,
			// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
			const char *userSuppliedName
		);

		/// <summary> Adds integer property to Fusion debugger display. </summary>
		/// <param name="getLatestFromExt"> Pointer to function to read the current value from your code. Null if it never changes. </param>
		/// <param name="saveUserInputToExt"> Pointer to function to run if user submits a new value via Fusion debugger.
		///									  Null if you want it uneditable. Return true if edit was accepted by your ext. </param>
		/// <param name="initialInteger"> Initial number to have in this item. </param>
		/// <param name="refreshMS"> Milliseconds before reader() should be called again to update the cached integer. </param>
		void AddItemToDebugger(
			// Supply NULL if it will not ever change.
			int (*getLatestFromExt)(Extension *const ext),
			// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not
			bool (*saveUserInputToExt)(Extension *const ext, int newValue),
			// Supply 0 if no caching should be used, otherwise will re-call reader() every time Fusion requests.
			size_t refreshMS,
			// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
			const char *userSuppliedName
		);

		/// <summary> Updates the debug item with the given name from the Fusion debugger. </summary>
		void UpdateItemInDebugger(const char *userSuppliedName, const TCHAR *newText);
		void UpdateItemInDebugger(const char *userSuppliedName, int newValue);

		FusionDebugger(Extension *const ext);
		//~FusionDebugger() = delete;
		FusionDebugger(FusionDebugger &&) = delete;
	};


	// True if Fusion 2.5. False if Fusion 2.0. Set during SDK ctor.
	extern bool IsFusion25;
	// Returns the Fusion event number for this group. Works in CF2.5 and MMF2.0
	std::uint16_t GetEventNumber(eventGroup *);
	// Returns the Fusion event number the ext is executing. Works in CF2.5 and MMF2.0
	int GetCurrentFusionEventNum(const Extension * const ext);

	// allows the compiler to check printf format matches parameters
#ifdef _MSC_VER
#define PrintFHintInside _In_z_ _Printf_format_string_
#define PrintFHintAfter(formatParamIndex,dotsParamIndex) /* no op */
#elif defined(__clang__)
#define PrintFHintInside /* no op */
// Where formatParamIndex is 1-based index of the format param, and dots is the 1-based index of ...
// Note class member functions should include the "this" pointer in the indexing.
// You can use 0 for dotsParamIndex for vprintf-like format instead.
#define PrintFHintAfter(formatParamIndex,dotsParamIndex) __printflike(formatParamIndex, dotsParamIndex)
#else
#define PrintFHintInside /* no op */
#define PrintFHintAfter(formatParamIndex,dotsParamIndex) /* no op */
#endif

	// =====
	// This region does message boxes
	// =====

	// Extension name; ANSI/Wide on Windows, UTF-8 elsewhere.
	extern std::tstring ExtensionName;
	extern DWORD MainThreadID;
	extern HWND Internal_WindowHandle;

	void BreakIfDebuggerAttached();

	namespace MsgBox
	{
		void WarningOK(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNo(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNoCancel(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Error(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Info(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
	}
}


// This region does type checking on LinkXXX functions.
// Since the JSON should never change between Debug versus Edittime/Runtime versions,
// we'll only do the slow check in Debug Mode.

#include <algorithm>
#include <sstream>

#if EditorBuild && defined(_DEBUG)

// Uses code from https://stackoverflow.com/a/21272048 (bottom-most section)
// and from https://stackoverflow.com/a/20170989 (pre the C++17 update section)

#include <tuple>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <cstdio>

// Check VS version compatibility
// DarkEdif SDK requires Visual Studio; version 2017 or greater.
#if _MSC_VER < 1900
#define CONSTEXPR11_TN
#define CONSTEXPR14_TN
#define NOEXCEPT_TN
#elif _MSC_VER < 2000
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN
#define NOEXCEPT_TN noexcept
#else
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN constexpr
#define NOEXCEPT_TN noexcept
#endif

class static_string
{
	const char* const p_;
	const std::size_t sz_;

public:
	typedef const char* const_iterator;

	template <std::size_t N>
	CONSTEXPR11_TN static_string(const char(&a)[N]) NOEXCEPT_TN
		: p_(a)
		, sz_(N - 1)
	{}

	CONSTEXPR11_TN static_string(const char* p, std::size_t N) NOEXCEPT_TN
		: p_(p)
		, sz_(N)
	{}

	CONSTEXPR11_TN const char* data() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN std::size_t size() const NOEXCEPT_TN { return sz_; }

	CONSTEXPR11_TN const_iterator begin() const NOEXCEPT_TN { return p_; }
	CONSTEXPR11_TN const_iterator end()	const NOEXCEPT_TN { return p_ + sz_; }

	CONSTEXPR11_TN char operator[](std::size_t n) const
	{
		return n < sz_ ? p_[n] : throw std::out_of_range("static_string");
	}
};

inline std::ostream& operator<<(std::ostream& os, static_string const& s)
{
	return os.write(s.data(), s.size());
}

template <class T>
CONSTEXPR14_TN static_string type_name()
{
	static_string p = __FUNCSIG__;
	return static_string(p.data() + 38, p.size() - 38 - 7);
}

// static_string to std::string_view
#define typestr(type) std::string(type_name<type>().begin(), type_name<type>().end())

template<int acParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(acParamIndexPlusOne > 0), void>::type
forLoopAC(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const, Params * const condRetType = nullptr) {
	constexpr int acParamIndex = acParamIndexPlusOne - 1U;
	do
	{
		bool isFloat = false;
		Params p = ReadParameterType(json["Parameters"][acParamIndex][0], isFloat);

		// Note that the function signature loses the top-level const-ness of parameters,
		// so we only need to check for the non-const parameter type.
		using cppType = typename std::tuple_element<acParamIndex, std::tuple<Args...>>::type;
		const std::string cppTypeAsString(typestr(cppType));
		std::string expCppType = "?"s;

		// Handles comparisons by adding error and storing in condRetType as applicable
		const auto ComparisonHandler = [&]() {
			// Actions can't use comparison types
			if (condRetType == nullptr)
			{
				str << "Action "sv << (const char *)json["Title"] << " uses a comparison JSON parameter, but comparison "sv
					<< "parameters can only be used in conditions.\r\n"sv;
				return;
			}

			// First comparison type parameter, store it
			if ((int)*condRetType == 0)
				*condRetType = p;
			else // Multiple comparison parameter types, error out
			{
				str << "Condition "sv << (const char *)json["Title"] << " has two comparison JSON parameters. Only one "sv
					<< "comparison parameter can be used.\r\n"sv;
			}
		};

		if (isFloat)
		{
			if (std::is_same<cppType, float>())
				continue;
			expCppType = "float"sv;
		}
		else if (p == Params::String || p == Params::String_Comparison || p == Params::String_Expression ||
			p == Params::Filename || p == Params::Filename_2)
		{
			// Comparison type parameter
			if (p == Params::String_Comparison)
				ComparisonHandler();

			// The const-ness being lost by function signature applies to top-level const,
			// e.g. to "TCHAR * const" and "const TCHAR * const", NOT "const TCHAR *", hence we do two checks.
			if (std::is_same<cppType, TCHAR *>() || std::is_same<cppType, const TCHAR *>())
				continue;
			expCppType = "const TCHAR *"sv;
		}
		else if (p == Params::Integer || p == Params::Expression || p == Params::Comparison || p == Params::Compare_Time)
		{
			if (p == Params::Comparison || p == Params::Compare_Time)
				ComparisonHandler();

			if (!_stricmp(json["Parameters"][acParamIndex][0], "Unsigned Integer"))
			{
				if (std::is_same<cppType, unsigned int>())
					break;

				expCppType = "unsigned int"sv;
			}
			else
			{
				if (std::is_same<cppType, int>())
					break;
				expCppType = "int"sv;
			}
		}
		else
			continue;

		str << "Has JSON parameter "sv << (const char *)json["Parameters"][acParamIndex][1] << ", JSON type "sv << (const char *)json["Parameters"][acParamIndex][0]
			<< "; expected C++ type "sv << expCppType << ", but actual C++ type is "sv << cppTypeAsString << ".\r\n"sv;
	} while (false);

	// Recurse into next iteration
	forLoopAC<acParamIndex>(ID, json, str, Function, condRetType);
}

template<int acParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(acParamIndexPlusOne == 0), void>::type
forLoopAC(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const, Params * const condRetType = nullptr) {
	// end of loop; do nothing
}

template<int acParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(acParamIndexPlusOne > 0), void>::type
forLoopE(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const) {
	constexpr int acParamIndex = acParamIndexPlusOne - 1U;
	do
	{
		bool isFloat = false;
		ExpParams p = ReadExpressionParameterType(json["Parameters"][acParamIndex][0], isFloat);

		// Note that the function signature loses the top-level const-ness of parameters,
		// so we only need to check for the non-const parameter type.
		using cppType = typename std::tuple_element<acParamIndex, std::tuple<Args...>>::type;
		const std::string cppTypeAsString(typestr(cppType));
		std::string expCppType = "?";

		// Check parameter type matches. isFloat = true overrides the ExpParams return,
		// as both Float and Integer use the same ExpParams.
		if (isFloat)
		{
			if (std::is_same<cppType, float>())
				break;
			expCppType = "float"sv;
		}
		else if (p == ExpParams::String)
		{
			// The const-ness being lost by function signature applies to top-level const,
			// e.g. to "TCHAR * const" and "const TCHAR * const", NOT "const TCHAR *", hence we do two checks.
			if (std::is_same<cppType, const TCHAR *>() || std::is_same<cppType, TCHAR *>())
				break;
			expCppType = "const TCHAR *"sv;
		}
		else if (p == ExpParams::Integer)
		{
			if (!_stricmp(json["Parameters"][acParamIndex][0], "Unsigned Integer"))
			{
				if (std::is_same<cppType, unsigned int>())
					break;
				expCppType = "unsigned int"sv;
			}
			else
			{
				if (std::is_same<cppType, int>())
					break;
				expCppType = "int"sv;
			}
		}
		else // ?
			break;

		str << "Has JSON parameter "sv << (const char *)json["Parameters"][acParamIndex][1] << ", JSON type "sv << (const char *)json["Parameters"][acParamIndex][0]
			<< "; expected C++ type "sv << expCppType << ", but actual C++ type is "sv << cppTypeAsString << ".\r\n"sv;
	} while (false);

	// Recurse into next iteration
	forLoopE<acParamIndex>(ID, json, str, Function);
}

template<int acParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(acParamIndexPlusOne == 0), void>::type
forLoopE(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const) {
	// end of loop; do nothing
}

template<class Ret, class Struct, class... Args>
void LinkActionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Actions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << "; it has no Actions JSON item."sv;
			break;
		}
		const json_value &json = curLang["Actions"][ID];

		// const void is a thing, mainly because writing checks to prevent it makes template interpretation systems a lot harder.
		if (!std::is_same<std::remove_const_t<Ret>, void>())
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << ", "sv << (const char *)json["Title"] << "; it has return type "sv
				<< typestr(Ret) << " instead of void in the C++ function definition."sv;
			break;
		}

		const int cppParamCount = sizeof...(Args);// Also, don't set NumAutoProps to negative.

		int jsonParamCount = json["Parameters"].type == json_none ? 0 : json["Parameters"].u.array.length;

		// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
		// CNC_XX Expression macros to get parameters themselves.
		// This lets users decide at runtime whether a parameter is float or integer, which in practice seems the only use.
		// These initially-unread parameters are not passed to the C++ function and so won't be in the C++ definition.
		const json_value &numAutoProps = json["NumAutoProps"];
		if (numAutoProps.type == json_integer)
			jsonParamCount = (int)numAutoProps.u.integer;

		if (cppParamCount != jsonParamCount)
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << ", "sv << (const char *)json["Title"] << "; it has "sv
				<< jsonParamCount << " parameters in the Actions JSON item, but "sv << cppParamCount << " parameters in the C++ function definition."sv;
		}
		else if (jsonParamCount > 0)
		{
			forLoopAC<sizeof...(Args), Ret, Struct, Args...>(ID, json, errorStream, Function);

			// forLoopAC may have made multiple errors, remove final \r\n
			if (errorStream.str().size() > 0)
			{
				std::stringstream str2;
				str2 << curLangName << ": error in linking action ID "sv << ID << ", "sv << (const char *)json["Title"] << ":\r\n"sv << errorStream.str();
				std::string realError = str2.str();
				errorStream.str(realError.substr(0U, realError.size() - 2U));
			}
		}
	}

	// Exit with error
	if (errorStream.str().size() > 0)
	{
		char extName[128];
		Edif::GetExtensionName(extName);
		strcat_s(extName, " - Linking Error");
		MessageBoxA(NULL, errorStream.str().c_str(), extName, MB_OK);
	}

	SDK->ActionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkConditionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Conditions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking condition ID "sv << ID << "; it has no Conditions JSON item.\r\n"sv;
			break;
		}
		const json_value &json = curLang["Conditions"][ID];

		const int cppParamCount = sizeof...(Args);
		const int jsonParamCount = json["Parameters"].type == json_none ? 0 : json["Parameters"].u.array.length;
		if (cppParamCount != jsonParamCount)
		{
			errorStream << curLangName << ": error in linking condition ID "sv << ID << ", "sv << (const char *)json["Title"] << "; it has "sv
				<< jsonParamCount << " parameters in the Conditions JSON item, but "sv << cppParamCount << " parameters in the C++ function definition.\r\n"sv;
		}
		else if (jsonParamCount > 0)
		{
			// Condition parameter types Params::Comparison, Params::String_Comparison, and Params::Compare_Time don't use bool return types.
			// Instead, they should return int, or text pointer, like an expression.
			// The comparison itself is done by Fusion.
			// isComparisonType will start off as 0, and be set to whatever of the comparison types forLoopAC() comes across.
			// (If it comes across multiple Params of any comparison type, it reports an error.)
			Params isComparisonReturnType = (Params)0;
			forLoopAC<sizeof...(Args), Ret, Struct, Args...>(ID, json, errorStream, Function, &isComparisonReturnType);

			// Not a comparison type, should return bool
			if ((int)isComparisonReturnType == 0)
			{
				if (!std::is_same<std::remove_const_t<Ret>, bool>())
				{
					errorStream << "Condition has return type "sv << typestr(Ret) << " instead of the expected type bool"sv
						<< " in the C++ function definition.\r\n"sv;
				}
			}
			else // A Comparison type parameter was detected by forLoopAC()
			{
				if (isComparisonReturnType == Params::String_Comparison)
				{
					if (!std::is_same<std::remove_const_t<Ret>, const TCHAR *>() && !std::is_same<std::remove_const_t<Ret>, TCHAR *>())
					{
						errorStream << "Condition has a string-comparison parameter, but has return type "sv
							<< typestr(Ret) << " instead of the expected type const TCHAR * in the C++ function definition.\r\n"sv;
					}
				}
				else
				{
					if (!std::is_same<std::remove_const_t<Ret>, int>())
					{
						errorStream << "Condition has a integer/time comparison parameter, but has return type "sv
							<< typestr(Ret) << " instead of the expected type int in the C++ function definition.\r\n"sv;
					}
				}
			}

			// forLoopAC may have made multiple errors, remove final \r\n
			if (errorStream.str().size() > 0)
			{
				std::stringstream str2;
				str2 << curLangName << ": error in linking condition ID "sv << ID << ", "sv << (const char *)json["Title"] << "\r\n"sv << errorStream.str();
				std::string realError = str2.str();
				errorStream.str(realError.substr(0U, realError.size() - 2U));
				break; // We found an error in this language
			}
		}
	}

	// Exit with error
	if (errorStream.str().size() > 0)
	{
		char extName[128];
		Edif::GetExtensionName(extName);
		strcat_s(extName, " - Linking Error");
		MessageBoxA(NULL, errorStream.str().c_str(), extName, MB_OK);
	}

	SDK->ConditionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkExpressionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Expressions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << "; it has no Expressions JSON item."sv;
			break;
		}

		std::string expCppRetType = "<unknown>"s;
		std::string cppRetType = typestr(Ret);
		bool retTypeOK = false;
		const json_value &json = curLang["Expressions"][ID];
		ExpReturnType jsonRetType = ReadExpressionReturnType(json["Returns"]);

		if (jsonRetType == ExpReturnType::Integer)
		{
			if (!_stricmp(json["Returns"], "Unsigned Integer"))
			{
				retTypeOK = std::is_same<std::remove_const_t<Ret>, unsigned int>();
				expCppRetType = "unsigned int"sv;
			}
			else
			{
				retTypeOK = std::is_same<std::remove_const_t<Ret>, int>();
				expCppRetType = "int"sv;
			}
		}
		else if (jsonRetType == ExpReturnType::Float)
		{
			retTypeOK = std::is_same<std::remove_const_t<Ret>, float>();
			expCppRetType = "float"sv;
		}
		else if (jsonRetType == ExpReturnType::String)
		{
			retTypeOK = std::is_same<std::remove_const_t<Ret>, TCHAR *>() || std::is_same<std::remove_const_t<Ret>, const TCHAR *>();
			expCppRetType = "const TCHAR *"sv;
		}
		// else failure by default

		std::string exprName = (const char *)json["Title"];
		if (exprName[exprName.size() - 1U] == '(')
			exprName.resize(exprName.size() - 1U);

		if (!retTypeOK)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << ", "sv << exprName << "; it has return type "sv
				<< (const char *)json["Returns"] << " in the JSON (C++ type "sv << expCppRetType << "), but "sv << typestr(Ret) << " in the C++ function definition."sv;
			break;
		}

		constexpr int cppParamCount = sizeof...(Args);
		int jsonParamCount = json["Parameters"].type == json_none ? 0 : json["Parameters"].u.array.length;

		// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
		// CNC_XX Expression macros to get parameters themselves.
		// This lets users decide at runtime whether a parameter is float or integer, which in practice seems the only use.
		// These initially-unread parameters are not passed to the C++ function and so won't be in the C++ definition.
		const json_value &numAutoProps = json["NumAutoProps"];
		if (numAutoProps.type == json_integer)
			jsonParamCount = (int)numAutoProps.u.integer;

		if (cppParamCount != jsonParamCount)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << ", "sv << exprName << "; it has "sv
				<< jsonParamCount << " parameters in the Expressions JSON item, but "sv << cppParamCount << " parameters in the C++ function definition."sv;
		}
		else if (jsonParamCount > 0)
		{
			forLoopE<sizeof...(Args), Ret, Struct, Args...>(ID, json, errorStream, Function);

			// remove final \r\n
			if (errorStream.str().size() > 0)
			{
				std::stringstream str2;
				str2 << curLangName << ": error in linking expression ID "sv << ID << ", "sv << exprName << ":\r\n"sv << errorStream.str();
				std::string realError = str2.str();
				errorStream.str(realError.substr(0U, realError.size() - 2U));
			}
		}
	}

	// Exit with error
	if (errorStream.str().size() > 0)
	{
		char extName[128];
		Edif::GetExtensionName(extName);
		strcat_s(extName, " - Linking Error");
		MessageBoxA(NULL, errorStream.str().c_str(), extName, MB_OK);
	}

	SDK->ExpressionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

// Combine the two:
// returnType X() const;
// returnType X();
// to resolve ambiguity complaints between Extension::* and const Extension::*

template<class Ret, class Struct, class... Args>
void LinkActionDebug(unsigned int ID, Ret(Struct::*Function)(Args...))
{
	LinkActionDebug(ID, (Ret(Struct::*)(Args...) const)Function);
}
template<class Ret, class Struct, class... Args>
void LinkConditionDebug(unsigned int ID, Ret(Struct::*Function)(Args...))
{
	LinkConditionDebug(ID, (Ret(Struct::*)(Args...) const)Function);
}
template<class Ret, class Struct, class... Args>
void LinkExpressionDebug(unsigned int ID, Ret(Struct::*Function)(Args...))
{
	LinkExpressionDebug(ID, (Ret(Struct::*)(Args...) const)Function);
}

#endif // _DEBUG
