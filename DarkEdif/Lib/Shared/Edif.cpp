#include "Common.h"
#include "Extension.h"
#include "DarkEdif.h"

// Used for Win32 resource ID numbers
#include "Resource.h"

// ============================================================================
// GLOBAL DEFINES
// Contains the definitions of all the Edif class global variables.
// ============================================================================

// Global SDK pointer
Edif::SDK * SDK = nullptr;

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

Params ReadActionOrConditionParameterType(const char * Text, bool &IsFloat)
{
	if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
		return Params::String_Expression;

	if (!_stricmp(Text, "Filename") || !_stricmp(Text, "File"))
		return Params::Filename;

	if (!_stricmp(Text, "Float"))
	{
		IsFloat = true;
		return Params::Expression;
	}

	if (!_stricmp(Text, "Integer"))
		return Params::Expression;

	if (!_stricmp(Text, "Unsigned Integer"))
		return Params::Expression;

	if (!_stricmp(Text, "Object"))
		return Params::Object;

	if (!_stricmp(Text, "Position"))
		return Params::Position;

	if (!_stricmp(Text, "Create"))
		return Params::Create;

	if (!_stricmp(Text, "SysCreate"))
		return Params::System_Create;

	if (!_stricmp(Text, "Animation"))
		return Params::Animation;

	if (!_stricmp(Text, "Nop"))
		return Params::NoP;

	if (!_stricmp(Text, "Player"))
		return Params::Player;

	if (!_stricmp(Text, "Every"))
		return Params::Every;

	if (!_stricmp(Text, "Key"))
		return Params::Key;

	if (!_stricmp(Text, "Speed"))
		return Params::Speed;

	if (!_stricmp(Text, "JoyDirection"))
		return Params::Joystick_Direction;

	if (!_stricmp(Text, "Shoot"))
		return Params::Shoot;

	if (!_stricmp(Text, "Zone"))
		return Params::Playfield_Zone;

	if (!_stricmp(Text, "Comparison"))
		return Params::Comparison;

	if (!_stricmp(Text, "StringComparison"))
		return Params::String_Comparison;

	if (!_stricmp(Text, "Colour") || !_stricmp(Text, "Color"))
		return Params::Colour;

	if (!_stricmp(Text, "Frame"))
		return Params::Frame;

	if (!_stricmp(Text, "SampleLoop"))
		return Params::Sample_Loop;

	if (!_stricmp(Text, "MusicLoop"))
		return Params::Music_Loop;

	if (!_stricmp(Text, "NewDirection"))
		return Params::New_Direction;

	if (!_stricmp(Text, "TextNumber"))
		return Params::Text_Number;

	if (!_stricmp(Text, "Click"))
		return Params::Click;

	if (!_stricmp(Text, "Program"))
		return Params::Program;

	if (!_strnicmp(Text, "Custom", sizeof("Custom") - 1))
		return (Params)((short)Params::Custom_Base + ((short)atoi(Text + sizeof("Custom") - 1)));

	DarkEdif::MsgBox::Error(_T("DarkEdif Params error"), _T("Error reading parameter type \"%s\", couldn't match it to a Params value."), DarkEdif::UTF8ToTString(Text).c_str());
	return (Params)(std::uint16_t)0;
}

ExpParams ReadExpressionParameterType(const char * Text, bool &IsFloat)
{
	if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
		return ExpParams::String;

	if (!_stricmp(Text, "Float"))
	{
		IsFloat = true;
		return ExpParams::Float;
	}

	if (!_stricmp(Text, "Integer"))
		return ExpParams::Integer;

	if (!_stricmp(Text, "Unsigned Integer"))
		return ExpParams::UnsignedInteger;

	DarkEdif::MsgBox::Error(_T("DarkEdif ExpParams error"), _T("Error reading expression parameter type \"%s\", couldn't match it to a ExpParams value."), DarkEdif::UTF8ToTString(Text).c_str());
	return (ExpParams)(std::uint16_t)0;
}

ExpReturnType ReadExpressionReturnType(const char * Text)
{
	if (!_stricmp(Text, "Integer"))
		return ExpReturnType::Integer;

	if (!_stricmp(Text, "Float"))
		return ExpReturnType::Float;

	if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
		return ExpReturnType::String;

	// More specialised, but not allowed for
	if (!_stricmp(Text, "Short"))
		return ExpReturnType::Integer;

	if (!_stricmp(Text, "Unsigned Integer"))
		return ExpReturnType::UnsignedInteger;

	DarkEdif::MsgBox::Error(_T("DarkEdif ExpReturnType error"), _T("Error reading expression return type \"%s\", couldn't match it to a ExpReturnType value."), DarkEdif::UTF8ToTString(Text).c_str());
	return ExpReturnType::Integer; // default
}

void Edif::Init(mv * mV, EDITDATA * edPtr)
{
#ifdef _WIN32
	::SDK->mV = mV;

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

int Edif::Init(mv * mV, bool fusionStartupScreen)
{
	_tcscpy (LanguageCode, _T ("EN"));

	// We want DarkEdif::MsgBox::XX as soon as possible.
	// Main thread ID is used to prevent crashes from message boxes not being task-modal.
	// Since we're initializing this, might as well set all the DarkEdif mV variables.
	DarkEdif::MainThreadID = std::this_thread::get_id();
#ifdef _WIN32
	DarkEdif::IsFusion25 = ((mV->GetVersion() & MMFVERSION_MASK) == CFVERSION_25);
	DarkEdif::Internal_WindowHandle = mV->HMainWin;
#endif

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
#else // Not editor build, missing things that will let Fusion use it in editor.
	DarkEdif::RunMode = DarkEdif::MFXRunMode::BuiltEXE;
#endif

#ifdef _WIN32
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

		int nCode = _ttoi(langCode);
		switch (nCode) {
			case 0x40C: // MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH);
				_tcscpy (LanguageCode, _T ("FR"));
				break;
			case 0x411: // MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
				_tcscpy (LanguageCode, _T ("JP"));
				break;
		}

		// Free resources
		FreeLibrary(hRes);
	}
#endif

	// Get JSON file
	char * JSON;
	size_t JSON_Size;

	int result = Edif::GetDependency (JSON, JSON_Size, _T("json"), IDR_EDIF_JSON);

	if (result == Edif::DependencyNotFound)
		return DarkEdif::MsgBox::Error(_T("Couldn't find JSON"), _T("JSON file for " PROJECT_NAME " not found on disk or in MFX resources")), -1;

	Edif::ExternalJSON = (result == Edif::DependencyWasFile);

	char * copy = (char *) malloc (JSON_Size + 1);
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

	// Workaround for subapp bug (cheers LB), where Init/Free is called more than once,
	// even if object is not in subapp and thus doesn't apply
	// http://community.clickteam.com/threads/97219-MFX-not-found-when-in-subapp?p=693431#post693431
#if !defined(_WIN32) || WINVER >= 0x0600 || !defined(__cpp_threadsafe_static_init)
	static Edif::SDK gSDK(mV, *json);
	::SDK = &gSDK;
#else
	// Static local initialization bug. For more detail, see the MultiTarget MD file on XP targeting.
	// On XP, the code above zero-fills gSDK, and doesn't run the constructor, resulting in a
	// crash later, when GetRunObjectInfos() tries to use the null ::SDK->json via "CurLang".
	::SDK = new Edif::SDK(mV, *json);
#endif

#ifdef INTENSE_FUNCTIONS_EXTENSION
	Extension::AutoGenerateExpressions(*::SDK);
#endif

	return 0;	// no error
}

#if EditorBuild
// Used for reading the icon image file
FusionAPIImport BOOL FusionAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);

#endif

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;

	if (!::SDK)
		::SDK = this;

#if EditorBuild
	cSurface * proto = nullptr;
	if (GetSurfacePrototype(&proto, 32, (int)SurfaceType::Memory_DeviceContext, (int)SurfaceDriver::Bitmap) == FALSE)
		DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Getting surface prototype failed."));

	Icon = new cSurface();
	if (mV->ImgFilterMgr)
	{
		char * IconData;
		size_t IconSize;

		int result = Edif::GetDependency (IconData, IconSize, _T("png"), IDR_EDIF_ICON);
		if (result != Edif::DependencyNotFound)
		{
			CInputMemFile * File = CInputMemFile::NewInstance();
			File->Create((LPBYTE)IconData, IconSize);

			std::unique_ptr<cSurface> tempIcon = std::make_unique<cSurface>();
			ImportImageFromInputFile(mV->ImgFilterMgr, File, tempIcon.get(), NULL, 0);

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

			if (tempIcon->Blit(*Icon) == FALSE)
				DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Blitting to ext icon surface failed. Last error: %i."), tempIcon->GetLastError());
		}
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
#endif // EditorBuild

	if (CurLang.type != json_object)
	{
		DarkEdif::MsgBox::Error(_T("Internal JSON error"), _T("The JSON parser could not find a JSON object to use for current language."));
		return;
	}

	const json_value &Actions = CurLang["Actions"];
	const json_value &Conditions = CurLang["Conditions"];
	const json_value &Expressions = CurLang["Expressions"];

#ifdef _WIN32
	ActionJumps = new void * [Actions.u.object.length + 1];
	ConditionJumps = new void * [Conditions.u.object.length + 1];
	ExpressionJumps = new void * [Expressions.u.object.length + 1];

	ActionJumps [Actions.u.object.length] = 0;
	ConditionJumps [Conditions.u.object.length] = 0;
	ExpressionJumps [Expressions.u.object.length] = 0;
#endif

	for (size_t i = 0; i < Actions.u.object.length; ++ i)
	{
	#ifdef _WIN32
		ActionJumps [i] = (void *) Edif::ActionJump;
	#endif

		ActionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewActionInfo();
	}

	for (size_t i = 0; i < Conditions.u.object.length; ++ i)
	{
	#ifdef _WIN32
		ConditionJumps [i] = (void *) Edif::ConditionJump;
	#endif

		ConditionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewConditionInfo();
	}

	for (size_t i = 0; i < Expressions.u.object.length; ++ i)
	{
	#ifdef _WIN32
		ExpressionJumps [i] = (void *) Edif::ExpressionJump;
	#endif

		ExpressionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewExpressionInfo();
	}

#if EditorBuild
	// Object properties, as they appear in Properties tab, in the frame editor only.
	DarkEdif::DLL::GeneratePropDataFromJSON();

	ActionMenu = LoadMenuJSON(Edif::ActionID(0), CurLang["ActionMenu"]);
	ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), CurLang["ConditionMenu"]);
	ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), CurLang["ExpressionMenu"]);

	// Check for ext dev forgetting to overwrite some of the Template properties
	#if defined(_DEBUG) && !defined(IS_DARKEDIF_TEMPLATE)
		const json_value & about = CurLang["About"];
		bool unchangedPropsFound =
			!_stricmp(about["Name"], "DarkEdif Template") ||
			!_stricmp(about["Author"], "Your Name") ||
			!_stricmp(about["Comment"], "A sentence or two to describe your extension") ||
			!_stricmp(about["Help"], "Help/Example.chm") ||
			!_stricmp(about["URL"], "https://www.example.com/");
		if (!unchangedPropsFound)
		{
			std::string copy = about["Copyright"];
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

	#ifdef INTENSE_FUNCTIONS_EXTENSION
		Extension::AutoGenerateExpressions();
	#endif
#endif // EditorBuild
}

Edif::SDK::~SDK()
{
	OutputDebugStringA("Edif::SDK::~SDK() call.\r\n");
	json_value_free (&json);

#if EditorBuild
	delete [] ActionJumps;
	delete [] ConditionJumps;
	delete [] ExpressionJumps;

	delete Icon;
#endif
}

long ActionOrCondition(void * Function, int ID, Extension * ext, const ACEInfo * info, ACEParamReader &params, bool isCond)
{
	int ParameterCount = info->NumOfParams;
	long Parameters[16];
	long Result = 0L;
#if defined(__arm__) && defined(__ANDROID__)
	long argStackCount; // Must be declared here or error reports in param reading won't compile
#elif defined(_WIN32)
	// Reset by CNC_GetParam inside params.GetXX(). CurrentParam being correct only matters if you have object parameters, though.
	EventParam* saveCurParam = ext->rdPtr->rHo.CurrentParam;
#endif

	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value & numAutoProps = CurLang[isCond ? "Conditions" : "Actions"][ID]["NumAutoProps"];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	bool isComparisonCondition = false;

	for (int i = 0; i < ParameterCount; ++ i)
	{
		switch (info->Parameter[i].p)
		{
			case Params::Expression:
				if (info->FloatFlags & (1 << i))
				{
					float f = params.GetFloat(i);
					Parameters[i] = *(int*)(&f);
				}
				else
					Parameters[i] = params.GetInteger(i);
				break;

			case Params::String_Comparison:
			case Params::String_Expression:
			case Params::Filename:
				Parameters[i] = (long)params.GetString(i);
				// Catch null string parameters and return default 0
				if (!Parameters[i])
				{
					DarkEdif::MsgBox::Error(_T("ActionOrCondition() error"),
						_T("Error calling %s \"%s\" (ID %i); text parameter index %i was given a null string pointer.\n"
							"Was the parameter type different when the %s was created in the MFA?"),
						isCond ? _T("condition") : _T("action"),
						DarkEdif::UTF8ToTString((const char *)CurLang[isCond ? "Conditions" : "Actions"][ID]["Title"]).c_str(),
						ID, i,
						isCond ? _T("condition") : _T("action"));
					goto endFunc;
				}

				isComparisonCondition |= info->Parameter[i].p == Params::String_Comparison;
				break;

			case Params::Compare_Time:
			case Params::Comparison:
				Parameters[i] = params.GetInteger(i);
				isComparisonCondition = true;
				break;

			default:
				Parameters[i] = params.GetInteger(i);
				break;
		}
	}

#ifdef _WIN32
	ext->rdPtr->rHo.CurrentParam = saveCurParam;
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

	// Comparisons return an integer or string pointer, pass as-is
	if (isComparisonCondition)
		return Result;

	// Bool returns aren't 0x0 or 0x1, they botch the other 24 bits.
	return (long)*(bool *)&Result;
}

#ifdef _WIN32
struct ConditionOrActionManager_Windows : ACEParamReader
{
	RUNDATA * rdPtr;
	ConditionOrActionManager_Windows(bool isCondition, RUNDATA * rdPtr, long param1, long param2)
		: rdPtr(rdPtr)
	{
		rdPtr->pExtension->Runtime.param1 = param1;
		rdPtr->pExtension->Runtime.param2 = param2;
	}
	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		int i = (int)CNC_GetFloatParameter(rdPtr);
		return *(float*)&i;
	}

	virtual const TCHAR * GetString(int index)
	{
		return (const TCHAR *)CNC_GetStringParameter(rdPtr);
	}

	virtual std::int32_t GetInteger(int index)
	{
		return (std::int32_t)CNC_GetIntParameter(rdPtr);
	}
};

#elif defined(__ANDROID__)

typedef jobject CRun;

struct ConditionOrActionManager_Android : ACEParamReader
{
	Extension * ext;
	jobject javaActOrCndObj;
	bool isCondition;

	ConditionOrActionManager_Android(bool isCondition, Extension * ext, jobject javaActOrCndObj)
		: ext(ext), javaActOrCndObj(javaActOrCndObj), isCondition(isCondition)
	{
		stringIndex = 0;
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		float f = (isCondition ? ext->runFuncs.cnd_getParamExpFloat : ext->runFuncs.act_getParamExpFloat)(ext->javaExtPtr, javaActOrCndObj);
		LOGV("Got float param, cond=%d, index %d OK: %f.\n", isCondition ? 1 : 0, index, f);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		const TCHAR * str = trackString((isCondition ? ext->runFuncs.cnd_getParamExpString : ext->runFuncs.act_getParamExpString)(ext->javaExtPtr, javaActOrCndObj));
		LOGV("Got string param, cond=%d, index %d OK: \"%s\".\n", isCondition ? 1 : 0, index, str);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		std::int32_t in = (isCondition ? ext->runFuncs.cnd_getParamExpression : ext->runFuncs.act_getParamExpression)(ext->javaExtPtr, javaActOrCndObj);
		LOGV("Got integer param, cond=%d, index %d OK: %d.\n", isCondition ? 1 : 0, index, in);
		return in;
	}

	RuntimeFunctions::string strings[16];
	int stringIndex;

	inline const char * trackString(RuntimeFunctions::string s)
	{
		strings[stringIndex++] = s;
		return s.ptr;
	}

	~ConditionOrActionManager_Android()
	{
		while (--stringIndex >= 0)
			ext->runFuncs.freeString(ext, strings[stringIndex]);
	}
};
#else

typedef void * CRun;

extern "C"
{
	void DarkEdif_generateEvent(void * ext, int code, int param);
	void DarkEdif_reHandle(void * ext);

	int DarkEdif_actGetParamExpression(void * ext, void * act, int paramNum);
	const char * DarkEdif_actGetParamExpString(void * ext, void * act, int paramNum);
	double DarkEdif_actGetParamExpDouble(void * ext, void * act, int paramNum);

	int DarkEdif_cndGetParamExpression(void * ext, void * cnd, int paramNum);
	const char * DarkEdif_cndGetParamExpString(void * ext, void * cnd, int paramNum);
	double DarkEdif_cndGetParamExpDouble(void * ext, void * cnd, int paramNum);
	bool DarkEdif_cndCompareValues(void * ext, void * cnd, int paramNum);
	bool DarkEdif_cndCompareTime(void * ext, void * cnd, int paramNum);

	int DarkEdif_expGetParamInt(void * ext);
	const char * DarkEdif_expGetParamString(void * ext);
	float DarkEdif_expGetParamFloat(void * ext);

	void DarkEdif_expSetReturnInt(void * ext, int toRet);
	void DarkEdif_expSetReturnString(void * ext, const char * toRet);
	void DarkEdif_expSetReturnFloat(void * ext, float toRet);

	void DarkEdif_freeString(void * ext, const char * cstr);
}

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
		double f = (isCondition ? DarkEdif_cndGetParamExpDouble : DarkEdif_actGetParamExpDouble)(ext->objCExtPtr, objCActOrCndObj, index);
		LOGV("Got float param, cond=%d, index %d OK: %f.\n", isCondition ? 1 : 0, index, f);
		return (float)f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		const TCHAR* str = (isCondition ? DarkEdif_cndGetParamExpString : DarkEdif_actGetParamExpString)(ext->objCExtPtr, objCActOrCndObj, index);
		LOGV("Got string param, cond=%d, index %d OK: \"%s\".\n", isCondition ? 1 : 0, index, str);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, cond=%d, index %d.\n", isCondition ? 1 : 0, index);
		std::int32_t in = (isCondition ? DarkEdif_cndGetParamExpression : DarkEdif_actGetParamExpression)(ext->objCExtPtr, objCActOrCndObj, index);
		LOGV("Got integer param, cond=%d, index %d OK: %i.\n", isCondition ? 1 : 0, index, in);
		return in;
	}

	~ConditionOrActionManager_iOS()
	{
	}
};
#endif

#ifdef _WIN32
long FusionAPI Edif::ConditionJump(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * ext = rdPtr->pExtension;
	int ID = rdPtr->rHo.EventNumber;
	ConditionOrActionManager_Windows params(true, rdPtr, param1, param2);
#elif defined(__ANDROID__)
ProjectFunc jlong conditionJump(JNIEnv *, jobject, jlong extPtr, int ID, CCndExtension cndExt)
{
	Extension * ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(true, ext, (jobject)cndExt);
#else
ProjectFunc long PROJ_FUNC_GEN(PROJECT_NAME_RAW, _conditionJump(void * cppExtPtr, int ID, CCndExtension cndExt))
{
	Extension* ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(true, ext, cndExt);
#endif
	LOGV(_T("Condition ID %i start.\n"), ID);

	if (::SDK->ConditionFunctions.size() < (unsigned int)ID) {
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("Missing condition ID %d in extension %s. This ID was not linked in Extension ctor with LinkCondition()."),
			ID, _T("" PROJECT_NAME));
		return ext->UnlinkedCondition(ID);
	}

	void * Function = ::SDK->ConditionFunctions[ID];

	if (!Function) {
		DarkEdif::MsgBox::Error(_T("Condition linking error"), _T("Missing condition ID %d in extension %s. Condition existed in vector, but was NULL. Might not be linked."),
			ID, _T("" PROJECT_NAME));
		return ext->UnlinkedCondition(ID);
	}

	long Result = ActionOrCondition(Function, ID, ext, ::SDK->ConditionInfos[ID], params, true);

	LOGV(_T("Condition ID %i end.\n"), ID);
	return Result;
}

// For some reason, actions are expected to return a short.
// On Android/iOS, we make the Fusion wrapper, so we can return void instead.
// To handle that, the "actreturn"	hack is used.

#ifdef _WIN32
short FusionAPI Edif::ActionJump(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * ext = rdPtr->pExtension;
	/* int ID = rdPtr->rHo.hoAdRunHeader->rh4.rh4ActionStart->evtNum; */
	int ID = rdPtr->rHo.EventNumber;
	ConditionOrActionManager_Windows params(false, rdPtr, param1, param2);
#define actreturn 0
#elif defined (__ANDROID__)
ProjectFunc void actionJump(JNIEnv *, jobject, jlong extPtr, jint ID, CActExtension act)
{
	Extension * ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(false, ext, act);
#define actreturn /* void */
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _actionJump(void * cppExtPtr, int ID, CActExtension act))
{
	Extension* ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(false, ext, act);
#define actreturn /* void */
#endif
	LOGV(_T("Action ID %i start.\n"), ID);

	if (::SDK->ActionFunctions.size() < (unsigned int)ID)
	{
		ext->UnlinkedAction(ID);
		return actreturn;
	}
	void * Function = ::SDK->ActionFunctions[ID];

	if (!Function)
	{
		ext->UnlinkedAction(ID);
		return actreturn;
	}

	ActionOrCondition(Function, ID, ext, ::SDK->ActionInfos[ID], params, false);

	LOGV(_T("Action ID %i end.\n"), ID);
	return actreturn;
#undef actreturn
}

#ifdef __ANDROID__

typedef jobject CNativeExpInstance;
typedef jobject CRun;
#include <jni.h>
struct ExpressionManager_Android : ACEParamReader {
	CNativeExpInstance expJavaObj;
	Extension * ext;

	RuntimeFunctions::string strings[16];
	int stringIndex;

	inline const char * trackString(RuntimeFunctions::string s)
	{
		strings[stringIndex++] = s;
		return s.ptr;
	}

	ExpressionManager_Android(Extension * ext, CNativeExpInstance expJavaObj) :
		 expJavaObj(expJavaObj), ext(ext)
	{
		stringIndex = 0;
	}
	void SetValue(int a) {
		ext->runFuncs.exp_setReturnInt(ext->javaExtPtr, expJavaObj, a);
	}
	void SetValue(float a) {
		ext->runFuncs.exp_setReturnFloat(ext->javaExtPtr, expJavaObj, a);
	}
	void SetValue(const char * a) {
		LOGV("Setting expression return as text...\n");
		ext->runFuncs.exp_setReturnString(ext->javaExtPtr, expJavaObj, a);
		LOGV("Setting expression return as text \"%s\" OK.\n", a);
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, expr, index %d OK.\n", index);
		float f = ext->runFuncs.exp_getParamFloat(ext->javaExtPtr, expJavaObj);
		LOGV("Got float param, expr, index %d OK.\n", index);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.\n", index);
		const TCHAR * str = trackString(ext->runFuncs.exp_getParamString(ext->javaExtPtr, expJavaObj));
		LOGV("Got string param, expr, index %d OK.\n", index);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, expr, index %d OK.\n", index);
		std::int32_t i = ext->runFuncs.exp_getParamInt(ext->javaExtPtr, expJavaObj);
		LOGV("Got integer param, expr, index %d OK.\n", index);
		return i;
	}

	void SetReturnType(ExpReturnType rt)
	{
		// Do nothing. We only care on Windows.
	}

	~ExpressionManager_Android() {
		while (--stringIndex >= 0)
			ext->runFuncs.freeString(ext, strings[stringIndex]);
	}
};

#elif defined(_WIN32)

struct ExpressionManager_Windows : ACEParamReader {
	RUNDATA * rdPtr;
	ExpressionManager_Windows(RUNDATA * rdPtr) : rdPtr(rdPtr)
	{

	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index) override
	{
		int i = (int)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_FLOAT, index);
		return *(float*)&i;
	}

	virtual const TCHAR * GetString(int index) override
	{
		// We don't have to free this memory; Fusion will manage it
		return (const TCHAR *)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_STRING, index);
	}

	virtual std::int32_t GetInteger(int index) override
	{
		return (std::int32_t)CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1 + (index > 0), TYPE_INT, index);
	}

	void SetReturnType(ExpReturnType rt)
	{
		if (rt == ExpReturnType::Float)
			rdPtr->rHo.Flags |= HeaderObjectFlags::Float;
		else if (rt == ExpReturnType::String)
			rdPtr->rHo.Flags |= HeaderObjectFlags::String;
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

class CValue;
struct ExpressionManager_iOS : ACEParamReader {
	Extension* ext;
	void * expObjCObj;

	RuntimeFunctions::string strings[16];

	ExpressionManager_iOS(Extension* ext) :
		ext(ext)
	{
	}
	void SetValue(int a) {
		DarkEdif_expSetReturnInt(ext->objCExtPtr, a);
	}
	void SetValue(float a) {
		DarkEdif_expSetReturnFloat(ext->objCExtPtr, a);
	}
	void SetValue(const char* a) {
		DarkEdif_expSetReturnString(ext->objCExtPtr, a);
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, expr, index %d OK.\n", index);
		float f = DarkEdif_expGetParamFloat(ext->objCExtPtr);
		LOGV("Got float param, expr, index %d OK: %f.\n", index, f);
		return f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.\n", index);
		const TCHAR* str = DarkEdif_expGetParamString(ext->objCExtPtr);
		LOGV("Got string param, expr, index %d OK: \"%s\".\n", index, str);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, expr, index %d OK.\n", index);
		std::int32_t i = DarkEdif_expGetParamInt(ext->objCExtPtr);
		LOGV("Got integer param, expr, index %d OK: %d.\n", index, i);
		return i;
	}

	void SetReturnType(ExpReturnType rt)
	{
		// Do nothing. We only care on Windows.
	}

	~ExpressionManager_iOS() {
	}
};
#endif

#ifdef _WIN32
long FusionAPI Edif::ExpressionJump(RUNDATA * rdPtr, long param)
{
	int ID = rdPtr->rHo.EventNumber;
	Extension * ext = (Extension *)rdPtr->pExtension;
	ExpressionManager_Windows params(rdPtr);
	LOGV(_T("Expression ID %i start.\n"), ID);
#elif defined(__ANDROID__)
ProjectFunc void expressionJump(JNIEnv *, jobject, jlong extPtr, jint ID, CNativeExpInstance expU)
{
	Extension * ext = (Extension *)extPtr;
	ExpressionManager_Android params(ext, expU);
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _expressionJump(void * cppExtPtr, int ID))
{
	Extension* ext = (Extension*)cppExtPtr;
	ExpressionManager_iOS params(ext);
#endif

	if (::SDK->ExpressionFunctions.size() < (unsigned int)ID)
		return params.SetValue((int)ext->UnlinkedExpression(ID));

	void * Function = ::SDK->ExpressionFunctions[ID];

	if (!Function)
		return params.SetValue((int)ext->UnlinkedExpression(ID));

	const ACEInfo * info = ::SDK->ExpressionInfos[ID];
	ExpReturnType ExpressionRet = info->Flags.ef;

	int ParameterCount = info->NumOfParams;

	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX Expression macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value & numAutoProps = CurLang["Expressions"][ID]["NumAutoProps"];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	long Parameters[16];
	std::uintptr_t Result = 0;

	int ExpressionRet2 = (int)ExpressionRet;
#ifdef _WIN32
	//int ExpressionRet2 = (int)ExpressionRet; // easier for ASM
#else
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
					(const char *)CurLang["Expressions"][ID]["Title"], ID, i);

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
				Parameters[i] = params.GetInteger(i);
			break;
		default:
		{
			DarkEdif::MsgBox::Error(_T("Edif::Expression() error"), _T("Error calling expression \"%s\" (ID %i); parameter index %i has unrecognised ExpParams %hi."),
				(const char *)CurLang["Expressions"][ID]["Title"], ID, i, (short)info->Parameter[i].ep);
			if (ExpressionRet == ExpReturnType::String)
				Result = (long)ext->Runtime.CopyString(_T(""));
			goto endFunc;
		}
		}
	}

#ifdef INTENSE_FUNCTIONS_EXTENSION
	_CrtCheckMemory();

	if (::SDK->ExpressionFunctions[ID] == Edif::MemberFunctionPointer(&Extension::VariableFunction))
	{
		Result = ext->VariableFunction((TCHAR *)Parameters[paramInc], ID, &Parameters[1 + paramInc]);
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
	LOGV(_T("Expression ID %i end.\n"), ID);
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
		Buffer[Size] = 0;

		fread(Buffer, 1, Size, File);

		fclose(File);

		return DependencyWasFile;
	}

	if (!Resource)
		return DependencyNotFound;

	HRSRC res = FindResource (hInstLib, MAKEINTRESOURCE (Resource), _T("EDIF"));

	if (!res)
		return DependencyNotFound;

	Size = SizeofResource (hInstLib, res);
	Buffer = (char *) LockResource (LoadResource (hInstLib, res));

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

#define COMBINE(a,b) a ## b
	Buffer = (char *)(void *)COMBINE(PROJECT_NAME_RAW, darkExtJSON);
	Size = COMBINE(PROJECT_NAME_RAW, darkExtJSONSize);
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
	TCHAR * Extension = (TCHAR *)
		alloca ((_tcslen (FileExtension) + _tcslen (LanguageCode) + 2) * sizeof(TCHAR));

	_tcscpy (Extension, LanguageCode);
	_tcscat (Extension, _T ("."));
	_tcscat (Extension, FileExtension);

	::GetSiblingPath (Buffer, Extension);

	if (*Buffer)
		return;

	::GetSiblingPath (Buffer, FileExtension);
}

#endif

#ifdef _UNICODE
wchar_t * Edif::ConvertString(const char* utf8String)
{
#ifndef _WIN32
	std::string s(utf8String);
	std::wstring ws(s.size(), L' '); // Overestimate number of code points.
	ws.resize(std::mbstowcs(&ws[0], s.c_str(), s.size())); // Shrink to fit.
	return wcsdup(ws.c_str());
#else
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, 0, 0);
	if ( Length == 0 )
		Length = 1;
	wchar_t * tstr = (wchar_t *)calloc(Length, sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, Length);
	return tstr;
#endif
}

wchar_t * Edif::ConvertAndCopyString(wchar_t * tstr, const char* utf8String, int maxLength)
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
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, maxLength);
#endif
	return tstr;
}
#else
char* Edif::ConvertString(const char* utf8String)
{
#ifndef _WIN32
	char* str = strdup(utf8String);

	return str;
#else
	// Convert string to Unicode
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, 0, 0);
	if ( Length == 0 )
		Length = 1;
	wchar_t * wstr = (wchar_t *)calloc(Length, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, wstr, Length);

	// Convert Unicode string using current user code page
	int len2 = WideCharToMultiByte(CP_ACP, 0, wstr, -1, 0, 0, nullptr, nullptr);
	if ( len2 == 0 )
		len2 = 1;
	char* str = (char*)calloc(len2, sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len2, nullptr, nullptr);
	free(wstr);

	return str;
#endif
}

char* Edif::ConvertAndCopyString(char* str, const char* utf8String, int maxLength)
{
#ifndef _WIN32
	return _strdup(utf8String);
#else
	// MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, maxLength);

	// Convert string to Unicode
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, 0, 0);
	if ( Length == 0 )
		Length = 1;
	WCHAR* wstr = (WCHAR*)calloc(Length, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, wstr, Length);

	// Convert Unicode string using current user code page
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, maxLength, nullptr, nullptr);
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
