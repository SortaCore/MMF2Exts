#pragma once

// JSON can have multiple languages; we'll use the last language in Runtime builds.
#if RuntimeBuild
	#define CurLang (*Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].value)
#else
	#define CurLang (DarkEdif::JSON::LanguageJSON())
#endif

#include "Edif.hpp"

// Struct contains information about a/c/e
struct ACEInfo final
{
	// We can't copy this consistently because of the zero-sized array. Prevent creating copy/move constructors.
	NO_DEFAULT_CTORS_OR_DTORS(ACEInfo);

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
	Params ReadActionOrConditionParameterType(const std::string_view&, bool&);
	ExpParams ReadExpressionParameterType(const std::string_view&, bool&);
	ExpReturnType ReadExpressionReturnType(const std::string_view&);
	ACEInfo* ACEInfoAlloc(unsigned int NumParams);

	bool CreateNewActionInfo();
	bool CreateNewConditionInfo();
	bool CreateNewExpressionInfo();
	bool IS_COMPATIBLE(mv* mV);
}

namespace DarkEdif {

	// SDK version and changes are documented in repo/DarkEdif/#MFAs and documentation/DarkEdif changelog.md
	static const int SDKVersion = 20;
#if EditorBuild

	// Gets DarkEdif.ini setting. Returns empty if file missing or key not in file.
	// Will generate a languages file if absent.
	std::string GetIniSetting(const std::string_view key);

	namespace SDKUpdater
	{
		// Starts an update check in async. Will ignore second runs.
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
		// Checks if update is needed. Returns type if so. Optionally returns update log.
		ExtUpdateType ReadUpdateStatus(std::string * logData);

		// Updates ::SDK->Icon to draw on it; optionally displays a message box.
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

			std::uint64_t refreshMS;
			std::uint64_t nextRefreshTime;
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
				nextRefreshTime = refreshMS ? GetTickCount64() + refreshMS : -1;
			}
			DebugItem(decltype(textReadFromExt) reader, decltype(textStoreDataToExt) editor,
				size_t refreshMS, const char *userSuppliedName) :
				isInt(false), refreshMS(refreshMS), userSuppliedName(userSuppliedName),
				cachedInt(-1), intReadFromExt(NULL), intStoreDataToExt(NULL),
				textReadFromExt(reader), textStoreDataToExt(editor)
			{
				cachedText.reserve(256);
				nextRefreshTime = refreshMS ? GetTickCount64() + refreshMS : -1;
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

		/** Adds textual property to Fusion debugger display.
		 * @param getLatestFromExt	 Pointer to function to read the current text from your code. Null if it never changes.
		 * @param saveUserInputToExt Pointer to function to run if user submits a new value via Fusion debugger.
		 *							 Null if you want it uneditable. Return true if edit was accepted by your ext.
		 * @param refreshMS			 Milliseconds before getLatestFromExt() should be called again to update the cached text.
		 * @param userSuppliedName	 The property name, case-sensitive. Null is allowed if property is not removable.
		*/
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

		/** Adds integer property to Fusion debugger display.
		 * @param getLatestFromExt	 Pointer to function to read the current text from your code. Null if it never changes.
		 * @param saveUserInputToExt Pointer to function to run if user submits a new value via Fusion debugger.
		 *							 Null if you want it uneditable. Return true if edit was accepted by your ext.
		 * @param refreshMS			 Milliseconds before getLatestFromExt() should be called again to update the cached text.
		 * @param userSuppliedName	 The property name, case-sensitive. Null is allowed if property is not removable.
		*/
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

		// Updates the debug item with the given name from the Fusion debugger.
		void UpdateItemInDebugger(const char *userSuppliedName, const TCHAR *newText);
		void UpdateItemInDebugger(const char *userSuppliedName, int newValue);

		FusionDebugger(Extension *const ext);
		//~FusionDebugger() = delete;
		FusionDebugger(FusionDebugger &&) = delete;
	};

#ifdef _WIN32
	extern bool IsFusion25;
	extern bool IsHWAFloatAngles;
	extern bool IsRunningUnderWine;
#else
	// True if Fusion 2.5. False if Fusion 2.0. Always true for non-Windows builds.
	constexpr bool IsFusion25 = true;
	// True if angle variables are degrees as floats. Always true for non-Windows builds.
	constexpr bool IsHWAFloatAngles = true;
	// True if running under Wine. Always false for non-Windows builds, as Wine will be using Windows app + exts.
	constexpr bool IsRunningUnderWine = false;
#endif
	// Returns the Fusion event number for this group. Works in CF2.5 and MMF2.0
	std::uint16_t GetEventNumber(EventGroupMP *);
	// Returns the Fusion event number the ext is executing. Works in CF2.5 and MMF2.0
	int GetCurrentFusionEventNum(const Extension * const ext);
	// Returns path as is if valid, an extracted binary file's path, or an error; error indicated by returning '>' at start of text
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

	// =====
	// This region does logging and debugging
	// =====

	// Breaks if debugger is attached - on Windows. On Non-Windows, raises SIGINT and may lock the control flow.
	void BreakIfDebuggerAttached();

	// For use with SetDataBreakpoint()
	enum class DataBreakpointType
	{
		// Breaks when this address is executed as code
		ExecuteCode = 0,
		// Breaks on any data write to this address
		Write = 1,
		// Breaks on any data read from this address
		Read = 2,
		// Breaks on any data reading from or writing to this address
		ReadWrite = 3,
	};

	// Windows, Debug only! Sets an area of memory to debugger-pause if modified. Assumes current thread will be modifying it.
	// Will hard-fail if it cannot set. Only 4 data breakpoints can be active at once.
	void SetDataBreakpoint(const void * memory, std::size_t size, DataBreakpointType dbt = DataBreakpointType::Write);

	[[noreturn]]
	void LOGFInternal(PrintFHintInside const TCHAR* fmt, ...) PrintFHintAfter(1, 2);

	// For calling LOGX variants with va_list, the vprintf to printf
	void LogV(int logLevel, PrintFHintInside const TCHAR* msgFormat, va_list) PrintFHintAfter(2, 0);

	// =====
	// This region does message boxes
	// =====

	namespace MsgBox
	{
		void WarningOK(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNo(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int WarningYesNoCancel(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Error(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		void Info(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(2, 3);
		int Custom(const int flags, const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...) PrintFHintAfter(3, 4);
	}

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

#if TEXT_OEFLAG_EXTENSION
	// Cross-platform safe, Fusion-editor friendly font struct. At runtime, load into a FontInfoMultiPlat.
	// @remarks This is roughly based on LOGFONT, but uses UTF-8, not ANSI.
	struct EditDataFont
	{
		// This struct's intended use is in EDITDATA's C memory, so it
		// should not be getting created C++ style.
		NO_DEFAULT_CTORS_OR_DTORS(EditDataFont);

		// Can be 0 for default height, negative for character height, positive for cell height
		std::int32_t height;
		// Width in logical units, can be 0 for default
		std::int32_t width;
		// Angle in tenth of degrees
		std::int32_t escapement;
		// Angle in tenths of degrees
		std::int32_t orientation;
		// Font weight, default FW_DONTCARE (0), with range FW_THIN (100) to FW_HEAVY (900)
		std::int32_t weight;
		// Italic, underline, strikeout
		bool italic, underline, strikeOut;
		// See Windows define ANSI_CHARSET
		std::uint8_t charSet;
		// See Windows define OUT_DEFAULT_PRECIS
		std::uint8_t outPrecision;
		// See Windows define CLIP_EMBEDDED
		std::uint8_t clipPrecision;
		// See Windows define ANTIALIASED_QUALITY 
		std::uint8_t quality;
		// See Windows defines DEFAULT_PITCH, FF_DECORATIVE
		std::uint8_t pitchAndFamily;
		// Font name in UTF-8. For TCHAR, see GetFontName().
		// @remarks LF_FACESIZE (32) is max font name size in LOGFONTW's UTF-16.
		//			In UTF-8, it can be expanded to 1.5x storage, see https://stackoverflow.com/a/58581109
		char fontNameU8[(32 / 2) * 3];
		// Font color - a COLORREF
		std::uint32_t fontColor;
		// DrawText DT flags, including text alignment and RTL settings.
		std::uint32_t dtFlags;

	public:
#ifdef _WIN32
		// Gets a LOGFONT from the internal font.
		std::unique_ptr<LOGFONT> GetWindowsLogFont() const;
		std::unique_ptr<LOGFONTA> GetWindowsLogFontA() const;
		std::unique_ptr<LOGFONTW> GetWindowsLogFontW() const;

		void SetWindowsLogFont(const LOGFONT* const);
		void SetWindowsLogFontA(const LOGFONTA* const);
		void SetWindowsLogFontW(const LOGFONTW* const);

		// Initializes from frame editor e.g. for CreateObject. Uses the frame default font,
		// falls back on Arial, default size.
		void Initialize(mv* mV);

		/* Gets the internal DT_XX text align flags converted to TEXT_ALIGN_XX flags.
		   @return TEXT_ALIGN_XX align flags, suitable for Fusion's GetTextAlignment */
		std::uint32_t GetFusionTextAlignment() const;

		/* Sets part of the DT_XX align flags based on TEXT_ALIGN_XX flags read from Extension::TextCapacity.
		 * @param textAlignFlags TEXT_ALIGN_XX flags from Fusion SetTextAlignment
		 * @param setSingleLine  If true (default), will set and clear DT_SINGLELINE based on vertical align */
		void SetFusionTextAlignment(TextCapacity textAlignFlags, bool setSingleLine = true);
#endif // _WIN32
		// Gets font name
		std::tstring GetFontName() const;
		void SetFontName(std::tstring_view);
	};
#endif // TEXT_OEFLAG_EXTENSION

#define MULTIPLAT_FONT
	// Native Fusion type for runtime fonts
	struct FontInfoMultiPlat
	{
		// Can be 0 for default height, negative for character height, positive for cell height
		// Always positive on non-Windows
		std::int32_t height = 0;
		// Width in logical units, can be 0 for default
		// Not usable on non-Windows
		std::int32_t width = 0;
		// Angle in tenth of degrees
		// Not usable on non-Windows
		std::int32_t escapement = 0;
		// Angle in tenths of degrees
		// Not usable on non-Windows
		std::int32_t orientation = 0;
		// Font weight, default FW_DONTCARE (0), with range FW_THIN (100) to FW_HEAVY (900)
		// Weight 500+ on Android, 600+ on iOS/Mac, is bold
		// Bold, italic is ignored on iOS/Mac if font name is specified
		std::int32_t weight = 0;
		// Italic, underline, strikeout
		bool italic = false, underline = false, strikeOut = false;
		// See Windows define ANSI_CHARSET
		// Not usable on non-Windows
		std::uint8_t charSet = 0;
		// See Windows define OUT_DEFAULT_PRECIS
		std::uint8_t outPrecision = 0;
		// See Windows define CLIP_EMBEDDED
		// Not usable on non-Windows
		std::uint8_t clipPrecision = 0;
		// See Windows define ANTIALIASED_QUALITY
		// Not usable on non-Windows
		std::uint8_t quality = 0;
		// See Windows defines DEFAULT_PITCH, FF_DECORATIVE
		// Not usable on non-Windows
		std::uint8_t pitchAndFamily = 0;
		// Font name desired - may be swapped or modified
		std::tstring fontNameDesired;
		// Font color - a COLORREF
		std::uint32_t fontColor = 0;
		// DT_XX flags, including text alignment and Windows stuff
		// Text alignment - Fusion alignment and RTL settings.
		// To convert to DrawText DT_XX flags, use GetDrawTextFlags().
		std::uint32_t drawTextFlags = 0;
#ifdef _WIN32
		HFONT fontHandle = NULL;
		std::unique_ptr<LOGFONT> logFont;
		// Creates a font info pointing to a native font
		FontInfoMultiPlat(HFONT nativeFont);
		// Creates a copy of font settings of this native font
		void SetFont(HFONT nativeFont);
		// Creates a copy of font settings of this native font
		void SetFont(const LOGFONT* nativeFont);
	private:
		std::tstring fontName;
	public:
#elif defined(__ANDROID__)
		global<jobject> cfontinfo;
		global<jclass> cfontinfoClass;
		static jfieldID lfHeight, lfWeight, lfItalic, lfUnderline, lfStrikeOut, lfFaceName;
		// Creates a font info pointing to a native font
		FontInfoMultiPlat(jobject nativeFont);
		// Creates a copy of font settings of this native font
		void SetFont(const jobject nativeFont);
#else // Apple
		CFontInfo* cfontinfo = nullptr;
		// Creates a font info pointing to a native font
		FontInfoMultiPlat(CFontInfo* nativeFont);
		// Creates a copy of font settings of this native font
		void SetFont(const void* const nativeFont);
#endif
#if TEXT_OEFLAG_EXTENSION
		// Creates a runtime-usable font from a EDITDATA font, tying it to an ext
		void CopyFromEditFont(Extension* const ext, const DarkEdif::EditDataFont&);
#endif
		// Creates a font info pointing to no specific font. See SetFont
		FontInfoMultiPlat();
		~FontInfoMultiPlat();
		void SetFont(const std::tstring_view fontName, int fontSize);
		std::tstring GetActualFontName();
		void UpdateNativeSide();
	};

}

// Included here so the font struct above can be used inside
#ifndef SURFACE_MULTI_PLAT_DEFINED
#include "SurfaceMultiPlat.hpp"
#endif

namespace DarkEdif
{
	// =====
	// This region does JSON-based properties.
	// =====

	// Functions redirected from Fusion DLL calls.
	// The DLL_XX prefixes are necessary due to DEF files getting confused despite the namespaces.
	namespace DLL
	{
		// TODO: Is this meant to be defined in EditorBuild, or runtime too? Is GetRunObjectInfos called at runtime?
		// Returns size of EDITDATA and all properties if they were using their default values from JSON
		std::uint16_t Internal_GetEDITDATASizeFromJSON();

#if EditorBuild
		void GeneratePropDataFromJSON();
#endif
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
		BOOL DLL_EditProp(mv* mV, EDITDATA*& edPtr, unsigned int PropID);

		HGLOBAL DLL_UpdateEditStructure(mv* mV, EDITDATA* OldEdPtr);
#endif

		// Loads all default values into EDITDATA. Only used for new objects.
		// std::vector<std::string> PopulateEDITDATA(mv * mV, EDITDATA *& edPtr, EDITDATA ** oldEdPtr, void *(*reallocFunc)(mv * mV, void * ptr, size_t s));

		struct PropAccesser;
		struct ConverterReturnAccessor;

		int DLL_CreateObject(mv* mV, LevelObject* loPtr, EDITDATA* edPtr);
	}
	struct RuntimePropSet;

#pragma pack (push, 1)
	struct Properties final
	{
		NO_DEFAULT_CTORS_OR_DTORS(Properties);

		// =====
		// DarkEdif functions, use within Extension ctor.
		// =====

		// Returns property checked or unchecked from property name.
		bool IsPropChecked(std::string_view propName) const;
		// Returns property checked or unchecked from JSON property index.
		bool IsPropChecked(int jsonPropID) const;
		// Returns std::tstring property setting from property name.
		std::tstring GetPropertyStr(std::string_view propName) const;
		// Returns std::tstring property string from a JSON property index.
		std::tstring GetPropertyStr(int jsonPropID) const;
		// Returns a float/int property setting from property name.
		float GetPropertyNum(std::string_view propName) const;
		// Returns float/int property setting from JSON property index.
		float GetPropertyNum(int jsonPropID) const;
		// Returns Fusion image bank ID from a image list JSON property ID,
		// suitable for DarkEdif::Surface::CreateFromImageBankID()
		std::uint16_t GetPropertyImageID(int jsonPropID, std::uint16_t imgIndex) const;
		// Returns Fusion image bank ID from a image list property name,
		// suitable for DarkEdif::Surface::CreateFromImageBankID()
		std::uint16_t GetPropertyImageID(std::string_view propName, std::uint16_t imgIndex) const;
		// Returns number of images in a image list property by JSON property ID
		std::uint16_t GetPropertyNumImages(int jsonPropID) const;
		// Returns number of images in a image list property by property name
		std::uint16_t GetPropertyNumImages(std::string_view propName) const;
		// Returns a Size property value by property name
		DarkEdif::Size GetSizeProperty(std::string_view propName) const;
		// Returns a Size property value by JSON property ID
		DarkEdif::Size GetSizeProperty(int jsonPropID) const;

		struct Data;
		// Iterates all the OI List in entire frame
		struct PropSetIterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = void*;
			using difference_type = std::ptrdiff_t;
			using pointer = void*;
			using reference = void*;

		public:
			// Iterator for all the OI List in entire frame
			PropSetIterator& operator++();
			// x++, instead of ++x
			PropSetIterator operator++(int);
			bool operator==(PropSetIterator other) const;
			bool operator!=(PropSetIterator other) const;
			reference operator*() const;

			PropSetIterator begin() const;
			PropSetIterator end() const;

			void SetCurrentIndex(std::uint16_t idx);
			std::uint16_t GetNumSetRepeats() const;
		private:
			const std::size_t nameListJSONIdx;
			const std::size_t numSkippedSetsBefore;
			Properties* const props;

			std::uint16_t curEntryIdx = 0;
			Properties::Data* runSetEntry = nullptr;
			RuntimePropSet* runPropSet = nullptr;

			explicit PropSetIterator(const std::size_t nameListJSONIdx, const std::size_t setIdx, Data* runSetEntry, Properties* const props, bool);

		protected:
			friend Properties;
			PropSetIterator(const std::size_t nameListJSONIdx, const std::size_t setIdx, Data* runSetEntry, Properties* const props);
		};

		/** Gets a property set iterator, suitable for switching current property set index.
		 * @param setName  Set name, as specified in JSON.
		 * @param numSkips Keep as 0, unless using subsets. For subsets, n to skip n parent/grandparent sets. */
		[[maybe_unused]] PropSetIterator LoopPropSet(std::string_view setName, std::size_t numSkips = 0) const;

		//const PropSetIterator GetPropertySetIterator(std::string_view setName);

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
			void Return_Error(PrintFHintInside const TCHAR* error, ...) PrintFHintAfter(2, 3);

			// The source data doesn't have this, and clearly the source is unreliable
			// This converter should no longer be used
			void Return_ConverterUnsuitable();

			ConverterReturn();
		};

		// Abstract (pure virtual) struct for reading properties.
		struct PropertyReader
		{
			// Start the property reader/converter. Return ConverterUnsuitable if the converter isn't usable.
			virtual void Initialize(ConverterState& convState, ConverterReturn* const convRet) = 0;

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
#if EditorBuild && defined(_DEBUG)
		static constexpr bool DebugProperties = true;
#else
		static constexpr bool DebugProperties = false;
#endif

		// Smart properties can change the version Fusion is told, so a migration from v1 -> v2 smart props
		// triggers the migration procedure even if Extension::Version is not modified by ext dev
		// Smart props v1: 0x0000
		// Smart props v2: 0x0100
		// Smart props v3: 0x0200?
		static constexpr std::uint16_t VersionFlags = 0x0100;
		// All flags, for removing the version increment; this will be 0x0100 | 0x0200 if props v3 is necessary
		static constexpr std::uint16_t AllVersionFlags = 0x0100;

		// We hide some stuff so newbie ext devs don't mistakenly use it
		friend DarkEdif::DLL::PropAccesser;

	protected:
		struct V1Data;
		// Version of this DarkEdif::Properties struct.
		// Currently DAR2. DAR1 had no visibleEditorProps, and no Data::jsonPropIndex.
		std::uint32_t propVersion;
		// fnv1a hashes, used to read JSON properties to see if a property layout change has been made.
		std::uint32_t hash; // property titles and types
		std::uint32_t hashTypes; // property types only
		// Number of properties
		std::uint16_t numProps;

		// VS decided to pad this struct
		std::uint16_t pad;
		// Size of DataForProps - including EDITDATA (and thus EDITDATA::Properties)
		// Note that this is uint32, because initial EDITDATA is capped to uint16 by GetRunObjectInfos()'s EDITDATASize,
		// but the size after initial setup is in EDITDATA::eHeader::extSize, which is uint32.
		std::uint32_t sizeBytes;

		union {
			// If properties are visible. Included but invalid in non-Editor builds.
			// @remarks This could be optimized to a bitfield but honestly, not worth.
			bool* visibleEditorProps;
			// Padding if Fusion editor is 32-bit
			std::uint64_t ptrPad;
		};

		// The actual data for properties, merged together
		// Starts with checkboxes, then data, which is Data struct: type ID followed by binary.
		std::uint8_t dataForProps[];
		// Note: There is a single bit for each checkbox.
		// Use numProps / 8 for num of bytes used by checkboxes.

		const Data* Internal_FirstData() const;
		// Gets entry by its prop index in JSON, using current set selection,
		// or by num of Next calls
		const Data* Internal_DataAt(int ID, bool idIsJSON = true) const;
		// Gets index of prop in Next() calls from its JSON index, interpreting from RuntimePropSets.
		// Optionally returns the data directly, with the RuntimePropSet that contains it.
		std::size_t PropIdxFromJSONIdx(std::size_t ID, const Data** dataPtr = nullptr, const Data** rsContainer = nullptr) const;
		// Gets prop index from name or returns max
		std::uint16_t PropJSONIdxFromName(const TCHAR * const func, const std::string_view&) const;

		std::tstring Internal_GetPropStr(const Properties::Data* data) const;
		float Internal_GetPropNum(const Properties::Data* data) const;
		DarkEdif::Size Internal_GetSizeProperty(const Properties::Data* data) const;
		std::uint16_t Internal_GetPropertyImageID(const Properties::Data* data, std::uint16_t imgIndex) const;
		std::uint16_t Internal_GetPropertyNumImages(const Properties::Data* data) const;
#if EditorBuild
		Data* Internal_FirstData();
		// Gets entry by its prop index in JSON, using current set selection,
		// or by Next index in Prop
		Data* Internal_DataAt(int ID, bool idIsJSON = true);
		std::size_t PropIdxFromJSONIdx(std::size_t ID, Data** dataPtr = nullptr, Data** rsContainer = nullptr);
		Prop* GetProperty(std::size_t);

		static void Internal_PropChange(mv* mV, EDITDATA*& edPtr, unsigned int PropID, const void* newData, size_t newSize);
#endif
	};

	struct RuntimePropSet
	{
		// Always 'S', compared with 'L' for non-set list.
		std::uint8_t setIndicator;
		// Number of repeats of this set, 1 is minimum and means one of this set
		std::uint16_t numRepeats;
		// Property that ends this set's data, as a JSON index, inclusive
		std::uint16_t lastSetJSONPropIndex;
		// First property that begins this set's data, as a JSON index, inclusive
		std::uint16_t firstSetJSONPropIndex;
		// Name property JSON index that will appear in list when switching set entry
		std::uint16_t setNameJSONPropIndex;
		// Current set index selected (0+), present at runtime too, but not used there
		std::uint16_t setIndexSelected;
		// Set name, as specified in JSON. Don't confuse with user-specified set name.
		char setName[];

		// Number of properties from firstSetJSONPropIndex that are this set's data
		// Expecting at least one: the setName
		std::uint16_t GetNumPropsInThisEntry(DarkEdif::Properties::Data* const propHoldingThisRS) const;
		// Gets the set name
		std::string GetPropSetName(const Properties::Data* const propHoldingThisRS) const;
	};
#pragma pack (pop)

	struct EdittimePropSet
	{
		std::string setName;
		// JSON indexes
		std::uint16_t addButtonIdx = UINT16_MAX, deleteButtonIdx = UINT16_MAX,
			nameEditboxIdx = UINT16_MAX, nameListIdx = UINT16_MAX,
			startSetIdx = UINT16_MAX, endSetIdx = UINT16_MAX;
	};
#else // NOPROPS

	// Dummy variables to allow building in NOPROPS mode
	struct Properties
	{
		NO_DEFAULT_CTORS_OR_DTORS(Properties);
		static constexpr std::uint16_t VersionFlags = 0;
		static constexpr std::uint16_t AllVersionFlags = 0;
		static constexpr bool DebugProperties = false;
		struct Data;
		struct V1Data;
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

// Ignore analysis warning C6287 about left and right sub-expressions being identical - they're not
#pragma warning(push)
#pragma warning(disable: 6287)

template<int acParamIndexPlusOne, class Ret, class Struct, class... Args>
typename std::enable_if<(acParamIndexPlusOne > 0), void>::type
forLoopAC(unsigned int ID, const _json_value &json, std::stringstream &str, Ret(Struct::*Function)(Args...) const, Params * const condRetType = nullptr) {
	constexpr int acParamIndex = acParamIndexPlusOne - 1U;
	do
	{
		bool isFloat = false;
		Params p = Edif::ReadActionOrConditionParameterType(json["Parameters"sv][acParamIndex][0], isFloat);

		// Note that the function signature loses the top-level const-ness of parameters,
		// so we only need to check for the non-const parameter type.
		using cppType = typename std::tuple_element<acParamIndex, std::tuple<Args...>>::type;
		const std::string cppTypeAsString(typestr(cppType));
		std::string expCppType = "?"s;
		const std::string_view title = json["Title"sv];

		// Handles comparisons by adding error and storing in condRetType as applicable
		const auto ComparisonHandler = [&]() {
			// Actions can't use comparison types
			if (condRetType == nullptr)
			{
				str << "Action "sv << title << " uses a comparison JSON parameter, but comparison "sv
					<< "parameters can only be used in conditions.\r\n"sv;
				return;
			}

			// First comparison type parameter, store it
			if ((int)*condRetType == 0)
				*condRetType = p;
			else // Multiple comparison parameter types, error out
			{
				str << "Condition "sv << title << " has two comparison JSON parameters. Only one "sv
					<< "comparison parameter can be used.\r\n"sv;
			}
		};

		if (isFloat)
		{
			// no need to check for const float. Compiler uses a direct const for restrictions
			// and optimizing, but doesn't put it in built func sig.
			// In "const type * const", the right-hand const is effectively removed in func sig;
			// const is also removed with "const type".
			if (std::is_same<cppType, float>())
				break;
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
				break;
			expCppType = "const TCHAR *"sv;
		}
		else if (p == Params::Integer || p == Params::Expression || p == Params::Comparison || p == Params::Compare_Time || p == Params::Time ||
			p == Params::Color || p == Params::Joystick_Direction || p == Params::_8Dirs || p == Params::New_Direction)
		{
			if (p == Params::Comparison || p == Params::Compare_Time)
				ComparisonHandler();

			// Several params expect unsigned int, e.g. COLORREF is unsigned, New Direction is a mask
			if (DarkEdif::SVICompare(json["Parameters"sv][acParamIndex][0], "Unsigned Integer"sv) || p == Params::Color || p == Params::Joystick_Direction ||
				p == Params::_8Dirs || p == Params::New_Direction)
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
		else if (p == Params::Object)
		{
			// Actions get passed a RunObject * for Object params, also allow const RunObject *; see notes above about top-level const
			if (condRetType == nullptr)
			{
				if (std::is_same<cppType, RunObject*>() || std::is_same<cppType, const RunObject*>())
					break;
				expCppType = "RunObject *"sv;
			}
			else // Conditions get passed an oiList number; although it's a short, for easier ASM/CPU register consistency later, we'll make it an int
			{
				if (std::is_same<cppType, int>())
					break;
				expCppType = "int"sv;
			}
		}
		else // unimplemented param type test
		{
			str << "Has JSON parameter "sv << (std::string_view)json["Parameters"sv][acParamIndex][1] << ", JSON type "sv << (std::string_view)json["Parameters"sv][acParamIndex][0]
				<< "; C++ type is "sv << cppTypeAsString << ", but DarkEdif SDK does not test that type yet. Please report it to Phi.\r\n"sv;
			break;
		}

		str << "Has JSON parameter "sv << (std::string_view)json["Parameters"sv][acParamIndex][1] << ", JSON type "sv << (std::string_view)json["Parameters"sv][acParamIndex][0]
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
		ExpParams p = Edif::ReadExpressionParameterType(json["Parameters"sv][expParamIndex][0], isFloat);

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
			if (DarkEdif::SVICompare(json["Parameters"sv][expParamIndex][0], "Unsigned Integer"sv))
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

		str << "Has JSON parameter \""sv << (std::string_view)json["Parameters"sv][expParamIndex][1] <<
			"\", JSON type \""sv << (std::string_view)json["Parameters"sv][expParamIndex][0]
			<< "\"; expected C++ type "sv << expCppType << ", but actual C++ type is "sv
			<< cppTypeAsString << ".\r\n"sv;
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
	for (std::size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; ++curLangIndex)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"sv]["Name"sv].type != json_string)
			continue;

		if (curLang["Actions"sv].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << "; it has no Actions JSON item.\r\n"sv;
			continue;
		}
		const json_value &json = curLang["Actions"sv][ID];
		const std::string_view title = json["Title"sv];

		// const void is a thing, mainly because writing checks to prevent it makes template interpretation systems a lot harder.
		if (!std::is_same<std::remove_const_t<Ret>, void>())
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << ", "sv << title << "; it has return type "sv
				<< typestr(Ret) << " instead of void in the C++ function definition."sv;
			break;
		}

		const int cppParamCount = sizeof...(Args);// Also, don't set NumAutoProps to negative.

		int jsonParamCount = json["Parameters"sv].type == json_none ? 0 : json["Parameters"sv].u.array.length;

		// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
		// CNC_XX Expression macros to get parameters themselves.
		// This lets users decide at runtime whether a parameter is float or integer, which in practice seems the only use.
		// These initially-unread parameters are not passed to the C++ function and so won't be in the C++ definition.
		const json_value &numAutoProps = json["NumAutoProps"sv];
		if (numAutoProps.type == json_integer)
			jsonParamCount = (int)numAutoProps.u.integer;

		if (cppParamCount != jsonParamCount)
		{
			errorStream << curLangName << ": error in linking action ID "sv << ID << ", "sv << title << "; it has "sv
				<< jsonParamCount << " parameters in the Actions JSON item, but "sv << cppParamCount << " parameters in the C++ function definition."sv;
		}
		else if (jsonParamCount > 0)
		{
			forLoopAC<sizeof...(Args), Ret, Struct, Args...>(ID, json, errorStream, Function);

			// forLoopAC may have made multiple errors, remove final \r\n
			if (errorStream.str().size() > 0)
			{
				std::stringstream str2;
				str2 << curLangName << ": error in linking action ID "sv << ID << ", "sv << title << ":\r\n"sv << errorStream.str();
				std::string realError = str2.str();
				errorStream.str(realError.substr(0U, realError.size() - 2U));
			}
		}
	}

	// Exit with error
	if (errorStream.str().size() > 0)
		DarkEdif::MsgBox::Error(_T("Action linking error"), _T("%s"), DarkEdif::UTF8ToTString(errorStream.str()).c_str());

	Edif::SDK->ActionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkConditionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (std::size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; ++curLangIndex)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"sv]["Name"sv].type != json_string)
			continue;

		if (curLang["Conditions"sv].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking condition ID "sv << ID << "; it has no Conditions JSON item.\r\n"sv;
			continue;
		}
		const json_value &json = curLang["Conditions"sv][ID];

		const int cppParamCount = sizeof...(Args);
		const int jsonParamCount = json["Parameters"sv].type == json_none ? 0 : json["Parameters"sv].u.array.length;
		if (cppParamCount != jsonParamCount)
		{
			errorStream << curLangName << ": error in linking condition ID "sv << ID << ", "sv << (std::string_view)json["Title"sv] << "; it has "sv
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
				str2 << curLangName << ": error in linking condition ID "sv << ID << ", "sv << (std::string_view)json["Title"sv] << "\r\n"sv << errorStream.str();
				std::string realError = str2.str();
				errorStream.str(realError.substr(0U, realError.size() - 2U));
				break; // We found an error in this language
			}
		}
	}

	// Exit with error
	if (errorStream.str().size() > 0)
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("%s"), DarkEdif::UTF8ToTString(errorStream.str()).c_str());

	Edif::SDK->ConditionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

template<class Ret, class Struct, class... Args>
void LinkExpressionDebug(unsigned int ID, Ret(Struct::*Function)(Args...) const)
{
	std::stringstream errorStream;
	for (std::size_t curLangIndex = 0; curLangIndex < Edif::SDK->json.u.object.length; ++curLangIndex)
	{
		const json_value &curLang = *Edif::SDK->json.u.object.values[curLangIndex].value;
		const char * const curLangName = Edif::SDK->json.u.object.values[curLangIndex].name;

		// JSON item is not a language, so ignore it
		if (curLang.type != json_object || curLang["About"sv]["Name"sv].type != json_string)
			continue;

		if (curLang["Expressions"sv].u.array.length <= ID)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << "; it has no Expressions JSON item.\r\n"sv;
			continue;
		}

		std::string expCppRetType = "<unknown>"s;
		std::string cppRetType = typestr(Ret);
		bool retTypeOK = false;
		const json_value &json = curLang["Expressions"sv][ID];
		ExpReturnType jsonRetType = Edif::ReadExpressionReturnType(json["Returns"sv]);

		if (jsonRetType == ExpReturnType::Integer)
		{
			if (DarkEdif::SVICompare(json["Returns"sv], "Unsigned Integer"sv))
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

		std::string_view exprName = json["Title"sv];
		if (exprName[exprName.size() - 1U] == '(')
			exprName.remove_suffix(1);

		if (!retTypeOK)
		{
			errorStream << curLangName << ": error in linking expression ID "sv << ID << ", "sv << exprName << "; it has return type "sv
				<< (std::string_view)json["Returns"sv] << " in the JSON (C++ type "sv << expCppRetType << "), but "sv << typestr(Ret) << " in the C++ function definition."sv;
			break;
		}

		constexpr int cppParamCount = sizeof...(Args);
		int jsonParamCount = json["Parameters"sv].type == json_none ? 0 : json["Parameters"sv].u.array.length;

		// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
		// CNC_XX Expression macros to get parameters themselves.
		// This lets users decide at runtime whether a parameter is float or integer, which in practice seems the only use.
		// These initially-unread parameters are not passed to the C++ function and so won't be in the C++ definition.
		const json_value &numAutoProps = json["NumAutoProps"sv];
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

	Edif::SDK->ExpressionFunctions[ID] = Edif::MemberFunctionPointer(Function);
}

// Restore analysis warning C6287
#pragma warning(pop)

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
extern "C" int DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, getCurrentFusionEventNum)(void* objCExt);

// Reads files from inside anywhere Fusion runtime can readily access.
// Uses the CRun functions used by Edit Box to load files.
// Works with Data Elements - pass filename only, not full Windows/iOS path.
// May work with other paths like iOS app files... not obvious yet.
//
// Errors are indicated by > at start of return, which is illegal filename char;
// otherwise the return is the file path in a temporary folder, suitable for fopen()
// Return is malloc'd and must be free'd, even for errors
extern "C" char* DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, makePathUnembeddedIfNeeded)(void* ext, const char* fileName);
#endif
