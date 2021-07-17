#include "Common.h"

Edif::SDK * SDK = nullptr;


TCHAR Edif::LanguageCode[3];

bool Edif::IsEdittime;
bool Edif::ExternalJSON;

#ifdef __ANDROID__
#include <unistd.h>
#include <android/log.h>
#include <dlfcn.h>
// Do not use everywhere! JNIEnv * are thread-specific. Use Edif::Runtime JNI functions to get a thread-local one.
JNIEnv * mainThreadJNIEnv;
JavaVM * global_vm;
#elif defined(__APPLE__)
#include <unistd.h>
#include <dlfcn.h>
#endif

std::string Edif::CurrentFolder()
{
	char result[PATH_MAX];
#ifdef _WIN32
	size_t count = GetModuleFileNameA(hInstLib, result, sizeof(result));
#elif __ANDROID__
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	LOGI("Got %s as the path.", result);
#else
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	printf("Got %s as the path.", result);
#endif
	return std::string(result, count > 0 ? count : 0);
}
// Classname_FuncName().
// No addition parameters, and no Java_ prefix needed.
void Edif::GetExtensionName(char * const writeTo)
{
#ifdef _WIN32
	std::string curFolder = CurrentFolder();
	strcpy(writeTo, strrchr(curFolder.c_str(), '\\') + 1);
	writeTo[strlen(writeTo) - 4] = 0; // Remove ".mfx"
	// ::SDK and json not available.
#else
	strcpy(writeTo, PROJECT_NAME);
#endif
}

void Edif::Log(const char * format, ...)
{
	if (!format[0])
		return;
	va_list v;
	va_start(v, format);

#ifdef _WIN32
	char data[512];
	vsprintf_s(data, format, v);
	OutputDebugStringA(data);
#elif defined (__ANDROID__)
	size_t s = strlen(format);
	std::string format2(format, s - (format[s - 1] == '\n' ? 1 : 0));
	__android_log_vprint(ANDROID_LOG_DEBUG, PROJECT_NAME_UNDERSCORES, format2.c_str(), v);
#else
	size_t s = strlen(format);
	std::string format2(format, s - (format[s - 1] == '\n' ? 1 : 0));
	vprintf(format2.c_str(), v);
#endif

	va_end(v);
}

HMENU Edif::ActionMenu, Edif::ConditionMenu, Edif::ExpressionMenu;

Params ReadParameterType(const char * Text, bool &IsFloat)
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

	if (!_strnicmp(Text, "Custom", 6))
		return (Params)short(short(Params::Custom_Base) + atoi(Text+6));
	
	std::stringstream str;
	str << "Error reading Parameter type \""sv << Text << "\"; text did not match anything."sv;
	MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Error", MB_OK | MB_ICONERROR);
	return (Params)(ushort)0;
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

	std::stringstream str;
	str << "Error reading Parameter type \"" << Text << "\"; text did not match anything.";
	MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Error", MB_OK | MB_ICONERROR);
	return (ExpParams)(ushort)0;
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

	std::stringstream str;
	str << "Error reading expression return; returns \"" << Text << "\"; which is unrecognised.";
	MessageBoxA(NULL, str.str().c_str(), "DarkEdif - Error", MB_OK | MB_ICONERROR);
	return ExpReturnType::Integer; // default
}

void Edif::Init(mv * mV, EDITDATA * edPtr)
{
#ifdef _WIN32
	IsEdittime = mV->HMainWin != 0;

	mvInvalidateObject(mV, edPtr);
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

#include "Resource.h"
#include "Extension.h"

int Edif::Init(mv * mV)
{
	_tcscpy (LanguageCode, _T ("EN"));

	// Get pathname of MMF2
#ifdef _WIN32
	TCHAR * mmfname = (TCHAR *)calloc(MAX_PATH, sizeof(TCHAR));
	if ( mmfname != NULL )
	{
		// Load resources
		GetModuleFileName (NULL, mmfname, MAX_PATH);
		HINSTANCE hRes = LoadLibraryEx(mmfname, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
		if ( hRes != NULL )
		{
			// Load string 720, contains the language code
			TCHAR langCode[20];
			LoadString(hRes, 720, langCode, 20);

			int nCode = _ttoi(langCode);
			switch (nCode) {
				case 0x40C:
					_tcscpy (LanguageCode, _T ("FR"));
					break;
				case 0x411:
					_tcscpy (LanguageCode, _T ("JP"));
					break;
			}

			// Free resources
			FreeLibrary(hRes);
		}
		free(mmfname);
	}
#endif
	
	// Get filename.mfx so we can use it in error messages
	char errorMsgTitle [MAX_PATH];
	Edif::GetExtensionName(errorMsgTitle);
	
	// Get JSON file
	char * JSON;
	size_t JSON_Size;

	int result = Edif::GetDependency (JSON, JSON_Size, _T("json"), IDR_EDIF_JSON);

	if (result == Edif::DependencyNotFound)
	{
		strcat_s(errorMsgTitle, " - Error");
		
		MessageBoxA(0, "JSON file not found on disk or in MFX resources", errorMsgTitle, 0);
		return -1;	// error, init failed
	}
	
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
	{
		strcat_s(errorMsgTitle, " - Error parsing JSON");

		MessageBoxA(0, json_error, errorMsgTitle, MB_OK);
		return -1;
	}

	// Workaround for subapp bug (cheers LB), where Init/Free is called more than once,
	// even if object is not in subapp and thus doesn't apply
	// http://community.clickteam.com/threads/97219-MFX-not-found-when-in-subapp?p=693431#post693431
	static Edif::SDK gSDK (mV, *json);
	::SDK = &gSDK;

#ifdef INTENSE_FUNCTIONS_EXTENSION
	Extension::AutoGenerateExpressions(&gSDK);
#endif
	
	return 0;	// no error
}

#if EditorBuild
// Used for reading the icon image file
PhiDLLImport BOOL FusionAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);

#endif

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;
#ifdef _WIN32
	DarkEdif::IsFusion25 = ((mV->GetVersion() & MMFVERSION_MASK) == CFVERSION_25);
#endif

	#if EditorBuild
		cSurface * proto = nullptr;
		if (GetSurfacePrototype(&proto, 32, ST_MEMORYWITHDC, SD_BITMAP) == FALSE)
			MessageBoxA(NULL, "Getting surface prototype failed.", PROJECT_NAME " - DarkEdif error", MB_ICONERROR);

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
				DWORD PNG = 'PNG ';
				ImportImageFromInputFile(mV->ImgFilterMgr, File, tempIcon.get(), &PNG, 0);

				File->Delete();

				if (!tempIcon->HasAlpha())
					tempIcon->SetTransparentColor(RGB(255, 0, 255));

				if (result != Edif::DependencyWasResource)
					free(IconData);

				Icon->Create(tempIcon->GetWidth(), tempIcon->GetHeight(), proto);

				if (tempIcon->Blit(*Icon) == FALSE)
					MessageBoxA(NULL, "Blitting to ext icon surface failed.", PROJECT_NAME " - DarkEdif error", MB_ICONERROR);
			}
		}

		#if USE_DARKEDIF_UPDATE_CHECKER
		// Is in editor, not EXE using Run Application?
		// Note this check is also done by Edif::Init(mv,edPtr) and stored in Edif::IsEdittime, but Edif::Init(mv,edPtr) is called after this.
		if (mV->HMainWin != 0)
			DarkEdif::SDKUpdater::StartUpdateCheck();
		#endif
	#elif defined(_WIN32)
		Icon = nullptr;
	#endif

	
	if (!::SDK)
		::SDK = this;

	if (CurLang.type != json_object)
	{
		MessageBoxA(NULL, "The JSON parser could not find the current language object.", "DarkEdif - Internal JSON error.", MB_OK);
		return;
	}

	const json_value &Actions = CurLang["Actions"];
	const json_value &Conditions = CurLang["Conditions"];
	const json_value &Expressions = CurLang["Expressions"];

#ifdef _WIN32
	const json_value & Properties = CurLang["Properties"];

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
		ActionJumps [i] = (void *) Edif::Action;
#endif

		ActionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewActionInfo();
	}

	for (size_t i = 0; i < Conditions.u.object.length; ++ i)
	{
#ifdef _WIN32
		ConditionJumps [i] = (void *) Edif::Condition;
#endif

		ConditionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewConditionInfo();
	}

	for (size_t i = 0; i < Expressions.u.object.length; ++ i)
	{
#ifdef _WIN32
		ExpressionJumps [i] = (void *) Edif::Expression;
#endif

		ExpressionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewExpressionInfo();
	}

	// Phi woz 'ere
#if EditorBuild
	{
		std::vector <PropData> VariableProps;
		PropData * CurrentProperty;

		for (size_t i = 0; i < Properties.u.array.length; ++ i)
		{
			const json_value &Property = Properties[i];
			CurrentProperty = nullptr;

			// Reserved/invalid properties are marked with ! at the start.
			// If some muppet attempts to use 'em, throw an error.
			if ( ((const char *) Property["Type"])[0] == '!')
			{
				std::string error = "You have specified an invalid Parameter type: [" + 
									std::string((const char *)Property["Type"]) + 
									"].\r\nPlease ensure you are permitted to use this.";
			
				MessageBoxA(NULL, error.c_str(), "DarkEdif - JSON Property parser", MB_OK);
			}
			else // Property is not reserved
			{
				#define SetAllProps(opt,lParams) CurrentProperty->SetAllProperties(Options|(opt), (LPARAM)(lParams)); break
				using namespace Edif::Properties;

				// Custom Parameter: Read the number CustomXXX and use that.
				if (! _strnicmp(Property["Type"], "Custom", 6))
				{
					MessageBoxA(NULL, "Detected as custom.", "Edif note", MB_OK);
					CurrentProperty = new PropData((int)VariableProps.size(), (unsigned int)i + PROPTYPE_LAST_ITEM);
				}
				else if (! _stricmp(Property["Type"], Names[PROPTYPE_FOLDER_END]))
				{
					CurrentProperty = new PropData(-1, PROPTYPE_FOLDER_END);
				}
				else // Regular Parameter
				{

					// Loop through Parameter names and compareth them.
					for(unsigned int j = PROPTYPE_FIRST_ITEM;
						j < (PROPTYPE_LAST_ITEM - PROPTYPE_FIRST_ITEM);
						++ j)
					{
						if (!_stricmp(Property["Type"], Names[j]))
						{
							// Unicode Properties have IDs 1000 greater than their
							// ANSI equivalents. If necessary, you can boost all of them.
#ifndef _UNICODE
							CurrentProperty = new PropData(VariableProps.size(), j);
#else
							CurrentProperty = new PropData(VariableProps.size(), j + 1000);
#endif
							break;
						}
					}
				
					if (!CurrentProperty)
					{
						char temp[256];
						sprintf_s(temp, "The Parameter type specified was unrecognised: [%s]."
											 "Check your spelling of the \"Type\" Parameter.", (const char *)Property["Type"]);
						MessageBoxA(NULL, temp, "DarkEdif error", MB_OK);
					}
					else
					{
						// If checkbox is enabled, pass that as flags as well.
						unsigned int Options = (bool(Property["Checkbox"]) ? PROPOPT_CHECKBOX : 0)		// Checkbox enabled by property option in JSON
											 | (bool(Property["Bold"]) ? PROPOPT_BOLD: 0)				// Bold enabled by property option in JSON
											 | (bool(Property["Removable"]) ? PROPOPT_REMOVABLE: 0)		// Removable enabled by property option in JSON
											 | (bool(Property["Renameable"]) ? PROPOPT_RENAMEABLE: 0)	// Renamable enabled by property option in JSON
											 | (bool(Property["Moveable"]) ? PROPOPT_MOVABLE: 0)		// Movable enabled by property option in JSON
											 | (bool(Property["List"]) ? PROPOPT_LIST: 0)				// List enabled by property option in JSON
											 | (bool(Property["SingleSelect"]) ? PROPOPT_SINGLESEL: 0);	// Single-select enabled by property option in JSON
						// Todo: passing of LParams requires ParamsREQUIRED option.
						// Find out what opt is.
						// Two settings may be specified by |=ing the options unsigned int.
					
						CurrentProperty->Title = Edif::ConvertString(Property["Title"]);
						CurrentProperty->Info = Edif::ConvertString(Property["Info"]);

						switch (CurrentProperty->Type_ID % 1000)
						{
							// Simple static text
							case PROPTYPE_STATIC:
								SetAllProps(0, NULL);
					
							// Folder
							case PROPTYPE_FOLDER:
								SetAllProps(0, NULL);

							// Edit button, Params1 = button text, or nullptr if Edit
							case PROPTYPE_EDITBUTTON:
								SetAllProps(PROPOPT_PARAMREQUIRED, (((const char *)Property["Text"])[0] == '\0') ? 0 : (const char *)Property["Text"]);
					
							// Edit box for strings, Parameter = max length
							case PROPTYPE_EDIT_STRING:
								Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((Property["Password"]) ? PROPOPT_EDIT_PASSWORD: 0);
								SetAllProps(PROPOPT_PARAMREQUIRED, ((std::int64_t)Property["MaxLength"] & 0xFFFFFFFF));

					
							// Edit box for numbers, Parameters = min value, max value
							case PROPTYPE_EDIT_NUMBER:
							{
								int * temp = new int[2];
								temp[0] = ((std::int64_t)Property["Minimum"]) & 0xFFFFFFFF;
								temp[1] = ((std::int64_t)Property["Maximum"]) & 0xFFFFFFFF;
								SetAllProps(PROPOPT_PARAMREQUIRED, temp);
							}

							// Combo box, Parameters = list of strings, options (sorted, etc)
							case PROPTYPE_COMBOBOX:
							{
								if (Property["Items"].u.object.length == 0)
									MessageBoxA(NULL, "Warning: no items detected in combobox property.", "DarkEdif error", MB_OK);
							
								const TCHAR ** Fixed = new const TCHAR * [Property["Items"].u.object.length+2];

								// NULL is required at start of array
								Fixed[0] = Fixed[Property["Items"].u.object.length+1] = nullptr;

								// Use incrementation and copy to fixed list.
								for (size_t index = 1; index < Property["Items"].u.object.length+1; ++ index)
									Fixed[index] = Edif::ConvertString(Property["Items"][index-1]);

								// Pass fixed list as Parameter
								SetAllProps(PROPOPT_PARAMREQUIRED, (LPARAM)Fixed);
							}

							// Size
							case PROPTYPE_SIZE:

							// Color
							case PROPTYPE_COLOR:

							// Checkbox
							case PROPTYPE_LEFTCHECKBOX:
								// Enforce option to show it is a checkbox
								SetAllProps(PROPOPT_CHECKBOX, NULL);

							// Edit + Slider
							case PROPTYPE_SLIDEREDIT:
								SetAllProps(0, NULL);

							// Edit + Spin
							case PROPTYPE_SPINEDIT:
								SetAllProps(0, NULL);

							// Direction Selector
							case PROPTYPE_DIRCTRL:
								SetAllProps(0, NULL);
					
							// Group
							case PROPTYPE_GROUP:
								SetAllProps(0, NULL);
					
							// Edit box + browse file button, Parameter = FilenameCreateParams
							case PROPTYPE_FILENAME:
								SetAllProps(0, NULL);

							// Font dialog box
							case PROPTYPE_FONT:
								SetAllProps(0, NULL);
					
							// Edit box + browse image file button
							case PROPTYPE_PICTUREFILENAME:
								SetAllProps(0, NULL);
					
							// Combo box, Parameters = list of strings, options (sorted, etc)
							case PROPTYPE_COMBOBOXBTN:
								SetAllProps(0, NULL);
					
							// Edit box for floating point numbers, Parameters = min value, max value, options (signed, float, spin)
							case PROPTYPE_EDIT_FLOAT:
								SetAllProps(0, NULL);
					
							// Edit box for multiline texts, no Parameter
							case PROPTYPE_EDIT_MULTILINE:
								Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)	|		// Checkbox enabled by property option in JSON
											((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0);			// Checkbox enabled by property option in JSON
								SetAllProps(0, NULL);
					
							// Image list
							case PROPTYPE_IMAGELIST:
								SetAllProps(0, NULL);
					
							// Combo box with icons
							case PROPTYPE_ICONCOMBOBOX:
								SetAllProps(0, NULL);
					
							// URL button
							case PROPTYPE_URLBUTTON:
								SetAllProps(0, NULL);
					
							// Directory pathname
							case PROPTYPE_DIRECTORYNAME:
								SetAllProps(0, NULL);
					
							// Edit + Spin, value = floating point number
							case PROPTYPE_SPINEDITFLOAT:
								SetAllProps(0, NULL);

							// Unrecognised
							default:
							{
								char temp [256];
								sprintf_s(temp, std::size(temp), "The Parameter type specified was unrecognised: [%s]."
													 "Check your spelling of the \"Type\" Parameter.", (const char *)Property["Type"]);
								MessageBoxA(NULL, temp, "DarkEdif JSON error", MB_OK);
								SetAllProps(0, NULL);
							}
						}
					}
				}

				// Add to properties
				VariableProps.push_back(*CurrentProperty);
			}
		}
		EdittimeProperties = new PropData[VariableProps.size()+1];
		// Use incrementation and copy to fixed list.
		for(unsigned int l = 0; l < VariableProps.size(); ++l)
			EdittimeProperties[l] = VariableProps[l];
		
		// End with completely null byte
		memset(&EdittimeProperties[VariableProps.size()], 0, sizeof(PropData));
	}

	ActionMenu = LoadMenuJSON(Edif::ActionID(0), CurLang["ActionMenu"]);
	ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), CurLang["ConditionMenu"]);
	ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), CurLang["ExpressionMenu"]);

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
		unchangedPropsFound = copy.rfind("by your name") != std::string::npos;
	}
	if (unchangedPropsFound)
	{
		// If you are getting this message, you need to update the About section of your JSON file.
		// Make sure to do it for all languages in the JSON.
		MessageBoxA(NULL, "Template properties are unedited in JSON. Please amend them.\n\n"
			"You need to update the About section of your JSON file. "
			"Make sure to do it for all languages in the JSON.",
			PROJECT_NAME " - DarkEdif SDK warning", MB_ICONWARNING);
	}
#endif

#ifdef INTENSE_FUNCTIONS_EXTENSION
	Extension::AutoGenerateExpressions();
#endif
#endif // EditorBuild
}

Edif::SDK::~SDK()
{
	json_value_free (&json);

#if EditorBuild
	delete [] ActionJumps;
	delete [] ConditionJumps;
	delete [] ExpressionJumps;
	delete [] EdittimeProperties;

	delete Icon;
#endif
}

#if defined(__arm__) && defined(__ANDROID__)
const static size_t paramInc = 1; // In ARM, the first parameter must be Extension *
#else
const static size_t paramInc = 0;
#endif

long ActionOrCondition(void * Function, int ID, Extension * ext, const ACEInfo * info, ACEParamReader &params, bool isCond)
{
	int ParameterCount = info->NumOfParams;
	long * Parameters;
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

	Parameters = (long *)alloca(sizeof(long) * ParameterCount);
	bool isComparisonCondition = false;

	if constexpr (paramInc == 1) {
		Parameters[0] = (long)ext;
	}

	for (int i = 0; i < ParameterCount; ++ i)
	{
		switch (info->Parameter[i].p)
		{
			case Params::Expression:
				if (info->FloatFlags & (1 << i))
				{
					float f = params.GetFloat(i);
					Parameters[i + paramInc] = *(int*)(&f);
				}
				else
					Parameters[i + paramInc] = params.GetInteger(i);
				break;

			case Params::String_Comparison:
			case Params::String_Expression:
			case Params::Filename:
				Parameters[i + paramInc] = (long)params.GetString(i);
				// Catch null string parameters and return default 0
				if (!Parameters[i + paramInc])
				{
					MessageBoxA(NULL, "Error calling action/condition: null pointer given as string parameter.", PROJECT_NAME " - DarkEdif ActionOrCondition() error", MB_ICONERROR);
					goto endFunc;
				}

				isComparisonCondition |= info->Parameter[i].p == Params::String_Comparison;
				break;

			case Params::Compare_Time:
			case Params::Comparison:
				Parameters[i + paramInc] = params.GetInteger(i);
				isComparisonCondition = true;
				break;

			default:
				Parameters[i + paramInc] = params.GetInteger(i);
				break;
		}
	}

#ifdef _WIN32
	ext->rdPtr->rHo.CurrentParam = saveCurParam;
	__asm
	{
		pushad				  ; Start new register set (do not interfere with already existing registers) \
		
		mov ecx, ParameterCount ; Store ParameterCount in ecx
		
		cmp ecx, 0			  ; If no parameters, call function immediately
		je CallNow

		mov edx, Parameters	 ; Otherwise store pointer to int * in Parameters

		mov ebx, ecx			; Copy ecx, or ParameterCount, to ebx
		shl ebx, 2			  ; Multiply parameter count by 2^2 (size of 32-bit variable)

		add edx, ebx			; add (ParameterCount * 4) to Parameters, making edx point to Parameters[param count]
		sub edx, 4			  ; subtract 4 from edx, making it 0-based (ending array index)

		PushLoop:

			push [edx]		  ; Push value pointed to by Parameters[edx]
			sub edx, 4		  ; Decrement next loop`s Parameter index:	for (><; ><; edx -= 4)

			dec ecx			 ; Decrement loop index:					 for (><; ><; ecx--)

			cmp ecx, 0		  ; If ecx == 0, end loop:					for (><; ecx == 0; ><)
			jne PushLoop		; Optimisation: "cmp ecx, 0 / jne" can be replaced with "jcxz"

		CallNow:

		mov ecx, ext			; Move Extension to ecx
		call Function			; Call the function inside Extension
		
		mov Result, eax		 ; Function`s return is stored in eax; copy it to Result

		popad					; End new register set (restore registers that existed before popad)
	}

#elif defined(__arm__) && defined(__ANDROID__)
	// Hat tip to https://stackoverflow.com/questions/50021839/assembly-x86-convert-to-arm-function-call-with-varying-number-of-parameters-to-a#50022446

	argStackCount = ParameterCount + paramInc;
	// if > 4 params, they're stored on stack
	if (argStackCount > 4) {
		argStackCount = argStackCount - 4;
	}

	// build stack, fill registers and call functions  
	// ! volatile ... otherwise compiler "optimize out" our ASM code
	__asm__ volatile (
		"mov r4, %[ARGV]\n\t"	// remember pointers (SP will be changed)
		"ldr r5, %[ACT]\n\t"
		"ldr r0, %[CNT]\n\t"	// arg_stack_count  => R0
		"lsl r0, r0, #2\n\t"	// R0 * 4			=> R0
		"mov r6, r0\n\t"		// R4				=> R6
		"mov r1, r0\n"		  // arg_stack_count  => R1
		"loop: \n\t"
		"cmp r1, #0\n\t"
		"beq end\n\t"			// R1 == 0	  => jump to end
		"sub r1, r1, #4\n\t"	// R1--
		"mov r3, r4\n\t"		// argv_stack	=> R3
		"add r3, r3, #16\n\t"
		"ldr r2, [r3, r1]\n\t"  // argv[r1]
		"push {r2}\n\t"		 // argv[r1] => push to stack
		"b loop\n"			  //		  => repeat
		"end:\n\t"
		"ldr r0, [r4]\n\t"	  // 1st argument
		"ldr r1, [r4, #4]\n\t"  // 2nd argument
		"ldr r2, [r4, #8]\n\t"  // 3rd argument
		"ldr r3, [r4, #12]\n\t" // 4th argument
		"blx r5\n\t"			// call function
		"add sp, sp, r6\n\t"	// fix stack position
		"mov %[ER], r0\n\t"	 // store result
		: [ER] "=r"(Result)
		: [ARGV] "r" (Parameters),
		[ACT] "m"(Function),
		[CNT] "m" (argStackCount)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6");
	// blx 
#else
#ifndef __INTELLISENSE__
	if (isCond)
	{
		switch (ID)
		{
			#define DARKEDIF_ACE_CALL_TABLE_INDEX 0
			#include "Temp_ACECallTable.cpp"

			default:
				MessageBoxA(NULL, "Error calling condition: ID not found.", "DarkEdif - ActionOrCondition() error", MB_OK);
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
				MessageBoxA(NULL, "Error calling condition: ID not found.", "DarkEdif - ActionOrCondition() error", MB_OK);
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
HMENU Edif::LoadMenuJSON(int BaseID, const json_value &Source, HMENU Parent)
{
	if (!Parent)
		Parent = CreateMenu();

	for (unsigned int i = 0; i < Source.u.object.length; ++ i)
	{
		const json_value &MenuItem = Source[i];

		if (MenuItem.type == json_string)
		{
			if (!_stricmp(MenuItem, "Separator") || !strcmp(MenuItem, "---"))
			{
				AppendMenu(Parent, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
				continue;
			}

			continue;
		}
		
		if (MenuItem[0].type == json_string && MenuItem[1].type == json_array)
		{
			HMENU SubMenu = CreatePopupMenu();
			LoadMenuJSON(BaseID, MenuItem, SubMenu);

			TCHAR* str = ConvertString(MenuItem[0]);
			AppendMenu(Parent, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT) SubMenu, str);
			FreeString(str);

			continue;
		}

		unsigned int ItemOffset = 0;

		int ID = BaseID + (int) MenuItem[ItemOffset].u.integer;
		TCHAR * Text = ConvertString(MenuItem[ItemOffset + 1]);
		bool Disabled = MenuItem.u.object.length > (ItemOffset + 2) ? ((bool) MenuItem[ItemOffset + 2]) != 0 : false;

		AppendMenu(Parent, (Disabled ? MF_GRAYED | MF_UNCHECKED : 0) | MF_BYPOSITION | MF_STRING, ID, Text);

		FreeString(Text);
	}

	return Parent;
}
#endif

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
		LOGV("Getting float param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		float f = (isCondition ? ext->runFuncs.cnd_getParamExpFloat : ext->runFuncs.act_getParamExpFloat)(ext->javaExtPtr, javaActOrCndObj);
		LOGV("Got float param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		const TCHAR * str = trackString((isCondition ? ext->runFuncs.cnd_getParamExpString : ext->runFuncs.act_getParamExpString)(ext->javaExtPtr, javaActOrCndObj));
		LOGV("Got string param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		std::int32_t in = (isCondition ? ext->runFuncs.cnd_getParamExpression : ext->runFuncs.act_getParamExpression)(ext->javaExtPtr, javaActOrCndObj);
		LOGV("Got integer param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
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

	int DarkEdif_actGetParamExpression(void * ext, void * act);
	const char * DarkEdif_actGetParamExpString(void * ext, void * act);
	double DarkEdif_actGetParamExpDouble(void * ext, void * act);

	int DarkEdif_cndGetParamExpression(void * ext, void * cnd);
	const char * DarkEdif_cndGetParamExpString(void * ext, void * cnd);
	double DarkEdif_cndGetParamExpDouble(void * ext, void * cnd);
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
		LOGV("Getting float param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		double f = (isCondition ? DarkEdif_cndGetParamExpDouble : DarkEdif_actGetParamExpDouble)(ext->objCExtPtr, objCActOrCndObj);
		LOGV("Got float param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
		return (float)f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		const TCHAR* str = (isCondition ? DarkEdif_cndGetParamExpString : DarkEdif_actGetParamExpString)(ext->objCExtPtr, objCActOrCndObj);
		LOGV("Got string param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, cond=%d, index %d.", isCondition ? 1 : 0, index);
		std::int32_t in = (isCondition ? DarkEdif_cndGetParamExpression : DarkEdif_actGetParamExpression)(ext->objCExtPtr, objCActOrCndObj);
		LOGV("Got integer param, cond=%d, index %d OK.", isCondition ? 1 : 0, index);
		return in;
	}

	~ConditionOrActionManager_iOS()
	{
	}
};
#endif

#ifdef _WIN32
long __stdcall Edif::Condition(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * ext = rdPtr->pExtension;
	int ID = rdPtr->rHo.EventNumber;
	ConditionOrActionManager_Windows params(true, rdPtr, param1, param2);
#elif defined(__ANDROID__)
ProjectFunc jlong condition(JNIEnv *, jobject, jlong extPtr, int ID, CCndExtension cndExt)
{
	Extension * ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(true, ext, (jobject)cndExt);
	LOGV("Condition ID %i start.", ID);
#else
ProjectFunc long PROJ_FUNC_GEN(PROJECT_NAME_RAW, _condition(void * cppExtPtr, int ID, CCndExtension cndExt))
{
	Extension* ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(true, ext, cndExt);
#endif

	if (::SDK->ConditionFunctions.size() < (unsigned int)ID) {
		MessageBoxA(NULL, "Missing condition ID %d in extension %s, this ID was not linked.", PROJECT_NAME " - DarkEdif error", MB_ICONERROR);
		return ext->Condition(ID);
	}
	
	void * Function = ::SDK->ConditionFunctions[ID];

	if (!Function) {
		MessageBoxA(NULL, "Missing condition ID %d. Condition existed in vector, but was NULL. Might not be linked.", PROJECT_NAME " - DarkEdif error", MB_ICONERROR);
		return ext->Condition(ID);
	}

	long Result = ActionOrCondition(Function, ID, ext, ::SDK->ConditionInfos[ID], params, true);

#ifdef __ANDROID__
	LOGV("Condition ID %i end.", ID);
#endif

	return Result;
}

#ifdef _WIN32
short __stdcall Edif::Action(RUNDATA * rdPtr, long param1, long param2)
{
	Extension * ext = rdPtr->pExtension;
	/* int ID = rdPtr->rHo.hoAdRunHeader->rh4.rh4ActionStart->evtNum; */
	int ID = rdPtr->rHo.EventNumber;
	ConditionOrActionManager_Windows params(false, rdPtr, param1, param2);
#define nowt 0
#elif defined (__ANDROID__)
ProjectFunc void action(JNIEnv *, jobject, jlong extPtr, jint ID, CActExtension act)
{
	Extension * ext = (Extension *)extPtr;
	ConditionOrActionManager_Android params(false, ext, act);
	LOGV("Action ID %i start.", ID);
#define nowt 
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _action(void * cppExtPtr, int ID, CActExtension act))
{
	Extension* ext = (Extension*)cppExtPtr;
	ConditionOrActionManager_iOS params(false, ext, act);
#define nowt 
#endif

	if (::SDK->ActionFunctions.size() < (unsigned int)ID)
	{
		ext->Action(ID);
		return nowt;
	}
	void * Function = ::SDK->ActionFunctions[ID];

	if (!Function)
	{
		ext->Action(ID);
		return nowt;
	}

	ActionOrCondition(Function, ID, ext, ::SDK->ActionInfos[ID], params, false);

#ifdef __ANDROID__
	LOGV("Action ID %i end.", ID);
#endif
	return nowt;
#undef nowt
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
		LOGV("Setting expression return as text...");
		ext->runFuncs.exp_setReturnString(ext->javaExtPtr, expJavaObj, a);
		LOGV("Setting expression return as text \"%s\" OK.", a);
	}

	// Inherited via ACEParamReader
	virtual float GetFloat(int index)
	{
		LOGV("Getting float param, expr, index %d OK.", index);
		float f = ext->runFuncs.exp_getParamFloat(ext->javaExtPtr, expJavaObj);
		LOGV("Got float param, expr, index %d OK.", index);
		return f;
	}

	virtual const TCHAR * GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.", index);
		const TCHAR * str = trackString(ext->runFuncs.exp_getParamString(ext->javaExtPtr, expJavaObj));
		LOGV("Got string param, expr, index %d OK.", index);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, expr, index %d OK.", index);
		std::int32_t i = ext->runFuncs.exp_getParamInt(ext->javaExtPtr, expJavaObj);
		LOGV("Got integer param, expr, index %d OK.", index);
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
		LOGV("Getting float param, expr, index %d OK.", index);
		float f = DarkEdif_expGetParamFloat(ext->objCExtPtr);
		LOGV("Got float param, expr, index %d OK.", index);
		return f;
	}

	virtual const TCHAR* GetString(int index)
	{
		LOGV("Getting string param, expr, index %d.", index);
		const TCHAR* str = DarkEdif_expGetParamString(ext->objCExtPtr);
		LOGV("Got string param, expr, index %d OK.", index);
		return str;
	}

	virtual std::int32_t GetInteger(int index)
	{
		LOGV("Getting integer param, expr, index %d OK.", index);
		std::int32_t i = DarkEdif_expGetParamInt(ext->objCExtPtr);
		LOGV("Got integer param, expr, index %d OK.", index);
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
long FusionAPI Edif::Expression(RUNDATA * rdPtr, long param)
{
	int ID = rdPtr->rHo.EventNumber;
	Extension * ext = (Extension *)rdPtr->pExtension;
	ExpressionManager_Windows params(rdPtr);
#elif defined(__ANDROID__)
ProjectFunc void expression(JNIEnv *, jobject, jlong extPtr, jint ID, CNativeExpInstance expU)
{
	Extension * ext = (Extension *)extPtr;
	ExpressionManager_Android params(ext, expU);
	LOGV("Expression ID %i start.", ID);
#else
ProjectFunc void PROJ_FUNC_GEN(PROJECT_NAME_RAW, _expression(void * cppExtPtr, int ID))
{
	Extension* ext = (Extension*)cppExtPtr;
	ExpressionManager_iOS params(ext);
#endif

	if (::SDK->ExpressionFunctions.size() < (unsigned int)ID)
		return params.SetValue((int)ext->Expression(ID));

	void * Function = ::SDK->ExpressionFunctions[ID];

	if (!Function)
		return params.SetValue((int)ext->Expression(ID));

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


	long * Parameters = (long *)alloca(sizeof(long) * (ParameterCount + paramInc));
	memset(Parameters, 0, sizeof(long) * (ParameterCount + paramInc));
	long Result = 0;

#ifdef _WIN32
	int ExpressionRet2 = (int)ExpressionRet; // easier for ASM
#else
	Parameters[0] = (long)ext;
	int argStackCount = ParameterCount + paramInc;
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
			Parameters[i + paramInc] = (long)params.GetString(i);

			// Catch null string parameters and return "" or 0 as appropriate
			if (!Parameters[i + paramInc])
			{
				MessageBoxA(NULL, "Error calling expression: null pointer given as string parameter.", "DarkEdif - Expression() error", MB_OK);
				if (ExpressionRet == ExpReturnType::String)
					Result = (long)ext->Runtime.CopyString(_T(""));
				goto endFunc;
			}
			break;

		case ExpParams::Integer: // Handles float as well
			if ((info->FloatFlags & (1 << i)) != 0)
			{
				float f = params.GetFloat(i);
				Parameters[i + paramInc] = *(int*)&f;
			}
			else
				Parameters[i + paramInc] = params.GetInteger(i);
			break;
		default:
		{
			MessageBoxA(NULL, "Error calling expression: not sure what parameter type it is.", "DarkEdif - Expression() error", MB_OK);
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
		mov edx, Parameters
		mov ebx, ecx
		shl ebx, 2
		add edx, ebx
		sub edx, 4
		PushLoop:
			push[edx]
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
	return Result;

#else // CLANG


// Nicely ported
#if defined(__arm__) && defined(__ANDROID__)

	// In ARM, the floats are returned in the same ASM register as int/pointers when using soft float ABI.
	// While Android OS may use a different ABI, that's not relevant, as this ASM calls functions within our own extension.
	// Hat tip to https://stackoverflow.com/questions/50021839/assembly-x86-convert-to-arm-function-call-with-varying-number-of-parameters-to-a#50022446

	// build stack, fill registers and call functions  
	// ! volatile ... otherwise compiler "optimize out" our ASM code
	__asm__ volatile (
		"mov r4, %[ARGV]\n\t"	// remember pointers (SP will be changed)
		"ldr r5, %[ACT]\n\t"
		"ldr r0, %[CNT]\n\t"	// argStackCount  => R0
		"lsl r0, r0, #2\n\t"	// R0 * 4			=> R0
		"mov r6, r0\n\t"		// R4				=> R6			
		"mov r1, r0\n"		  // argStackCount  => R1			
		"loop2: \n\t"
		"cmp r1, #0\n\t"
		"beq end2\n\t"			// R1 == 0	  => jump to end
		"sub r1, r1, #4\n\t"	// R1--
		"mov r3, r4\n\t"		// argv_stack	=> R3
		"add r3, r3, #16\n\t"
		"ldr r2, [r3, r1]\n\t"  // argv[r1]
		"push {r2}\n\t"		 // argv[r1] => push to stack
		"b loop2\n"			  //		  => repeat
		"end2:\n\t"
		"ldr r0, [r4]\n\t"	  // 1st argument
		"ldr r1, [r4, #4]\n\t"  // 2nd argument
		"ldr r2, [r4, #8]\n\t"  // 3rd argument
		"ldr r3, [r4, #12]\n\t" // 4th argument
		"blx r5\n\t"			// call function
		"add sp, sp, r6\n\t"	// fix stack position
		"mov %[ER], r0\n\t"	 // store result
		: [ER] "=r"(Result)
		: [ARGV] "r" (Parameters),
		[ACT] "m"(Function),
		[CNT] "m" (argStackCount)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6");

#else
	switch (ID)
	{
		#ifndef __INTELLISENSE__
		#undef DARKEDIF_ACE_CALL_TABLE_INDEX
		#define DARKEDIF_ACE_CALL_TABLE_INDEX 2
		#include "Temp_ACECallTable.cpp"
		#endif // __INTELLISENSE__
		default:
			MessageBoxA(NULL, "Error calling condition: ID not found.", "DarkEdif - ActionOrCondition() error", MB_OK);
			goto endFunc;
	}
#endif
	endFunc:
	
		// Must set return type after the expression func is evaluated, as sub-expressions inside the
		// expression func (e.g. from generating events) could change it to something else
		params.SetReturnType(ExpressionRet);
	
		if (ExpressionRet == ExpReturnType::String)
		{
			lw_trace("Returning string for expression ID %i.", ID);
			params.SetValue((const char *)Result);
		}
		else if (ExpressionRet == ExpReturnType::Integer)
			params.SetValue((int)Result);
		else if (ExpressionRet == ExpReturnType::Float)
			params.SetValue(*(float*)&Result);
		else
			MessageBoxA(NULL, "Unrecognised return type.", PROJECT_NAME " - DarkEdif SDK - ASM", MB_ICONERROR);
		
	#ifdef __ANDROID__
		LOGV("Expression ID %i end.", ID);
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
	if (GetFileAttributes(FullFilename) == 0xFFFFFFFF)
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
		if (GetFileAttributes(FullFilename) == 0xFFFFFFFF)
		{
			// No => try Data/Runtime
			_stprintf_s(FullFilename, sizeof(ExecutablePath)/sizeof(TCHAR), _T("%s/Data/Runtime/%s"), ExecutablePath, Filename);
			if (GetFileAttributes(FullFilename) == 0xFFFFFFFF)
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
		LOGE("Failed to convert  text... %d", 0);
		wcscpy(tstr, L"<Failed to convert text>");
	}
	else
	{
		size_t outSize = std::mbstowcs(tstr, utf8String, sSize);
		if (outSize == -1)
		{
			LOGE("Failed to convert text... %d", 1);
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


#if defined(_DEBUG) && !defined(__APPLE__)


Edif::recursive_mutex::recursive_mutex()
{
	this->log << "New recursive mutex.\n";
}
Edif::recursive_mutex::~recursive_mutex()
{
	this->log << "Recursive mutex dying.\n";
}

#ifndef _DEBUG
static const char * file = "(release mode)";
static const char * func = "(release mode)";
static int line = -1;
#endif
void Edif::recursive_mutex::lock(edif_lock_debugParams)
{
	try {
		this->intern.lock();
	}
	catch (std::system_error err)
	{
		FILE * f = fopen("/storage/emulated/0/crashlog.txt", "wb");
		if (f == NULL) {
#ifndef _WIN32
			LOGV("Failed to write log file, error %d.", errno);
#endif
		}
		else
		{
			this->log << "FAILED TO LOCK in function " << func << ", file " << file << ", line " << line << ", error " << err.what() << ".\n";
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
#ifndef _WIN32
			LOGE("%s", str.c_str());
#endif
			throw err;
		}
	}
	this->log << "Locked in function " << func << ", line " << line << ".\n";
}
bool Edif::recursive_mutex::try_lock(edif_lock_debugParams)
{
	bool b = false;
	try {
		b = this->intern.try_lock();
	}
	catch (std::system_error err)
	{
		FILE * f = fopen("/storage/emulated/0/crashlog.txt", "wb");
		if (f == NULL) {
#ifndef _WIN32
			LOGV("Failed to write log file, error %d.", errno);
#endif
		}
		else
		{
			this->log << "FAILED TO TRY LOCK in function " << func << ", line " << line << ", error " << err.what() << ".\n";
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
#ifndef _WIN32
			LOGE("%s", str.c_str());
#endif
			throw err;
		}
	}
	this->log << "Try lock " << (b ? "OK" : "FAIL") << " in function " << func << ", line " << line << ".\n";
	return b;
}
void Edif::recursive_mutex::unlock(edif_lock_debugParams)
{
	try {
		this->intern.unlock();
	}
	catch (std::system_error err)
	{
		FILE * f = fopen("/storage/emulated/0/crashlog.txt", "wb");
		if (f == NULL) {
#ifndef _WIN32
			LOGV("Failed to write log file, error %d.", errno);
#endif
		}
		else
		{
			this->log << "FAILED TO UNLOCK in function " << func << ", line " << line << ", error " << err.what() << ".\n";
			std::string str(this->log.str());
			fwrite(str.c_str(), 1, str.size(), f);
			fclose(f);
#ifndef _WIN32
			LOGE("%s", str.c_str());
#endif
			throw err;
		}
	}
	this->log << "Unlocked in function " << func << ", line " << line << ".\n";
}

#else // Not debug

Edif::recursive_mutex::recursive_mutex()
{
}
Edif::recursive_mutex::~recursive_mutex()
{
}
void Edif::recursive_mutex::lock()
{
	this->intern.lock();
}
bool Edif::recursive_mutex::try_lock()
{
	return this->intern.try_lock();
}
void Edif::recursive_mutex::unlock()
{
	this->intern.unlock();
}

#endif // _DEBUG
