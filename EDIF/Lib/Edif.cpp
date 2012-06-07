#include "Common.h"
#include "Edif.h"

Edif::SDK * SDK;

TCHAR Edif::LanguageCode[3];

bool Edif::IsEdittime;
bool Edif::ExternalJSON;

HMENU Edif::ActionMenu, Edif::ConditionMenu, Edif::ExpressionMenu;

short ReadParameterType(const char * Text)
{
    if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
        return PARAM_EXPSTRING;

    if (!_stricmp(Text, "Filename") || !_stricmp(Text, "File"))
        return PARAM_FILENAME;

    if (!_stricmp(Text, "Float"))
        return PARAM_EXPRESSION;

    if (!_stricmp(Text, "Integer"))
        return PARAM_EXPRESSION;

    if (!_stricmp(Text, "Object"))
        return PARAM_OBJECT;

	if (!_stricmp(Text, "Position"))
		return PARAM_POSITION;

	if (!_stricmp(Text, "Create"))
		return PARAM_CREATE;

	if (!_stricmp(Text, "SysCreate"))
		return PARAM_SYSCREATE;

	if (!_stricmp(Text, "Animation"))
		return PARAM_ANIMATION;

	if (!_stricmp(Text, "Nop"))
		return PARAM_NOP;

	if (!_stricmp(Text, "Player"))
		return PARAM_PLAYER;

	if (!_stricmp(Text, "Every"))
		return PARAM_EVERY;

	if (!_stricmp(Text, "Key"))
		return PARAM_KEY;

	if (!_stricmp(Text, "Speed"))
		return PARAM_SPEED;

	if (!_stricmp(Text, "JoyDirection"))
		return PARAM_JOYDIRECTION;

	if (!_stricmp(Text, "Shoot"))
		return PARAM_SHOOT;

	if (!_stricmp(Text, "Zone"))
		return PARAM_ZONE;

	if (!_stricmp(Text, "Comparison"))
		return PARAM_COMPARAISON;
		
	if (!_stricmp(Text, "StringComparison"))
		return PARAM_CMPSTRING;

	if (!_stricmp(Text, "Colour") || !_stricmp(Text, "Color"))
		return PARAM_COLOUR;

	if (!_stricmp(Text, "Frame"))
		return PARAM_FRAME;

	if (!_stricmp(Text, "SampleLoop"))
		return PARAM_SAMLOOP;

	if (!_stricmp(Text, "MusicLoop"))
		return PARAM_MUSLOOP;

	if (!_stricmp(Text, "NewDirection"))
		return PARAM_NEWDIRECTION;

	if (!_stricmp(Text, "TextNumber"))
		return PARAM_TEXTNUMBER;

	if (!_stricmp(Text, "Click"))
		return PARAM_CLICK;

	if (!_stricmp(Text, "Program"))
		return PARAM_PROGRAM;

	if (!_strnicmp(Text, "Custom", 6))
		return PARAM_EXTBASE+atoi(Text+6);

    return 0;
}

short ReadExpressionParameterType(const char * Text)
{
    if (!_stricmp(Text, "Text") || !_stricmp(Text, "String"))
        return EXPPARAM_STRING;

    if (!_stricmp(Text, "Float"))
        return EXPPARAM_LONG;

    if (!_stricmp(Text, "Integer"))
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
	::SDK = NULL;
}

void Edif::Free(LPEDATA edPtr)
{
}

int Edif::Init(mv _far * mV)
{
    _tcscpy (LanguageCode, _T ("EN"));

	// Get pathname of MMF2
	LPTSTR mmfname = (LPTSTR)calloc(_MAX_PATH, sizeof(TCHAR));
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
		LPTSTR filetitle = _tcsrchr(temp, _T('\\'));
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

Edif::SDK::SDK(mv * mV, json_value &_json) : json (_json)
{
	this->mV = mV;

#ifndef RUN_ONLY
    Icon = new cSurface;
    if (mV->mvImgFilterMgr)
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

            if (!Icon->HasAlpha())
                Icon->SetTransparentColor(RGB(255, 0, 255));
			if ( result != Edif::DependencyWasResource )
				free(IconData);
        }
    }
#endif // RUN_ONLY

    const json_value &Actions = json["Actions"];
    const json_value &Conditions = json["Conditions"];
    const json_value &Expressions = json["Expressions"];

    ActionJumps = new void * [Actions.u.array.length + 1];
    ConditionJumps = new void * [Conditions.u.array.length + 1];
    ExpressionJumps = new void * [Expressions.u.array.length + 1];

    ActionJumps [Actions.u.array.length] = 0;
    ConditionJumps [Conditions.u.array.length] = 0;
    ExpressionJumps [Expressions.u.array.length] = 0;
    
    for (unsigned int i = 0; i < Actions.u.array.length; ++ i)
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

            for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if (Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ActionInfos.push_back(ReadParameterType(Type));
                
                if (!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ActionFloatFlags.push_back(IsFloat);
        }

        for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ActionInfos.push_back(0);
    }

    for (unsigned int i = 0; i < Conditions.u.array.length; ++ i)
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

            for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if (Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ConditionInfos.push_back(ReadParameterType(Type));
                
                if (!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ConditionFloatFlags.push_back(IsFloat);
        }

        for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ConditionInfos.push_back(0);
    }

    for (unsigned int i = 0; i < Expressions.u.array.length; ++ i)
    {
        const json_value &Expression = Expressions[i];

        ExpressionJumps [i] = (void *) Edif::Expression;

        ExpressionFunctions.push_back(0);

        if (!_stricmp(Expression["Returns"], "Integer"))
            ExpressionTypes.push_back(0);
        else if (!_stricmp(Expression["Returns"], "Float"))
            ExpressionTypes.push_back(1);
        else if (!_stricmp(Expression["Returns"], "Text") || !_stricmp(Expression["Returns"], "String"))
            ExpressionTypes.push_back(2);
        else
            ExpressionTypes.push_back(0);

        int ExpressionID = Edif::ExpressionID(i);

        ExpressionInfos.push_back(ExpressionID);
        ExpressionInfos.push_back(ExpressionID);
        ExpressionInfos.push_back(i);

        {   short FlagsValue = 0;

            if (ExpressionTypes.back() == 1)
                FlagsValue |= EXPFLAG_DOUBLE;

            if (ExpressionTypes.back() == 2)
                FlagsValue |= EXPFLAG_STRING;

            ExpressionInfos.push_back(FlagsValue);
        }

        const json_value &Parameters = Expression["Parameters"];

        ExpressionInfos.push_back(Parameters.u.array.length);

        {   short IsFloat = 0;

            for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            {
                const json_value &Parameter = Parameters[i];

                const char * Type;

                if (Parameter.type == json_object)
                    Type = Parameter["Type"];
                else
                    Type = Parameter[0];

                ExpressionInfos.push_back(ReadExpressionParameterType(Type));
                
                if (!_stricmp(Type, "Float"))
                    IsFloat |= 1 << i;
            }

            ExpressionFloatFlags.push_back(IsFloat);
        }

        for (unsigned int i = 0; i < Parameters.u.array.length; ++ i)
            ExpressionInfos.push_back(0);
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

	delete Icon;
}

int ActionOrCondition(vector<short> &FloatFlags, LPEVENTINFOS2 Info, void * Function, int ID, LPRDATA rdPtr, long param1, long param2)
{
    int * Parameters;
    int ParameterCount;

    {   Info = GetEventInformations(Info, ID);

        ParameterCount = Info->infos.nParams;
        Parameters = (int *) alloca(sizeof(int) * ParameterCount);

        for (int i = 0; i < ParameterCount; ++ i)
        {
            switch(EVINFO2_PARAM(Info, i))
            {
            case PARAM_EXPRESSION:

                if ((FloatFlags[ID] & (1 << i)) != 0)
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
    if (!Parent)
        Parent = CreateMenu();

    for (unsigned int i = 0; i < Source.u.array.length; ++ i)
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

    if (!Function)
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

    if (!Function)
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

    if (!Function)
        return rdPtr->pExtension->Expression(ID, rdPtr, param);

    {   LPEVENTINFOS2 Infos = GetEventInformations((LPEVENTINFOS2) &::SDK->ExpressionInfos[0], ID);

        ParameterCount = Infos->infos.nParams;
        Parameters = (int *) alloca(sizeof(int) * ParameterCount);

        for (int i = 0; i < ParameterCount; ++ i)
        {
            switch(EVINFO2_PARAM(Infos, i))
            {
            case EXPPARAM_STRING:

                if (!i)
                {
                    Parameters[i] = CNC_GetFirstExpressionParameter(rdPtr, param, TYPE_STRING);
                    break;
                }

                Parameters[i] = CNC_GetNextExpressionParameter(rdPtr, param, TYPE_STRING);
                break;  

            case EXPPARAM_LONG:

                int Type = ((::SDK->ExpressionFloatFlags[ID] & (1 << i)) != 0) ? TYPE_FLOAT : TYPE_LONG;

                if (!i)
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

	Buffer = NULL;
    if (*Filename)
    {
        FILE * File = NULL;
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

    while (*Filename != '.')
        -- Filename;

    _tcscpy(++ Filename, FileExtension);

    Filename = Filename + _tcslen(Filename) - 1;

    while (*Filename != '\\' && *Filename != '/')
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

			while (*Iterator != '\\' && *Iterator != '/')
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
	int len2 = WideCharToMultiByte(CP_ACP, 0, wstr, -1, 0, 0, NULL, NULL);
	if ( len2 == 0 )
		len2 = 1;
	char* str = (char*)calloc(len2, sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len2, NULL, NULL);
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
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, maxLength, NULL, NULL);
	free(wstr);

	return str;
}
#endif // _UNICODE
