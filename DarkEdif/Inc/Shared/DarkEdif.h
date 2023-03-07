#pragma once

// JSON can have multiple languages; we'll use the last language in Runtime builds.
#if RuntimeBuild
	#define CurLang (*Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].value)
#else
	#define CurLang (DarkEdif::JSON::LanguageJSON())
#endif

#include "Edif.h"

// Struct contains information about a/c/e
struct ACEInfo {
	// We can't copy this consistently because of the zero-sized array. Prevent creating copy/move constructors.
	NO_DEFAULT_CTORS(ACEInfo);

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

	BothParams	Parameter[]; // Parameter information (max sizeof(FloatFlags)*CHAR_BIT params)

	ACEInfo(short _ID, BothFlags _Flags, short _NumOfParams) :
			FloatFlags(0), ID(_ID),
			Flags(_Flags), NumOfParams(_NumOfParams)
	{
		// No code needed. Leave Parameters[n] construction to parent struct.
	}

	// Simple function to return the part of the struct that is expected by MMF2
	inline void * FusionPtr () { return &ID; }
};

namespace Edif {
	Params ReadActionOrConditionParameterType(const char*, bool&);
	ExpParams ReadExpressionParameterType(const char*, bool&);
	ExpReturnType ReadExpressionReturnType(const char* text);
	ACEInfo* ACEInfoAlloc(unsigned int NumParams);

	bool CreateNewActionInfo();
	bool CreateNewConditionInfo();
	bool CreateNewExpressionInfo();
	bool IS_COMPATIBLE(mv* mV);
}

namespace DarkEdif {

	// SDK version and changes are documented in repo/DarkEdif/#MFAs and documentation/DarkEdif changelog.md
	static const int SDKVersion = 17;
#if EditorBuild

	/// <summary> Gets DarkEdif.ini setting. Returns empty if file missing or key not in file.
	///			  Will generate a languages file if absent. </summary>
	std::string GetIniSetting(const std::string_view key);

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
			// Check returned an install corruption (UC tag malfunction)
			ReinstallNeeded,
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
	// Returns path as is if valid, an extracted binary file's path, or an error; error indicated by returning '>' at start of text
	// On Windows, just copies string
	std::tstring MakePathUnembeddedIfNeeded(const Extension* ext, const std::tstring_view filePath);

	// =====
	// This region does message boxes
	// =====

	// Extension name; ANSI/Wide on Windows, UTF-8 elsewhere.
	extern std::tstring ExtensionName;
	extern std::thread::id MainThreadID;
	extern WindowHandleType Internal_WindowHandle;

	enum class MFXRunMode
	{
		// Haven't calculated it yet (this should never happen in user code)
		Unset = -1,

		// Running as a built EXE; can happen for both edittime and runtime MFXs, although it is expected only runtime MFXs are used.
		BuiltEXE,

		// Editor builds can be used by all builds, including copying to Data\Runtime to be used as runtime.
		// Runtime builds lack A/C/E menus and other details, so they can only be used in built EXEs.
		#if EditorBuild
			// Running in Fusion load-up splash screen - or during a Refresh button press in Create New Object window
			SplashScreen,
			// Sitting in the Fusion frame editor
			Editor,
			// Being used by a Run Application/Frame/Project test.
			// Does not include Build and Run, because that is BuiltEXE.
			RunApplication,
		#endif
	};
	extern MFXRunMode RunMode;

#ifdef _MSC_VER
	_Enum_is_bitflag_
#endif
	enum GetRunningApplicationPathType {
		FullPath = 0,
		AppFolderOnly = 1,
		// If "compress the runtime" is used, Fusion EXEs extract a stdrt.exe file to parse the content of the MFA.
		// Specifying this flag makes the GetRunningApplicationPath return the path of stdrt.exe in temp,
		// instead of the actual exe path.
		GetSTDRTNotApp = 2,
	};

	std::tstring_view GetRunningApplicationPath(GetRunningApplicationPathType type);

	// For use with GetMFXRelativeFolder()
	enum class GetFusionFolderType
	{
		// Gets Fusion root folder
		FusionRoot,
		// Gets MFX folder, always Extensions even in Extensions\\Unicode folders
		FusionExtensions,
		// Gets MFX folder, be it Extensions\\ or Extensions\\Unicode
		MFXLocation,
	};
	// Returns a path relative to this MFX, with an appended slash.
	std::tstring_view GetMFXRelativeFolder(GetFusionFolderType type);

	// Removes the ending text if it exists, and returns true. If it doesn't exist, changes nothing and returns false.
	bool RemoveSuffixIfExists(std::tstring_view &tstr, const std::tstring_view suffix, bool caseInsensitive = true);

	// Checks if first parameter ends with second parameter, returns true if so.
	bool EndsWith(const std::tstring_view tstr, const std::tstring_view suffix, bool caseInsensitive = true);

	// Checks if path exists, and is a file
	bool FileExists(const std::tstring_view filePath);


	void BreakIfDebuggerAttached();
	void LOGFInternal(PrintFHintInside const TCHAR* fmt, ...) PrintFHintAfter(1, 2);

	namespace MsgBox
	{
		void WarningOK(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNo(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNoCancel(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Error(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Info(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int Custom(const int flags, const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(3, 4);
	}

	void Log(int logLevel, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
	void LogV(int logLevel, PrintFHintInside const TCHAR* msgFormat, va_list) PrintFHintAfter(2, 0);

	// =====
	// This region does looking up correct languages from INI.
	// Internally singleton; no need to cache the results.
	// =====
	namespace JSON
	{
		// Language JSON (equivalent to SDK->json[language name])
		const json_value & LanguageJSON();
		// Language JSON name
		const TCHAR* const LanguageName();
	}

	std::tstring ANSIToTString(const std::string_view);
	std::string ANSIToUTF8(const std::string_view);
	std::wstring ANSIToWide(const std::string_view);
	std::string UTF8ToANSI(const std::string_view, bool* const allValidChars = nullptr);
	std::tstring UTF8ToTString(const std::string_view, bool* const allValidChars = nullptr);
	std::wstring UTF8ToWide(const std::string_view);
	std::string WideToANSI(const std::wstring_view, bool* const allValidChars = nullptr);
	std::tstring WideToTString(const std::wstring_view, bool* const allCharsValid = nullptr);
	std::string WideToUTF8(const std::wstring_view);
	std::string TStringToANSI(const std::tstring_view, bool* const allValidChars = nullptr);
	std::string TStringToUTF8(const std::tstring_view);
	std::wstring TStringToWide(const std::tstring_view);

	// =====
	// This region does JSON-based properties.
	// =====


	// Functions redirected from Fusion DLL calls.
	// The DLL_XX prefixes are necessary due to DEF files getting confused despite the namespaces.
	namespace DLL
	{
		// Returns size of EDITDATA and all properties if they were using their default values from JSON
		std::uint16_t Internal_GetEDITDATASizeFromJSON();

		void GeneratePropDataFromJSON();
	}

#ifndef NOPROPS
	namespace DLL
	{
#if EditorBuild
		// Ext devs should not use these! Returns properties to Fusion itself for rendering.
		BOOL DLL_GetProperties(mv* mV, EDITDATA* edPtr, bool masterItem);
		void DLL_ReleaseProperties(mv* mV, EDITDATA* edPtr, BOOL bMasterItem);

		LPARAM DLL_GetPropCreateParam(mv* mV, EDITDATA* edPtr, unsigned int PropID);
		void DLL_ReleasePropCreateParam(mv* mV, EDITDATA* edPtr, unsigned int PropID, LPARAM lParam);
		void* DLL_GetPropValue(mv* mV, EDITDATA* edPtr, unsigned int PropID);
		void DLL_SetPropValue(mv* mV, EDITDATA* edPtr, unsigned int PropID, void* Param);
		BOOL DLL_GetPropCheck(mv* mV, EDITDATA* edPtr, unsigned int PropID);
		void DLL_SetPropCheck(mv* mV, EDITDATA* edPtr, unsigned int PropID, BOOL checked);
		BOOL DLL_IsPropEnabled(mv* mV, EDITDATA* edPtr, unsigned int PropID);

		HGLOBAL DLL_UpdateEditStructure(mv* mV, EDITDATA* OldEdPtr);
#endif

		// /// <summary> Loads all default values into EDITDATA. Only used for new objects. </summary>
		// std::vector<std::string> PopulateEDITDATA(mv * mV, EDITDATA *& edPtr, EDITDATA ** oldEdPtr, void *(*reallocFunc)(mv * mV, void * ptr, size_t s));

		struct PropAccesser;
		struct ConverterReturnAccessor;

		int DLL_CreateObject(mv* mV, LevelObject* loPtr, EDITDATA* edPtr);
	}

	struct Properties
	{
		NO_DEFAULT_CTORS(Properties);

		// =====
		// DarkEdif functions, use within Extension ctor.
		// =====

		// Returns property checked or unchecked from property name.
		bool IsPropChecked(std::string_view propName) const;
		// Returns property checked or unchecked from property ID.
		bool IsPropChecked(int propID) const;
		// Returns std::tstring property setting from property name.
		std::tstring GetPropertyStr(std::string_view propName) const;
		// Returns std::tstring property string from property ID.
		std::tstring GetPropertyStr(int propID) const;
		// Returns a float property setting from property name.
		float GetPropertyNum(std::string_view propName) const;
		// Returns float property setting from a property ID.
		float GetPropertyNum(int propID) const;

#if EditorBuild
		// =====
		// Custom property upgrader. Fusion ext developers can use it, if they want to die.
		// =====

		struct ConverterState
		{
			// Will be null when object is created.
			const EDITDATA* oldEdPtr;
			// Will be null when object is created. Will also be null if converting from pre-smart properties.
			const Properties* oldEdPtrProps;
			// Always set to CurLang["Properties"]. A json_array.
			const json_value& jsonProps;
			// Number of properties reset to JSON (as opposed to upgrading from old props)
			size_t numPropsReset;
			// List of reset prop name = value with newline delimiter.
			::std::stringstream resetPropertiesStream;

			// You won't use this.
			ConverterState(EDITDATA*, const json_value& json);
		};
		enum class ConvReturnType
		{
			// Converter hasn't set the response
			Unset,
			// Converter read the value okay
			OK,
			// Converter failed to read, but it should be okay to use this converter after
			// Passing to next level of converter is requested (e.g. reading default from JSON)
			Pass,
			// Should have been okay to return it, but had a third-party error (e.g. memcpy)
			// Report error to user
			Error,
			// The converter can't interpret the source data, so this converter is the wrong one, and shouldn't be used
			ConverterUnsuitable
		};
		struct ConverterReturn
		{
			friend struct DLL::ConverterReturnAccessor;
		protected:
			ConvReturnType convRetType;
			const void* data;
			size_t dataSize;
			int checkboxState; // 0, 1, or -1 if unset

			// If non-null, a pointer to a freeing function.
			void (*freeData)(const void*);
		public:

			// Return a value with optional deleter, and optional checkbox state
			void Return_OK(const void* data, size_t dataSize, void (*freeData)(const void*) = nullptr, int checkboxState = -1);

			// Failed to read, but it should be okay to use this converter after
			// Passing to next converter is requested
			void Return_Pass();

			// Was okay to return it, but had a third-party error
			// Report this error to user
			void Return_Error(PrintFHintInside const TCHAR * error, ...) PrintFHintAfter(2,3);

			// The source data doesn't have this, and clearly the source is unreliable
			// This converter should no longer be used
			void Return_ConverterUnsuitable();

			ConverterReturn();
		};

		// Abstract (pure virtual) struct for reading properties.
		struct PropertyReader
		{
			// Start the property reader/converter. Return ConverterUnsuitable if the converter isn't usable.
			virtual void Initialise(ConverterState& convState, ConverterReturn* const convRet) = 0;

			// Get property by ID.
			// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped (non-monotonic).
			virtual void GetProperty(size_t id, ConverterReturn* const convRet) = 0;

			// Get property by ID.
			// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped (non-monotonic).
			virtual void GetPropertyCheckbox(size_t id, ConverterReturn* const convRet) = 0;
		};

		// =====
		// Internal DarkEdif use. Fusion ext developers should not try to use it.
		// =====

		// Defined later. Should error out if someone tries to use it.
		struct PreSmartPropertyReader;
		struct SmartPropertyReader;
		struct JSONPropertyReader;

#endif // EditorBuild

		// Turn on for lots of logging.
		static constexpr bool DebugProperties = false;

		struct Data;

		// We hide some stuff so newbie ext devs don't mistakenly use it
		friend DarkEdif::DLL::PropAccesser;

	protected:
		// Version of this DarkEdif::Properties struct. If smart properties' layouts ever need updating, voila.
		std::uint32_t propVersion = 'DAR1';
		// fnv1a hashes, used to read JSON properties to see if a property layout change has been made.
		std::uint32_t hash; // property titles and types
		std::uint32_t hashTypes; // property types only
		// Number of properties
		std::uint16_t numProps;

		// VS decided to pad this struct, so let's continue the idiocy officially
		std::uint16_t pad;
		// Size of DataForProps - including EDITDATA (and thus EDITDATA::Properties)
		// Note that this is uint32, because initial EDITDATA is capped to uint16 by GetRunObjectInfos()'s EDITDATASize,
		// but the size after initial setup is in EDITDATA::eHeader::extSize, which is uint32.
		std::uint32_t sizeBytes;
		// The actual data for properties, merged together
		// Starts with checkboxes, then data, which is Data struct: type ID followed by binary.
		std::uint8_t dataForProps[];
		// Note: There is a single bit for each checkbox.
		// Use numProps / 8 for num of bytes used by checkboxes.

		const Data* Internal_FirstData() const;
		const Data* Internal_DataAt(int ID) const;
#if EditorBuild
		Data* Internal_FirstData();
		Data* Internal_DataAt(int ID);
		Prop* GetProperty(size_t);

		static void Internal_PropChange(mv* mV, EDITDATA*& edPtr, unsigned int PropID, const void* newData, size_t newSize);
#endif
	};

#endif // NOPROPS
}


// This region does type checking on LinkXXX functions.
// Since the JSON should never change between Debug versus Edittime/Runtime versions,
// we'll only do the slow check in Debug Mode.

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
		Params p = Edif::ReadActionOrConditionParameterType(json["Parameters"][acParamIndex][0], isFloat);

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

template<int expParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(expParamIndexPlusOne > 0), void>::type
forLoopE(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const) {
	constexpr int expParamIndex = expParamIndexPlusOne - 1U;
	do
	{
		bool isFloat = false;
		ExpParams p = Edif::ReadExpressionParameterType(json["Parameters"][expParamIndex][0], isFloat);

		// Note that the function signature loses the top-level const-ness of parameters,
		// so we only need to check for the non-const parameter type.
		using cppType = typename std::tuple_element<expParamIndex, std::tuple<Args...>>::type;
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
			if (!_stricmp(json["Parameters"][expParamIndex][0], "Unsigned Integer"))
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

		str << "Has JSON parameter \""sv << (const char *)json["Parameters"][expParamIndex][1] << "\", JSON type \""sv << (const char *)json["Parameters"][expParamIndex][0]
			<< "\"; expected C++ type "sv << expCppType << ", but actual C++ type is "sv << cppTypeAsString << ".\r\n"sv;
	} while (false);

	// Recurse into next iteration
	forLoopE<expParamIndex>(ID, json, str, Function);
}

template<int expParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(expParamIndexPlusOne == 0), void>::type
forLoopE(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const) {
	// end of loop; do nothing
}

template<class Ret, class Struct, class... Args>
void LinkActionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Actions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << "; it has no Actions JSON item.\r\n"sv;
			continue;
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
		DarkEdif::MsgBox::Error(_T("Action linking error"), _T("%s"), DarkEdif::UTF8ToTString(errorStream.str()).c_str());

	(Edif::SDK)->ActionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkConditionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Conditions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking condition ID "sv << ID << "; it has no Conditions JSON item.\r\n"sv;
			continue;
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
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("%s"), DarkEdif::UTF8ToTString(errorStream.str()).c_str());

	(Edif::SDK)->ConditionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkExpressionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; curLangIndex++)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"]["Name"].type != json_string)
			continue;

		if (curLang["Expressions"].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << "; it has no Expressions JSON item.\r\n"sv;
			continue;
		}

		std::string expCppRetType = "<unknown>"s;
		std::string cppRetType = typestr(Ret);
		bool retTypeOK = false;
		const json_value &json = curLang["Expressions"][ID];
		ExpReturnType jsonRetType = Edif::ReadExpressionReturnType(json["Returns"]);

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
		DarkEdif::MsgBox::Error(_T("Expression linking error"), _T("%s"), DarkEdif::UTF8ToTString(errorStream.str()).c_str());

	(Edif::SDK)->ExpressionFunctions[ID] = Edif::MemberFunctionPointer(Function);
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

#ifdef __APPLE__
//
extern "C" int DarkEdifObjCFunc(PROJECT_NAME_RAW, getCurrentFusionEventNum)(void* objCExt);

// Reads files from inside anywhere Fusion runtime can readily access.
// Uses the CRun functions used by Edit Box to load files.
// Works with Data Elements - pass filename only, not full Windows/iOS path.
// May work with other paths like iOS app files... not obvious yet.
//
// Errors are indicated by > at start of return, which is illegal filename char;
// otherwise the return is the file path in a temporary folder, suitable for fopen()
// Return is malloc'd and must be free'd, even for errors
extern "C" char* DarkEdifObjCFunc(PROJECT_NAME_RAW, makePathUnembeddedIfNeeded)(void* ext, const char* fileName);
#endif
