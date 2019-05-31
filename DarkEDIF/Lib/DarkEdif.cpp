#include "Common.h"
extern HINSTANCE hInstLib;
extern Edif::SDK * SDK;

#ifndef RUN_ONLY
static const _json_value * StoredCurrentLanguage = &json_value_none;

static const _json_value * DefaultLanguageIndex()
{
	// Misuse of function; called before ::SDK is valid
	if (!::SDK)
	{
		char msgTitle[128] = {0};
		Edif::GetExtensionName(msgTitle);
		strcat_s(msgTitle, "- DarkEDIF error");
		MessageBoxA(NULL, "Premature function call!\n  Called DefaultLanguageIndex() before ::SDK was a valid pointer.", msgTitle, MB_OK);

		return &json_value_none;
	}

	for (unsigned int i = 0; i < SDK->json.u.object.length; ++i)
	{
		if ((*SDK->json.u.object.values[i].value).type == json_object 
			&& (*SDK->json.u.object.values[i].value)["About"]["Name"].type == json_string)
		{
			return SDK->json.u.object.values[i].value;
		}
	}

	// Fallback on last object
	return SDK->json.u.object.values[SDK->json.u.object.length - 1].value;
}

const json_value & CurrentLanguage()
{
	if (StoredCurrentLanguage->type != json_none)
		return *StoredCurrentLanguage;

	char FileToLookup [MAX_PATH];
	{
 		GetModuleFileNameA(hInstLib, FileToLookup, sizeof(FileToLookup));

		// This mass of code converts Extensions\Bla.mfx and Extensions\Unicode\Bla.mfx to Extensions\DarkEDIF.ini
		char * Filename = FileToLookup + strlen(FileToLookup) - 1;
		while (*Filename != '\\' && *Filename != '/')
			-- Filename;

		// Look in Extensions, not Extensions\Unicode
		if (!_strnicmp("Unicode", Filename - (sizeof("Unicode") - 1), sizeof("Unicode") - 1))
			Filename -= sizeof("Unicode\\") - 1;

		strcpy(++ Filename, "DarkEDIF.ini");
		
		// Is the file in the directory of the MFX? (should be, languages are only needed in edittime)
		if (GetFileAttributesA(FileToLookup) == INVALID_FILE_ATTRIBUTES)
		{
			// DarkEDIF.ini non-existent
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
				MessageBoxA(NULL, "Error opening DarkEDIF.ini.", "DarkEDIF SDK - Error", MB_OK);
			
			return *DefaultLanguageIndex();
		}
	}

	// TODO: Change to WinAPI?
	// Open DarkEDIF.ini settings file in read binary, and deny other apps writing permissions.
	FILE * F = _fsopen(FileToLookup, "rb", _SH_DENYWR);
	
	// Could not open; abort (should report error)
	if (!F)
		return *DefaultLanguageIndex();

	fseek(F, 0, SEEK_END);
	long S = ftell(F);
	fseek(F, 0, SEEK_SET);

	// Copy file contents into buffer
	char * temp2 = (char *)malloc(S);
	// Out of memory, no buffer allocated	
	if (!temp2)
	{
		fclose(F);
		return *DefaultLanguageIndex();
	}
	
	// Could not read all of the file properly
	if (S != fread_s(temp2, S, sizeof(char), S, F))
	{
		fclose(F);
		free(temp2);
		return *DefaultLanguageIndex();
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
			return *DefaultLanguageIndex();
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
		for (unsigned int i = 0; i < ::SDK->json.u.object.length; ++i)
		{
			// Return index
			if ((*::SDK->json.u.object.values[i].value).type == json_object && 
				!_stricmp(::SDK->json.u.object.values[i].name, Language.c_str()))
			{
				StoredCurrentLanguage = SDK->json.u.object.values[i].value;
				return *StoredCurrentLanguage;
			}
		}
		Reading += Language.size()+1;
		if (FullFile.at(Reading-1) == '\r')
			break;
	}

	return *DefaultLanguageIndex();
}
#endif // !RUN_ONLY

inline ACEInfo * ACEInfoAlloc(unsigned int NumParams)
{
	// Allocate space for ACEInfo struct, plus Parameter[NumParams] so it has valid memory
	return (ACEInfo *)calloc(sizeof(ACEInfo) + (NumParams * sizeof(short) * 2), 1);	// The *2 is for reserved variables
}
ExpReturnType ReadExpressionReturnType(const char * Text);

bool CreateNewActionInfo(void)
{
	// Get ID and thus properties by counting currently existing actions.
	const json_value & Action = CurLang["Actions"][::SDK->ActionInfos.size()];
	
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

	ActInfo->ID = (short)SDK->ActionInfos.size();
	ActInfo->NumOfParams = Param.u.object.length;

	if (ActInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < ActInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ActInfo->Parameter[c].p = ReadParameterType(Param[c][0], IsFloat);	// Store parameter type
			ActInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
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
	const json_value & Condition = CurLang["Conditions"][::SDK->ConditionInfos.size()];
	
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
	CondInfo->ID = (short)::SDK->ConditionInfos.size();
	CondInfo->NumOfParams = Param.u.object.length;
	CondInfo->Flags.ev = bool (Condition["Triggered"]) ? EVFLAGS::NONE : (EVFLAGS::ALWAYS | EVFLAGS::NOTABLE);

	if (CondInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < CondInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			CondInfo->Parameter[c].p = ReadParameterType(Param[c][0], IsFloat);	// Store parameter type
			CondInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
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
	const json_value & Expression = CurLang["Expressions"][::SDK->ExpressionInfos.size()];
	
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
	ExpInfo->ID = (short)::SDK->ExpressionInfos.size();
	ExpInfo->NumOfParams = Param.u.object.length;
	ExpInfo->Flags.ef = ReadExpressionReturnType(Expression["Returns"]);
	
	if (ExpInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (char c = 0; c < ExpInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ExpInfo->Parameter[c].ep = ReadExpressionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ExpInfo->FloatFlags |= (IsFloat << c);										// Store whether it is a float or not with a single bit
		}

		// For some reason in EDIF an extra short is provided, initialised to 0, so duplicate that
		memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	::SDK->ExpressionInfos.push_back(ExpInfo);
	return true;
}

// DarkEDIF - automatic property setup
using namespace Edif::Properties;

#include <sstream>

void InitialisePropertiesFromJSON(mv * mV, EDITDATA * edPtr)
{
	std::stringstream mystr;
	char * chkboxes = (char *)calloc(size_t(ceil(CurLang["Properties"].u.array.length / 8.0f)), 1);
	
	// Set default object settings from DefaultState.
	for (unsigned int i = 0; i < CurLang["Properties"].u.array.length; ++i)
	{
		const json_value & JProp = CurLang["Properties"][i];
		
		// TODO: If default state is missing, say the name of the property for easy repair by dev
		switch (::SDK->EdittimeProperties[i].Type_ID % 1000)
		{
			case PROPTYPE_LEFTCHECKBOX:
			{
				if (JProp["DefaultState"].type != json_boolean)
					MessageBoxA(NULL, "Invalid or no default checkbox value specified.", "DarkEDIF setup warning", MB_OK);

				if (JProp["DefaultState"])
					chkboxes[i >> 3] |= 1 << (i % 8);

				break;
			}

			case PROPTYPE_COLOR:
			case PROPTYPE_EDIT_NUMBER:
			{
				if (JProp["DefaultState"].type != json_integer)
					MessageBoxA(NULL, "Invalid or no default integer value specified.", "DarkEDIF setup warning", MB_OK);
				
				unsigned int i = unsigned int(long long(JProp["DefaultState"]) & 0xFFFFFFFF);
				mystr.write((char *)&i, sizeof(unsigned int)); // embedded nulls upset the << operator
				
				if (JProp["ChkDefault"])
					chkboxes[i >> 3] |= 1 << (i % 8);

				break;
			}
				
			case PROPTYPE_STATIC:
			case PROPTYPE_FOLDER:
			case PROPTYPE_FOLDER_END:
			case PROPTYPE_EDITBUTTON:
				break; // do not store
			

			case PROPTYPE_EDIT_STRING:
			{
				if (JProp["DefaultState"].type != json_string)
					MessageBoxA(NULL, "Invalid or no default string value specified.", "DarkEDIF - setup warning", MB_OK);
				
				// No casing change necessary				
				if (_stricmp(JProp["Case"], "Upper") && _stricmp(JProp["Case"], "Lower")) {
					mystr << (const char *)(JProp["DefaultState"]) << char(0);
				}
				else
				{
					std::string dup(JProp["DefaultState"]);
					std::transform(dup.begin(), dup.end(), dup.begin(), !_stricmp(JProp["Case"], "Upper") ? ::toupper : ::tolower);
					mystr << dup << char(0);
				}
				
				if (JProp["ChkDefault"])
					chkboxes[i >> 3] |= 1 << (i % 8);

				break;
			}
				

			case PROPTYPE_COMBOBOX:
			{
				unsigned int i = 0U;
				if (JProp["DefaultState"].type != json_string)
					MessageBoxA(NULL, "Invalid or no default string specified.", "DarkEDIF - setup warning", MB_OK);
				else
				{
					for (size_t j = 0; j < JProp["Items"].u.array.length; j++)
					{
						if (!_stricmp((const char *)JProp["DefaultState"], JProp["Items"][j]))
						{
							i = j;
							goto ok;
						}
					}

					MessageBoxA(NULL, "Specified a default string in a combobox property that does not exist in items list.", "DarkEDIF - setup warning", MB_OK);
				}
			ok:
				mystr << i;

				if (JProp["ChkDefault"])
					chkboxes[i >> 3] |= 1 << (i % 8);

				break;
			}

			// These have no ID or property that can be changed
			default:
				break;
		}
	}

	std::string mystr2(chkboxes, _msize(chkboxes));
	mystr2 += mystr.str();

	free(chkboxes);
	mystr.clear();
	
	edPtr = (EDITDATA *) mvReAllocEditData(mV, edPtr, sizeof(EDITDATA) + mystr2.size());
	if (!edPtr)
	{
		MessageBoxA(NULL, "Could not reallocate EDITDATA.\n\n*cough* MMF2's fault.", "DarkEDIF - setup warning", MB_OK);
		return;
	}

	edPtr->DarkEDIF_Prop_Size = sizeof(EDITDATA) + mystr2.size();
	
	memset(edPtr->DarkEDIF_Props, 0, mystr2.size());
	memcpy(edPtr->DarkEDIF_Props, mystr2.data(), mystr2.size());
}

Prop * GetProperty(EDITDATA * edPtr, size_t ID)
{
	// Premature call
	if (edPtr->DarkEDIF_Prop_Size == 0)
	{
		char msgTitle [128] = {0};
		Edif::GetExtensionName(msgTitle);
		strcat_s(msgTitle, " - DarkEDIF error");
		MessageBoxA(NULL, "Premature function call!\n  GetProperty() called without edPtr->DarkEDIF_Props being valid.", msgTitle, MB_OK);
		return nullptr;
	}

	const json_value &jsonItem = CurLang["Properties"][ID];
	const char * curStr = jsonItem["Type"];
	Prop * ret = nullptr;
	bool allConv;
	if (!_stricmp(curStr, "Text") || !_stricmp(curStr, "Edit button"))
	{
		ret = new Prop_Str(UTF8ToTString((const char *)jsonItem["DefaultState"], &allConv).c_str());
		if (!allConv)
			MessageBoxA(NULL, "Warning: The property's Unicode string couldn't be converted to ANSI. "
				"Characters will be replaced with filler.", "DarkEDIF Property Error", MB_OK | MB_ICONWARNING);
		return ret;
	}

	unsigned int size;
	char * Current = PropIndex(edPtr, ID, &size);

	if (!_stricmp(curStr, "Editbox String"))
	{
		ret = new Prop_Str(UTF8ToTString(Current, &allConv).c_str());
		if (!allConv)
			MessageBoxA(NULL, "Warning: The property's Unicode string couldn't be converted to ANSI. "
				"Characters will be replaced with filler.", "DarkEDIF Property Error", MB_OK | MB_ICONWARNING);
	}
	else if (!_stricmp(curStr, "Editbox Number") || !_stricmp(curStr, "Combo Box"))
		ret = new Prop_UInt(*(unsigned int *)Current);
	else if (_stricmp(curStr, "Checkbox"))
		MessageBoxA(NULL, "Don't understand JSON property type, can't return Prop.", "DarkEDIF Property Error", MB_OK | MB_ICONERROR);

	return ret;
}
void PropChangeChkbox(EDITDATA * edPtr, unsigned int PropID, const bool newValue)
{
	// The DarkEDIF_Props consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	int byteIndex = PropID >> 3, bitIndex = PropID % 8;

	if (newValue)
		edPtr->DarkEDIF_Props[byteIndex] |= 1 << bitIndex;
	else
		edPtr->DarkEDIF_Props[byteIndex] &= ~(1 << bitIndex);
}
void PropChange(mv * mV, EDITDATA * &edPtr, unsigned int PropID, const void * newPropValue, size_t newPropValueSize)
{
	unsigned int oldPropValueSize; // Set by PropIndex
	const char * curTypeStr = CurLang["Properties"][PropID]["Type"];
	char * oldPropValue = PropIndex(edPtr, PropID, &oldPropValueSize);
	bool rearrangementRequired = false;

	if (!_stricmp(curTypeStr, "Editbox String"))
		rearrangementRequired = newPropValueSize != oldPropValueSize; // May need resizing
	else if (!_stricmp(curTypeStr, "Editbox Number"))
		rearrangementRequired = false; // Number of editbox, always same data size
	else if (!_stricmp(curTypeStr, "Combo Box"))
		rearrangementRequired = false; // Index of combo box Item, always same data size
	else if (_stricmp(curTypeStr, "Checkbox")) //
		return; // Checkbox is handled by PropChangeChkbox()
	else
		MessageBoxA(NULL, "Don't understand JSON property type, can't return Prop.", "DarkEDIF Fatal Error", MB_OK);

	if (!rearrangementRequired)
	{
		memcpy(oldPropValue, newPropValue, newPropValueSize);
		return;
	}

	// Even an empty string should be 1 (null char). Warn if not.
	if (oldPropValueSize == 0)
		MessageBoxA(NULL, "Property size is 0!", "DarkEDIF - Debug info", MB_OK | MB_ICONERROR);
	
	size_t beforeOldSize = sizeof(EDITDATA) +
		(oldPropValue - edPtr->DarkEDIF_Props); // Pointer to O|<P|O
	size_t afterOldSize = edPtr->DarkEDIF_Prop_Size - oldPropValueSize - beforeOldSize;			// Pointer to O|P>|O
	size_t odps = edPtr->DarkEDIF_Prop_Size;

	// Duplicate memory to another buffer (if new arragement is smaller - we can't just copy from old buffer after realloc)
	char * newEdPtr = (char *)malloc(edPtr->DarkEDIF_Prop_Size + (newPropValueSize - oldPropValueSize));

	if (!newEdPtr)
	{
		MessageBoxA(NULL, "Out of memory attempting to rewrite properties!", "DarkEDIF - Property Error", MB_OK);
		return;
	}
	((EDITDATA *)newEdPtr)->DarkEDIF_Prop_Size = _msize(newEdPtr);

	// Copy the part before new data into new address
	memcpy(newEdPtr, edPtr, beforeOldSize);

	// Copy the new data into new address
	memcpy(newEdPtr + beforeOldSize, newPropValue, newPropValueSize);

	// Copy the part after new data into new address
	memcpy(newEdPtr + beforeOldSize + newPropValueSize,
		oldPropValue + oldPropValueSize,
		afterOldSize);

	// Reallocate edPtr
	EDITDATA * fusionNewEdPtr = (EDITDATA *)mvReAllocEditData(mV, edPtr, _msize(newEdPtr));
	if (!fusionNewEdPtr)
	{
		MessageBoxA(NULL, "NULL returned from EDITDATA reallocation. Property changed cancelled.", "DarkEDIF - Propery Error", MB_OK);
		free(newEdPtr);
		return;
	}

	// Copy into edPtr (copy everything after eHeader, leave eHeader alone)
	// eHeader::extSize and such will be changed by Fusion by mvReAllocEditData,
	// so should not be considered ours to interact with
	memcpy(((char *)fusionNewEdPtr) + sizeof(EDITDATA::eHeader),
		newEdPtr + sizeof(EDITDATA::eHeader),
		_msize(newEdPtr) - sizeof(EDITDATA::eHeader));
	free(newEdPtr);

	edPtr = fusionNewEdPtr; // Inform caller of new address
}	

char * PropIndex(EDITDATA * edPtr, unsigned int ID, unsigned int * size)
{
	char * Current = &edPtr->DarkEDIF_Props[(size_t)ceil(CurLang["Properties"].u.array.length / 8.0f)], * StartPos, * EndPos;
	
	json_value j = CurLang["Properties"];
	if (j.type != json_array)
	{
		char msgTitle [128] = {0};
		Edif::GetExtensionName(msgTitle);
		strcat_s(msgTitle, " - DarkEDIF error");
		MessageBoxA(NULL, "Premature function call!\n  GetProperty() called without edPtr->DarkEDIF_Props being valid.", msgTitle, MB_OK);
		return nullptr;
	}
	
	const char * curStr = (const char *)j[ID]["Type"];
	// Read unchangable properties
	if (!_stricmp(curStr, "Text") || !_stricmp(curStr, "Checkbox"))
		return nullptr;
	// if (curTypeStr == "other stuff")
	//	return new Prop_XXX();

	// Read changable properties
	unsigned int i = 0;
	while (i <= ID)
	{
		curStr = (const char *)j[i]["Type"];
		
		if (!_stricmp(curStr, "Editbox String"))
			Current += strlen(Current) + 1;
		else if (!_stricmp(curStr, "Editbox Number") || !_stricmp(curStr, "Combo Box"))
			Current += sizeof(unsigned int);

		if (i == ID - 1)
			StartPos = Current;

		++i;
	}
	
	EndPos = Current;

	if (size)
		*size = EndPos - StartPos;
	return StartPos;
}

// =====
// Get event number (CF2.5+ feature)
// =====


/// <summary> If error, -1 is returned. Frame index is 1+. </summary>
int GetFusionEventLocation(Extension *ext, int& frameNum)
{
	// Can we read current event?
	if (!ext->rhPtr->EventGroup)
		return -1;

	int eventNum = ext->rhPtr->EventGroup->evgIdentifier;
	if (eventNum == 0)
		return -1;

	frameNum = ext->rhPtr->App ? ext->rhPtr->App->nCurrentFrame : -1;
	return eventNum;
}






// =====
// Text conversion - definitions
// =====


#include <assert.h>

#ifdef _WIN32
// For Windows, TString can be Wide or ANSI.
// ANSI function calls are internally converted to Wide by Windows.
// ANSI is not UTF-8, the earliest OS version that *can* use UTF-8 for OS function calls is
// Windows 10 Insider Preview Build 17035, and even that is non-default and in beta.

std::tstring ANSIToTString(const std::string input) {
	return WideToTString(ANSIToWide(input));
}
std::string ANSIToUTF8(const std::string input) {
	return WideToUTF8(ANSIToWide(input));
}
std::wstring ANSIToWide(const std::string input) {
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
#if _HAS_CXX17
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
#else
	wchar_t * outputBuf = (wchar_t *)_malloca((length + 1) * sizeof(wchar_t));
	// Actually convert
	length = MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), outputBuf, length + 1);
	assert(length > 0 && "Failed to convert between string encodings.");
	std::wstring outputStr(outputBuf, length);
	_freea(outputBuf);
#endif
	OutputDebugStringW(L"Converted ANSI To Wide: [");
	OutputDebugStringA(input.c_str());
	OutputDebugStringW(L"] to [");
	OutputDebugStringW(outputStr.c_str());
	OutputDebugStringW(L"]\n");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");

	return outputStr;
}
std::string UTF8ToANSI(const std::string input, bool * const allValidChars /* = nullptr */) {
	return WideToANSI(UTF8ToWide(input), allValidChars);
}
std::tstring UTF8ToTString(const std::string input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true; // UTF-8 and UTF-16 share all chars
	return UTF8ToWide(input);
#else
	return UTF8ToANSI(input, allValidChars);
#endif
}
std::wstring UTF8ToWide(const std::string input)
{
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
#if _HAS_CXX17
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
#else
	wchar_t * outputBuf = (wchar_t*)_malloca((length + 1) * sizeof(wchar_t));
	// Actually convert
	length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), input.size(), outputBuf, length + 1);
	assert(length > 0 && "Failed to convert between string encodings.");
	std::wstring outputStr(outputBuf, length);
	_freea(outputBuf);
#endif
	OutputDebugStringW(L"Converted UTF8 To Wide: [");
	OutputDebugStringA(input.c_str());
	OutputDebugStringW(L"] to [");
	OutputDebugStringW(outputStr.c_str());
	OutputDebugStringW(L"]\n");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");
	return outputStr;
}
std::string WideToANSI(const std::wstring input, bool * const allValidChars /* = nullptr */) {
	if (input.empty())
		return std::string();

	BOOL someFailed;

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_ACP, 0, input.c_str(), input.size(), NULL, 0, 0, allValidChars ? &someFailed : NULL);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");

	if (allValidChars)
		*allValidChars = (someFailed == FALSE);

#if _HAS_CXX17
	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_ACP, 0, input.c_str(), input.size(), outputStr.data(), outputStr.size(), 0, NULL);
	assert(length > 0 && "Failed to convert between string encodings.");
#else
	char * outputBuf = (char *)_malloca(length + 1);
	// Actually convert
	length = WideCharToMultiByte(CP_ACP, 0, input.c_str(), input.size(), outputBuf, length, 0, 0);
	assert(length > 0 && "Failed to convert between string encodings.");
	std::string outputStr(outputBuf, length);
	_freea(outputBuf);
#endif
	OutputDebugStringW(L"Converted Wide To ANSI: [");
	OutputDebugStringW(input.c_str());
	OutputDebugStringW(L"] to [");
	OutputDebugStringA(outputStr.c_str());
	OutputDebugStringW(L"]\n");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::tstring WideToTString(const std::wstring input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true;
	return input;
#else
	return WideToANSI(input, allValidChars);
#endif
}
std::string WideToUTF8(const std::wstring input)
{
	if (input.empty())
		return std::string();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), input.size(), NULL, 0, 0, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
#if _HAS_CXX17
	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), input.size(), outputStr.data(), outputStr.size(), 0, 0);
	assert(length > 0 && "Failed to convert between string encodings.");
#else
	char * outputBuf = (char *)_malloca(length + 1);
	// Actually convert
	length = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), input.size(), outputBuf, length, 0, 0);
	assert(length > 0 && "Failed to convert between string encodings.");
	std::string outputStr(outputBuf, length);
	_freea(outputBuf);
#endif
	OutputDebugStringW(L"Converted Wide To UTF8: [");
	OutputDebugStringW(input.c_str());
	OutputDebugStringW(L"] to [");
	OutputDebugStringA(outputStr.c_str());
	OutputDebugStringW(L"]\n");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::string TStringToANSI(const std::tstring input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	return WideToANSI(input, allValidChars);
#else
	if (allValidChars)
		*allValidChars = true;
	return input;
#endif
}
std::string TStringToUTF8(const std::tstring input) {
#ifdef _UNICODE
	return WideToUTF8(input);
#else
	return ANSIToUTF8(input);
#endif
}
std::wstring TStringToWide(const std::tstring input) {
#ifdef _UNICODE
	return input;
#else
	return ANSIToWide(input);
#endif
}

#else // !_WIN32

// Linux-based OSes including Android uses UTF-8 by default.
// ANSI and UTF-8 can be considered equivalent.
// Wide-char is barely used at all in Linux, but when it is, it's UTF-32.
// iconv() would be needed, and it's beyond the scope of a regular extension.
// Instead, this code merely returns back.

std::tstring ANSIToTString(const std::string input) {
	return UTF8ToTString(input);
}
std::string ANSIToUTF8(const std::string input) {
	return input;
}
std::wstring ANSIToWide(const std::string input) {
	assert(false && "Linux-based Wide not programmed yet.");
}
std::string UTF8ToANSI(const std::string input, bool * const allValidChars /* = nullptr */) {
	return input;
}
std::tstring UTF8ToTString(const std::string input, bool * const allValidChars /* = nullptr */) {
	return input;
}
std::wstring UTF8ToWide(const std::string input) {
	assert(false && "Linux-based Wide not programmed yet.");
}
std::string WideToANSI(const std::wstring input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
}
std::tstring WideToTString(const std::wstring input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
}
std::string WideToUTF8(const std::wstring input) {
	assert(false && "Linux-based Wide not programmed yet.");
}
std::string TStringToANSI(const std::tstring input, bool * const allValidChars /* = nullptr */) {
	return TStringToUTF8(input);
}
std::string TStringToUTF8(const std::tstring input) {
	return input;
}
std::wstring TStringToWide(const std::tstring input) {
	assert(false && "Linux-based Wide not programmed yet.");
}

#endif

/// <summary> Creates a Prop_Str from UTF-8 char *. Allocated by new. </summary>
Prop_Str * Prop_Str_FromUTF8(const char * u8)
{
	return new Prop_Str(UTF8ToTString(u8).c_str());
}

// =====
// Object properties; read user values from properties in Extension ctor
// =====

// Returns property checked or unchecked.
bool EDITDATA::IsPropChecked(int propID)
{
	return (DarkEDIF_Props[propID >> 3] >> (propID % 8) & 1);
}
// Returns std::tstring property setting from property name.
std::tstring EDITDATA::GetPropertyStr(const char * propName)
{
	const json_value &props = CurLang["Properties"];
	for (size_t i = 0; i < props.u.array.length; i++)
	{
		if (!_stricmp(props[i]["Title"], propName))
			return GetPropertyStr(i);
	}
	return _T("Property name not found.");
}
// Returns std::tstring property string from property ID.
std::tstring EDITDATA::GetPropertyStr(int propID)
{
	if (propID < 0 || (size_t)propID > CurLang["Properties"].u.array.length)
		return _T("Property ID not found.");

	const json_value &prop = CurLang["Properties"][propID];
	if (!_stricmp(prop["Type"], "Combo Box"))
		return UTF8ToTString((const char  *)prop["Items"][*(unsigned int *)PropIndex(this, propID, nullptr)]);
	else if (!_stricmp(prop["Type"], "Editbox String"))
	{
		unsigned int propDataSize = 0;
		char * propDataStart = PropIndex(this, propID, &propDataSize);
		// Size - 1 to remove null
		return UTF8ToTString(std::string(propDataStart, propDataSize - 1));
	}
	else
		return _T("Property not textual.");
}
