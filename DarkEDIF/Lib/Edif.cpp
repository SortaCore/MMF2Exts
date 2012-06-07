#include "Common.h"

Edif::SDK * SDK;

TCHAR Edif::LanguageCode[3];

bool Edif::IsEdittime;
bool Edif::ExternalJSON;

HMENU Edif::ActionMenu, Edif::ConditionMenu, Edif::ExpressionMenu;

// #__LINE__ is invalid, # can only be used for macro parameters.
// This definition allows use of __LINE__ in MessageBoxA() errors.
#define strfy(line) #line


short ReadParameterType(const char * Text)
{
    if(!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
        return PARAM_EXPSTRING;

    if(!_stricmp(Text, "Filename") || !_stricmp(Text, "File"))
        return PARAM_FILENAME;

    if(!_stricmp(Text, "Float"))
        return PARAM_EXPRESSION;

    if(!_stricmp(Text, "Integer"))
        return PARAM_EXPRESSION;

    if(!_stricmp(Text, "Object"))
        return PARAM_OBJECT;

	if(!_stricmp(Text, "Position"))
		return PARAM_POSITION;

	if(!_stricmp(Text, "Create"))
		return PARAM_CREATE;

	if(!_stricmp(Text, "SysCreate"))
		return PARAM_SYSCREATE;

	if(!_stricmp(Text, "Animation"))
		return PARAM_ANIMATION;

	if(!_stricmp(Text, "Nop"))
		return PARAM_NOP;

	if(!_stricmp(Text, "Player"))
		return PARAM_PLAYER;

	if(!_stricmp(Text, "Every"))
		return PARAM_EVERY;

	if(!_stricmp(Text, "Key"))
		return PARAM_KEY;

	if(!_stricmp(Text, "Speed"))
		return PARAM_SPEED;

	if(!_stricmp(Text, "JoyDirection"))
		return PARAM_JOYDIRECTION;

	if(!_stricmp(Text, "Shoot"))
		return PARAM_SHOOT;

	if(!_stricmp(Text, "Zone"))
		return PARAM_ZONE;

	if(!_stricmp(Text, "Comparison"))
		return PARAM_COMPARAISON;
		
	if(!_stricmp(Text, "StringComparison"))
		return PARAM_CMPSTRING;

	if(!_stricmp(Text, "Colour") || !_stricmp(Text, "Color"))
		return PARAM_COLOUR;

	if(!_stricmp(Text, "Frame"))
		return PARAM_FRAME;

	if(!_stricmp(Text, "SampleLoop"))
		return PARAM_SAMLOOP;

	if(!_stricmp(Text, "MusicLoop"))
		return PARAM_MUSLOOP;

	if(!_stricmp(Text, "NewDirection"))
		return PARAM_NEWDIRECTION;

	if(!_stricmp(Text, "TextNumber"))
		return PARAM_TEXTNUMBER;

	if(!_stricmp(Text, "Click"))
		return PARAM_CLICK;

	if(!_stricmp(Text, "Program"))
		return PARAM_PROGRAM;

	if(!_strnicmp(Text, "Custom", 6))
		return PARAM_EXTBASE+atoi(Text+6);

    return 0;
}

short ReadExpressionParameterType(const char * Text)
{
    if(!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
        return EXPPARAM_STRING;

    if(!_stricmp(Text, "Float"))
        return EXPPARAM_LONG;

    if(!_stricmp(Text, "Integer"))
        return EXPPARAM_LONG;

    return 0;
}

void Edif::Init(mv * _far mV, LPEDATA edPtr)
{   
    IsEdittime = mV->mvHMainWin != 0;

    mvInvalidateObject(mV, edPtr);
}

void Edif::Free(mv * _far mV)
{   
	delete ::SDK;
	::SDK = nullptr;
}

void Edif::Free(LPEDATA edPtr)
{
}

int Edif::Init(mv _far * mV)
{
    _tcscpy (LanguageCode, _T ("EN"));

	// Get pathname of MMF2
	LPTSTR mmfname = (LPTSTR)calloc(_MAX_PATH, sizeof(TCHAR));
	if ( mmfname != nullptr )
	{
		// Load resources
		GetModuleFileName (nullptr, mmfname, _MAX_PATH);
		HINSTANCE hRes = LoadLibraryEx(mmfname, nullptr, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
		if ( hRes != nullptr )
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
		LPTSTR filetitle = _tcsrchr(temp, _T('\\'));
		if ( filetitle != nullptr )
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

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;

#ifndef RUN_ONLY
    Icon = new cSurface;
    if(mV->mvImgFilterMgr)
    {
        char * IconData;
        size_t IconSize;

        int result = Edif::GetDependency (IconData, IconSize, _T("png"), IDR_EDIF_ICON);
		if (result != Edif::DependencyNotFound)
        {
            CInputMemFile * File = CInputMemFile::NewInstance ();
            File->Create ((LPBYTE)IconData, IconSize);

            DWORD PNG = FILTERID_PNG;
            ImportImageFromInputFile(mV->mvImgFilterMgr, File, Icon, &PNG, 0);
                
            File->Delete();

            if(!Icon->HasAlpha())
                Icon->SetTransparentColor(RGB(255, 0, 255));
			if ( result != Edif::DependencyWasResource )
				free(IconData);
        }
    }
#endif // RUN_ONLY

    const json_value &Actions = json["Actions"];
    const json_value &Conditions = json["Conditions"];
    const json_value &Expressions = json["Expressions"];
	const json_value &Properties = json["Properties"];

    ActionJumps = new void * [Actions.u.array.length + 1];
    ConditionJumps = new void * [Conditions.u.array.length + 1];
    ExpressionJumps = new void * [Expressions.u.array.length + 1];

    ActionJumps [Actions.u.array.length] = 0;
    ConditionJumps [Conditions.u.array.length] = 0;
    ExpressionJumps [Expressions.u.array.length] = 0;
    
    for(unsigned int i = 0; i < Actions.u.array.length; ++ i)
    {
        const json_value &Action = Actions[i];

        ActionJumps [i] = (void *) Edif::Action;

        ActionFunctions.push_back(0);

        int ActionID = Edif::ActionID(i);

        ActionInfos.push_back(ActionID);  // Menu ID
        ActionInfos.push_back(ActionID);  // String ID
        ActionInfos.push_back(i);         // Code

        ActionInfos.push_back(0);         // Flags

        const json_value &Parameters = Action["Parameters"];

        ActionInfos.push_back(Parameters.u.array.length);

        {   short IsFloat = 0;

            for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if(Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ActionInfos.push_back(ReadParameterType(Type));
                
                if(!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ActionFloatFlags.push_back(IsFloat);
        }

        for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ActionInfos.push_back(0);
    }

    for(unsigned int i = 0; i < Conditions.u.array.length; ++ i)
    {
        const json_value &Condition = Conditions[i];

        ConditionJumps [i] = (void *) Edif::Condition;

        ConditionFunctions.push_back(0);

        int ConditionID = Edif::ConditionID(i);

        ConditionInfos.push_back(ConditionID);
        ConditionInfos.push_back(ConditionID);
        ConditionInfos.push_back(i);

        ConditionInfos.push_back(((bool) Condition["Triggered"]) ? 0 : (EVFLAGS_ALWAYS | EVFLAGS_NOTABLE));

        const json_value &Parameters = Condition["Parameters"];

        ConditionInfos.push_back(Parameters.u.array.length);

        {   short IsFloat = 0;

            for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if(Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ConditionInfos.push_back(ReadParameterType(Type));
                
                if(!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ConditionFloatFlags.push_back(IsFloat);
        }

        for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ConditionInfos.push_back(0);
    }

    for(unsigned int i = 0; i < Expressions.u.array.length; ++ i)
    {
        const json_value &Expression = Expressions[i];

        ExpressionJumps [i] = (void *) Edif::Expression;

        ExpressionFunctions.push_back(0);

        if(!_stricmp(Expression["Returns"], "Integer"))
            ExpressionTypes.push_back(0);
        else if(!_stricmp(Expression["Returns"], "Float"))
            ExpressionTypes.push_back(1);
        else if(!_stricmp(Expression["Returns"], "Text") || !_stricmp(Expression["Returns"], "String"))
            ExpressionTypes.push_back(2);
        else
            ExpressionTypes.push_back(0);

        int ExpressionID = Edif::ExpressionID(i);

        ExpressionInfos.push_back(ExpressionID);
        ExpressionInfos.push_back(ExpressionID);
        ExpressionInfos.push_back(i);

        {   short FlagsValue = 0;

            if(ExpressionTypes.back() == 1)
                FlagsValue |= EXPFLAG_DOUBLE;

            if(ExpressionTypes.back() == 2)
                FlagsValue |= EXPFLAG_STRING;

            ExpressionInfos.push_back(FlagsValue);
        }

        const json_value &Parameters = Expression["Parameters"];

        ExpressionInfos.push_back(Parameters.u.array.length);

        {   short IsFloat = 0;

            for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if(Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ExpressionInfos.push_back(ReadExpressionParameterType(Type));
                
                if(!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ExpressionFloatFlags.push_back(IsFloat);
        }

        for(unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ExpressionInfos.push_back(0);
    }

	// Phi woz 'ere
	std::vector<PropData> VariableProps;

	for(unsigned  int i = 0; i < Properties.u.array.length; ++ i)
    {
        const json_value &Property = Properties[i];
		PropData * CurrentProperty = nullptr;
		MessageBoxA(NULL, Property["Type"], "Property type????:", MB_OK);

		// Reserved/invalid properties are marked with ! at the start.
		// If some muppet attempts to use 'em, throw an error.
		if ( ((const char *) Property["Type"])[0] == '!')
		{
			MessageBoxA(NULL, "You have specified an invalid parameter type.\r\n"
							  "Please ensure you are permitted to use this.",
							  "Dark EDIF - JSON Property parser", MB_OK);
		}
		else // Property is not reserved
		{
			#define SetAllProps(opt,lparam) CurrentProperty->SetAllProperties(Property["Name"], Property["Info"], Options|(opt), (LPARAM)lparam); break
			
			// Custom parameter: Read the number CustomXXX and use that.
			if (!_strnicmp(Property["Type"], "Custom", 6))
			{
				MessageBoxA(NULL, "Detected as custom.", "Edif note", MB_OK);
				CurrentProperty = new PropData(VariableProps.size(), i + Edif::Properties::PROPTYPE_LAST_ITEM);
			}
			else // Regular parameter
			{
				char * temp = (char *)calloc(256, sizeof(char));
				
				// Loop through parameter names and compareth them.
				for(unsigned int j = Edif::Properties::PROPTYPE_FIRST_ITEM;
					j < (Edif::Properties::PROPTYPE_LAST_ITEM - Edif::Properties::PROPTYPE_FIRST_ITEM);
					++ j)
				{
					if(!_stricmp(Property["Type"], Edif::Properties::Names[j]))
					{
						sprintf_s(temp, 256, "ID chosen: %i.", VariableProps.size());
						MessageBoxA(NULL, temp, "Line "strfy(__LINE__)":", MB_OK);
						CurrentProperty = new PropData(VariableProps.size(), i);
						break;
					}
				}
				
				sprintf(temp, "ID chosen: %i.", VariableProps.size());
				MessageBoxA(NULL, temp, "Line "strfy(__LINE__)":", MB_OK);
				
				if(!CurrentProperty)
				{
					sprintf_s(temp, 256, "The parameter type specified was unrecognised: [%s]."
										 "Check your spelling of the \"Type\" parameter.", (const char *)Property["Type"]);
					MessageBoxA(NULL, temp, "Dark EDIF error", MB_OK);
					
				}
				else
				{
					// If checkbox is enabled, pass that as flags as well.
					DWORD Options = (Property["Checkbox"] ? PROPOPT_CHECKBOX : 0)		// Checkbox enabled by property option in JSON
								  | (Property["Bold"] ? PROPOPT_BOLD: 0)				// Bold enabled by property option in JSON
								  | (Property["Removable"] ? PROPOPT_REMOVABLE: 0)		// Removable enabled by property option in JSON
								  | (Property["Renameable"] ? PROPOPT_RENAMEABLE: 0)	// Renamable enabled by property option in JSON
								  | (Property["Moveable"] ? PROPOPT_MOVABLE: 0)			// Movable enabled by property option in JSON
								  | (Property["List"] ? PROPOPT_LIST: 0)				// List enabled by property option in JSON
								  | (Property["SingleSelect"] ? PROPOPT_SINGLESEL: 0);	// Single-select enabled by property option in JSON
					bool EnableLPARAM = false;
					// Todo: passing of LPARAM requires PARAMREQUIRED option.
					// Find out what opt is.
					// Two settings may be specified by |=ing the options DWORD.

					switch(CurrentProperty->lType)
					{
						/*
						#define PropData_Folder(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_Folder_End() PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_Group(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_StaticString(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_StaticString_Opt(opt,	(LPARAM)NULL}
#define PropData_StaticString_List(PROPOPT_LIST,	(LPARAM)NULL}
#define PropData_EditString(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_EditString_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_EditString_Opt(opt,	(LPARAM)NULL}
#define PropData_EditNumber(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_EditNumber_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_EditNumber_Opt(opt,	(LPARAM)NULL}
#define PropData_EditFloat(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_EditFloat_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_EditFloat_Opt(opt,	(LPARAM)NULL}
#define PropData_EditMultiLine(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_EditMultiLine_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_EditMultiLine_Opt(opt,	(LPARAM)NULL}
#define PropData_SliderEdit(PROPOPT_PARAMREQUIRED,	(LPARAM)minmax}
#define PropData_SliderEdit_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)minmax}
#define PropData_SliderEdit_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)minmax}
#define PropData_SpinEdit(PROPOPT_PARAMREQUIRED,	(LPARAM)minmax}
#define PropData_SpinEdit_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)minmax}
#define PropData_SpinEdit_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)minmax}
#define PropData_Button(PROPOPT_NIL,	 (LPARAM)text}
#define PropData_Button_Check(PROPOPT_CHECKBOX,	(LPARAM)text}
#define PropData_Button_Opt(opt,	(LPARAM)text}
#define PropData_EditButton(PROPOPT_NIL,	 (LPARAM)NULL}
#define PropData_EditButton_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_EditButton_Opt(opt,	(LPARAM)NULL}
#define PropData_Size(	PROPOPT_PARAMREQUIRED,	(LPARAM)tab}
#define PropData_Size_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)tab}
#define PropData_Size_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)tab}
#define PropData_Color(PROPOPT_NIL,	(LPARAM)NULL}
#define PropData_Color_Check(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_Color_Opt(opt,	(LPARAM)NULL}
#define PropData_ComboBox(PROPOPT_PARAMREQUIRED,	(LPARAM)list}
#define PropData_ComboBox_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)list}
#define PropData_ComboBox_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)list}
#define PropData_CheckBox(PROPOPT_CHECKBOX,	(LPARAM)NULL}
#define PropData_CheckBox_Opt((PROPOPT_CHECKBOX|opt),	(LPARAM)NULL}
#define PropData_DirCtrl(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_DirCtrl_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_DirCtrl_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_Filename(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_Filename_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_Filename_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_PictureFilename(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_PictureFilename_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_PictureFilename_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_Font(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_Font_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_Font_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_Custom(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_Custom_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_Custom_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_ComboBoxBtn(PROPOPT_PARAMREQUIRED,	(LPARAM)list}
#define PropData_ComboBoxBtn_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)list}
#define PropData_ComboBoxBtn_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)list}
#define PropData_ImageList(0,	NULL}
#define PropData_ImageList_Check(PROPOPT_CHECKBOX,	NULL}
#define PropData_ImageList_Opt(opt,	NULL}
#define PropData_IconComboBox(PROPOPT_PARAMREQUIRED,	(LPARAM)list}
#define PropData_IconComboBox_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)list}
#define PropData_IconComboBox_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)list}
#define PropData_URLButton(PROPOPT_PARAMREQUIRED,	 (LPARAM)url}
#define PropData_URLButton_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)url}
#define PropData_URLButton_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)url}
#define PropData_DirectoryName(PROPOPT_PARAMREQUIRED,	(LPARAM)param}
#define PropData_DirectoryName_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)param}
#define PropData_DirectoryName_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)param}
#define PropData_SpinEditFloat(PROPOPT_PARAMREQUIRED,	(LPARAM)minmaxdelta}
#define PropData_SpinEditFloat_Check((PROPOPT_PARAMREQUIRED|PROPOPT_CHECKBOX),	(LPARAM)minmaxdelta}
#define PropData_SpinEditFloat_Opt((PROPOPT_PARAMREQUIRED|opt),	(LPARAM)minmaxdelta}
#define PropData_End() {0}
						*/
						// Simple static text
						case PROPTYPE_STATIC:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Folder
						case PROPTYPE_FOLDER:
							SetAllProps(PROPOPT_NIL, NULL);

						// Folder End
						case PROPTYPE_FOLDER_END:
							CurrentProperty->dwID = -1;
							CurrentProperty->SetAllProperties("", "", PROPOPT_NIL, NULL);
							break;
					
						// Edit button, param1 = button text, or nullptr if Edit
						case PROPTYPE_EDITBUTTON:
							SetAllProps(PROPOPT_PARAMREQUIRED, Property["Text"]);
					
						// Edit box for strings, parameter = max length
						case PROPTYPE_EDIT_STRING:
							Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)			// Checkbox enabled by property option in JSON
									|  ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0)			// Checkbox enabled by property option in JSON
									|  ((Property["Password"]) ? PROPOPT_EDIT_PASSWORD: 0);
							SetAllProps(PROPOPT_PARAMREQUIRED, (long)Property["MaxLength"]);
					
						// Edit box for numbers, parameters = min value, max value
						case PROPTYPE_EDIT_NUMBER:

						// Combo box, parameters = list of strings, options (sorted, etc)
						case PROPTYPE_COMBOBOX:
						{
							std::vector<char *> Dynamic;
							Dynamic.push_back(nullptr); // NULL is required at start of array
							unsigned int index = 1;
							
							while(true)
							{
								// If string doesn't exist, end after adding a NULL (the array must terminate with one)
								// And yes, the two casts are necessary.
								Dynamic.push_back((char *)(const char *)Property["Items"][index]);
								if((const char *)Property["Items"][index] == nullptr)
									break;
								++index;
							}
							
							// Now that we know how many items are in the JSON, we can use a fixed char ** array.
							#pragma message ("Note: Is forced-char incompatible with Unicode?")
							char ** Fixed = new char * [Dynamic.size()];

							// Use decrementation and copy to fixed list.
							while(index >= 0)
								Fixed[index] = Dynamic[index--];

							// Pass fixed list as parameter
							SetAllProps(PROPOPT_PARAMREQUIRED, Fixed);
						}

						// Size
						case PROPTYPE_SIZE:

						// Color
						case PROPTYPE_COLOR:

						// Checkbox
						case PROPTYPE_LEFTCHECKBOX:
							// Add option to show it is a checkbox
							CurrentProperty->dwOptions |= PROPOPT_CHECKBOX;
							SetAllProps(PROPOPT_NIL, NULL);

						// Edit + Slider
						case PROPTYPE_SLIDEREDIT:
							SetAllProps(PROPOPT_NIL, NULL);

						// Edit + Spin
						case PROPTYPE_SPINEDIT:
							SetAllProps(PROPOPT_NIL, NULL);

						// Direction Selector
						case PROPTYPE_DIRCTRL:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Group
						case PROPTYPE_GROUP:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Edit box + browse file button, parameter = FilenameCreateParam
						case PROPTYPE_FILENAME:
							SetAllProps(PROPOPT_NIL, NULL);

						// Font dialog box
						case PROPTYPE_FONT:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Edit box + browse image file button
						case PROPTYPE_PICTUREFILENAME:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Combo box, parameters = list of strings, options (sorted, etc)
						case PROPTYPE_COMBOBOXBTN:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Edit box for floating point numbers, parameters = min value, max value, options (signed, float, spin)
						case PROPTYPE_EDIT_FLOAT:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Edit box for multiline texts, no parameter
						case PROPTYPE_EDIT_MULTILINE:
							Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE: 0)			// Checkbox enabled by property option in JSON
									|  ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE: 0);			// Checkbox enabled by property option in JSON
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Image list
						case PROPTYPE_IMAGELIST:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Combo box with icons
						case PROPTYPE_ICONCOMBOBOX:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// URL button
						case PROPTYPE_URLBUTTON:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Directory pathname
						case PROPTYPE_DIRECTORYNAME:
							SetAllProps(PROPOPT_NIL, NULL);
					
						// Edit + Spin, value = floating point number
						case PROPTYPE_SPINEDITFLOAT:
							SetAllProps(PROPOPT_NIL, NULL);

						// Unrecognised
						default:
							sprintf_s(temp, 256, "The parameter type specified was unrecognised: [%s]."
												 "Check your spelling of the \"Type\" parameter.", (const char *)Property["Type"]);
							MessageBoxA(NULL, temp, "Edif error", MB_OK);
							SetAllProps(PROPOPT_NIL, NULL);
					}
				}

				free(temp);
			}

			// Add to properties
			VariableProps.push_back(*CurrentProperty);
			

		}

		EdittimeProperties = new PropData[VariableProps.size()];
		// Use decrementation and copy to fixed list.
		for(unsigned int l = 0; l < VariableProps.size(); ++l)
			EdittimeProperties[l] = VariableProps[l--];
			
    }

    ActionMenu = LoadMenuJSON(Edif::ActionID(0), json["ActionMenu"]);
    ConditionMenu = LoadMenuJSON(Edif::ConditionID(0), json["ConditionMenu"]);
    ExpressionMenu = LoadMenuJSON(Edif::ExpressionID(0), json["ExpressionMenu"]);
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

int ActionOrCondition(vector<short> &FloatFlags, LPEVENTINFOS2 Info, void * Function, int ID, LPRDATA rdPtr, long param1, long param2)
{
    int * Parameters;
    int ParameterCount;

    {   Info = GetEventInformations(Info, ID);

        ParameterCount = Info->infos.nParams;
        Parameters = (int *) alloca(sizeof(int) * ParameterCount);

        for(int i = 0; i < ParameterCount; ++ i)
        {
            switch(EVINFO2_PARAM(Info, i))
            {
            case PARAM_EXPRESSION:

                if((FloatFlags[ID] & (1 << i)) != 0)
                {
                    Parameters[i] = CNC_GetFloatParameter(rdPtr);
                    break;
                }

                Parameters[i] = CNC_GetIntParameter(rdPtr);
                break;

            case PARAM_EXPSTRING:
            case PARAM_FILENAME:

                Parameters[i] = CNC_GetStringParameter(rdPtr);
                break;

            default:

                Parameters[i] = CNC_GetParameter(rdPtr);
                break;
            }
        }
    }

    void * Extension = rdPtr->pExtension;

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
            
        mov Result, eax

        popad
    }

    return Result;
}

HMENU Edif::LoadMenuJSON(int BaseID, const json_value &Source, HMENU Parent)
{
    if(!Parent)
        Parent = CreateMenu();

    for(unsigned int i = 0; i < Source.u.array.length; ++ i)
    {
        const json_value &MenuItem = Source[i];

        if(MenuItem.type == json_string)
        {
            if(!_stricmp(MenuItem, "Separator"))
            {
                AppendMenu(Parent, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
                continue;
            }

            continue;
        }

        if(MenuItem[0].type == json_string && MenuItem[1].type == json_array)
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
        bool Disabled = MenuItem.u.array.length > (ItemOffset + 2) ? ((bool) MenuItem[ItemOffset + 2]) != 0 : false;

        AppendMenu(Parent, Disabled ? MF_GRAYED | MF_UNCHECKED | MF_BYPOSITION | MF_STRING
                                : MF_BYPOSITION | MF_STRING, ID, Text);

		FreeString(Text);
    }

    return Parent;
}

long __stdcall Edif::Condition(LPRDATA rdPtr, long param1, long param2)
{
    int ID = rdPtr->rHo.hoEventNumber;

    rdPtr->pExtension->Runtime.param1 = param1;
    rdPtr->pExtension->Runtime.param2 = param2;

    void * Function = ::SDK->ConditionFunctions[ID];

    if(!Function)
        return rdPtr->pExtension->Condition(ID, rdPtr, param1, param2);

    int Result = ActionOrCondition(::SDK->ConditionFloatFlags, (LPEVENTINFOS2) &::SDK->ConditionInfos[0],
                                Function, ID, rdPtr, param1, param2);

    return *(char *) &Result;
}

short __stdcall Edif::Action(LPRDATA rdPtr, long param1, long param2)
{
    /* int ID = rdPtr->rHo.hoAdRunHeader->rh4.rh4ActionStart->evtNum; */
    int ID = rdPtr->rHo.hoEventNumber;

    rdPtr->pExtension->Runtime.param1 = param1;
    rdPtr->pExtension->Runtime.param2 = param2;

    void * Function = ::SDK->ActionFunctions[ID];

    if(!Function)
    {
        rdPtr->pExtension->Action(ID, rdPtr, param1, param2);
        return 0;
    }

    ActionOrCondition(::SDK->ActionFloatFlags, (LPEVENTINFOS2) &::SDK->ActionInfos[0],
                                Function, ID, rdPtr, param1, param2);

    return 0;
}


long __stdcall Edif::Expression(LPRDATA rdPtr, long param)
{
    int ID = rdPtr->rHo.hoEventNumber;

    rdPtr->pExtension->Runtime.param1 = param;
    rdPtr->pExtension->Runtime.param2 = 0;

    int * Parameters;
    int ParameterCount;

    void * Function = ::SDK->ExpressionFunctions[ID];

    if(!Function)
        return rdPtr->pExtension->Expression(ID, rdPtr, param);

    {   LPEVENTINFOS2 Infos = GetEventInformations((LPEVENTINFOS2) &::SDK->ExpressionInfos[0], ID);

        ParameterCount = Infos->infos.nParams;
        Parameters = (int *) alloca(sizeof(int) * ParameterCount);

        for(int i = 0; i < ParameterCount; ++ i)
        {
            switch(EVINFO2_PARAM(Infos, i))
            {
            case EXPPARAM_STRING:

                if(!i)
                {
                    Parameters[i] = CNC_GetFirstExpressionParameter(rdPtr, param, TYPE_STRING);
                    break;
                }

                Parameters[i] = CNC_GetNextExpressionParameter(rdPtr, param, TYPE_STRING);
                break;  

            case EXPPARAM_LONG:

                int Type = ((::SDK->ExpressionFloatFlags[ID] & (1 << i)) != 0) ? TYPE_FLOAT : TYPE_LONG;

                if(!i)
                {
                    Parameters[i] = CNC_GetFirstExpressionParameter(rdPtr, param, Type);
                    break;
                }

                Parameters[i] = CNC_GetNextExpressionParameter(rdPtr, param, Type);
                break;
            }
        }
    }

    void * Extension = rdPtr->pExtension;

    int Result;
    int ExpressionType = ::SDK->ExpressionTypes[ID];
    
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

        mov ecx, ExpressionType;

        cmp ecx, 1
        jne NotFloat

        fstp Result
        jmp End

    NotFloat:
        
        mov Result, eax
        
    End:

        popad
    }
    
    switch(ExpressionType)
    {
        case 1: // Float
        {
            rdPtr->rHo.hoFlags |= HOF_FLOAT;
            break;
        }

        case 2: // String
        {
            rdPtr->rHo.hoFlags |= HOF_STRING;
            break;
        }
    };
    
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

        fseek (File, 0, SEEK_END);
        Size = ftell (File);
        fseek (File, 0, SEEK_SET);

        Buffer = (char *) malloc (Size + 1);
        Buffer [Size] = 0;

        fread (Buffer, 1, Size, File);
        
        fclose (File);

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
    if(GetFileAttributes(FullFilename) == 0xFFFFFFFF)
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
		if(GetFileAttributes(FullFilename) == 0xFFFFFFFF)
		{
		    // No => try Data/Runtime
		    _stprintf_s(FullFilename, sizeof(ExecutablePath)/sizeof(TCHAR), _T("%s/Data/Runtime/%s"), ExecutablePath, Filename);
		    if(GetFileAttributes(FullFilename) == 0xFFFFFFFF)
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
