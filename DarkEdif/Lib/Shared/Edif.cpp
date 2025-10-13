#include "Common.hpp"
#include "Extension.hpp"
#include "DarkEdif.hpp"
#include "Edif.hpp"

// Used for Win32 resource ID numbers
#include "Resource.h"
using namespace DarkEdif;

// ============================================================================
// GLOBAL DEFINES
// Contains the definitions of all the Edif class global variables.
// ============================================================================
Edif::SDKClass * Edif::SDK = nullptr;

// 2-char language code; EN, FR or JP, since that's all the Fusion versions.
TCHAR Edif::LanguageCode[3];

// If true, running in Fusion editor, Fusion loading screen, Run Application.
// If false, running in built EXEs.
[[deprecated("Use DarkEdif::RunMode")]]
bool Edif::IsEdittime;

// If true, JSON file is external. If false, it is an embedded resource (default).
bool Edif::ExternalJSON;

#ifdef __ANDROID__
// Do not use everywhere! JNIEnv * are thread-specific. Use Edif::Runtime JNI functions to get a thread-local one.
JNIEnv * mainThreadJNIEnv;
JavaVM * global_vm;
#endif
namespace Edif {
// Checks Fusion runtime is compatible with your extension.
// In Runtime, this expression should not be called and always returns false.
bool IS_COMPATIBLE(mv * v)
{
#if RuntimeBuild
	// mV is not valid at runtime; so someone's trying to use a Runtime MFX as Editor,
	// which won't work anyway because Runtime MFX lacks A/C/E menus and such.
	return false;
#else
	// No GetVersion function provided, abort
	if (!v->GetVersion)
		return false;

	static int isCompatibleResult = -1;
	if (isCompatibleResult == -1)
	{
		isCompatibleResult = -2;
		isCompatibleResult = IS_COMPATIBLE(v);
	}
	if (isCompatibleResult > -1)
		return isCompatibleResult != 0;

	// Build too low, abort
	const unsigned int fusionVer = v->GetVersion();
	if ((fusionVer & MMFBUILD_MASK) < Extension::MinimumBuild)
		return false;

	// Fusion 1.5 or lower, abort
	if ((fusionVer & MMFVERSION_MASK) < MMFVERSION_20)
		return false;

	// Confirm Fusion branch - TGF (HOME), Standard, Developer (PRO) - called HOME
	#if defined(MMFEXT)
		return ((fusionVer & MMFVERFLAG_MASK) & MMFVERFLAG_HOME) == 0; // Not == HOME, i.e. TGF
	#elif defined(PROEXT)
		return ((fusionVer & MMFVERFLAG_MASK) & MMFVERFLAG_PRO) != 0;
	#else // TGFEXT
		return true;
	#endif
#endif
}
}

std::string Edif::CurrentFolder()
{
	// Deprecated because ambiguous:
	// extension path? application path? slash at end? data folder?
	char result[PATH_MAX];
#ifdef _WIN32
	size_t count = GetModuleFileNameA(hInstLib, result, sizeof(result));
#else
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	LOGI("Got %s as the path.\n", result);
#endif
	return std::string(result, count > 0 ? count : 0);
}
void Edif::GetExtensionName(char * const writeTo)
{
	// Deprecated because ambiguous:
	// extension name with underscores - e.g. MFX filename?
	// extension name as user-named object property?
	// extension name as in project name?
	strcpy(writeTo, PROJECT_NAME);
}

Params Edif::ReadActionOrConditionParameterType(const std::string_view &Text, bool &IsFloat)
{
	if (SVICompare(Text, "Text"sv) || SVICompare(Text, "String"sv))
		return Params::String_Expression;

	if (SVICompare(Text, "Filename"sv) || SVICompare(Text, "File"sv))
		return Params::Filename;

	if (SVICompare(Text, "Float"sv))
	{
		IsFloat = true;
		return Params::Expression;
	}

	if (SVICompare(Text, "Integer"sv))
		return Params::Expression;

	if (SVICompare(Text, "Unsigned Integer"sv))
		return Params::Expression;

	if (SVICompare(Text, "Object"sv))
		return Params::Object;

	if (SVICompare(Text, "Time"sv))
		return Params::Time;

	if (SVICompare(Text, "Position"sv))
		return Params::Position;

	if (SVICompare(Text, "Create"sv))
		return Params::Create;

	if (SVICompare(Text, "SysCreate"))
		return Params::System_Create;

	if (SVICompare(Text, "Animation"))
		return Params::Animation;

	if (SVICompare(Text, "Nop"))
		return Params::NoP;

	if (SVICompare(Text, "Player"))
		return Params::Player;

	if (SVICompare(Text, "Every"))
		return Params::Every;

	if (SVICompare(Text, "Key"))
		return Params::Key;

	if (SVICompare(Text, "Speed"))
		return Params::Speed;

	if (SVICompare(Text, "JoyDirection"))
		return Params::Joystick_Direction;

	if (SVICompare(Text, "Shoot"))
		return Params::Shoot;

	if (SVICompare(Text, "Zone"sv))
		return Params::Playfield_Zone;

	if (SVICompare(Text, "Comparison"sv))
		return Params::Comparison;

	if (SVICompare(Text, "StringComparison"sv))
		return Params::String_Comparison;

	if (SVICompare(Text, "Color"sv) || SVICompare(Text, "Colour"sv))
		return Params::Color;

	if (SVICompare(Text, "Frame"sv))
		return Params::Frame;

	if (SVICompare(Text, "SampleLoop"sv))
		return Params::Sample_Loop;

	if (SVICompare(Text, "MusicLoop"sv))
		return Params::Music_Loop;

	if (SVICompare(Text, "NewDirection"sv))
		return Params::New_Direction;

	if (SVICompare(Text, "TextNumber"sv))
		return Params::Text_Number;

	if (SVICompare(Text, "Click"sv))
		return Params::Click;

	if (SVICompare(Text, "Program"sv))
		return Params::Program;

	if (SVIComparePrefix(Text, "Custom"sv))
		return (Params)((short)Params::Custom_Base + ((short)atoi(Text.data() + sizeof("Custom") - 1)));

	MsgBox::Error(_T("DarkEdif Params error"), _T("Error reading parameter type \"%s\", couldn't match it to a Params value."),
		UTF8ToTString(Text).c_str());
	return (Params)(std::uint16_t)0;
}

ExpParams Edif::ReadExpressionParameterType(const std::string_view& Text, bool &IsFloat)
{
	if (SVICompare(Text, "Text"sv) || SVICompare(Text, "String"sv))
		return ExpParams::String;

	if (SVICompare(Text, "Float"sv))
	{
		IsFloat = true;
		return ExpParams::Float;
	}

	if (SVICompare(Text, "Integer"sv))
		return ExpParams::Integer;

	if (SVICompare(Text, "Unsigned Integer"sv))
		return ExpParams::UnsignedInteger;

	DarkEdif::MsgBox::Error(_T("DarkEdif ExpParams error"), _T("Error reading expression parameter type \"%s\", couldn't match it to a ExpParams value."), DarkEdif::UTF8ToTString(Text).c_str());
	return (ExpParams)(std::uint16_t)0;
}

ExpReturnType Edif::ReadExpressionReturnType(const std::string_view & Text)
{
	if (SVICompare(Text, "Integer"sv))
		return ExpReturnType::Integer;

	if (SVICompare(Text, "Float"sv))
		return ExpReturnType::Float;

	if (SVICompare(Text, "Text"sv) || SVICompare(Text, "String"sv))
		return ExpReturnType::String;

	// More specialised, but not allowed for
	if (SVICompare(Text, "Short"sv))
		return ExpReturnType::Integer;

	if (SVICompare(Text, "Unsigned Integer"sv))
		return ExpReturnType::UnsignedInteger;

	MsgBox::Error(_T("DarkEdif ExpReturnType error"), _T("Error reading expression return type \"%s\", couldn't match it to a ExpReturnType value."), DarkEdif::UTF8ToTString(Text).c_str());
	return ExpReturnType::Integer; // default
}

void Edif::Init(mv * mV, EDITDATA * edPtr)
{
#ifdef _WIN32
	Edif::SDK->mV = mV;

	// HMainWin may be edited since we last used it
	DarkEdif::Internal_WindowHandle = mV->HMainWin;

	// Redraw the object in frame editor
	#if EditorBuild
		if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor)
			mvInvalidateObject(mV, edPtr);
	#endif
#endif
}

void Edif::Free(mv * mV)
{
	// Workaround for subapp bug (see end of Init below)
	// Don't delete SDK. The world dies when you do that.
	// Why? Because Fusion confuses everyone.
}

void Edif::Free(EDITDATA * edPtr)
{
	// ??
}

#if defined(_WIN32) && !defined(_DEBUG)
// CRT invalid parameter handler
extern "C" void DarkEdif_Invalid_Parameter(const wchar_t* /*expression - NULL*/, const wchar_t* /*function - NULL */,
	const wchar_t* /* file - NULL */, unsigned int /* line = 0 */, uintptr_t /* pReserved - irrelevant */)
{
#pragma DllExportHint
	// All these parameters are NULL in non-Debug builds. Since Debug builds have their own handler,
	// we don't replace it except in non-Debug, so the parameters are useless.
	// After this, the caller may use _invoke_watson - which will never continue the program - or it may continue after this,
	// returning EINVAL instead.

	// To get proper debug information, build your application under Debug Unicode (or Debug), not Edittime/Runtime.
	LOGE(_T("CRT invalid parameter crash - since it's not Debug, no trace information."));
	MsgBox::Error(_T("Invalid parameter crash!"), _T("Intercepted a crash from invalid parameter in a CRT function.\n"
		"If you're a Fusion extension developer, attach a debugger now for information.\n"
		"Otherwise, the program will attempt to continue."));
}
#endif

int Edif::Init(mv * mV, bool fusionStartupScreen)
{
	_tcscpy (LanguageCode, _T ("EN"));

	// We want DarkEdif::MsgBox::XX as soon as possible.
	// Main thread ID is used to prevent crashes from message boxes not being task-modal.
	// Since we're initializing this, might as well set all the DarkEdif mV variables.
	DarkEdif::MainThreadID = std::this_thread::get_id();

#ifdef _WIN32
	if (mV->GetVersion == NULL)
	{
		// The ZipToJson tool is used to package a Fusion zip containing MFX, help file, examples etc.,
		// and upload it to Clickteam Extension Manager.
		// The tool loads this ext with a fusionStartupScreen of false and a zeroed mV,
		// making most mv-related function pointers and variables break if they're called.
		// We'll allow mv functions to not work if running under this tool, otherwise we'll get upset.
#if EditorBuild
		const TCHAR zipToJSONFilename[] = _T("\\ZipToJson.exe");
		const std::tstring path(DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType::FullPath));
		if (!path.empty() && !_tcsicmp(path.substr(path.size() - (std::size(zipToJSONFilename) - 1)).data(), zipToJSONFilename))
		{
			// Assume ZipToJson is targeting CF2.5 exts
			DarkEdif::IsHWAFloatAngles = DarkEdif::IsFusion25 = true;
		}
		else
#endif
		{
			DarkEdif::MsgBox::Error(_T("Unexpected mV"), _T("The extension is incorrectly initialized. Pass a valid mV struct."));
			std::abort(); // ZipToJSON ignores return 0
			return 0;
		}
	}
	else // mv functions are available
	{
		DarkEdif::IsFusion25 = ((mV->GetVersion() & MMFVERSION_MASK) == CFVERSION_25);
		DarkEdif::Internal_WindowHandle = mV->HMainWin;

		// 2.0 uses floats for angles if it's a Direct3D display, Software or DirectDraw uses int
		// Fusion 2.5 always uses floats, even in Software, and doesn't use DirectDraw at all
		DarkEdif::IsHWAFloatAngles = DarkEdif::IsFusion25 || mvIsHWAVersion(mV) != FALSE;

		// Detect wine by presence of wine_get_version() inside their wrapper ntdll
		HMODULE ntDll = GetModuleHandle(_T("ntdll.dll"));
		if (ntDll == NULL)
			return DarkEdif::MsgBox::Error(_T("DarkEdif Wine detection error"), _T("Couldn't search for Wine, couldn't load ntdll: error %u."), GetLastError()), -1;

		const char* (__cdecl * pwine_get_version)(void) =
			(decltype(pwine_get_version))GetProcAddress(ntDll, "wine_get_version");
		DarkEdif::IsRunningUnderWine = pwine_get_version != NULL;

#if EditorBuild
		// Calculate run mode
		if (fusionStartupScreen)
			DarkEdif::RunMode = DarkEdif::MFXRunMode::SplashScreen;
		else if (mV->HMainWin != 0)
			DarkEdif::RunMode = DarkEdif::MFXRunMode::Editor;
		else
		{
			// This could be either Run Application or Built EXE. We'll assume built EXE for simpler code, but check.
			DarkEdif::RunMode = DarkEdif::MFXRunMode::BuiltEXE;

			std::tstring_view mfxFolder = DarkEdif::GetMFXRelativeFolder(DarkEdif::GetFusionFolderType::MFXLocation);

			DarkEdif::RemoveSuffixIfExists(mfxFolder, _T("Unicode\\"sv));

			if (DarkEdif::EndsWith(mfxFolder, _T("Extensions\\"sv)))
			{
				// Note: In Run Application, the Extensions MFXs are used, but the edrt app is in Data\Runtime, not Extensions.
				std::tstring_view appPath = DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType::FullPath);

				if (DarkEdif::RemoveSuffixIfExists(appPath, _T("\\edrt.exe"sv)) || DarkEdif::RemoveSuffixIfExists(appPath, _T("\\edrtex.exe"sv)))
				{
					DarkEdif::RemoveSuffixIfExists(appPath, _T("\\Unicode"sv));
					DarkEdif::RemoveSuffixIfExists(appPath, _T("\\Hwa"sv));

					// else program is named edrt[ex].exe, but isn't running in Data\Runtime, so it's just an app called edrt.exe
					if (DarkEdif::RemoveSuffixIfExists(appPath, _T("Data\\Runtime"sv)))
					{
						std::tstring fusionPath(appPath);
						if (mvIsUnicodeVersion(mV))
							fusionPath += _T("mmf2u.exe"sv);
						else
							fusionPath += _T("mmf2.exe"sv);

						// We found Fusion!
						if (DarkEdif::FileExists(fusionPath))
							DarkEdif::RunMode = DarkEdif::MFXRunMode::RunApplication;
					}
				}
			}
		}

		if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor || DarkEdif::RunMode == DarkEdif::MFXRunMode::SplashScreen)
		{
			int localeNum = 1033;

			// CF2.5 editor language code is a setting, stored in registry 
			if (DarkEdif::IsFusion25)
			{
				HKEY key = NULL;
				TCHAR value[6];
				DWORD value_length = std::size(value), err;
				const std::tstring settingsRegPath =
					((mV->GetVersion() & MMFVERFLAG_MASK) == MMFVERFLAG_PRO) ?
					_T("Software\\Clickteam\\Fusion Developer 2.5\\General"s) :
					_T("Software\\Clickteam\\Fusion 2.5\\General"s);
				if ((err = RegOpenKey(HKEY_CURRENT_USER, settingsRegPath.c_str(), &key)) ||
					(err = RegQueryValueEx(key, _T("language"), NULL, NULL, (LPBYTE)value, &value_length)) ||
					value_length >= std::size(value))
				{
					// If the user never went into Tools > Preferences and pressed OK,
					// a ton of registry keys won't exist. Default to US English.
					if (err != ERROR_FILE_NOT_FOUND)
						DarkEdif::MsgBox::Error(_T("Language Code error"), _T("Failed to look up Fusion editor language code: error %u."), err);
				}
				else
					localeNum = _ttoi(value);

				if (key)
					RegCloseKey(key);
			}
			else // MMF2 uses different editor EXEs, and stores language in resources
			{
				// You shouldn't use .data() on a std::string_view and expect null terminator,
				// but since we know it points to the std::string appPath, we'll make an exception.
				const std::tstring_view appPath = DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType::FullPath);

				// Look up the running app and get the language code from its resources.
				const HINSTANCE hRes = LoadLibraryEx(appPath.data(), NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
				if (hRes != NULL)
				{
					// Load string resource ID 720, contains the language code
					TCHAR langCode[20];
					LoadString(hRes, 720, langCode, std::size(langCode));

					localeNum = _ttoi(langCode);

					// Free resources
					FreeLibrary(hRes);
				}
			}

			switch (localeNum) {
			case 0x40C: // MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH);
				_tcscpy(LanguageCode, _T("FR"));
				break;
			case 0x411: // MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
				_tcscpy(LanguageCode, _T("JP"));
				break;
			}
		}
#else // Not editor build, missing things that will let Fusion use it in editor.
		DarkEdif::RunMode = DarkEdif::MFXRunMode::BuiltEXE;
#endif

		// Non-Unicode ext used in Unicode-compatible runtime
#if !defined(_UNICODE) && !defined(ALLOW_ANSI_EXT_IN_UNICODE_RUNTIME)
		if (mvIsUnicodeVersion(mV))
		{
			DarkEdif::MsgBox::Error(_T("Not using Unicode"), _T("You are using a non-Unicode extension when the Fusion runtime and ")
				PROJECT_TARGET_NAME " extension is capable of Unicode.\nEnsure you have extracted all the " PROJECT_TARGET_NAME " extension files.");
		}
#endif


		// This section catches CRT invalid parameter errors at runtime, rather than insta-crashing.
		// Invalid parameters include e.g. sprintf format errors.
		// Debug builds have their own handler that makes a message box, so don't register ours
#ifndef _DEBUG
		// Don't register for splash screen, or override an existing handler
		if (
#if EditorBuild
			DarkEdif::RunMode != DarkEdif::MFXRunMode::SplashScreen &&
#endif
			_get_invalid_parameter_handler() == NULL)
		{
			_set_invalid_parameter_handler(DarkEdif_Invalid_Parameter);
		}
#endif // not Debug
	}
#endif // Windows

	// Get JSON file
	char * JSON;
	size_t JSON_Size;

	int result = Edif::GetDependency (JSON, JSON_Size, _T("json"), IDR_EDIF_JSON);

	if (result == Edif::DependencyNotFound)
		return DarkEdif::MsgBox::Error(_T("Couldn't find JSON"), _T("JSON file for " PROJECT_NAME " not found on disk or in MFX resources")), -1;

	Edif::ExternalJSON = (result == Edif::DependencyWasFile);

	char * copy = (char *) malloc (JSON_Size + 1);
	if (!copy)
		return DarkEdif::MsgBox::Error(_T("Out of memory"), _T("JSON file for " PROJECT_NAME " couldn't allocate %zu bytes while loading"), JSON_Size + 1), -1;
	memcpy (copy, JSON, JSON_Size);
	copy [JSON_Size] = 0;
	if ( result != Edif::DependencyWasResource )
		free(JSON);

	char json_error [256];

	json_settings settings;
	memset (&settings, 0, sizeof (settings));

	json_value * json = json_parse_ex (&settings, copy, JSON_Size, json_error, sizeof(json_error));

	if (!json)
		return DarkEdif::MsgBox::Error(_T("Error parsing JSON"), _T("JSON file for " PROJECT_NAME " couldn't be parsed:\n%s"), DarkEdif::UTF8ToTString(json_error).c_str()), -1;

	// Use static as a workaround for subapp bug (cheers LB), where Init/Free is called more than once,
	// even if object is not in subapp and thus doesn't apply
	// https://community.clickteam.com/forum/thread/97219-mfx-not-found-when-in-subapp/?postID=693431#post693431
	//
	// However, static causes issues on XP, so this #if fixes the static local initialization bug.
	// For more detail, see the MultiTarget MD file on XP targeting.
	// On XP, the code above zero-fills gSDK, and doesn't run the constructor, resulting in a
	// crash later, when GetRunObjectInfos() tries to use the null ::SDK->json via "CurLang".
#ifdef ThreadSafeStaticInitIsSafe
	static Edif::SDKClass gSDK(mV, *json);
	Edif::SDK = &gSDK;
#else // workaround, don't use static but singleton
	Edif::SDK = new Edif::SDKClass(mV, *json);
#endif

	return 0;	// no error
}

#if EditorBuild
// Used for reading the icon image file
FusionAPIImport BOOL FusionAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);

#endif

Edif::SDKClass::SDKClass(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;

	if (!Edif::SDK)
		Edif::SDK = this;

	#if EditorBuild
		cSurface * proto = nullptr;
		if (GetSurfacePrototype(&proto, 32, (int)SurfaceType::Memory_DeviceContext, (int)SurfaceDriver::Bitmap) == FALSE)
			DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Getting surface prototype failed."));

		if (mV->ImgFilterMgr)
		{
			Icon = new cSurface();
			char * IconData;
			size_t IconSize;

			int result = Edif::GetDependency (IconData, IconSize, _T("png"), IDR_EDIF_ICON);
			if (result != Edif::DependencyNotFound)
			{
				CInputMemFile * File = CInputMemFile::NewInstance();
				File->Create((LPBYTE)IconData, IconSize);

				const std::unique_ptr<cSurface> tempIcon = std::make_unique<cSurface>();
				const bool loadedOK = ImportImageFromInputFile(mV->ImgFilterMgr, File, tempIcon.get(), NULL, 0);
				if (!loadedOK)
				{
					// Read PNG bit depth: Skip 8 byte PNG header, IHDR 4 byte chunk length/type, img width/height,
					// and then get bit depth byte.
					// 4 bit or less bit depth is not loadable by Fusion's PNG filter.
					const std::uint8_t bitDepth = File->GetMemBuffer()[8 + 4 + 4 + 4 + 4];

					if (bitDepth <= 4)
						DarkEdif::MsgBox::Error(_T("Failed to load ext icon"), _T("" PROJECT_NAME "'s Icon.png uses a bit depth of %hhu, which is too small."), bitDepth);
					else
						DarkEdif::MsgBox::Error(_T("Failed to load ext icon"), _T("" PROJECT_NAME "'s Icon.png failed to load."));
				}

				File->Delete();

				if (!tempIcon->HasAlpha())
					tempIcon->SetTransparentColor(RGB(255, 0, 255));

				if (result != Edif::DependencyWasResource)
					free(IconData);

				Icon->Create(tempIcon->GetWidth(), tempIcon->GetHeight(), proto);

				if (!tempIcon->HasAlpha())
					Icon->SetTransparentColor(RGB(255, 0, 255));
				else
					Icon->CreateAlpha();

				if (loadedOK && tempIcon->Blit(*Icon) == FALSE)
					DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Blitting to ext icon surface failed. Last error: %i."), tempIcon->GetLastError());
			}
			ExtIcon = new DarkEdif::Surface(nullptr, Icon);
		}

		#if USE_DARKEDIF_UPDATE_CHECKER
			// Is in editor, not EXE using Run Application, and not in startup screen
			// Startup screen seems like a clever place to check, but if the update server is down,
			// you get plenty of delaying exts when loading Fusion
			if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor)
				DarkEdif::SDKUpdater::StartUpdateCheck();
		#endif

		// Is not in editor, but using an Edittime-based MFX. This is UC tag avoiding behaviour
		// Since PDB files and no optimization is only possible in Debug builds, the ext dev might have copied
		// it legitimately to debug a Run App error. So we will allow Debug builds to skip the UC tag.
		#if USE_DARKEDIF_UC_TAGGING && !defined(_DEBUG)
			if (DarkEdif::RunMode == DarkEdif::MFXRunMode::BuiltEXE)
			{
			#ifdef _UNICODE
				const TCHAR * isUni = _T("\\Unicode");
			#else
				const TCHAR * isUni = _T("");
			#endif
				DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Couldn't find UC tag; did you copy an Extensions%s MFX into Data\\Runtime%s?"), isUni, isUni);
				std::abort();
			}
		#endif // Using UC Tagging and not Debug build

	#elif /* EditorBuild == 0 && */ USE_DARKEDIF_UC_TAGGING && !defined(_DEBUG) && defined(_WIN32)

	// If UC tagging is enabled, Runtime MFXes must be tagged by loading the ext in Fusion editor.
	// This prevents anyone malicious getting the MFX blacklisted for other Fusion developers.
	// If the edit is not found in a Runtime MFX, abort the application.
	HRSRC hsrcForRes = FindResourceEx(hInstLib, RT_STRING, _T("UCTAG"), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	if (hsrcForRes == NULL)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Couldn't find UC tag, error %u. Contact the application developer."), GetLastError());
		std::abort();
	}
	#endif // EditorBuild

	if (CurLang.type != json_object)
	{
		DarkEdif::MsgBox::Error(_T("Internal JSON error"), _T("The JSON parser could not find a JSON object to use for current language."));
		return;
	}

	const json_value &Actions = CurLang["Actions"sv];
	const json_value &Conditions = CurLang["Conditions"sv];
	const json_value &Expressions = CurLang["Expressions"sv];

	#ifdef _WIN32
		ActionJumps = new void * [Actions.u.object.length + 1];
		ConditionJumps = new void * [Conditions.u.object.length + 1];
		ExpressionJumps = new void * [Expressions.u.object.length + 1];

		ActionJumps [Actions.u.object.length] = 0;
		ConditionJumps [Conditions.u.object.length] = 0;
		ExpressionJumps [Expressions.u.object.length] = 0;
	#endif

	for (std::size_t i = 0; i < Actions.u.object.length; ++i)
	{
		#ifdef _WIN32
			ActionJumps[i] = (void*)Edif::ActionJump;
		#endif

		ActionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewActionInfo();
	}

	for (std::size_t i = 0; i < Conditions.u.object.length; ++i)
	{
		#ifdef _WIN32
			ConditionJumps[i] = (void*)Edif::ConditionJump;
		#endif

		ConditionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewConditionInfo();
	}

	for (std::size_t i = 0; i < Expressions.u.object.length; ++i)
	{
		#ifdef _WIN32
			ExpressionJumps[i] = (void*)Edif::ExpressionJump;
		#endif

		ExpressionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewExpressionInfo();
	}

#ifdef DARKSCRIPT_EXTENSION
	// Generates all the extra expressions dynamically
	Extension::AutoGenerateExpressions();
#endif
#if EditorBuild
	// Object properties, as they appear in Properties tab, in the frame editor only.
	DarkEdif::DLL::GeneratePropDataFromJSON();

	ActionMenu = LoadMenuJSON(Edif::ActionID(0), CurLang["ActionMenu"sv]);
	ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), CurLang["ConditionMenu"sv]);
	ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), CurLang["ExpressionMenu"sv]);

	// Check for ext dev forgetting to overwrite some of the Template properties
	#if defined(_DEBUG) && !defined(IS_DARKEDIF_TEMPLATE)
		const json_value& about = CurLang["About"sv];
		bool unchangedPropsFound =
			SVICompare(about["Name"sv], "DarkEdif Template"sv) ||
			SVICompare(about["Author"sv], "Your Name"sv) ||
			SVICompare(about["Comment"sv], "A sentence or two to describe your extension"sv) ||
			SVICompare(about["Help"sv], "Help/Example.chm"sv) ||
			SVICompare(about["URL"sv], "https://www.example.com/"sv);
		if (!unchangedPropsFound)
		{
			std::string copy(about["Copyright"sv]);
			std::transform(copy.begin(), copy.end(), copy.begin(),
				[](unsigned char c) { return std::tolower(c); });
			unchangedPropsFound = copy.rfind("by your name"sv) != std::string::npos;
		}
		if (unchangedPropsFound)
		{
			// If you are getting this message, you need to update the About section of your JSON file.
			// Make sure to do it for all languages in the JSON.
			DarkEdif::MsgBox::WarningOK(_T("DarkEdif SDK warning"),
				_T("Template properties are unedited in " PROJECT_NAME "'s JSON file. Please amend them.\n\n")
				_T("You need to update the About section of your JSON file. ")
				_T("Make sure to do it for all languages in the JSON."));
		}
	#endif

#endif // EditorBuild
}

Edif::SDKClass::~SDKClass()
{
	LOGV(_T("Edif::SDK::~SDK() call.\n"));
	json_value_free(&json);

#if EditorBuild
	delete[] ActionJumps;
	delete[] ConditionJumps;
	delete[] ExpressionJumps;

	delete Icon;
#endif
}
#ifdef DARKSCRIPT_EXTENSION
long* actParameters;
#endif
#ifdef _WIN32
// The access restrictions prevent the ext dev accidentally modifying these, but we have to bypass it to set it ourselves
struct ForbiddenInternals {
	static inline int GetEventNumber(RunObject* rdPtr) {
		return rdPtr->rHo.hoEventNumber;
	}
};

// WinGDI intercepts our reasonable function name to GetObjectA/W, prevent that
#ifdef GetObject
	#undef GetObject
#endif

#endif


long ActionOrCondition(void * Function, int ID, Extension * ext, const ACEInfo * info, ACEParamReader &params, bool isCond)
{
	int ParameterCount = info->NumOfParams;
	long Parameters[16];
	long Result = 0L;
#if defined(__arm__) && defined(__ANDROID__)
	// long argStackCount; // Must be declared here or error reports in param reading won't compile
#endif

#ifdef DARKSCRIPT_EXTENSION
	// Store action parameters so when evaluating the Func() expression, the prior Action parameters are available
	if (!isCond)
		actParameters = Parameters;
#endif
#ifdef __ANDROID__
	// Java variables related to object selection and current event are cached: invalidate them
	ext->Runtime.InvalidateByNewACE();
#endif

	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value & numAutoProps = CurLang[isCond ? "Conditions"sv : "Actions"sv][ID]["NumAutoProps"sv];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	bool isComparisonCondition = false;

	for (int i = 0; i < ParameterCount; ++i)
	{
		switch (info->Parameter[i].p)
		{
			case Params::Expression:
				if ((info->FloatFlags & (1 << i)) != 0)
				{
					float f = params.GetFloat(i);
					Parameters[i] = *(int*)(&f);
				}
				else
					Parameters[i] = params.GetInteger(i, info->Parameter[i].p);
				break;

			case Params::String_Comparison:
			case Params::String_Expression:
			case Params::Filename:
			case Params::Filename_2:
				Parameters[i] = (long)params.GetString(i);
				// Catch null string parameters and return default 0
				if (!Parameters[i])
				{
					DarkEdif::MsgBox::Error(_T("ActionOrCondition() error"),
						_T("Error calling %s \"%s\" (ID %i); text parameter index %i was given a null string pointer.\n"
							"Was the parameter type different when the %s was created in the MFA?"),
						isCond ? _T("condition") : _T("action"),
						DarkEdif::UTF8ToTString(CurLang[isCond ? "Conditions"sv : "Actions"sv][ID]["Title"sv]).c_str(),
						ID, i,
						isCond ? _T("condition") : _T("action"));
					goto endFunc;
				}

				isComparisonCondition |= info->Parameter[i].p == Params::String_Comparison;
				break;

			case Params::Compare_Time:
			case Params::Comparison:
				isComparisonCondition = true;
				[[fallthrough]];
			case Params::New_Direction:
			case Params::Time:
				Parameters[i] = params.GetInteger(i, info->Parameter[i].p);
				break;
			// Returns a RunObject * for actions, and a OINUM for conditions
			case Params::Object:
				Parameters[i] = params.GetObject(i);
				break;

			default:
				// GetString uses GetParam, as opposed to GetIntParam.
				Parameters[i] = (long)params.GetString(i);
				break;
		}
	}

#ifdef _WIN32
	__asm
	{
		pushad					; Start new register set (do not interfere with already existing registers)
		mov ecx, ParameterCount	; Store ParameterCount in ecx
		cmp ecx, 0				; If no parameters, call function immediately
			je CallNow
		lea edx, Parameters		; Otherwise store address of (first element of) Parameters
		mov ebx, ecx			; Copy ecx, or ParameterCount, to ebx
		shl ebx, 2				; Multiply parameter count by 2^2 (size of 32-bit variable)
		add edx, ebx			; add (ParameterCount * 4) to Parameters, making edx point to Parameters[param count]
		sub edx, 4				; subtract 4 from edx, making it 0-based (ending array index)
		PushLoop:
			push [edx]			; Push value pointed to by Parameters[edx]
			sub edx, 4			; Decrement next loop`s Parameter index:	for (><; ><; edx -= 4)
			dec ecx				; Decrement loop index:						for (><; ><; ecx--)
			cmp ecx, 0			; If ecx == 0, end loop:					for (><; ecx == 0; ><)
				jne PushLoop	; Optimisation: "cmp ecx, 0 / jne" can be replaced with "jcxz"
		CallNow:
		mov ecx, ext			; Move Extension to ecx
		call Function			; Call the function inside Extension
		mov Result, eax			; Function`s return is stored in eax; copy it to Result
		popad					; End new register set (restore registers that existed before popad)
	}
	// if you add back old ARM ASM, remember to increment paramInc
#else
#ifndef __INTELLISENSE__
	if (isCond)
	{
		switch (ID)
		{
			#define DARKEDIF_ACE_CALL_TABLE_INDEX 0
			#include "Temp_ACECallTable.cpp"

			default:
				DarkEdif::MsgBox::Error(_T("ActionOrCondition error"), _T("Error calling condition ID %i, not found."), ID);
				goto endFunc;
		}
	}
	else // action
	{
		switch (ID)
		{
			#undef DARKEDIF_ACE_CALL_TABLE_INDEX
			#define DARKEDIF_ACE_CALL_TABLE_INDEX 1
			#include "Temp_ACECallTable.cpp"

			default:
				DarkEdif::MsgBox::Error(_T("ActionOrCondition error"), _T("Error calling action ID %i, not found."), ID);
				goto endFunc;
		}
	}
#endif // __INTELLISENSE__
#endif

endFunc:
#ifdef __ANDROID__
	JNIExceptionCheck();
#endif
	
	// Comparisons return an integer or string pointer, pass as-is
	if (isComparisonCondition)
		return Result;

	// Bool returns aren't 0x0 or 0x1, they botch the other 24 bits.
	return (long)*(bool *)&Result;
}

#ifdef _WIN32
struct ConditionOrActionManager_Windows : ACEParamReader
{
	RunObject * const rdPtr;
	Extension* const ext;
	bool isTwoOrLess;
	bool isCondition;
	ConditionOrActionManager_Windows(bool isCondition, RunObject * rdPtr, long param1, long param2)
		: rdPtr(rdPtr), ext(rdPtr->GetExtension()), isCondition(isCondition)
	{
		const ACEInfo* const info = (isCondition ? Edif::SDK->ConditionInfos : Edif::SDK->ActionInfos)[rdPtr->rHo.hoEventNumber];

		ext->Runtime.ParamZero = rdPtr->rHo.hoCurrentParam;

		// param1/param2 are runtime pre-evaluated parameters on Windows, if there is 2 or less params.
		// This is an old tactic from pre-MMF2 where only two parameters were possible.
		// But we can't use them as-is, if the params are float.
		// Fusion provides the pre-calculations for numbers as integer parameters, even if the math
		// was float, and thus it calculates them incorrectly.
		isTwoOrLess = info->NumOfParams <= 2;

		// If we auto-read the 2 params pased, we want to increment hoCurrentParam to fix the runtime bug
		// Using CNC_GetXX to read parameters if <= 2 params is not recommended,
		// may mess up if the pre-evaluated params generated events.
		for (int i = 0; isTwoOrLess && i < info->NumOfParams; ++i)
		{
			if ((info->FloatFlags & (1 << i)) != 0)
				(i == 0 ? param1 : param2) = CNC_GetFloatValue(rdPtr, i);
			
			rdPtr->rHo.hoCurrentParam = (EventParam*)(((char*)rdPtr->rHo.hoCurrentParam) + rdPtr->rHo.hoCurrentParam->size);
		}

		ext->Runtime.param1 = param1;
		ext->Runtime.param2 = param2;
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		if (isTwoOrLess)
			return *(const float*)(index == 0 ? &ext->Runtime.param1 : &ext->Runtime.param2);
		// CNC_GetFloatValue works more consistently when events are generated inside params,
		// than CNC_GetFloatParameter does
		int i = (int)CNC_GetFloatParameter(rdPtr);
		return *(float*)&i;
	}

	virtual const TCHAR * GetString(int index)
	{
		if (isTwoOrLess)
			return (const TCHAR *)(index == 0 ? ext->Runtime.param1 : ext->Runtime.param2);
		return (const TCHAR *)CNC_GetStringParameter(rdPtr);
	}

	virtual std::int32_t GetInteger(int index, Params type)
	{
		// TODO: Does pre-evaluated parameters work with NewDirection, or does it loop directions?
		if (isTwoOrLess)
			return (index == 0 ? ext->Runtime.param1 : ext->Runtime.param2);

		// PARAM_NEWDIRECTION will return directions as 0-31, instead of bitmask, if you use GetIntParameter(),
		// and then repeat the condition for each direction.
		// When the user says 1+1 expression on New Direction, it's stored as a code 22, Expression.
		// This value -> bitmask workaround is handled by the DarkEdif wrappers in Android/iOS.
		if (type == Params::New_Direction)
		{
			// TODO: Should hoCurrentParam be incremented to next param?
			if (rdPtr->rHo.hoCurrentParam->Code == (int)Params::New_Direction)
				return rdPtr->rHo.hoCurrentParam->evp.L[0];
			// Convert 0-31 to New Direction bitmask; also, -1 becomes empty bitmask (0)
			int ret = (std::int32_t)CNC_GetIntParameter(rdPtr);
			if (ret == -1)
				return 0;
			if (ret < 0 || ret > 31)
			{
				LOGE(_T("Invalid New Direction value %d passed by user, going with none.\n"), ret);
				return 0;
			}
			return 1 << ret;
		}
		return (std::int32_t)CNC_GetIntParameter(rdPtr);
	}

	virtual long GetObject(int index)
	{
		long ret;
		if (isTwoOrLess)
			ret = (index == 0 ? ext->Runtime.param1 : ext->Runtime.param2);
		else
		{
			// The String parameter returns a pointer, and this works for object parameters too.
			// CNC_GetParameter() is defined exactly the same as CNC_GetStringParameter().
			ret = (long)CNC_GetStringParameter(rdPtr);
		}

		// Conditions pass Object parameters as EventParam *. Dereference it and read the OI number.
		if (isCondition)
		{
			// ...or they should, all runtimes do it, but the param1/2 here strangely points to
			// a RunObject *, so delegate OI to GetOIListIndexFromObjectParam(), which reads based on paramZero
			if (isTwoOrLess)
				return ext->Runtime.GetOIListIndexFromObjectParam(index);

			if ((Params)((EventParam*)ret)->Code != Params::Object)
				LOGE(_T("ConditionOrActionManager::GetObject: Returning a OiList index for a non-Object parameter.\n"));
			return ((EventParam*)ret)->evp.W[0];
		}

		// Actions pass a RunObject *. We'll pass it as is so the ext dev can choose to switch to manual looping as needed.
		return ret;
	}
};

#elif defined(__ANDROID__)

// how many a/c/e are running in this ext at the mo
int aceIndex = 0;

/*typedef jobject CRun;

jclass GetExtClass(void* javaExtPtr) {
	assert(threadEnv && mainThreadJNIEnv == threadEnv);
	static global<jclass> clazz(mainThreadJNIEnv->GetObjectClass((jobject)javaExtPtr), "static global<> ext class, GetExtClass(), from javaExtPtr");
	return clazz;
};
jobject GetRH(void* javaExtPtr) {
	assert(threadEnv && mainThreadJNIEnv == threadEnv);
	static jfieldID getRH(mainThreadJNIEnv->GetFieldID(GetExtClass(javaExtPtr), "rh", "LRunLoop/CRun;"));
	return mainThreadJNIEnv->GetObjectField((jobject)javaExtPtr, getRH);
};*/

struct ConditionOrActionManager_Android : ACEParamReader
{
	jobject javaActOrCndObj, javaExtPtr, javaExtRHPtr;
	bool isCondition;
	int ourAceIndex;

	static global<jclass> extClass, cndClass, actClass;
	static jmethodID getActOrCondParamInt,
		getActParamString, getActParamFloat, getActParamObject,
		setActRetInt, setActRetFloat, setActRetString,
		getCndParamString, getCndParamFloat, getCndParamObject,
		setCndRetInt, setCndRetFloat, setCndRetString;
	static jfieldID getRH;

	JavaAndCString strings[16];
	int stringIndex = 0;
	std::vector<std::shared_ptr<RunObject>> objects;
	Extension* const ext;

	ConditionOrActionManager_Android(bool isCondition, Extension* ext, jobject javaActOrCndObj)
		: javaActOrCndObj(javaActOrCndObj), isCondition(isCondition), ext(ext)
	{
		ourAceIndex = aceIndex++;
		javaExtPtr = ext->javaExtPtr;
		javaExtRHPtr = ext->rhPtr->crun;

		if (extClass.invalid())
		{
			extClass = global<jclass>(mainThreadJNIEnv->GetObjectClass(javaExtPtr), "static global<> ext class, GetExtClass(), from javaExtPtr");

			// Shared by both A/C, has special cases for Direction, Time and Object
			getActOrCondParamInt = mainThreadJNIEnv->GetMethodID(extClass, "darkedif_jni_getActionOrConditionIntParam", "(LEvents/CEvent;II)I");

			// init one from passed param, the counterpart from a search
			(isCondition ? cndClass : actClass) = global<jclass>(mainThreadJNIEnv->GetObjectClass(javaActOrCndObj), "Act or Cnd class in ConditionOrActionManager ctor [1]");
			(isCondition ? actClass : cndClass) = global<jclass>(mainThreadJNIEnv->FindClass(isCondition ? "Actions/CActExtension" : "Conditions/CCndExtension"), "Act or Cnd class in ConditionOrActionManager ctor [2]");

			// getParamFilename2 handles both filename params and plain string expressions, so we use that
			getActParamString = mainThreadJNIEnv->GetMethodID(actClass, "getParamFilename2", "(LRunLoop/CRun;I)Ljava/lang/String;");
			getActParamFloat = mainThreadJNIEnv->GetMethodID(actClass, "getParamExpFloat", "(LRunLoop/CRun;I)F");
			getActParamObject = mainThreadJNIEnv->GetMethodID(actClass, "getParamObject", "(LRunLoop/CRun;I)LObjects/CObject;");

			getCndParamString = mainThreadJNIEnv->GetMethodID(cndClass, "getParamFilename2", "(LRunLoop/CRun;I)Ljava/lang/String;");
			getCndParamFloat = mainThreadJNIEnv->GetMethodID(cndClass, "getParamExpFloat", "(LRunLoop/CRun;I)F");
			getCndParamObject = mainThreadJNIEnv->GetMethodID(cndClass, "getParamObject", "(LRunLoop/CRun;I)LParams/PARAM_OBJECT;");

			getRH = mainThreadJNIEnv->GetFieldID(extClass, "rh", "LRunLoop/CRun;");
		}
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		const float f = mainThreadJNIEnv->CallFloatMethod(javaActOrCndObj, isCondition ? getCndParamFloat : getActParamFloat, javaExtRHPtr, index);
		JNIExceptionCheck();
		LOGV("Got float param, cond=%d, index %d OK: %f.\n", isCondition ? 1 : 0, index, f);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		strings[stringIndex].init(
			(jstring)mainThreadJNIEnv->CallObjectMethod(javaActOrCndObj,
				isCondition ? getCndParamString : getActParamString, javaExtRHPtr, index));
		JNIExceptionCheck();
		LOGV("Got string param, cond=%d, index %d OK: \"%s\".\n", isCondition ? 1 : 0, index, strings[stringIndex].str().data());
		return strings[stringIndex++].str().data();
	}

	virtual std::int32_t GetInteger(int index, Params type)
	{
		LOGV("Getting integer param, cond=%d, index %d, type %d.\n", isCondition ? 1 : 0, index, (int)type);
		const std::int32_t in = mainThreadJNIEnv->CallIntMethod(javaExtPtr, getActOrCondParamInt, javaActOrCndObj, index, type);
		JNIExceptionCheck();
		LOGV("Got integer param, cond=%d, index %d, type %d OK: %d.\n", isCondition ? 1 : 0, index, (int)type, in);
		return in;
	}

	virtual long GetObject(int index)
	{
		// If condition, getParamObject() returns a PARAM_OBJECT; dereference it and read its OIList var via Java side
		if (isCondition)
		{
			LOGV("Getting object param, cond=%d, index %d, type %d.\n", isCondition ? 1 : 0, index, (int)Params::Object);
			const std::int32_t oiListInt = mainThreadJNIEnv->CallIntMethod(javaExtPtr, getActOrCondParamInt, javaActOrCndObj, index, (int)Params::Object);
			JNIExceptionCheck();
			LOGV("Got object param, cond=%d, index %d OK: %i.\n", isCondition ? 1 : 0, index, oiListInt);
			return oiListInt;
		}

		LOGV("Getting object param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);

		// Not sure if this can have a zero selection and thus return null, but allow it
		JNIExceptionCheck();
		jobject jObj = mainThreadJNIEnv->CallObjectMethod(javaActOrCndObj, isCondition ? getCndParamObject : getActParamObject, javaExtRHPtr, index);
		JNIExceptionCheck();
		if (jObj == nullptr)
			return 0; // null

		LOGV("Got object param, cond=%d, index %d OK: java %p.\n", isCondition ? 1 : 0, index, jObj);

		// Move from a generic jobject to its index in ObjectList, to prevent two separate shared_ptrs for same RunObject,
		// inviting inconsistencies. To faster read the Object variable, we'll directly look it up from object list,
		// instead of constructing a RunObject, HeaderObject and the global refs involved.
		short objNumber = HeaderObject::GetObjectParamNumber(jObj);
		RunObjectMultiPlatPtr ptr = ext->rhPtr->GetObjectListOblOffsetByIndex(objNumber);
		objects.push_back(ptr); // store a shared_ptr copy until the Manager destructs
		return (long)ptr.get();
	}

	~ConditionOrActionManager_Android()
	{
		--aceIndex;
	}
};

// Pointless static definition with default ctor
global<jclass> ConditionOrActionManager_Android::extClass, ConditionOrActionManager_Android::actClass, ConditionOrActionManager_Android::cndClass;
jmethodID ConditionOrActionManager_Android::getActOrCondParamInt,
	ConditionOrActionManager_Android::getActParamString, ConditionOrActionManager_Android::getActParamFloat, ConditionOrActionManager_Android::getActParamObject,
	ConditionOrActionManager_Android::setActRetInt, ConditionOrActionManager_Android::setActRetFloat, ConditionOrActionManager_Android::setActRetString,
	ConditionOrActionManager_Android::getCndParamString, ConditionOrActionManager_Android::getCndParamFloat, ConditionOrActionManager_Android::getCndParamObject,
	ConditionOrActionManager_Android::setCndRetInt, ConditionOrActionManager_Android::setCndRetFloat, ConditionOrActionManager_Android::setCndRetString;
jfieldID ConditionOrActionManager_Android::getRH;
#else

extern "C"
{
	// equivalent of void DarkEdif_Ext_Name_generateEvent(void * ext, int code, int param);
	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, generateEvent)(void* ext, int code, int param);
	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, reHandle)(void * ext);

	int DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpression)(void* ext, void* act, int paramNum, int paramType);
	const char* DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpString)(void* ext, void* act, int paramNum);
	double DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpDouble)(void* ext, void* act, int paramNum);

	int DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpression)(void * ext, void * cnd, int paramNum, int paramType);
	const char * DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpString)(void * ext, void * cnd, int paramNum);
	double DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpDouble)(void * ext, void * cnd, int paramNum);
	bool DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndCompareValues)(void * ext, void * cnd, int paramNum);
	bool DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndCompareTime)(void * ext, void * cnd, int paramNum);

	int DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamInt)(void * ext);
	const char * DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamString)(void * ext);
	float DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamFloat)(void * ext);

	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnInt)(void * ext, int toRet);
	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnString)(void * ext, const char * toRet);
	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnFloat)(void * ext, float toRet);

	void DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, freeString)(void * ext, const char * cstr);
}

// segregate to prevent two iOS exts conflicting
inline namespace FusionInternals {
struct ConditionOrActionManager_iOS : ACEParamReader
{
	::Extension* ext;
	void * objCActOrCndObj;
	bool isCondition;

	ConditionOrActionManager_iOS(bool isCondition, Extension* ext, void * objCActOrCndObj)
		: ext(ext), objCActOrCndObj(objCActOrCndObj), isCondition(isCondition)
	{
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		double f = (isCondition ? DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpDouble) : DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpDouble))(ext->objCExtPtr, objCActOrCndObj, index);
		LOGV("Got float param, cond=%d, index %d OK: %f.\n", isCondition ? 1 : 0, index, f);
		return (float)f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		const TCHAR* str = (isCondition ? DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpString) : DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpString))(ext->objCExtPtr, objCActOrCndObj, index);
		LOGV("Got string param, cond=%d, index %d OK: \"%s\".\n", isCondition ? 1 : 0, index, str);
		return str;
	}

	virtual std::int32_t GetInteger(int index, Params type)
	{
		LOGV("Getting integer param, cond=%d, index %d, type %d.\n", isCondition ? 1 : 0, index, (int)type);
		std::int32_t in = (isCondition ? DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, cndGetParamExpression) : DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, actGetParamExpression))(ext->objCExtPtr, objCActOrCndObj, index, (int)type);
		LOGV("Got integer param, cond=%d, index %d, type %d OK: %i.\n", isCondition ? 1 : 0, index, (int)type, in);
		return in;
	}

	virtual long GetObject(int index)
	{
		// If condition, the parameter look-up gets an EventParam from runtime, which we dereference into a OINUM
		if (isCondition)
		{
			LOGV("Getting object param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
			const LPEVP eventParam = [((CCndExtension*)objCActOrCndObj) getParamObject:((CRunExtension*)ext->objCExtPtr)->rh withNum : index];
			const short oiList = eventParam->evp.evpW.evpW0;
			LOGV("Got object param, cond=%d, index %d OK: %hi.\n", isCondition ? 1 : 0, index, oiList);
			return oiList;
		}

		LOGV("Getting object param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		void * const obj = [((CActExtension*)objCActOrCndObj) getParamObject:((CRunExtension*)ext->objCExtPtr)->rh withNum: index];
		LOGV("Got object param, cond=%d, index %d OK: %p.\n", isCondition ? 1 : 0, index, obj);
		return (long)obj;
	}

	~ConditionOrActionManager_iOS()
	{
	}
};
} // namespace FusionInternals
#endif

#ifdef _WIN32
long FusionAPI Edif::ConditionJump(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * const ext = ((RunObject*)rdPtr)->GetExtension();
	int ID = ForbiddenInternals::GetEventNumber(((RunObject*)rdPtr));
	ConditionOrActionManager_Windows params(true, (RunObject*)rdPtr, param1, param2);
#elif defined(__ANDROID__)
ProjectFunc jlong conditionJump(JNIEnv *, jobject, jlong extPtr, int ID, CCndExtension cndExt)
{
	Extension * const ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(true, ext, (jobject)cndExt);
	global<jobject> lastCEvent = ext->Runtime.curCEvent.swap_out(); // prevent subfunctions causing this variable to be incorrect
	ext->Runtime.curCEvent = global((jobject)cndExt, "Current Cnd ext");
#else
ProjectFunc long PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _conditionJump(void * cppExtPtr, int ID, void * cndExt))
{
	Extension* const ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(true, ext, cndExt);
	ext->Runtime.curCEvent = cndExt;
#endif
	LOGV(PROJECT_NAME _T(" Condition ID %i start.\n"), ID);

	if (Edif::SDK->ConditionFunctions.size() < (unsigned int)ID) {
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("Missing condition ID %d in extension %s. This ID was not linked in Extension ctor with LinkCondition()."),
			ID, _T("" PROJECT_NAME));
		return ext->UnlinkedCondition(ID);
	}

	void * Function = Edif::SDK->ConditionFunctions[ID];

	if (!Function) {
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("Missing condition ID %d in extension %s. Condition existed in vector, but was NULL. Might not be linked."),
			ID, _T("" PROJECT_NAME));
		return ext->UnlinkedCondition(ID);
	}

	long Result = ActionOrCondition(Function, ID, ext, Edif::SDK->ConditionInfos[ID], params, true);

	LOGV(_T(PROJECT_NAME " Condition ID %i end on event %i, returning %li (%s).\n"), ID, DarkEdif::GetCurrentFusionEventNum(ext), Result, Result != 0 ? _T("TRUE") : _T("FALSE"));

#ifdef __ANDROID__
	if (ext->Runtime.curCEvent.ref)
	{
		threadEnv->DeleteGlobalRef(ext->Runtime.curCEvent.ref);
		ext->Runtime.curCEvent.ref = nullptr;
	}
	ext->Runtime.curCEvent = lastCEvent.swap_out();
#endif

	return Result;
}

// For some reason, actions are expected to return a short.
// On Android/iOS, we make the Fusion wrapper, so we can return void instead.
// To handle that, the "actreturn"	hack is used.

#ifdef _WIN32
short FusionAPI Edif::ActionJump(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * const ext = ((RunObject*)rdPtr)->GetExtension();
	const int ID = ForbiddenInternals::GetEventNumber((RunObject*)rdPtr);
	ConditionOrActionManager_Windows params(false, (RunObject*)rdPtr, param1, param2);
#define actreturn 0
#elif defined (__ANDROID__)
ProjectFunc void actionJump(JNIEnv *, jobject, jlong extPtr, jint ID, CActExtension act)
{
	Extension * ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(false, ext, act);
	// To prevent subfunctions causing this variable to be incorrect, we'll copy it out and restore after
	global<jobject> lastCEvent = ext->Runtime.curCEvent.swap_out();
	ext->Runtime.curCEvent = global((jobject)act, "Current Act ext");
	const jobject lastAct = ext->Runtime.curRH4ActBasedOnCEventOnly;
	ext->Runtime.curRH4ActBasedOnCEventOnly = ext->Runtime.curCEvent.ref;
#define actreturn /* void */
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _actionJump(void * cppExtPtr, int ID, void * act))
{
	Extension* ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(false, ext, act);
	auto lastCEvent = ext->Runtime.curCEvent;
	ext->Runtime.curCEvent = act;
#define actreturn /* void */
#endif
	LOGV(PROJECT_NAME _T(" Action ID %i start.\n"), ID);

	if (Edif::SDK->ActionFunctions.size() < (unsigned int)ID)
	{
		ext->UnlinkedAction(ID);
		return actreturn;
	}
	void * Function = Edif::SDK->ActionFunctions[ID];

	if (!Function)
	{
		ext->UnlinkedAction(ID);
		return actreturn;
	}

	ActionOrCondition(Function, ID, ext, Edif::SDK->ActionInfos[ID], params, false);

	LOGV(PROJECT_NAME _T(" Action ID %i end.\n"), ID);
#ifdef __ANDROID__
	if (ext->Runtime.curCEvent.ref)
	{
		threadEnv->DeleteGlobalRef(ext->Runtime.curCEvent.ref);
		ext->Runtime.curCEvent.ref = nullptr;
	}
	ext->Runtime.curCEvent = lastCEvent.swap_out();
	ext->Runtime.curRH4ActBasedOnCEventOnly = lastAct;
#elif defined(__APPLE__)
	ext->Runtime.curCEvent = lastCEvent;
#endif
	return actreturn;
#undef actreturn
}

#ifdef __ANDROID__

struct ExpressionManager_Android : ACEParamReader {
	CNativeExpInstance expJavaObj;
	JavaAndCString strings[16];
	int stringIndex = 0;
	int ourAceIndex;
	jobject javaExtPtr;
	static global<jclass> expClass;
	static jmethodID setRetInt, setRetFloat, setRetString, getParamInt, getParamString, getParamFloat;

	ExpressionManager_Android(Extension * ext, CNativeExpInstance expJavaObj) :
		 expJavaObj(expJavaObj), javaExtPtr(ext->javaExtPtr)
	{
		ourAceIndex = aceIndex++;
		if (expClass.invalid())
		{
			expClass = global<jclass>(mainThreadJNIEnv->GetObjectClass(expJavaObj), "static global<> expClass, from ExpressionManager ctor");
			// these aren't memory, just ID numbers, which don't change during app lifetime
			getParamInt = mainThreadJNIEnv->GetMethodID(expClass, "getParamInt", "()I");
			getParamFloat = mainThreadJNIEnv->GetMethodID(expClass, "getParamFloat", "()F");
			getParamString = mainThreadJNIEnv->GetMethodID(expClass, "getParamString", "()Ljava/lang/String;");
			setRetInt = mainThreadJNIEnv->GetMethodID(expClass, "setReturnInt", "(I)V");
			setRetFloat = mainThreadJNIEnv->GetMethodID(expClass, "setReturnFloat", "(F)V");
			setRetString = mainThreadJNIEnv->GetMethodID(expClass, "setReturnString", "(Ljava/lang/String;)V");
		}
	}
	void SetValue(int a) {
		mainThreadJNIEnv->CallVoidMethod(expJavaObj, setRetInt, a);
	}
	void SetValue(float a) {
		mainThreadJNIEnv->CallVoidMethod(expJavaObj, setRetFloat, a);
	}
	void SetValue(const char * a) {
		LOGV("Setting expression return as text...\n");
		// Convert into Java memory
		jstring jStr = CStrToJStr(a);
		mainThreadJNIEnv->CallVoidMethod(expJavaObj,  setRetString, jStr);
		JNIExceptionCheck();
		mainThreadJNIEnv->DeleteLocalRef(jStr); // not strictly needed, it should auto-free when expressionJump ends, but can't hurt
		JNIExceptionCheck();
		LOGV("Setting expression return as text \"%s\" OK.\n", a);
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, expr, index %d OK.\n", index);
		const float f = mainThreadJNIEnv->CallFloatMethod(expJavaObj, getParamFloat);
		LOGV("Got float param, expr, index %d OK: %f.\n", index, (double)f);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.\n", index);
		strings[stringIndex].init((jstring)mainThreadJNIEnv->CallObjectMethod(expJavaObj, getParamString));
		LOGV("Got string param, expr, index %d OK: %s.\n", index, strings[stringIndex].str().data());
		return strings[stringIndex++].str().data();
	}

	virtual std::int32_t GetInteger(int index, Params)
	{
		LOGV("Getting integer param, expr, index %d OK.\n", index);
		const std::int32_t i = mainThreadJNIEnv->CallIntMethod(expJavaObj, getParamInt);
		LOGV("Got integer param, expr, index %d OK: %i.\n", index, i);
		return i;
	}
	virtual long GetObject(int)
	{
		// Expressions can't use object parameters
		return 0;
	}

	void SetReturnType(ExpReturnType rt) {
		// Do nothing. We only care on Windows.
	}

	~ExpressionManager_Android() {
		--aceIndex;
	}
};

// Pointless static definition with default ctors
global<jclass> ExpressionManager_Android::expClass;
jmethodID ExpressionManager_Android::setRetInt, ExpressionManager_Android::setRetFloat, ExpressionManager_Android::setRetString,
	ExpressionManager_Android::getParamInt, ExpressionManager_Android::getParamString, ExpressionManager_Android::getParamFloat;

#elif defined(_WIN32)

struct ExpressionManager_Windows : ACEParamReader {
	RunObject * rdPtr;
	bool isOneOrLess = false;
	int param = 0;
	ExpressionManager_Windows(RunObject * rdPtr) : rdPtr(rdPtr)
	{
	}
	void SetOneOrLess(long param)
	{
		isOneOrLess = true;
		this->param = param;
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index) override
	{
		if (isOneOrLess && index == 0)
			return *(float*)&param;
		int i = (int)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_FLOAT, index);
		return *(float*)&i;
	}

	virtual const TCHAR * GetString(int index) override
	{
		if (isOneOrLess && index == 0)
			return (const TCHAR*)param;
		// We don't have to free this memory; Fusion will manage it
		return (const TCHAR *)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_STRING, index);
	}

	virtual std::int32_t GetInteger(int index, Params) override
	{
		if (isOneOrLess && index == 0)
			return param;
		return (std::int32_t)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_INT, index);
	}
	virtual long GetObject(int)
	{
		// Expressions can't use object parameters
		return 0;
	}

	void SetReturnType(ExpReturnType rt)
	{
		if (rt == ExpReturnType::Float)
			rdPtr->rHo.hoFlags |= HeaderObjectFlags::Float;
		else if (rt == ExpReturnType::String)
			rdPtr->rHo.hoFlags |= HeaderObjectFlags::String;
	}
	long SetValue(int a) {
		return (long)a;
	}
	long SetValue(float a) {
		return (long)a;
	}
	long SetValue(const char * a) {
		// Note: must be Runtime.CopyString'd. We assume it has been.
		return (long)a;
	}

};
#else
// segregate to prevent two iOS exts conflicting
inline namespace FusionInternals {
struct ExpressionManager_iOS : ACEParamReader {
	Extension* const ext;

	ExpressionManager_iOS(Extension* ext) :
		ext(ext)
	{
	}
	void SetValue(int a) {
		DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnInt)(ext->objCExtPtr, a);
	}
	void SetValue(float a) {
		DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnFloat)(ext->objCExtPtr, a);
	}
	void SetValue(const char* a) {
		DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expSetReturnString)(ext->objCExtPtr, a);
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, expr, index %d OK.\n", index);
		float f = DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamFloat)(ext->objCExtPtr);
		LOGV("Got float param, expr, index %d OK: %f.\n", index, f);
		return f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.\n", index);
		const TCHAR* str = DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamString)(ext->objCExtPtr);
		LOGV("Got string param, expr, index %d OK: \"%s\".\n", index, str);
		return str;
	}

	virtual std::int32_t GetInteger(int index, Params)
	{
		LOGV("Getting integer param, expr, index %d OK.\n", index);
		std::int32_t i = DarkEdifObjCFunc(PROJECT_TARGET_NAME_UNDERSCORES_RAW, expGetParamInt)(ext->objCExtPtr);
		LOGV("Got integer param, expr, index %d OK: %d.\n", index, i);
		return i;
	}
	virtual long GetObject(int)
	{
		// Expressions can't use object parameters
		return 0;
	}

	void SetReturnType(ExpReturnType rt)
	{
		// Do nothing. We only care on Windows.
	}

	~ExpressionManager_iOS() {
	}
};
} // namespace FusionInternals
#endif

#ifdef _WIN32
long FusionAPI Edif::ExpressionJump(RUNDATA * rdPtr, long param)
{
	const int ID = ForbiddenInternals::GetEventNumber((RunObject*)rdPtr);
	Extension * const ext = ((RunObject*)rdPtr)->GetExtension();
	ExpressionManager_Windows params((RunObject*)rdPtr);
	LOGV(PROJECT_NAME _T(" Expression ID %i start.\n"), ID);
#elif defined(__ANDROID__)
ProjectFunc void expressionJump(JNIEnv *, jobject, jlong extPtr, jint ID, CNativeExpInstance expU)
{
	Extension * ext = (Extension *)extPtr;
	ExpressionManager_Android params(ext, expU);
	// Phi note 27/11/2023: Sometimes the expression is called and no CEvent to read action/condition index.
	// However, I don't want to override an existing act/condition's CEvent since it's actually a CEvent,
	// whereas an expresssion is a sub-variable of a CEvent.
	//if (ext->Runtime.curCEvent.invalid())
	//	ext->Runtime.curCEvent = global((jobject)expU, "Current Exp ext");
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _expressionJump(void * cppExtPtr, int ID))
{
	Extension* ext = (Extension*)cppExtPtr;
	ExpressionManager_iOS params(ext);
#endif

	if (Edif::SDK->ExpressionFunctions.size() < (unsigned int)ID)
		return params.SetValue((int)ext->UnlinkedExpression(ID));

	void * Function = Edif::SDK->ExpressionFunctions[ID];

	if (!Function)
		return params.SetValue((int)ext->UnlinkedExpression(ID));

	const ACEInfo * info = Edif::SDK->ExpressionInfos[ID];
	ExpReturnType ExpressionRet = info->Flags.ef;

	int ParameterCount = info->NumOfParams;
#if _WIN32
	if (ParameterCount <= 1)
		((ExpressionManager_Windows)params).SetOneOrLess(param);
#elif defined(__ANDROID__)
	// Java variables related to object selection and current event are cached: invalidate them
	ext->Runtime.InvalidateByNewACE();
#endif


	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX Expression macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value & numAutoProps = CurLang["Expressions"sv][ID]["NumAutoProps"sv];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	long Parameters[16];
	std::uintptr_t Result = 0;

	//int ExpressionRet2 = (int)ExpressionRet;
#ifdef _WIN32
	int ExpressionRet2 = (int)ExpressionRet; // easier for ASM
#elif 0
	int argStackCount = ParameterCount;
	// if > 4 params, they're stored on stack
	if (argStackCount > 4) {
		argStackCount = argStackCount - 4;
	}
#endif

	for (int i = 0; i < ParameterCount; ++i)
	{
		// if i == 0 (first parameter of expression) we call GET_PARAM_1, else we call GET_PARAM_2
		switch (info->Parameter[i].ep)
		{
		case ExpParams::String:
			Parameters[i] = (long)params.GetString(i);

			// Catch null string parameters and return "" or 0 as appropriate
			if (!Parameters[i])
			{
				DarkEdif::MsgBox::Error(_T("Edif::Expression() error"),
					_T("Error calling expression \"%s\" (ID %i); parameter index %i was given a null string pointer.\n"
					"Was the parameter type different when the expression was created in the MFA?"),
					DarkEdif::UTF8ToTString(CurLang["Expressions"sv][ID]["Title"sv]).c_str(), ID, i);

				if (ExpressionRet == ExpReturnType::String)
					Result = (long)ext->Runtime.CopyString(_T(""));
				goto endFunc;
			}
			break;

		case ExpParams::Integer: // Handles float as well
			if ((info->FloatFlags & (1 << i)) != 0)
			{
				float f = params.GetFloat(i);
				Parameters[i] = *(int*)&f;
			}
			else
				Parameters[i] = params.GetInteger(i, (Params)0);
			break;
		default:
		{
			DarkEdif::MsgBox::Error(_T("Edif::Expression() error"), _T("Error calling expression \"%s\" (ID %i); parameter index %i has unrecognised ExpParams %hi."),
				DarkEdif::UTF8ToTString(CurLang["Expressions"sv][ID]["Title"sv]).c_str(), ID, i, (short)info->Parameter[i].ep);
			if (ExpressionRet == ExpReturnType::String)
				Result = (long)ext->Runtime.CopyString(_T(""));
			goto endFunc;
		}
		}
	}

#ifdef DARKSCRIPT_EXTENSION
	_CrtCheckMemory();
#ifdef __ANDROID__
	threadEnv->PushLocalFrame(50);
#endif

	if (Edif::SDK->ExpressionFunctions[ID] == Edif::MemberFunctionPointer(&Extension::VariableFunction))
	{
		// Ignore undefined memory warning, we know Parameters[0] is inited,
		// if VariableFunction is our expression function
#ifdef _WIN32
#pragma warning(suppress: 6001)
#endif
		Result = ext->VariableFunction((const TCHAR *)Parameters[0], *info, Parameters);
		_CrtCheckMemory();
		goto endFunc;
	}
#endif

#ifdef _WIN32
	__asm
	{
		pushad
		mov ecx, ParameterCount
		cmp ecx, 0
			je CallNow
		lea edx, Parameters
		mov ebx, ecx
		shl ebx, 2
		add edx, ebx
		sub edx, 4
		PushLoop:
			push [edx]
			sub edx, 4
			dec ecx
			cmp ecx, 0
				jne PushLoop
		CallNow:
			mov ecx, ext
			call Function
			mov ecx, ExpressionRet2

			cmp ecx, 2
				jne NotFloat

			fstp Result
			jmp End
		NotFloat:
			mov Result, eax
		End:
			popad
	}
endFunc:
	// Must set return type after the expression func is evaluated, as sub-expressions inside the
	// expression func (e.g. from generating events) could change it to something else
	params.SetReturnType(ExpressionRet);
	LOGV(PROJECT_NAME _T(" Expression ID %i end.\n"), ID);
	return Result;

	// if you add back old ARM ASM, remember to increment paramInc
#else // CLANG

	switch (ID)
	{
		#ifndef __INTELLISENSE__
		#undef DARKEDIF_ACE_CALL_TABLE_INDEX
		#define DARKEDIF_ACE_CALL_TABLE_INDEX 2
		#include "Temp_ACECallTable.cpp"
		#endif // __INTELLISENSE__
		default:
			DarkEdif::MsgBox::Error(_T("Expression error"), _T("Error calling expression: expression ID %i not found."), ID);
			goto endFunc;
	}
	(void)0;
	endFunc:

		// Must set return type after the expression func is evaluated, as sub-expressions inside the
		// expression func (e.g. from generating events) could change it to something else
		params.SetReturnType(ExpressionRet);

		if (ExpressionRet == ExpReturnType::String)
			params.SetValue((const TCHAR *)Result);
		else if (ExpressionRet == ExpReturnType::Integer)
			params.SetValue((int)Result);
		else if (ExpressionRet == ExpReturnType::Float)
			params.SetValue(*(float *)&Result);
		else
			DarkEdif::MsgBox::Error(_T("Expression ASM error"), _T("Error calling expression ID %i: Unrecognised return type."), ID);

#if defined(DARKSCRIPT_EXTENSION) && defined(__ANDROID__)
		threadEnv->PopLocalFrame(NULL);
#endif
#endif
}

int Edif::GetDependency (char *& Buffer, size_t &Size, const TCHAR * FileExtension, int Resource)
{
#ifdef _WIN32
	TCHAR Filename [MAX_PATH];
	GetSiblingPath (Filename, FileExtension);

	Buffer = nullptr;
	if (*Filename)
	{
		FILE * File = nullptr;
		int error = _tfopen_s(&File, Filename, _T("rb"));

		if (!File)
			return DependencyNotFound;

		fseek(File, 0, SEEK_END);
		Size = ftell(File);
		fseek(File, 0, SEEK_SET);

		Buffer = (char *) malloc(Size + 1);
		if (!Buffer)
			throw std::runtime_error("Out of memory");
		Buffer[Size] = 0;

		if (fread(Buffer, 1, Size, File) != Size)
			throw std::runtime_error("Reading opened file resource into RAM failed");

		fclose(File);

		return DependencyWasFile;
	}

	if (!Resource)
		return DependencyNotFound;

	HRSRC res = FindResource (hInstLib, MAKEINTRESOURCE (Resource), _T("EDIF"));

	if (!res)
		return DependencyNotFound;

	Size = SizeofResource (hInstLib, res);
	HGLOBAL resHandle = LoadResource(hInstLib, res);
	if (!resHandle)
		throw std::runtime_error("Could not load resource");

	Buffer = (char *) LockResource (resHandle);
	if (!Buffer)
		throw std::runtime_error("Could not lock resource");

	return DependencyWasResource;
#elif defined(__ANDROID__)
	if (_tcsicmp(FileExtension, _T("json")))
		return DependencyNotFound;

	Buffer = (char *)(void *) darkExtJSON;
	Size = darkExtJSONSize;
	return DependencyWasResource;
#else
	if (_tcsicmp(FileExtension, _T("json")))
		return DependencyNotFound;

#define COMBINE2(a,b) a ## b
#define COMBINE(a,b) COMBINE2(a,b)
	Buffer = (char *)(void *)COMBINE(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _darkExtJSON);
	Size = COMBINE(PROJECT_TARGET_NAME_UNDERSCORES_RAW, _darkExtJSONSize);
	return DependencyWasResource;
	// A start at reading JSON from file.
#if 0
	// https://stackoverflow.com/questions/25559996/using-resource-files-in-ndk/25560443#25560443
	// https://developer.android.com/ndk/reference/asset__manager_8h.html
	// https://stackoverflow.com/questions/13317387/how-to-get-file-in-assets-from-android-ndk
	// --> https://stackoverflow.com/a/40935331


	// https://en.wikibooks.org/wiki/OpenGL_Programming/Android_GLUT_Wrapper#Accessing_assets
	JavaVM* vm = state_param->activity->vm;
	AAssetManager_fromJava(threadEnv, assetManager);
	// https://stackoverflow.com/a/40935331
	AssetManager * mgr = app->activity->assetManager;
	AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
	const char* filename = (const char*)NULL;
	while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
		AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
		char buf[BUFSIZ];
		int nb_read = 0;
		FILE* out = fopen(filename, "w");
		while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
			fwrite(buf, nb_read, 1, out);
		fclose(out);
		AAsset_close(asset);
	}
	AAssetDir_close(assetDir);
	return DependencyWasFile;
#endif // File reading
#endif // _WIN32
}

#ifdef _WIN32
static void GetSiblingPath (TCHAR * Buffer, const TCHAR * FileExtension)
{
	TCHAR temp [MAX_PATH];

	GetModuleFileName (hInstLib, temp, sizeof(temp)/sizeof(TCHAR));

	TCHAR * Filename = temp + _tcslen(temp) - 1;

	while(*Filename != '.')
		-- Filename;

	_tcscpy(++ Filename, FileExtension);

	Filename = Filename + _tcslen(Filename) - 1;

	while(*Filename != '\\' && *Filename != '/')
		-- Filename;

	++ Filename;

	// Is the file in the directory of the MFX? (if so, use this pathname)
	TCHAR FullFilename [MAX_PATH];
	_tcscpy(FullFilename, temp);
	if (!DarkEdif::FileExists(FullFilename))
	{
		// No => editor
		TCHAR ExecutablePath [MAX_PATH];
		GetModuleFileName (GetModuleHandle (0), ExecutablePath, sizeof(ExecutablePath)/sizeof(TCHAR));

		{	TCHAR * Iterator = ExecutablePath + _tcslen(ExecutablePath) - 1;

			while(*Iterator != '\\' && *Iterator != '/')
				-- Iterator;

			*Iterator = 0;
		}

		// Same path as the executable?
		_stprintf_s(FullFilename, sizeof(ExecutablePath)/sizeof(TCHAR), _T("%s/%s"), ExecutablePath, Filename);
		if (!DarkEdif::FileExists(FullFilename))
		{
			// No => try Data/Runtime
			_stprintf_s(FullFilename, sizeof(ExecutablePath)/sizeof(TCHAR), _T("%s/Data/Runtime/%s"), ExecutablePath, Filename);
			if (!DarkEdif::FileExists(FullFilename))
			{
				*Buffer = 0;
				return;
			}
		}
	}

	_tcscpy(Buffer, FullFilename);
}

void Edif::GetSiblingPath (TCHAR * Buffer, const TCHAR * FileExtension)
{
	*Buffer = 0;

	TCHAR Extension[32];
	if (_stprintf_s(Extension, std::size(Extension), _T("%s.%s"), LanguageCode, FileExtension) <= 0)
		return DarkEdif::MsgBox::Error(_T("GetSiblingPath fail"), _T("Invalid file extension %s"), FileExtension);
	::GetSiblingPath (Buffer, Extension);

	if (*Buffer)
		return;

	::GetSiblingPath (Buffer, FileExtension);
}

#endif

#ifdef _UNICODE
wchar_t * Edif::ConvertString(const std::string_view & utf8String)
{
#ifndef _WIN32
	std::string s(utf8String);
	std::wstring ws(s.size(), L' '); // Overestimate number of code points.
	ws.resize(std::mbstowcs(&ws[0], s.c_str(), s.size())); // Shrink to fit.
	return wcsdup(ws.c_str());
#else
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), 0, 0);
	assert(Length >= 0);
	wchar_t * tstr = (wchar_t *)calloc(++Length, sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), tstr, Length);
	return tstr;
#endif
}

wchar_t * Edif::ConvertAndCopyString(wchar_t * tstr, const std::string_view & utf8String, int maxLength)
{
#ifndef _WIN32
	size_t sSize = strlen(utf8String);
	if (sSize > maxLength)
	{
		LOGE("Failed to convert	text... %d\n", 0);
		wcscpy(tstr, L"<Failed to convert text>");
	}
	else
	{
		size_t outSize = std::mbstowcs(tstr, utf8String, sSize);
		if (outSize == -1)
		{
			LOGE("Failed to convert text... %d\n", 1);
			wcscpy(tstr, L"<Failed to convert text>");
		}
		else
			tstr[outSize + 1] = L'\0';
	}

#else
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size() + 1, tstr, maxLength);
	if (Length >= 0)
		tstr[Length] = _T('\0');
#endif
	return tstr;
}
#else
char* Edif::ConvertString(const std::string_view & utf8String)
{
#ifndef _WIN32
	char* str = strdup(utf8String.data());

	return str;
#else
	// Convert string to Unicode
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), 0, 0);
	wchar_t * wstr = (wchar_t *)calloc(++Length, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), wstr, Length);
	assert(wstr >= 0);

	// Convert Unicode string using current user code page
	int len2 = WideCharToMultiByte(CP_ACP, 0, wstr, -1, 0, 0, nullptr, nullptr);
	assert(len2 >= 0);
	++len2;
	char* str = (char*)calloc(len2, sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len2, nullptr, nullptr);
	free(wstr);

	return str;
#endif
}

char* Edif::ConvertAndCopyString(char* str, const std::string_view & utf8String, int maxLength)
{
#ifndef _WIN32
	std::size_t siz = std::min<std::size_t>(maxLength - 1, utf8String.size());
	memcpy(str, utf8String.data(), siz);
	str[siz] = '\0';
	return _strdup(str);
#else
	// MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, maxLength);

	// Convert string to Unicode
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), 0, 0);
	assert(Length >= 0);
	WCHAR* wstr = (WCHAR*)calloc(++Length, sizeof(WCHAR));
	Length = MultiByteToWideChar(CP_UTF8, 0, utf8String.data(), utf8String.size(), wstr, Length);
	assert(Length >= 0);

	// Convert Unicode string using current user code page
	Length = WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, maxLength, nullptr, nullptr);
	assert(Length >= 0);
	free(wstr);

	return str;
#endif
}
#endif // _UNICODE


#if defined(_DEBUG)


Edif::recursive_mutex::recursive_mutex()
{
	this->log << "New recursive mutex.\n"sv;
}
Edif::recursive_mutex::~recursive_mutex()
{
	this->log << "Recursive mutex dying.\n"sv;
}

void Edif::recursive_mutex::lock(edif_lock_debugParams)
{
	try {
		if (!this->intern.try_lock_for(std::chrono::milliseconds(500)))
		{
			std::string log2 = this->log.str();
			//if (log2.size() > 600)
			//	log2 = log2.substr(log2.size() - 500, 500);
			log2 += "Couldn't append in "sv;
			log2 += func;
			log2 += ", line "sv;
			log2 += std::to_string(line);
			log2 += '\n';
			OutputDebugStringA(log2.c_str());
			DarkEdif::BreakIfDebuggerAttached();
			throw std::runtime_error("timeout");
		}
	}
	catch (std::runtime_error err)
	{
		char exc_addr[128];
		sprintf_s(exc_addr, std::size(exc_addr), "crashlog%p.txt", this);
		FILE * f = fopen(exc_addr, "wb");
		if (f == NULL) {
			LOGF(_T("Failed to write log file, error %d.\n"), errno);
		}
		else
		{
			this->log << "FAILED TO LOCK in function "sv << func << ", file "sv << file << ", line "sv << line << ", error "sv << err.what() << ".\n"sv;
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
			LOGE(_T("%s"), DarkEdif::UTF8ToTString(str).c_str());
			throw err;
		}
	}
	this->log << "Locked in function "sv << func << ", line "sv << line << ".\n"sv;
}
bool Edif::recursive_mutex::try_lock(edif_lock_debugParams)
{
	bool b = false;
	try {
		b = this->intern.try_lock();
	}
	catch (std::system_error err)
	{
		char exc_addr[128];
		sprintf_s(exc_addr, std::size(exc_addr), "crashlog%p.txt", this);
		FILE* f = fopen(exc_addr, "wb");
		if (f == NULL) {
			LOGF(_T("Failed to write log file, error %d."), errno);
		}
		else
		{
			this->log << "FAILED TO TRY LOCK in function "sv << func << ", line "sv << line << ", error "sv << err.what() << ".\n"sv;
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
			LOGE(_T("%s"), DarkEdif::UTF8ToTString(str).c_str());
			throw err;
		}
	}
	// this->log isn't safe to use if we don't have the lock
	if (b)
		this->log << "Try lock OK in function "sv << func << ", line "sv << line << ".\n"sv;
	return b;
}
void Edif::recursive_mutex::unlock(edif_lock_debugParams)
{
	this->log << "Unlocked in function " << func << ", line " << line << ".\n";
	try {
#ifndef __analysis_assume_lock_acquired
#define __analysis_assume_lock_acquired(x) /* no op */
#endif
		__analysis_assume_lock_acquired(this->intern);
		this->intern.unlock();
	}
	catch (std::system_error err)
	{
		char exc_addr[128];
		sprintf_s(exc_addr, std::size(exc_addr), "crashlog%p.txt", this);
		FILE* f = fopen(exc_addr, "wb");
		if (f == NULL) {
			LOGF(_T("Failed to write log file, error %d.\n"), errno);
		}
		else
		{
			this->log << "FAILED TO UNLOCK in function "sv << func << ", line "sv << line << ", error "sv << err.what() << ".\n"sv;
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
			LOGE(_T("%s"), DarkEdif::UTF8ToTString(str).c_str());
			throw err;
		}
	}
}

#else // Not debug

Edif::recursive_mutex::recursive_mutex()
{
}
Edif::recursive_mutex::~recursive_mutex()
{
}
void Edif::recursive_mutex::lock(edif_lock_debugParams)
{
	this->intern.lock();
}
bool Edif::recursive_mutex::try_lock(edif_lock_debugParams)
{
	return this->intern.try_lock();
}
void Edif::recursive_mutex::unlock(edif_lock_debugParams)
{
	this->intern.unlock();
}

#endif // _DEBUG
