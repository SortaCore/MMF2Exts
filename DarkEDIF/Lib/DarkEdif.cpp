#include "Common.h"
extern HINSTANCE hInstLib;
extern Edif::SDK * SDK;

#ifndef RUN_ONLY
static int StoredCurrentLanguage = -1;

static const int DefaultLanguageIndex()
{
	if (::SDK)
		for(unsigned int i = 0; i < SDK->json.u.object.length; ++i)
			if (SDK->json[i]["About"]["Name"].type != json_none)
				return i;

	return 2;
}

int CurrentLanguage()
{
	if (StoredCurrentLanguage > -1)
		return StoredCurrentLanguage;

	char FileToLookup [MAX_PATH];
	{
 		GetModuleFileNameA(hInstLib, FileToLookup, sizeof(FileToLookup));

		char * Filename = FileToLookup + strlen(FileToLookup) - 1;

		while (*Filename != '\\' && *Filename != '/')
			-- Filename;

		strcpy(++ Filename, "DarkEDIF.ini");

		// Is the file in the directory of the MFX? (should be, languages are only needed in edittime)
		if (GetFileAttributesA(FileToLookup) == INVALID_FILE_ATTRIBUTES)
		{
			// DarkEDIF.ini non-existent
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
				MessageBoxA(NULL, "Error opening DarkEDIF.ini.", "DarkEDIF SDK - Error", MB_OK);
			return DefaultLanguageIndex();
		}
	}

	// Change to WinAPI?
	// Open DarkEDIF.ini settings file in read binary, and deny other apps writing permissions.
	FILE * F = _fsopen(FileToLookup, "rb", _SH_DENYWR);
	
	// Could not open; abort (should report error)
	if (!F)
		return DefaultLanguageIndex();

	fseek(F, 0, SEEK_END);
	long S = ftell(F);
	fseek(F, 0, SEEK_SET);

	// Copy file contents into buffer
	char * temp2 = (char *)malloc(S);
	// Out of memory, no buffer allocated	
	if (!temp2)
	{
		fclose(F);
		return DefaultLanguageIndex();
	}
	
	// Could not read all of the file properly
	if (S != fread_s(temp2, S, sizeof(char), S, F))
	{
		fclose(F);
		free(temp2);
		return DefaultLanguageIndex();
	}

	// Load entire file into a std::string for searches
	std::string FullFile(temp2, S);
	free(temp2);
	fclose(F);
	size_t Reading;
	
	// Look for two strings (one with space before =)
	if (FullFile.find("Languages=") != std::string::npos)
		Reading = FullFile.find("Languages=")+sizeof("Languages=")-1;
	else
	{
		if (FullFile.find("Languages =") != std::string::npos)
			Reading = FullFile.find("Languages =")+sizeof("Languages =")-1;
		else
		{
			MessageBoxA(NULL, "Languages not found in .ini file.", "DarkEDIF Debug CurrentLanguage()", MB_OK);
			return DefaultLanguageIndex();
		}
	}

	// If there's a space after the =
	if (FullFile[Reading] == ' ')
		++Reading;

	FullFile.append("\r");
	
	// Read string until newline. Expect ";" or "; "-delimited list of languages.
	while (FullFile.find_first_of(";\r", Reading) != std::string::npos) // Is the semi-colon after an end-of-line character?
	{
		// Read individual token
		std::string Language(FullFile.substr(Reading, FullFile.find_first_of(";\r", Reading)-Reading));

		// If languages are separated by "; " not ";"
		if (Language.front() == ' ')
		{
			++Reading;
			Language.erase(Language.begin());
		}

		// Language matched, get index of language in JSON
		if (::SDK->json[Language.c_str()].type != json_none)
		{
			for (unsigned int i = 0; i < ::SDK->json.u.object.length; ++i)
			{
				// Return index
				if (&::SDK->json[Language.c_str()] == &::SDK->json[i])
				{
					StoredCurrentLanguage = i;
					return (int)i;
				}
			}
		}
		Reading += Language.size()+1;
		if (FullFile.at(Reading-1) == '\r')
			break;
	}

	return DefaultLanguageIndex();
}
#endif // !RUN_ONLY

inline ACEInfo * ACEInfoAlloc(unsigned int NumParams)
{
	// Allocate space for ACEInfo struct, plus Parameter[NumParams] so it has valid memory
	return (ACEInfo *)calloc(sizeof(ACEInfo) + (NumParams * sizeof(short) * 2), 1);	// The *2 is for reserved variables
}
char ReadExpressionReturnType(const char *);

bool CreateNewActionInfo(void)
{
	// Get ID and thus properties by counting currently existing actions.
	const json_value & Action = SDK->json[CurLang]["Actions"][::SDK->ActionInfos.size()];
	
	// Invalid JSON reference
	if (Action.type != json_object)
	{
		MessageBoxA(NULL, "Invalid JSON reference, expected object.", "DarkEDIF - Error reading action", MB_OK);
		return false;
	}

	const json_value & Param = Action["Parameters"];
	
	// Num of parameters is beyond number of bits in FloatFlags
	if (sizeof(short)*8 < Param.u.object.length)
	{
		MessageBoxA(NULL, "Too many parameters in action.", "DarkEDIF - Error reading action", MB_OK);
		return false;
	}

	// Parameters checked; allocate new info
	ACEInfo * ActInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ActInfo)
	{
		MessageBoxA(NULL, "Could not allocate memory for action return.", "DarkEDIF - Error creating action info", MB_OK);
		return false;
	}

	ActInfo->ID = SDK->ActionInfos.size();
	ActInfo->NumOfParams = Param.u.object.length;

	if (ActInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < ActInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ActInfo->Parameter[c] = ReadParameterType(Param[c][0], IsFloat);	// Store parameter type
			ActInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a flag or not with a single bit
		}

		// For some reason in EDIF an extra short is provided, initialised to 0, so duplicate that
		memset(&ActInfo->Parameter[ActInfo->NumOfParams], 0, ActInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	SDK->ActionInfos.push_back(ActInfo);
	return true;
}

bool CreateNewConditionInfo(void)
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Condition = ::SDK->json[CurLang]["Conditions"][::SDK->ConditionInfos.size()];
	
	// Invalid JSON reference
	if (Condition.type != json_object)
	{
		MessageBoxA(NULL, "Invalid JSON reference, expected object.", "DarkEDIF - Error reading condition", MB_OK);
		return false;
	}

	const json_value & Param = Condition["Parameters"];
	
	// Num of parameters is beyond size of FloatFlags
	if (sizeof(short)*8 < Param.u.object.length)
	{
		MessageBoxA(NULL, "Too many parameters in condition.", "DarkEDIF - Error reading condition", MB_OK);
		return false;
	}

	// Parameters checked; allocate new info
	ACEInfo * CondInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!CondInfo)
	{
		MessageBoxA(NULL, "Could not allocate memory for condition return.", "DarkEDIF - Error creating condition info", MB_OK);
		return false;
	}

	// If a non-triggered condition, set the correct flags
	CondInfo->ID = ::SDK->ConditionInfos.size();
	CondInfo->NumOfParams = Param.u.object.length;
	CondInfo->Flags = (EVFLAGS::ALWAYS | EVFLAGS_NOTABLE) * (!bool (Condition["Triggered"]));

	if (CondInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < CondInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			CondInfo->Parameter[c] = ReadParameterType(Param[c][0], IsFloat);	// Store parameter type
			CondInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a flag or not with a single bit
		}

		// For some reason in EDIF an extra short is provided, initialised to 0, so duplicate that
		memset(&CondInfo->Parameter[CondInfo->NumOfParams], 0, CondInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	::SDK->ConditionInfos.push_back(CondInfo);
	return true;
}

bool CreateNewExpressionInfo(void)
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Expression = ::SDK->json[CurLang]["Expressions"][::SDK->ExpressionInfos.size()];
	
	// Invalid JSON reference
	if (Expression.type != json_object)
	{
		MessageBoxA(NULL, "Invalid JSON reference, expected object.", "DarkEDIF - Error reading expression", MB_OK);
		return false;
	}

	const json_value & Param = Expression["Parameters"];
	
	// Num of parameters is beyond size of FloatFlags
	if (sizeof(short)*8 < Param.u.object.length)
	{
		MessageBoxA(NULL, "Too many parameters in expression.", "DarkEDIF - Error reading expression", MB_OK);
		return false;
	}

	// Parameters checked; allocate new info
	ACEInfo * ExpInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ExpInfo)
	{
		MessageBoxA(NULL, "Could not allocate memory for expression return.", "DarkEDIF - Error creating expression info", MB_OK);
		return false;
	}

	// If a non-triggered condition, set the correct flags
	ExpInfo->ID = ::SDK->ExpressionInfos.size();
	ExpInfo->NumOfParams = Param.u.object.length;
	ExpInfo->Returns = ReadExpressionReturnType(Expression["Returns"]);

	if (ExpInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < ExpInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ExpInfo->Parameter[c] = ReadExpressionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ExpInfo->FloatFlags |= (IsFloat << c);										// Store whether it is a flag or not with a single bit
		}

		// For some reason in EDIF an extra short is provided, initialised to 0, so duplicate that
		memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	::SDK->ExpressionInfos.push_back(ExpInfo);
	return true;
}

// DarkEDIF - automatic property setup

#if 0 // NO PROPS!!!
void InitialisePropertiesFromJSON(mv * mV, EDITDATA * edPtr)
{
	using namespace Edif::Properties;
	edPtr->DarkEDIF_Prop_Size = sizeof(EDITDATA);
	
	// Set default object settings from DefaultState.
	for (unsigned int i = 0; i < ::SDK->json[CurLang]["Properties"].u.object.length; ++i)
	{
		const json_value &JProp = ::SDK->json[CurLang]["Properties"][i]["DefaultState"];
		#define AddSingleProp(Type,Construct) AddSingleProp2(Type,Construct,0)
		
		#define AddSingleProp2(ThisType,Construct,AddSize) \
			size_t PrevSize = edPtr->DarkEDIF_Prop_Size; \
			edPtr = (EDITDATA *)mvReAllocEditData(mV, edPtr, PrevSize + sizeof(ThisType) + AddSize + 1); \
			if (!edPtr) \
				MessageBoxA(NULL, "Could not reallocate edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK); \
			else \
			{ \
				ThisType * Var = new ThisType(Construct); \
				if (memcpy_s(((char *)edPtr)+PrevSize, sizeof(ThisType) + AddSize + 1, &*Var, sizeof(ThisType) + AddSize)) \
					MessageBoxA(NULL, "Could not copy variable into edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK); \
				else \
				{ \
					edPtr->DarkEDIF_Prop_Size += sizeof(ThisType) + AddSize + 1; \
					edPtr->DarkEDIF_Props[edPtr->DarkEDIF_Prop_Size] = \
						bool(::SDK->json[CurLang]["Properties"][i][(::SDK->EdittimeProperties[i].Type_ID == PROPTYPE_LEFTCHECKBOX) ? "DefaultState" : "ChkDefault"]); \
					Var->Delete(); \
				} \
			}
			
		
		switch (::SDK->EdittimeProperties[i].Type_ID)
		{
			case PROPTYPE_COLOR:
			case PROPTYPE_EDIT_NUMBER:
			{
				if (JProp.type != json_integer)
					MessageBoxA(NULL, "Invalid or no default integer value specified.", "DarkEDIF setup warning", MB_OK);
				AddSingleProp(Prop_SInt, (long)JProp);
				break;
			}
				
			case PROPTYPE_STATIC:
			case PROPTYPE_EDIT_STRING:
			{
				if (JProp.type != json_string)
					MessageBoxA(NULL, "Invalid or no default string value specified.", "DarkEDIF - setup warning", MB_OK);
				
				size_t PrevSize = edPtr->DarkEDIF_Prop_Size, StrLen = strlen(JProp) + 1;
				edPtr = (EDITDATA *)mvReAllocEditData(mV, edPtr, PrevSize + sizeof(Prop_AStr) + StrLen + 1);
				if (!edPtr)
					MessageBoxA(NULL, "Could not reallocate edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK);
				else
				{  
					Prop_AStr * Var = new Prop_AStr((const char *)JProp);
					if (memcpy_s(((char *)edPtr)+PrevSize, sizeof(Prop_AStr) + StrLen + 1, &*Var, sizeof(Prop_AStr)) ||
						memcpy_s(((char *)edPtr)+PrevSize+sizeof(Prop_AStr), StrLen + 1, Var->String, StrLen))
						MessageBoxA(NULL, "Could not copy variable into edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK);
					else
					{
						edPtr->DarkEDIF_Prop_Size += sizeof(Prop_AStr) + StrLen + 1;
						edPtr->DarkEDIF_Props[edPtr->DarkEDIF_Prop_Size] =
							bool(::SDK->json[CurLang]["Properties"][i][(::SDK->EdittimeProperties[i].Type_ID == PROPTYPE_LEFTCHECKBOX) ? "DefaultState" : "ChkDefault"]);
						Var->Delete();
						((Prop_AStr *)(edPtr->DarkEDIF_Props+PrevSize))->String = (edPtr->DarkEDIF_Props+PrevSize+sizeof(Prop_AStr));
					}
				}
				break;
			}
				
			case PROPTYPE_COMBOBOX:
			{
				if (JProp.type != json_array)
					MessageBoxA(NULL, "Invalid or no default array specified.", "DarkEDIF - setup warning", MB_OK);
					
				char ** DefaultItems = new char * [JProp.u.object.length+2];
					
				DefaultItems[0] = nullptr;
				for (unsigned int j = 0; j < JProp.u.object.length; ++j)
					DefaultItems[j+1] = _strdup(JProp[j]);
				DefaultItems[JProp.u.object.length+1] = nullptr;

				size_t PrevSize = edPtr->DarkEDIF_Prop_Size;
				edPtr = (EDITDATA *)mvReAllocEditData(mV, edPtr, PrevSize + sizeof(Prop_Buff) + (JProp.u.object.length * sizeof(char *)) + 1);
				if (!edPtr)
					MessageBoxA(NULL, "Could not reallocate edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK);
				else
				{
					Prop_Buff * Var = new Prop_Buff((JProp.u.object.length * sizeof(char *)), DefaultItems);
					if (memcpy_s(((char *)edPtr)+PrevSize, sizeof(Prop_Buff) + (JProp.u.object.length * sizeof(char *)) + 1, &*Var, sizeof(Prop_Buff)) ||
						memcpy_s(((char *)edPtr)+PrevSize+sizeof(Prop_Buff), (JProp.u.object.length * sizeof(char *)) + 1, Var->Address, Var->Size))
						MessageBoxA(NULL, "Could not copy variable into edPtr, initialisation of properties failed.", "DarkEDIF error", MB_OK);
					else
					{
						edPtr->DarkEDIF_Prop_Size += sizeof(Prop_Buff) + (JProp.u.object.length * sizeof(char *)) + 1;
						edPtr->DarkEDIF_Props[edPtr->DarkEDIF_Prop_Size] =
							bool(::SDK->json[CurLang]["Properties"][i][(::SDK->EdittimeProperties[i].Type_ID == PROPTYPE_LEFTCHECKBOX) ? "DefaultState" : "ChkDefault"]);
						Var->Delete();
						((Prop_Buff *)(edPtr->DarkEDIF_Props+PrevSize))->Address = (edPtr->DarkEDIF_Props+PrevSize+sizeof(Prop_Buff));
					}
				}


				delete [] DefaultItems; // Prop_Buff constructor would have duplicated it
				break;
			}

			// These have no ID or property that can be changed
			default:
				//AddSingleProp(nullptr);
				//edPtr->Properties.push_back(*(Prop *)(nullptr));
				break;
		}
/*		// Checkbox-only property uses "DefaultState"
		if (::SDK->EdittimeProperties[i].Type_ID == PROPTYPE_LEFTCHECKBOX)
			edPtr->PropCheckboxes.push_back(bool(::SDK->json[CurLang]["Properties"][i]["DefaultState"]));
		else // "DefaultState" reserved; use "ChkDefault"
			edPtr->PropCheckboxes.push_back(bool(::SDK->json[CurLang]["Properties"][i]["ChkDefault"]));*/
	}
}
Prop * GetProperty(EDITDATA * edPtr, size_t ID)
{
	//
	char * Current = &edPtr->DarkEDIF_Props[0];
	unsigned int i = 0;
	while (i < ID)
	{
		if ((char *)Current > (edPtr->DarkEDIF_Props + edPtr->DarkEDIF_Prop_Size))
		{
			MessageBoxA(NULL, "Error: extended past properties.", "DarkEDIF error", MB_OK);
			break;
		}
		switch (((Prop *)Current)->GetClassID())
		{
			case 'STRA':
				Current += sizeof(Prop_AStr)+(strlen((((Prop_AStr *)Current)->String)+1)*sizeof(char))+1;
				break;
			case 'STRW':
				Current += sizeof(Prop_WStr)+((wcslen(((Prop_WStr *)Current)->String)+1)*sizeof(wchar_t))+1;
				break;
			case 'LPTR':
				Current += sizeof(Prop_Buff)+((Prop_Buff *)Current)->Size+1;
				break;
			case 'INT ':
				Current += sizeof(Prop_SInt)+1;
				break;
			case 'INT2':
				Current += sizeof(Prop_Int64)+1;
				break;
			case 'DWRD':
				Current += sizeof(Prop_UInt)+1;
				break;
			case 'FLOT':
				Current += sizeof(Prop_Float)+1;
				break;
			default:
				MessageBoxA(NULL, "FREAK OUT!!!!", "DarkEDIF - error", MB_OK);
		}
		++i;
	}
	return (Prop *)Current;
}
char * GetPropertyChbx(EDITDATA * edPtr, size_t ID)
{
	return ((char *)GetProperty(edPtr, ID+1))+1;
}
#endif // NO PROPS!