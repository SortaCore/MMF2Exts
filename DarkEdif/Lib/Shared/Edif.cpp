#include "Common.h"

Edif::SDK * SDK = nullptr;


TCHAR Edif::LanguageCode[3];

bool Edif::IsEdittime;
bool Edif::ExternalJSON;

void Edif::GetExtensionName(char * const writeTo)
{
	char temp [MAX_PATH];
	GetModuleFileNameA(hInstLib, temp, sizeof(temp));
	strcpy(writeTo, strrchr(temp, '\\') + 1);
	writeTo[strlen(writeTo) - 4] = 0; // Remove ".mfx"
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

	char error [256];
	sprintf_s(error, "Error reading expression return; returns '%s', which is unrecognised.", Text);
	MessageBoxA(NULL, error, "DarkEdif - Error", MB_OK | MB_ICONERROR);
	return ExpReturnType::Integer; // default
}

void Edif::Init(mv * mV, EDITDATA * edPtr)
{
	IsEdittime = mV->HMainWin != 0;

	mvInvalidateObject(mV, edPtr);
}

int Edif::Init(mv * mV)
{
	_tcscpy (LanguageCode, _T ("EN"));

	// Get pathname of MMF2
	TCHAR * mmfname = (TCHAR *)calloc(_MAX_PATH, sizeof(TCHAR));
	if ( mmfname != NULL )
	{
		// Load resources
		GetModuleFileName (NULL, mmfname, _MAX_PATH);
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

// Used for reading the icon image file
PhiDLLImport BOOL FusionAPI ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;
	DarkEdif::IsFusion25 = ((mV->GetVersion() & MMFVERSION_MASK) == CFVERSION_25);

	#if EditorBuild
		cSurface * proto = nullptr;
		if (GetSurfacePrototype(&proto, 32, ST_MEMORYWITHDC, SD_BITMAP) == FALSE)
			MessageBoxA(NULL, "Getting surface prototype failed.", "DarkEdif error", MB_ICONERROR);

		Icon = new cSurface();
		if (mV->ImgFilterMgr)
		{
			char * IconData;
			size_t IconSize;

			int result = Edif::GetDependency (IconData, IconSize, _T("png"), IDR_EDIF_ICON);
			if (result != Edif::DependencyNotFound)
			{
				CInputMemFile * File = CInputMemFile::NewInstance();
				File->Create ((LPBYTE)IconData, IconSize);

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
					MessageBoxA(NULL, "Blitting to surface failed.", "DarkEdif error", MB_ICONERROR);
			}
		}

		#if USE_DARKEDIF_UPDATE_CHECKER
			DarkEdif::SDKUpdater::StartUpdateCheck();
		#endif
	#endif // EditorBuild

	if (!::SDK)
		::SDK = this;

	if (CurLang.type != json_object)
	{
		MessageBoxA(NULL, "The JSON parser could not find the current language object.", "DarkEdif - Internal JSON error.", MB_OK);
		DebugBreak();
		return;
	}

	const json_value &Actions = CurLang["Actions"];
	const json_value &Conditions = CurLang["Conditions"];
	const json_value &Expressions = CurLang["Expressions"];
	const json_value &Properties = CurLang["Properties"];

	ActionJumps = new void * [Actions.u.object.length + 1];
	ConditionJumps = new void * [Conditions.u.object.length + 1];
	ExpressionJumps = new void * [Expressions.u.object.length + 1];

	ActionJumps [Actions.u.object.length] = 0;
	ConditionJumps [Conditions.u.object.length] = 0;
	ExpressionJumps [Expressions.u.object.length] = 0;

	for (unsigned int i = 0; i < Actions.u.object.length; ++ i)
	{
		ActionJumps [i] = (void *) Edif::Action;

		ActionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewActionInfo();
	}

	for (unsigned int i = 0; i < Conditions.u.object.length; ++ i)
	{
		ConditionJumps [i] = (void *) Edif::Condition;

		ConditionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewConditionInfo();
	}

	for (unsigned int i = 0; i < Expressions.u.object.length; ++ i)
	{
		ExpressionJumps [i] = (void *) Edif::Expression;

		ExpressionFunctions.push_back(0);

		// Determines how to create itself based on already existing instances.
		CreateNewExpressionInfo();
	}

	// Phi woz 'ere
	#if EditorBuild
	{
		std::vector<PropData> VariableProps;
		PropData * CurrentProperty;

		for (unsigned  int i = 0; i < Properties.u.array.length; ++ i)
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
					CurrentProperty = new PropData(VariableProps.size(), i + PROPTYPE_LAST_ITEM);
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
						const char * c = Property["Type"];
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
						sprintf_s(temp, 256, "The Parameter type specified was unrecognised: [%s]."
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
						bool EnableLParams = false;
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
								SetAllProps(PROPOPT_PARAMREQUIRED, (Property["Text"] == "") ? 0 : Edif::ConvertString((const char *)Property["Text"]));

							// Edit box for strings, Parameter = max length
							case PROPTYPE_EDIT_STRING:
								Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((Property["Password"]) ? PROPOPT_EDIT_PASSWORD: 0);
								SetAllProps(PROPOPT_PARAMREQUIRED, ((long long)Property["MaxLength"] & 0xFFFFFFFF));


							// Edit box for numbers, Parameters = min value, max value
							case PROPTYPE_EDIT_NUMBER:
							{
								int * temp = new int[2];
								temp[0] = long long(Property["Minimum"]) & 0xFFFFFFFF;
								temp[1] = long long(Property["Maximum"]) & 0xFFFFFFFF;
								SetAllProps(PROPOPT_PARAMREQUIRED, temp);
							}

							// Combo box, Parameters = list of strings, options (sorted, etc)
							case PROPTYPE_COMBOBOX:
							{
								if (Property["Items"].u.object.length == 0)
									MessageBoxA(NULL, "Warning: no items detected in combobox property.", "DarkEdif error", MB_OK);

								const TCHAR ** Fixed = new const TCHAR * [Property["Items"].u.object.length + 2];

								// NULL is required at start of array
								Fixed[0] = Fixed[Property["Items"].u.object.length+1] = nullptr;

								// Use incrementation and copy to fixed list.
								for (unsigned int index = 1; index < Property["Items"].u.object.length + 1; ++index)
									Fixed[index] = Edif::ConvertString(Property["Items"][index - 1]);

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
								sprintf_s(temp, 256, "The Parameter type specified was unrecognised: [%s]."
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
	#endif // EditorBuild

	ActionMenu = LoadMenuJSON(Edif::ActionID(0), CurLang["ActionMenu"]);
	ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), CurLang["ConditionMenu"]);
	ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), CurLang["ExpressionMenu"]);

	#if defined(_DEBUG) && EditorBuild && !defined(IS_DARKEDIF_TEMPLATE)
	const json_value &about = CurLang["About"];
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
}

Edif::SDK::~SDK()
{
	json_value_free (&json);

	delete [] ActionJumps;
	delete [] ConditionJumps;
	delete [] ExpressionJumps;

#if EditorBuild
	delete [] EdittimeProperties;
	delete Icon;
#endif
}

int ActionOrCondition(void * Function, int ID, RUNDATA * rdPtr, long Params1, long Params2)
{
	bool Condition = (SDK->ConditionFunctions.size() >= (unsigned int)ID+1) && (SDK->ConditionFunctions[ID] == Function);
	int * Parameters;
	int ParameterCount;

	// Reset by CNC_GetParam(). CurrentParam being correct only matters if you have object parameters, though.
	EventParam* saveCurParam = rdPtr->rHo.CurrentParam;

	const ACEInfo * Info = Condition ? SDK->ConditionInfos[ID] : SDK->ActionInfos[ID];

	ParameterCount = Info->NumOfParams;

	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value &numAutoProps = CurLang[Condition ? "Conditions" : "Actions"][ID]["NumAutoProps"];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	Parameters = (int *) alloca(sizeof(int) * ParameterCount);

	for (int i = 0; i < ParameterCount; ++ i)
	{
		switch (Info->Parameter[i].p)
		{
			case Params::Expression:
				Parameters[i] = (Info->FloatFlags & (1 << i)) ?
								CNC_GetFloatParameter(rdPtr) :
								CNC_GetIntParameter(rdPtr);
				break;

			case Params::String_Expression:
			case Params::Filename:
				Parameters[i] = CNC_GetStringParameter(rdPtr);
				// Catch null string parameters and return default 0
				if (!Parameters[i])
				{
					MessageBoxA(NULL, "Error calling action/condition: null pointer given as string parameter.", "DarkEdif - ActionOrCondition() error", MB_OK);
					return 0L;
				}
				break;

			default:
				Parameters[i] = CNC_GetParameter(rdPtr);
				break;
		}
	}

	void * Extension = rdPtr->pExtension;
	rdPtr->rHo.CurrentParam = saveCurParam;

	int Result;

	__asm
	{
		pushad					; Start new register set (do not interfere with already existing registers)

		mov ecx, ParameterCount ; Store ParameterCount in ecx

		cmp ecx, 0				; If no parameters, call function immediately
		je CallNow

		mov edx, Parameters		; Otherwise store pointer to int * in Parameters

		mov ebx, ecx			; Copy ecx, or ParameterCount, to ebx
		shl ebx, 2				; Multiply parameter count by 2^2 (size of 32-bit variable)

		add edx, ebx			; add (ParameterCount * 4) to Parameters, making edx point to Parameters[param count]
		sub edx, 4				; subtract 4 from edx, making it 0-based (ending array index)

		PushLoop:

			push [edx]			; Push value pointed to by Parameters[edx]
			sub edx, 4			; Decrement next loop`s Parameter index:	for (><; ><; edx -= 4)

			dec ecx				; Decrement loop index:					 for (><; ><; ecx--)

			cmp ecx, 0			; If ecx == 0, end loop:					for (><; ecx == 0; ><)
			jne PushLoop		; Optimisation: "cmp ecx, 0 / jne" can be replaced with "jcxz"

		CallNow:

		mov ecx, Extension		; Move Extension to ecx
		call Function			; Call the function inside Extension

		mov Result, eax			; Function`s return is stored in eax; copy it to Result

		popad					; End new register set (restore registers that existed before popad)
	}

	return Result;
}

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

long FusionAPI Edif::Condition(RUNDATA * rdPtr, long param1, long param2)
{
	int ID = rdPtr->rHo.EventNumber;

	rdPtr->pExtension->Runtime.param1 = param1;
	rdPtr->pExtension->Runtime.param2 = param2;

	if (::SDK->ConditionFunctions.size() < (unsigned int)ID)
		return rdPtr->pExtension->Condition(ID, rdPtr, param1, param2);

	void * Function = ::SDK->ConditionFunctions[ID];

	if (!Function)
		return rdPtr->pExtension->Condition(ID, rdPtr, param1, param2);

	int Result = ActionOrCondition(Function, ID, rdPtr, param1, param2);

	return *(char *) &Result;
}

short FusionAPI Edif::Action(RUNDATA * rdPtr, long param1, long param2)
{
	/* int ID = rdPtr->rHo.hoAdRunHeader->rh4.rh4ActionStart->evtNum; */
	int ID = rdPtr->rHo.EventNumber;

	rdPtr->pExtension->Runtime.param1 = param1;
	rdPtr->pExtension->Runtime.param2 = param2;

	if (::SDK->ActionFunctions.size() < (unsigned int)ID)
	{
		rdPtr->pExtension->Action(ID, rdPtr, param1, param2);
		return 0;
	}
	void * Function = ::SDK->ActionFunctions[ID];

	if (!Function)
	{
		rdPtr->pExtension->Action(ID, rdPtr, param1, param2);
		return 0;
	}

	ActionOrCondition(Function, ID, rdPtr, param1, param2);

	return 0;
}

long FusionAPI Edif::Expression(RUNDATA * rdPtr, long param)
{
	int ID = rdPtr->rHo.EventNumber;

	rdPtr->pExtension->Runtime.param1 = param;
	rdPtr->pExtension->Runtime.param2 = 0;

	if (::SDK->ExpressionFunctions.size() < (unsigned int)ID)
		return rdPtr->pExtension->Expression(ID, rdPtr, param);

	void * Function = ::SDK->ExpressionFunctions[ID];

	if (!Function)
		return rdPtr->pExtension->Expression(ID, rdPtr, param);

	void * Extension = rdPtr->pExtension;

	const ACEInfo * Info = ::SDK->ExpressionInfos[ID];
	ExpReturnType ExpressionRet = Info->Flags.ef;

	int ExpressionRet2 = (int)ExpressionRet;

	int ParameterCount = Info->NumOfParams;

	// If this JSON variable is set, this func doesn't read all the ACE parameters, which allows advanced users to call
	// CNC_XX Expression macros to get parameters themselves.
	// Only useful when the dev decides to allow varying parameter type (e.g. float or int) and which type to
	// read it as is determined at runtime.
	// Since parameters can only be interpreted once per ACE, we can't read it in the ASM, and as we don't have
	// it in the ASM, we can't pass it to the function.
	// Worth noting that if all non-auto parameters are not interpreted, a crash will occur.
	// Also, don't set NumAutoProps to negative.
	const json_value &numAutoProps = CurLang["Expressions"][ID]["NumAutoProps"];
	if (numAutoProps.type == json_integer)
		ParameterCount = (int)numAutoProps.u.integer;

	int * Parameters = (int *) _alloca(sizeof(int) * ParameterCount);

	for (int i = 0; i < ParameterCount; ++ i)
	{
		// if i == 0 (first parameter of expression) we call GET_PARAM_1, else we call GET_PARAM_2
		switch (Info->Parameter[i].ep)
		{
			case ExpParams::String:
				Parameters[i] = CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1+(i > 0), TYPE_STRING, param);

				// Catch null string parameters and return "" or 0 as appropriate
				if (!Parameters[i])
				{
  					MessageBoxA(NULL, "Error calling expression: null pointer given as string parameter.", "DarkEdif - Expression() error", MB_OK);
					return (ExpressionRet != ExpReturnType::String) ? 0L :
								(long)rdPtr->pExtension->Runtime.CopyString(_T(""));
				}
				break;

			// In 3rd parameter, use 2 for float; use 0 for long/int
			// If you << 1, it has the same effect as multiplying by 2, only faster
			case ExpParams::Integer:
				Parameters[i] = CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1+(i > 0), ((Info->FloatFlags & (1 << i)) != 0) << 1, param);
				break;
		}
	}

	int Result;
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

		push [edx]
		sub edx, 4

		dec ecx

		cmp ecx, 0
		jne PushLoop

	CallNow:

		mov ecx, Extension
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

	// Must be after the expression func is evaluated, as sub-expressions inside the
	// expression func (e.g. from generating events) could change it to something else
	if (ExpressionRet == ExpReturnType::Float)
		rdPtr->rHo.Flags |= HeaderObjectFlags::Float;
	else if (ExpressionRet == ExpReturnType::String)
		rdPtr->rHo.Flags |= HeaderObjectFlags::String;

	return Result;
}

int Edif::GetDependency (char *& Buffer, size_t &Size, const TCHAR * FileExtension, int Resource)
{
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
	{
		TCHAR msgText[128];
		_stprintf_s(msgText, _T("Missing a resource! Type %s, number %i."), FileExtension, Resource);
		MessageBox(NULL, msgText, _T(PROJECT_NAME " - Error"), MB_ICONERROR);
		return DependencyNotFound;
	}

	Size = SizeofResource (hInstLib, res);
	Buffer = (char *) LockResource (LoadResource (hInstLib, res));

	return DependencyWasResource;
}

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

#ifdef _UNICODE
WCHAR* Edif::ConvertString(const char* utf8String)
{
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, 0, 0);
	if ( Length == 0 )
		Length = 1;
	WCHAR* tstr = (WCHAR*)calloc(Length, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, Length);
	return tstr;
}

WCHAR* Edif::ConvertAndCopyString(WCHAR* tstr, const char* utf8String, int maxLength)
{
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, tstr, maxLength);
	return tstr;
}
#else
char* Edif::ConvertString(const char* utf8String)
{
	// Convert string to Unicode
	size_t Length = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, 0, 0);
	if ( Length == 0 )
		Length = 1;
	WCHAR* wstr = (WCHAR*)calloc(Length, sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, wstr, Length);

	// Convert Unicode string using current user code page
	int len2 = WideCharToMultiByte(CP_ACP, 0, wstr, -1, 0, 0, nullptr, nullptr);
	if ( len2 == 0 )
		len2 = 1;
	char* str = (char*)calloc(len2, sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len2, nullptr, nullptr);
	free(wstr);

	return str;
}

char* Edif::ConvertAndCopyString(char* str, const char* utf8String, int maxLength)
{
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
}
#endif // _UNICODE

