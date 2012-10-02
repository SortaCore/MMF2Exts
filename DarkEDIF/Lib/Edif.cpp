#include "Common.h"

Edif::SDK * SDK = NULL;


TCHAR Edif::LanguageCode[3];

bool Edif::IsEdittime;
bool Edif::ExternalJSON;

HMENU Edif::ActionMenu, Edif::ConditionMenu, Edif::ExpressionMenu;

short ReadParameterType(const char * Text, bool &IsFloat)
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
		return Params::Custom_Base + atoi(Text+6);

	MessageBoxA(NULL, "Error reading Parameter type; text did not match anything.", "DarkEDIF - Error", MB_OK);
    return 0;
}

short ReadExpressionParameterType(const char * Text, bool &IsFloat)
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

	MessageBoxA(NULL, "Error reading Parameter type; text did not match anything.", "DarkEDIF - Error", MB_OK);
    return 0;
}

char ReadExpressionReturnType(const char * Text)
{
	if (!_stricmp(Text, "Integer"))
		return 0;

	if (!_stricmp(Text, "Float"))
		return 1;
	
	if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
		return 3;
	
	MessageBoxA(NULL, "Error reading expression return; returns text unrecognised.", "DarkEDIF - Error", MB_OK);
	return 0;
}

void Edif::Init(mv * mV, EDITDATA * edPtr)
{
	IsEdittime = mV->HMainWin != 0;
    mvInvalidateObject(mV, edPtr);
}

void Edif::Free(mv * mV)
{   
	if (::SDK)
	{
		delete ::SDK;
		::SDK = NULL;
	}
}

void Edif::Free(EDITDATA * edPtr)
{
	// ??
}

int Edif::Init(mv * mV)
{
	if (::SDK)
		return -1;
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

	// Get JSON file
    char * JSON;
    size_t JSON_Size;

    int result = Edif::GetDependency (JSON, JSON_Size, _T("json"), IDR_EDIF_JSON);
    
    if (result == Edif::DependencyNotFound)
    {
		TCHAR temp [MAX_PATH];
		GetModuleFileName (hInstLib, temp, sizeof(temp)/sizeof(TCHAR));
		TCHAR * filetitle = _tcsrchr(temp, _T('\\'));
		if ( filetitle != NULL )
			_tcscpy(temp, filetitle);
		else
			temp[0] = 0;
		_tcscat(temp, _T(" - Error"));

		MessageBox(0, _T("JSON file not found on disk or in MFX resources"), temp, 0);
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

    json_value * json = json_parse_ex (&settings, copy, json_error);

    if (!json)
    {
		MessageBoxA(0, json_error, "Error parsing JSON", 0);
		return -1;
    }

    ::SDK = new Edif::SDK(mV, *json);

    return 0;	// no error
}

// Used for reading the icon image file
BOOL DLLExport ImportImageFromInputFile(CImageFilterMgr* pImgMgr, CInputFile* pf, cSurface* psf, LPDWORD pDWFilterID, DWORD dwFlags);

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;

	#ifndef RUN_ONLY
		Icon = new cSurface;
		if (mV->ImgFilterMgr)
		{
			char * IconData;
			size_t IconSize;

			int result = Edif::GetDependency (IconData, IconSize, _T("png"), IDR_EDIF_ICON);
			if (result != Edif::DependencyNotFound)
			{
				CInputMemFile * File = CInputMemFile::NewInstance();
				File->Create ((LPBYTE)IconData, IconSize);
			
				unsigned long PNG = 'PNG ';
				ImportImageFromInputFile(mV->ImgFilterMgr, File, Icon, &PNG, 0);
		        
				File->Delete();

				if (!Icon->HasAlpha())
					Icon->SetTransparentColor(RGB(255, 0, 255));
				if (result != Edif::DependencyWasResource)
					free(IconData);
			}
		}
	#endif // !RUN_ONLY
	
	if (!::SDK)
		::SDK = this;
    const json_value &Actions = json[CurLang]["Actions"];
    const json_value &Conditions = json[CurLang]["Conditions"];
    const json_value &Expressions = json[CurLang]["Expressions"];
	const json_value &Properties = json[CurLang]["Properties"];

    ActionJumps = new void * [Actions.u.object.length + 1];
    ConditionJumps = new void * [Conditions.u.object.length + 1];
    ExpressionJumps = new void * [Expressions.u.object.length + 1];

    ActionJumps [Actions.u.object.length] = 0;
    ConditionJumps [Conditions.u.object.length] = 0;
    ExpressionJumps [Expressions.u.object.length] = 0;
    
    for(unsigned int i = 0; i < Actions.u.object.length; ++ i)
    {
		ActionJumps [i] = (void *) Edif::Action;

		ActionFunctions.push_back(0);

		// The function cleverly determines how to create itself based on already existing instances.
		CreateNewActionInfo();
    }

    for(unsigned int i = 0; i < Conditions.u.object.length; ++ i)
    {
		ConditionJumps [i] = (void *) Edif::Condition;

		ConditionFunctions.push_back(0);

		// The function cleverly determines how to create itself based on already existing instances.
		CreateNewConditionInfo();
    }

    for(unsigned int i = 0; i < Expressions.u.object.length; ++ i)
    {
		ExpressionJumps [i] = (void *) Edif::Expression;

		ExpressionFunctions.push_back(0);

		// The function cleverly determines how to create itself based on already existing instances.
		CreateNewExpressionInfo();
    }

	// Phi woz 'ere
	{
		std::vector <PropData> VariableProps;
		PropData * CurrentProperty;

		for (unsigned  int i = 0; i < Properties.u.object.length; ++ i)
		{
			const json_value &Property = Properties[i];
			CurrentProperty = nullptr;

			// Reserved/invalid properties are marked with ! at the start.
			// If some muppet attempts to use 'em, throw an error.
			if ( ((const char *) Property["Type"])[0] == '!')
			{
				std::string Error = "You have specified an invalid Parameter type: [" + 
									std::string((const char *)Property["Type"]) + 
									"].\r\nPlease ensure you are permitted to use this.";
			
				MessageBoxA(NULL, Error.c_str(), "DarkEDIF - JSON Property parser", MB_OK);
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
							CurrentProperty = new PropData(VariableProps.size(), j);
							break;
						}
					}
				
					if (!CurrentProperty)
					{
						char * temp = (char *)calloc(256, sizeof(char));
						if (temp)
						{
							sprintf_s(temp, 256, "The Parameter type specified was unrecognised: [%s]."
												 "Check your spelling of the \"Type\" Parameter.", (const char *)Property["Type"]);
							MessageBoxA(NULL, temp, "DarkEDIF error", MB_OK);
							free(temp);
						}
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
					
						CurrentProperty->Title = Property["Title"];
						CurrentProperty->Info = Property["Info"];

						switch (CurrentProperty->Type_ID)
						{
																																																																																				/*
					#define PropData_Folder(0,	(LParams)NULL}
#define PropData_Folder_End() 0,	(LParams)NULL}
#define PropData_Group(0,	(LParams)NULL}
#define PropData_StaticString(0,	(LParams)NULL}
#define PropData_StaticString_Opt(opt,	(LParams)NULL}
#define PropData_StaticString_List(PROPOPT_LIST,	(LParams)NULL}
#define PropData_EditString(0,	(LParams)NULL}
#define PropData_EditString_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_EditString_Opt(opt,	(LParams)NULL}
#define PropData_EditNumber(0,	(LParams)NULL}
#define PropData_EditNumber_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_EditNumber_Opt(opt,	(LParams)NULL}
#define PropData_EditFloat(0,	(LParams)NULL}
#define PropData_EditFloat_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_EditFloat_Opt(opt,	(LParams)NULL}
#define PropData_EditMultiLine(0,	(LParams)NULL}
#define PropData_EditMultiLine_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_EditMultiLine_Opt(opt,	(LParams)NULL}
#define PropData_SliderEdit(PROPOPT_ParamsREQUIRED,	(LParams)minmax}
#define PropData_SliderEdit_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)minmax}
#define PropData_SliderEdit_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)minmax}
#define PropData_SpinEdit(PROPOPT_ParamsREQUIRED,	(LParams)minmax}
#define PropData_SpinEdit_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)minmax}
#define PropData_SpinEdit_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)minmax}
#define PropData_Button(0,	 (LParams)text}
#define PropData_Button_Check(PROPOPT_CHECKBOX,	(LParams)text}
#define PropData_Button_Opt(opt,	(LParams)text}
#define PropData_EditButton(0,	 (LParams)NULL}
#define PropData_EditButton_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_EditButton_Opt(opt,	(LParams)NULL}
#define PropData_Size(	PROPOPT_ParamsREQUIRED,	(LParams)tab}
#define PropData_Size_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)tab}
#define PropData_Size_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)tab}
#define PropData_Color(0,	(LParams)NULL}
#define PropData_Color_Check(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_Color_Opt(opt,	(LParams)NULL}
#define PropData_ComboBox(PROPOPT_ParamsREQUIRED,	(LParams)list}
#define PropData_ComboBox_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)list}
#define PropData_ComboBox_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)list}
#define PropData_CheckBox(PROPOPT_CHECKBOX,	(LParams)NULL}
#define PropData_CheckBox_Opt((PROPOPT_CHECKBOX|opt),	(LParams)NULL}
#define PropData_DirCtrl(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_DirCtrl_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_DirCtrl_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_Filename(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_Filename_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_Filename_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_PictureFilename(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_PictureFilename_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_PictureFilename_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_Font(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_Font_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_Font_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_Custom(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_Custom_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_Custom_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_ComboBoxBtn(PROPOPT_ParamsREQUIRED,	(LParams)list}
#define PropData_ComboBoxBtn_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)list}
#define PropData_ComboBoxBtn_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)list}
#define PropData_ImageList(0,	NULL}
#define PropData_ImageList_Check(PROPOPT_CHECKBOX,	NULL}
#define PropData_ImageList_Opt(opt,	NULL}
#define PropData_IconComboBox(PROPOPT_ParamsREQUIRED,	(LParams)list}
#define PropData_IconComboBox_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)list}
#define PropData_IconComboBox_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)list}
#define PropData_URLButton(PROPOPT_ParamsREQUIRED,	 (LParams)url}
#define PropData_URLButton_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)url}
#define PropData_URLButton_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)url}
#define PropData_DirectoryName(PROPOPT_ParamsREQUIRED,	(LParams)Params}
#define PropData_DirectoryName_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)Params}
#define PropData_DirectoryName_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)Params}
#define PropData_SpinEditFloat(PROPOPT_ParamsREQUIRED,	(LParams)minmaxdelta}
#define PropData_SpinEditFloat_Check((PROPOPT_ParamsREQUIRED|PROPOPT_CHECKBOX),	(LParams)minmaxdelta}
#define PropData_SpinEditFloat_Opt((PROPOPT_ParamsREQUIRED|opt),	(LParams)minmaxdelta}
#define PropData_End() {0}
					*/
					// Simple static text
						
							case PROPTYPE_STATIC:
								SetAllProps(0, NULL);
					
							// Folder
							case PROPTYPE_FOLDER:
								SetAllProps(0, NULL);

							// Edit button, Params1 = button text, or nullptr if Edit
							case PROPTYPE_EDITBUTTON:
								SetAllProps(PROPOPT_PARAMREQUIRED, (Property["Text"] == "") ? 0 : (const char *)Property["Text"]);
					
							// Edit box for strings, Parameter = max length
							case PROPTYPE_EDIT_STRING:
								Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0)	// Checkbox enabled by property option in JSON
										|  ((Property["Password"]) ? PROPOPT_EDIT_PASSWORD: 0);
								SetAllProps(PROPOPT_PARAMREQUIRED, (long)Property["MaxLength"]);
					
							// Edit box for numbers, Parameters = min value, max value
							case PROPTYPE_EDIT_NUMBER:
							{
								int * temp = new int[2];
								temp[0] = long(Property["Minimum"]);
								temp[1] = long(Property["Maximum"]);
								SetAllProps(PROPOPT_PARAMREQUIRED, temp);
							}

							// Combo box, Parameters = list of strings, options (sorted, etc)
							case PROPTYPE_COMBOBOX:
							{
								if (Property["Items"].u.object.length == 0)
									MessageBoxA(NULL, "Warning: no default items detected in list.", "DarkEDIF warning", MB_OK);
							
								#pragma message ("Note: Is forced-char incompatible with Unicode?")
								const char ** Fixed = new const char * [Property["Items"].u.object.length+2];

								// NULL is required at start of array
								Fixed[0] = Fixed[Property["Items"].u.object.length+1] = nullptr;

								// Use decrementation and copy to fixed list.
								for (unsigned int index = 1; index < Property["Items"].u.object.length+1; ++ index)
								{
									Fixed[index] = Property["Items"][index-1];
								}

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
								char * temp = (char *)calloc(256, sizeof(char));
								if (temp)
								{
									sprintf_s(temp, 256, "The Parameter type specified was unrecognised: [%s]."
														 "Check your spelling of the \"Type\" Parameter.", Property["Type"]);
									MessageBoxA(NULL, temp, "DarkEdif JSON error", MB_OK);
									free(temp);
								}
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

    ActionMenu = LoadMenuJSON(Edif::ActionID(0), json[CurLang]["ActionMenu"]);
    ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), json[CurLang]["ConditionMenu"]);
    ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), json[CurLang]["ExpressionMenu"]);
}

Edif::SDK::~SDK()
{
    json_value_free (&json);

    delete [] ActionJumps;
    delete [] ConditionJumps;
    delete [] ExpressionJumps;
	delete [] EdittimeProperties;

	delete Icon;
}

int ActionOrCondition(void * Function, int ID, RUNDATA * rdPtr, long Params1, long Params2)
{
	bool Condition = (SDK->ConditionFunctions.size() >= (unsigned int)ID+1) && (SDK->ConditionFunctions[ID] == Function);
    int * Parameters;
    int ParameterCount;

	const ACEInfo * Info = Condition ? SDK->ConditionInfos[ID] : SDK->ActionInfos[ID];

	ParameterCount = Info->NumOfParams;
	Parameters = (int *) alloca(sizeof(int) * ParameterCount);
	for(int i = 0; i < ParameterCount; ++ i)
	{
		switch (Info->Parameter[i])
		{
			case Params::Expression:
				Parameters[i] = (Info->FloatFlags & (1 << i)) ?
								CNC_GetFloatParameter(rdPtr) :
								CNC_GetIntParameter(rdPtr);
				break;

			case Params::String_Expression:
			case Params::Filename:
				Parameters[i] = CNC_GetStringParameter(rdPtr);
				break;

			default:
				Parameters[i] = CNC_GetParameter(rdPtr);
				break;
		}
	}

    void * Extension = rdPtr->pExtension;

    int Result;
	
    __asm
    {
        pushad                  ; Start new register set (do not interfere with already existing registers)

        mov ecx, ParameterCount ; Store ParameterCount in ecx
        
        cmp ecx, 0              ; If no parameters, call function immediately
        je CallNow

        mov edx, Parameters     ; Otherwise store pointer to int * in Parameters

        mov ebx, ecx            ; Copy ecx, or ParameterCount, to ebx
        shl ebx, 2              ; Multiply parameter count by 2^2 (size of 32-bit variable)

        add edx, ebx            ; add (ParameterCount * 4) to Parameters, making edx point to Parameters[param count]
        sub edx, 4              ; subtract 4 from edx, making it 0-based (ending array index)

        PushLoop:

            push [edx]          ; Push value pointed to by Parameters[edx]
            sub edx, 4          ; Decrement next loop`s Parameter index:    for (><; ><; edx -= 4)

            dec ecx             ; Decrement loop index:                     for (><; ><; ecx--)

            cmp ecx, 0          ; If ecx == 0, end loop:                    for (><; ecx == 0; ><)
            jne PushLoop        ; Optimisation: "cmp ecx, 0 / jne" can be replaced with "jcxz"

        CallNow:

        mov ecx, Extension      ; Move Extension to ecx
        call Function           ; Call the function inside Extension
        
        mov Result, eax         ; Function`s return is stored in eax; copy it to Result

        popad                   ; End new register set (restore registers that existed before popad)
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
		    if (!_stricmp(MenuItem, "Separator"))
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

long __stdcall Edif::Condition(RUNDATA * rdPtr, long param1, long param2)
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

short __stdcall Edif::Action(RUNDATA * rdPtr, long param1, long param2)
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

long __stdcall Edif::Expression(RUNDATA * rdPtr, long param)
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
	int ExpressionRet = Info->Returns;
	
	if (ExpressionRet == ExpParams::Float)
		rdPtr->rHo.Flags |= HOF_FLOAT;
	else if (ExpressionRet == ExpParams::String)
		rdPtr->rHo.Flags |= HOF_STRING;
	
	int ParameterCount = Info->NumOfParams;
	int * Parameters = (int *) _alloca(sizeof(int) * ParameterCount);

	for (int i = 0; i < ParameterCount; ++ i)
	{
		// if i == 0 (first parameter of expression) we call GET_PARAM_1, else we call GET_PARAM_2
		switch (Info->Parameter[i])
		{
			case ExpParams::String:
				Parameters[i] = CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1+(i > 0), TYPE_STRING, param);
				
				// Catch null string parameters and return "" or 0 as appropriate
				if (!Parameters[i])
				{
  					MessageBoxA(NULL, "Error calling expression: null pointer given as string parameter.", "DarkEDIF - Expression() error", MB_OK);
					return (ExpressionRet != ExpParams::String) ? 0L :
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

		mov ecx, ExpressionRet

		cmp ecx, 1
		jne NotFloat

		fstp Result
		jmp End

    NotFloat:
		
		mov Result, eax
		
    End:

		popad
    }
     
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
		return DependencyNotFound;

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

		{   TCHAR * Iterator = ExecutablePath + _tcslen(ExecutablePath) - 1;

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
