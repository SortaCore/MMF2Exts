#include "Common.h"
#include <atomic>
#include "DarkEdif.h"
#include <math.h>
#include "Extension.h"

#ifdef _WIN32
extern HINSTANCE hInstLib;
#endif
extern Edif::SDK * SDK;

#if EditorBuild

static const _json_value * StoredCurrentLanguage = &json_value_none;

static const _json_value * DefaultLanguageIndex()
{
	// Misuse of function; called before ::SDK is valid
	if (!::SDK)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Premature function call!\n  Called DefaultLanguageIndex() before ::SDK was a valid pointer."));
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

	// Not in editor mode; the fancy language features are not necessary
	if (DarkEdif::RunMode == DarkEdif::MFXRunMode::RunApplication || DarkEdif::RunMode == DarkEdif::MFXRunMode::BuiltEXE)
		return *(StoredCurrentLanguage = DefaultLanguageIndex());

	std::string langList = DarkEdif::GetIniSetting("Languages"sv);
	if (langList.empty())
		return *DefaultLanguageIndex();

	langList.append("\r");

	size_t readIndex = 0;
	// Read string until newline. Expect ";" or "; "-delimited list of languages.
	while (langList.find_first_of(";\r", readIndex) != std::string::npos) // Is the semi-colon after an end-of-line character?
	{
		// Read individual token
		std::string langItem(langList.substr(readIndex, langList.find_first_of(";\r", readIndex)-readIndex));

		// If languages are separated by "; " not ";"
		if (langItem.front() == ' ')
		{
			++readIndex;
			langItem.erase(langItem.begin());
		}

		// langItem matched, get index of language in JSON
		for (unsigned int i = 0; i < ::SDK->json.u.object.length; ++i)
		{
			// Return index
			if ((*::SDK->json.u.object.values[i].value).type == json_object &&
				!_stricmp(::SDK->json.u.object.values[i].name, langItem.c_str()))
			{
				StoredCurrentLanguage = SDK->json.u.object.values[i].value;
				return *StoredCurrentLanguage;
			}
		}
		readIndex += langItem.size()+1;
		if (langList.at(readIndex-1) == '\r')
			break;
	}

	return *DefaultLanguageIndex();
}
#endif // EditorBuild

inline ACEInfo * ACEInfoAlloc(unsigned int NumParams)
{
	// Allocate space for ACEInfo struct, plus Parameter[NumParams] so it has valid memory
	return (ACEInfo *)calloc(sizeof(ACEInfo) + (NumParams * sizeof(short) * 2), 1);	// The *2 is for reserved variables
}
ExpReturnType ReadExpressionReturnType(const char * Text);

#ifdef RUN_ONLY
#define CurLang (*::SDK->json.u.object.values[::SDK->json.u.object.length - 1].value)
#endif

bool CreateNewActionInfo(void)
{
	// Get ID and thus properties by counting currently existing actions.
	const json_value & UnlinkedAction = CurLang["Actions"][(std::int32_t)::SDK->ActionInfos.size()];

	// Invalid JSON reference
	if (UnlinkedAction.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Invalid JSON reference for action ID %zu, expected object."), ::SDK->ActionInfos.size()), false;

	const json_value & Param = UnlinkedAction["Parameters"];

	// Num of parameters is beyond number of bits in FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Too many parameters in action ID %zu."), ::SDK->ActionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ActInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ActInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating action info"), _T("Could not allocate memory for action ID %zu return."), ::SDK->ActionInfos.size()), false;

	ActInfo->ID = (short)SDK->ActionInfos.size();
	ActInfo->NumOfParams = Param.u.object.length;

	if (ActInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < ActInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ActInfo->Parameter[c].p = ReadActionOrConditionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ActInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&ActInfo->Parameter[ActInfo->NumOfParams], 0, ActInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	SDK->ActionInfos.push_back(ActInfo);
	return true;
}

bool CreateNewConditionInfo(void)
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Condition = CurLang["Conditions"][(std::int32_t)::SDK->ConditionInfos.size()];

	// Invalid JSON reference
	if (Condition.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Invalid JSON reference for condition ID %zu, expected a JSON object."), ::SDK->ConditionInfos.size()), false;

	const json_value & Param = Condition["Parameters"];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags) *CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Too many parameters in condition ID %zu."), ::SDK->ConditionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * CondInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!CondInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating condition info"), _T("Could not allocate memory for condition ID %zu return."), ::SDK->ConditionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	CondInfo->ID = (short)::SDK->ConditionInfos.size();
	CondInfo->NumOfParams = Param.u.object.length;
	CondInfo->Flags.ev = bool (Condition["Triggered"]) ? EVFLAGS::NONE : (EVFLAGS::ALWAYS | EVFLAGS::NOTABLE);

	if (CondInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < CondInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			CondInfo->Parameter[c].p = ReadActionOrConditionParameterType(Param[c][0], IsFloat);	// Store parameter type
			CondInfo->FloatFlags |= (IsFloat << c);								// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&CondInfo->Parameter[CondInfo->NumOfParams], 0, CondInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	::SDK->ConditionInfos.push_back(CondInfo);
	return true;
}

bool CreateNewExpressionInfo(void)
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Expression = CurLang["Expressions"][(std::int32_t)::SDK->ExpressionInfos.size()];

	// Invalid JSON reference
	if (Expression.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Invalid JSON reference for expression ID %zu, expected a JSON object."), ::SDK->ExpressionInfos.size()), false;

	const json_value & Param = Expression["Parameters"];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Too many JSON parameters in expression ID %zu."), ::SDK->ExpressionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ExpInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ExpInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Could not allocate memory for expression ID %zu return."), ::SDK->ExpressionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	ExpInfo->ID = (short)::SDK->ExpressionInfos.size();
	ExpInfo->NumOfParams = Param.u.object.length;
	ExpInfo->Flags.ef = ReadExpressionReturnType(Expression["Returns"]);

	if (ExpInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < ExpInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ExpInfo->Parameter[c].ep = ReadExpressionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ExpInfo->FloatFlags |= (IsFloat << c);											// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	::SDK->ExpressionInfos.push_back(ExpInfo);
	return true;
}

// DarkEdif - automatic property setup

#ifndef NOPROPS

#if EditorBuild

using namespace Edif::Properties;
#include <sstream>

void InitializePropertiesFromJSON(mv * mV, EDITDATA * edPtr)
{
	std::stringstream propValues;
	const size_t propChkboxesSize = (size_t)std::ceil(CurLang["Properties"].u.array.length / 8.0f);
	std::unique_ptr<char[]> propChkboxes = std::make_unique<char[]>(propChkboxesSize);

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
				{
					DarkEdif::MsgBox::WarningOK(_T("Property warning"), _T("Invalid or no default checkbox value specified for property %s (ID %u)."),
						::SDK->EdittimeProperties[i].Title, i);
				}

				if (JProp["DefaultState"])
					propChkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

				break;
			}

			case PROPTYPE_COLOR:
			case PROPTYPE_EDIT_NUMBER:
			{
				if (JProp["DefaultState"].type != json_integer)
				{
					DarkEdif::MsgBox::WarningOK(_T("Property warning"), _T("Invalid or no default integer value specified for property %s (ID %u)."),
						::SDK->EdittimeProperties[i].Title, i);
				}

				unsigned int i = unsigned int(long long(JProp["DefaultState"]) & 0xFFFFFFFF);
				propValues.write((char *)&i, sizeof(unsigned int)); // embedded nulls upset the << operator

				if (JProp["ChkDefault"])
					propChkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

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
				{
					DarkEdif::MsgBox::WarningOK(_T("Property warning"), _T("Invalid or no default string value specified for property %s (ID %u)."),
						::SDK->EdittimeProperties[i].Title, i);
				}

				// No casing change necessary
				if (_stricmp(JProp["Case"], "Upper") && _stricmp(JProp["Case"], "Lower")) {
					propValues << (const char *)(JProp["DefaultState"]) << char(0);
				}
				else
				{
					std::string dup(JProp["DefaultState"]);
					std::transform(dup.begin(), dup.end(), dup.begin(), !_stricmp(JProp["Case"], "Upper") ? ::toupper : ::tolower);
					propValues << dup << char(0);
				}

				if (JProp["ChkDefault"])
					propChkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

				break;
			}


			case PROPTYPE_COMBOBOX:
			{
				unsigned int i = 0U;
				if (JProp["DefaultState"].type != json_string)
				{
					DarkEdif::MsgBox::WarningOK(_T("Property warning"), _T("Invalid or no default string value specified for property %s (ID %u)."),
						::SDK->EdittimeProperties[i].Title, i);
				}
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

					DarkEdif::MsgBox::WarningOK(_T("Property warning"), _T("Specified a default string in a combobox property that does not exist in items list - property %s (ID %u)."),
						::SDK->EdittimeProperties[i].Title, i);
				}
			ok:
				propValues.write((char *)&i, sizeof(i));

				if (JProp["ChkDefault"])
					propChkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

				break;
			}

			// These have no ID or property that can be changed
			default:
				break;
		}
	}

	std::string chkboxesAndValues(propChkboxes.get(), propChkboxesSize);
	chkboxesAndValues += propValues.str();

	size_t desiredEDITDATASize = sizeof(EDITDATA) + chkboxesAndValues.size();
	edPtr = (EDITDATA *) mvReAllocEditData(mV, edPtr, desiredEDITDATASize);
	if (!edPtr)
		return DarkEdif::MsgBox::Error(_T("Property error"), _T("Fusion runtime could not reallocate EDITDATA to size %zu. Property initialization failed."), desiredEDITDATASize);

	edPtr->DarkEdif_Prop_Size = desiredEDITDATASize;
	memcpy(edPtr->DarkEdif_Props, chkboxesAndValues.data(), chkboxesAndValues.size());
}

Prop * GetProperty(EDITDATA * edPtr, size_t ID)
{
	// Premature call
	if (edPtr->DarkEdif_Prop_Size == 0)
		return DarkEdif::MsgBox::Error(_T("Property error"), _T("Premature function call!\n  GetProperty() called without edPtr->DarkEdif_Props being valid.")), nullptr;

	const json_value &jsonItem = CurLang["Properties"][ID];
	const char * curStr = jsonItem["Type"];
	Prop * ret = nullptr;
	bool allConvToTString = false;

	// Edit button text isn't returned here; it's passed as lParam when EdittimeProperties are created.
	// Returning it here is pointless, as Fusion will ignore it.
	if (!_stricmp(curStr, "Edit button"))
		return nullptr;

	// Static text is returned here
	if (!_stricmp(curStr, "Text"))
	{
		ret = new Prop_Str(UTF8ToTString((const char *)jsonItem["DefaultState"], &allConvToTString).c_str());
		if (!allConvToTString)
		{
			// UTF-8 can't convert to ANSI easily, but should have no issue converting to UTF-16 (Wide)
			DarkEdif::MsgBox::WarningOK(_T("Property warning"),
				_T("Property %hs (ID %zu)'s JSON DefaultState string \"%hs\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."),
				(const char *)jsonItem["Title"], ID, (const char *)jsonItem["DefaultState"]);
		}
		return ret;
	}

	unsigned int size;
	char * Current = PropIndex(edPtr, ID, &size);

	if (!_stricmp(curStr, "Editbox String"))
	{
		ret = new Prop_Str(UTF8ToTString(Current, &allConvToTString).c_str());
		if (!allConvToTString)
		{
			// UTF-8 can't convert to ANSI easily, but should have no issue converting to UTF-16 (Wide)
			DarkEdif::MsgBox::WarningOK(_T("Property warning"),
				_T("Property %hs (ID %zu)'s Unicode string \"%hs\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."), (const char *)jsonItem["Title"], ID, Current);
		}
	}
	else if (!_stricmp(curStr, "Editbox Number") || !_stricmp(curStr, "Combo Box"))
		ret = new Prop_UInt(*(unsigned int *)Current);
	else if (_stricmp(curStr, "Checkbox") && _strnicmp(curStr, "Folder", sizeof("Folder") - 1))
	{
		// UTF-8 can't convert to ANSI easily, but should have no issue converting to UTF-16 (Wide)
		DarkEdif::MsgBox::Error(_T("Property error"), _T("Property %hs (ID %zu)'s type \"%hs\" wasn't understood. Can't return a Prop."),
			(const char *)jsonItem["Title"], ID, Current);
	}

	return ret;
}

void PropChangeChkbox(EDITDATA * edPtr, unsigned int PropID, const bool newValue)
{
	// The DarkEdif_Props consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	int byteIndex = PropID >> CHAR_BIT, bitIndex = PropID % CHAR_BIT;

	if (newValue)
		edPtr->DarkEdif_Props[byteIndex] |= 1 << bitIndex;
	else
		edPtr->DarkEdif_Props[byteIndex] &= ~(1 << bitIndex);
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
	else if (!_stricmp(curTypeStr, "Checkbox") || !_strnicmp(curTypeStr, "Folder", sizeof("Folder") - 1))
		return; // Checkbox is handled by PropChangeChkbox(), folder has no possible changes
	else
		DarkEdif::MsgBox::Error(_T("Property error"), _T("Don't understand JSON property type \"%s\", can't return changed Prop."), UTF8ToTString(curTypeStr).c_str());

	if (!rearrangementRequired)
	{
		memcpy(oldPropValue, newPropValue, newPropValueSize);
		return;
	}

	// Even an empty string should be 1 (null char). Warn if not.
	if (oldPropValueSize == 0)
		DarkEdif::MsgBox::WarningOK(_T("Debug info"), _T("Property %hs (ID %i)'s value size is 0!"), (const char *)CurLang["Properties"][PropID]["TItle"], PropID);

	size_t beforeOldSize = sizeof(EDITDATA) +
		(oldPropValue - edPtr->DarkEdif_Props); // Pointer to O|<P|O
	size_t afterOldSize = edPtr->DarkEdif_Prop_Size - oldPropValueSize - beforeOldSize;			// Pointer to O|P>|O
	size_t odps = edPtr->DarkEdif_Prop_Size;

	// Duplicate memory to another buffer (if new arragement is smaller - we can't just copy from old buffer after realloc)
	char * newEdPtr = (char *)malloc(edPtr->DarkEdif_Prop_Size + (newPropValueSize - oldPropValueSize));

	if (!newEdPtr)
	{
		DarkEdif::MsgBox::Error(_T("Property error"), _T("Out of memory attempting to rewrite properties!"));
		return;
	}
	((EDITDATA *)newEdPtr)->DarkEdif_Prop_Size = _msize(newEdPtr);

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
		DarkEdif::MsgBox::Error(_T("Property error"), _T("NULL returned from EDITDATA reallocation to %zu bytes. Property changed cancelled."), _msize(newEdPtr));
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

#endif // EditorBuild

char * PropIndex(EDITDATA * edPtr, unsigned int ID, unsigned int * size)
{
	char * Current = &edPtr->DarkEdif_Props[(size_t)ceil(CurLang["Properties"].u.array.length / 8.0f)], * StartPos, * EndPos;

	const json_value &j = CurLang["Properties"];
	if (j.type != json_array)
		return DarkEdif::MsgBox::Error(_T("Property error"), _T("Premature function call!\n  PropIndex() called without edPtr->DarkEdif_Props being valid.")), nullptr;

	const char * curStr = (const char *)j[(int)ID]["Type"];
	// Read unchangable properties
	if (!_stricmp(curStr, "Text") || !_stricmp(curStr, "Edit button") || !_stricmp(curStr, "Checkbox") || !_strnicmp(curStr, "Folder", sizeof("Folder") - 1))
		return nullptr;
	// if (curTypeStr == "other stuff")
	//	return new Prop_XXX();

	// Read changable properties
	StartPos = Current; // For ID 0
	size_t i = 0;
	while (i <= ID)
	{
		curStr = (const char *)j[(std::int32_t)i]["Type"];

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
		*size = (std::uint32_t)(EndPos - StartPos);
	return StartPos;
}

#endif // NOPROPS

// =====
// Get event number (CF2.5+ feature)
// =====


// Static definition; set during SDK::SDK()
bool DarkEdif::IsFusion25;

// Returns the Fusion event number for this group. Works in CF2.5 and MMF2.0
std::uint16_t DarkEdif::GetEventNumber(eventGroup * evg) {
	if (DarkEdif::IsFusion25) {
		return evg->evgInhibit;
	}
	return evg->evgIdentifier;
}

/// <summary> If error, -1 is returned. </summary>
int DarkEdif::GetCurrentFusionEventNum(const Extension * const ext)
{
	// Reading Fusion's internals requires the main runtime to not be editing them
	if (MainThreadID != std::this_thread::get_id()) {
		LOGE(_T("Read GetCurrentFusionEventNum from non-main thread. Returning -1.\n"));
		return -1;
	}

#ifdef _WIN32
	// Can we read current event?
	if (!ext->rhPtr->EventGroup)
		return -1;

	int eventNum = GetEventNumber(ext->rhPtr->EventGroup);
	if (eventNum != 0)
		return eventNum;
	return -1;
#elif defined(__ANDROID__)
	// Call `int darkedif_jni_getCurrentFusionEventNum()` Java function
	static jmethodID getEventIDMethod;
	if (getEventIDMethod == nullptr)
	{
		jclass javaExtClass = threadEnv->GetObjectClass(ext->javaExtPtr);
		getEventIDMethod = threadEnv->GetMethodID(javaExtClass, "darkedif_jni_getCurrentFusionEventNum", "()I");

		// This is a Java wrapper implementation failure and so its absence should be considered fatal
		if (getEventIDMethod == nullptr)
			LOGF("Failed to find CRun" PROJECT_NAME_UNDERSCORES "'s darkedif_jni_getCurrentFusionEventNum method in Java wrapper file.\n");
	}

	return threadEnv->CallIntMethod(ext->javaExtPtr, getEventIDMethod);
#else // iOS
	return DarkEdif_getCurrentFusionEventNum(ext->objCExtPtr);
#endif
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

std::tstring ANSIToTString(const std::string_view input) {
	return WideToTString(ANSIToWide(input));
}
std::string ANSIToUTF8(const std::string_view input) {
	return WideToUTF8(ANSIToWide(input));
}
std::wstring ANSIToWide(const std::string_view input) {
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_ACP, 0, input.data(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_ACP, 0, input.data(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");

	return outputStr;
}
std::string UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return WideToANSI(UTF8ToWide(input), allValidChars);
}
std::tstring UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true; // UTF-8 and UTF-16 share all chars
	return UTF8ToWide(input);
#else
	return UTF8ToANSI(input, allValidChars);
#endif
}
std::wstring UTF8ToWide(const std::string_view input)
{
	if (input.empty())
		return std::wstring();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = MultiByteToWideChar(CP_UTF8, 0, input.data(), input.size(), NULL, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::wstring outputStr(length, L'\0');

	// Actually convert
	length = MultiByteToWideChar(CP_UTF8, 0, input.data(), input.size(), outputStr.data(), outputStr.size());
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != '\0' && "Input ends with null.");
	assert(outputStr.back() != L'\0' && "Output ends with null.");
	return outputStr;
}
std::string WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	if (input.empty())
	{
		if (allValidChars)
			*allValidChars = true;
		return std::string();
	}

	BOOL someFailed = FALSE;

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_ACP, 0, input.data(), input.size(), NULL, 0, 0, allValidChars ? &someFailed : NULL);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");

	if (allValidChars)
		*allValidChars = (someFailed == FALSE);

	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_ACP, 0, input.data(), input.size(), outputStr.data(), outputStr.size(), 0, NULL);
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::tstring WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true;
	return std::wstring(input);
#else
	return WideToANSI(input, allValidChars);
#endif
}
std::string WideToUTF8(const std::wstring_view input)
{
	if (input.empty())
		return std::string();

	// First call WideCharToMultiByte() to get output size to reserve
	size_t length = WideCharToMultiByte(CP_UTF8, 0, input.data(), input.size(), NULL, 0, 0, 0);
	assert(length > 0 && "Failed to convert between string encodings, input string is broken.");
	std::string outputStr(length, '\0');

	// Actually convert
	length = WideCharToMultiByte(CP_UTF8, 0, input.data(), input.size(), outputStr.data(), outputStr.size(), 0, 0);
	assert(length > 0 && "Failed to convert between string encodings.");
	assert(input.back() != L'\0' && "Input ends with null.");
	assert(outputStr.back() != '\0' && "Output ends with null.");
	return outputStr;
}
std::string TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	return WideToANSI(input, allValidChars);
#else
	if (allValidChars)
		*allValidChars = true;
	return std::string(input);
#endif
}
std::string TStringToUTF8(const std::tstring_view input) {
#ifdef _UNICODE
	return WideToUTF8(input);
#else
	return ANSIToUTF8(input);
#endif
}
std::wstring TStringToWide(const std::tstring_view input) {
#ifdef _UNICODE
	return std::wstring(input);
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

std::tstring ANSIToTString(const std::string_view input) {
	return UTF8ToTString(input);
}
std::string ANSIToUTF8(const std::string_view input) {
	return std::string(input);
}
std::wstring ANSIToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::string(input);
}
std::tstring UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::tstring(input);
}
std::wstring UTF8ToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::tstring WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::tstring();
}
std::string WideToUTF8(const std::wstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::string TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
	return TStringToUTF8(input);
}
std::string TStringToUTF8(const std::tstring_view input) {
	return std::string(input);
}
std::wstring TStringToWide(const std::tstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}

#endif

// =====
// Object properties; read user values from properties in Extension ctor
// =====

#ifndef NOPROPS
// Returns property checked or unchecked.
bool EDITDATA::IsPropChecked(int propID)
{
	return (DarkEdif_Props[propID / CHAR_BIT] >> (propID % CHAR_BIT)) & 1;
}
// Returns std::tstring property setting from property name.
std::tstring EDITDATA::GetPropertyStr(const char * propName)
{
	const json_value &props = CurLang["Properties"];
	for (size_t i = 0; i < props.u.array.length; i++)
	{
		if (!_stricmp(props[(std::int32_t)i]["Title"], propName))
			return GetPropertyStr((int)i);
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
		return UTF8ToTString((const char  *)prop["Items"][*(int *)PropIndex(this, propID, nullptr)]);
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
#endif // not NOPROPS


#ifdef __ANDROID__
extern thread_local JNIEnv * threadEnv;
static jobject getGlobalContext()
{
	jclass activityThread = threadEnv->FindClass("android/app/ActivityThread");
	jmethodID currentActivityThread = threadEnv->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
	jobject at = threadEnv->CallStaticObjectMethod(activityThread, currentActivityThread);

	jmethodID getApplication = threadEnv->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
	jobject context = threadEnv->CallObjectMethod(at, getApplication);
	return context;
}
int MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	jclass toast = threadEnv->FindClass("android/widget/Toast");
	jobject globalContext = getGlobalContext();
	jmethodID methodMakeText = threadEnv->GetStaticMethodID(toast, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
	if (methodMakeText == NULL) {
		LOGE("toast.makeText not Found");
		return 0;
	}

	std::string toastText = caption + std::string(" -  ") + text;
	jstring toastTextJStr = CStrToJStr(toastText.c_str());

	jobject toastobj = threadEnv->CallStaticObjectMethod(toast, methodMakeText, globalContext, toastTextJStr, 1 /* toast length long, 0 for short*/);

	// Java: toastobj.show();
	jmethodID methodShow = threadEnv->GetMethodID(toast, "show", "()V");
	threadEnv->CallVoidMethod(toastobj, methodShow);

	__android_log_print(iconAndButtons, PROJECT_NAME_UNDERSCORES, "Msg Box swallowed: \"%s\", %s.", caption, text);
	if (!strncmp(caption, "DarkEdif", sizeof("DarkEdif") - 1) && (iconAndButtons & MB_ICONERROR) != 0)
		DarkEdif::BreakIfDebuggerAttached();
	return 0;
}

void DarkEdif::BreakIfDebuggerAttached()
{
	raise(SIGINT);
}
#elif defined(_WIN32)

void DarkEdif::BreakIfDebuggerAttached()
{
	if (IsDebuggerPresent())
		DebugBreak();
}
#else // APPLE
void DarkEdif::BreakIfDebuggerAttached()
{
	__builtin_trap();
}

int MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	::DarkEdif::Log(iconAndButtons, "Message box \"%s\" absorbed: \"%s\".", caption, text);
	DarkEdif::BreakIfDebuggerAttached();
	return 0;
}

void LOGF(const char * x, ...)
{
	char buf[2048];
	va_list va;
	va_start(va, x);
	vsprintf(buf, x, va);
	va_end(va);
}
#endif


// ============================================================================
//
// DEBUGGER (Interaction with Fusion debugger)
//
// ============================================================================

#if USE_DARKEDIF_FUSION_DEBUGGER

namespace DarkEdif
{
#if RuntimeBuild
	// Collection of no-op dummies. We'll let the user code it in, but since Fusion
	// debugger doesn't exist at edittime, no reaction is possible.

	struct FusionDebuggerAdmin { };
	void FusionDebugger::AddItemToDebugger(
		void (*getLatestFromExt)(Extension *const, std::tstring &),
		bool (*saveUserInputToExt)(Extension *const, std::tstring &),
		size_t, const char *
	) { /* no op in runtime */ }
	void FusionDebugger::AddItemToDebugger(
		int (*)(Extension *const),
		bool (*)(Extension *const, int),
		size_t, const char *) { /* no op in runtime */ }
	void FusionDebugger::UpdateItemInDebugger(
		const char *, int) { /* no op in runtime */ }
	void FusionDebugger::UpdateItemInDebugger(
		const char *, const TCHAR *) { /* no op in runtime */ }
	FusionDebugger::FusionDebugger(Extension *const) { /* runtime debugger not used */ }

#else // It's editor build
	// Prevent the ext dev from messing with internal DarkEdif functions
	struct FusionDebuggerAdmin
	{
		inline std::uint16_t * GetDebugTree(Extension *const ext) {
			return ext->FusionDebugger.GetDebugTree();
		}
		inline void StartEditForItemID(Extension *const ext, int debugItemID) {
			ext->FusionDebugger.StartEditForItemID(debugItemID);
		}
		inline void GetDebugItem(Extension *const ext, TCHAR *writeTo, int debugItemID) {
			ext->FusionDebugger.GetDebugItemFromCacheOrExt(writeTo, debugItemID);
		}
	};

	std::uint16_t *FusionDebugger::GetDebugTree() {
		return debugItemIDs.data();
	}
	void FusionDebugger::StartEditForItemID(int debugItemID)
	{
		if (debugItemID < 0 || (std::uint16_t)debugItems.size() < debugItemID)
			throw std::exception("Couldn't find debug ID in Fusion debugger list.");
		auto &di = debugItems[debugItemID];

		EditDebugInfo edi = {};
		if (di.isInt)
		{
			if (!di.intStoreDataToExt)
				throw std::exception("Item not editable.");
			edi.value = di.cachedInt;
			long ret = ext->Runtime.EditInteger(&edi);
			if (ret == IDOK)
			{
				int oldInteger = di.cachedInt;
				di.cachedInt = edi.value;

				if (!di.intStoreDataToExt(ext, edi.value))
				{
					di.cachedInt = oldInteger;
					di.nextRefreshTime = GetTickCount();
				}
			}

		}
		else
		{
			if (!di.textStoreDataToExt)
				throw std::exception("Item not editable.");
			edi.text = di.cachedText.data();
			di.cachedText.resize(_tcslen(edi.text));
			edi.lText = di.cachedText.size();
			long ret = ext->Runtime.EditText(&edi);
			if (ret == IDOK)
			{
				std::tstring oldText = di.cachedText;
				di.cachedText = edi.text;
				if (!di.textStoreDataToExt(ext, di.cachedText))
				{
					di.cachedText = oldText;
					di.nextRefreshTime = GetTickCount();
				}
			}
		}
	}
	void FusionDebugger::GetDebugItemFromCacheOrExt(TCHAR *writeTo, int debugItemID)
	{
		if (debugItemID < 0 || debugItemID >= (std::uint16_t)debugItems.size())
			throw std::exception("Couldn't find debug ID in Fusion debugger list.");

		// Reader function exists, and timer for refreshing (if it exists) has expired
		auto &di = debugItems[debugItemID];
		if ((di.isInt ? di.intReadFromExt != NULL : di.textReadFromExt != NULL) &&
			(!di.refreshMS || GetTickCount() >= di.nextRefreshTime))
		{
			if (di.isInt)
				di.cachedInt = di.intReadFromExt(ext);
			else
			{
				di.textReadFromExt(ext, di.cachedText);
				if (di.cachedText.size() > 255)
				{
#ifndef _UNICODE
					const std::tstring ellipse("...");
#else
					const std::tstring ellipse(L"…");
#endif
					di.cachedText.resize(255 - ellipse.size());
					di.cachedText += ellipse;
				}
			}
			if (di.refreshMS)
				di.nextRefreshTime = GetTickCount() + di.refreshMS;
		}
		_tcscpy_s(writeTo, 256U, di.cachedText.c_str());
	}

	void FusionDebugger::AddItemToDebugger(
		// Supply NULL if it will not ever change.
		void (*getLatestFromExt)(Extension *const ext, std::tstring &writeTo),
		// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not accepted.
		bool (*saveUserInputToExt)(Extension *const ext, std::tstring &newValue),
		// Supply 0 if no caching should be used, otherwise will re-call reader().
		size_t refreshMS,
		// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
		const char *userSuppliedName
	) {
		if (debugItems.size() == 127)
			throw std::exception("Too many items added to Fusion debugger.");

		debugItems.push_back(DebugItem(getLatestFromExt, saveUserInputToExt, refreshMS, userSuppliedName));
		// End it with DB_END, and second-to-last item is the new debug item ID
		debugItemIDs.push_back(DB_END);
		debugItemIDs[debugItemIDs.size() - 2] = (((std::uint16_t)debugItems.size()) - 1) | (saveUserInputToExt != NULL ? DB_EDITABLE : 0);
	}

	void FusionDebugger::AddItemToDebugger(
		// Supply NULL if it will not ever change.
		int (*getLatestFromExt)(Extension *const ext),
		// Supply NULL if not editable. In function, return true if cache should be updated, false if edit attempt was not
		bool (*saveUserInputToExt)(Extension *const ext, int newValue),
		// Supply 0 if no caching should be used, otherwise will re-call reader() every time Fusion requests.
		size_t refreshMS,
		// Supply NULL if not removable. Case-sensitive name, used for removing from Fusion debugger if needed.
		const char *userSuppliedName
	) {
		if (debugItems.size() == 127)
			throw std::exception("too many items added to Fusion debugger");

		if (userSuppliedName && std::any_of(debugItems.cbegin(), debugItems.cend(), [=](const DebugItem &d) { return d.DoesUserSuppliedNameMatch(userSuppliedName); }))
			throw std::exception("name already in use. Must be unique");

		debugItems.push_back(DebugItem(getLatestFromExt, saveUserInputToExt, refreshMS, userSuppliedName));
		// End it with DB_END, and second-to-last item is the new debug item ID
		debugItemIDs.push_back(DB_END);
		debugItemIDs[debugItemIDs.size() - 2] = (((std::uint16_t)debugItems.size()) - 1) | (saveUserInputToExt != NULL ? DB_EDITABLE : 0);
	}

	void FusionDebugger::UpdateItemInDebugger(
		const char *userSuppliedName, int newValue
	) {
		for (size_t i = 0; i < debugItems.size(); i++)
		{
			if (debugItems[i].DoesUserSuppliedNameMatch(userSuppliedName))
			{
				if (debugItems[i].isInt)
					debugItems[i].cachedInt = newValue;
				else
					throw std::exception("Fusion debugger item is text, not int type");
				return;
			}
		}
	}

	void FusionDebugger::UpdateItemInDebugger(
		const char *userSuppliedName, const TCHAR *newText
	) {
		if (!newText)
			throw std::exception("null not allowed");

		for (size_t i = 0; i < debugItems.size(); i++)
		{
			if (debugItems[i].DoesUserSuppliedNameMatch(userSuppliedName))
			{
				if (debugItems[i].isInt)
					throw std::exception("Fusion debugger item is text, not int type");
				else
					debugItems[i].cachedText = newText;
				return;
			}
		}
	}

	FusionDebugger::FusionDebugger(Extension *const ext) : ext(ext) {
		// due to DB_EDITABLE flag being 0x80 in db value IDs, or 128, we can't have more than 128 editable properties.
		// DB IDs are 16-bit, so it might be possible to skip all IDs with 0x80's and use IDs 0-127 then 256-383, etc,
		// leaving 0x80 bit untouched, but haven't tested that.
		debugItems.reserve(128);
		debugItemIDs.reserve(129);
		debugItemIDs.push_back(DB_END);
	}
#endif // EditorBuild
}

#if EditorBuild

static DarkEdif::FusionDebuggerAdmin FusionDebugAdmin;

// ============================================================================
//
// DEBUGGER ROUTINES
//
// ============================================================================

// This routine returns the address of the debugger tree
std::uint16_t * FusionAPI GetDebugTree(RUNDATA *rdPtr)
{
#pragma DllExportHint
	return FusionDebugAdmin.GetDebugTree(rdPtr->pExtension);
}

// This routine returns the text of a given item.
void FusionAPI GetDebugItem(TCHAR *pBuffer, RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.GetDebugItem(rdPtr->pExtension, pBuffer, id);
}

// This routine allows the user to edit the debugger's editable items.
void FusionAPI EditDebugItem(RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.StartEditForItemID(rdPtr->pExtension, id);
}
#endif // EditorBuild

#endif // USE_DARKEDIF_FUSION_DEBUGGER



// Removes the ending text if it exists, and returns true. If it doesn't exist, changes nothing and returns false.
bool DarkEdif::RemoveSuffixIfExists(std::tstring_view &tstr, const std::tstring_view suffix, bool caseInsensitive /*= true */)
{
	assert(suffix.size() != 0);

	// tstr is smaller than suffix, obviously can't match
	if (tstr.size() < suffix.size())
		return false;
	if (EndsWith(tstr, suffix, caseInsensitive))
	{
		tstr.remove_suffix(suffix.size());
		return true;
	}
	return false;
}

// Checks if first parameter ends with second parameter, returns true if so.
bool DarkEdif::EndsWith(const std::tstring_view tstr, const std::tstring_view suffix, bool caseInsensitive /* = true */)
{
	if (tstr.size() < suffix.size())
		return false;

	if (caseInsensitive)
		return 0 == _tcsnicmp(tstr.data() + tstr.size() - suffix.size(), suffix.data(), suffix.size());
	return 0 == tstr.compare(tstr.size() - suffix.size(), suffix.size(), suffix);
}

bool DarkEdif::FileExists(const std::tstring_view path)
{
#if _WIN32
	const std::tstring pathSafe(path);
	const DWORD fileAttr = GetFileAttributes(pathSafe.c_str());
	return fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
#else
	throw std::runtime_error("Function not implemented in non-Windows");
#endif
}


#if _WIN32

static std::tstring appPath, mfxPath, stdrtPath;
static std::tstring GetModulePath(HMODULE hModule)
{
	std::tstring path(MAX_PATH, _T('\0'));
	DWORD numCharsWrittenExcludingNull = GetModuleFileName(hModule, path.data(), path.size());

	// Something broke in our call
	if (numCharsWrittenExcludingNull == 0)
		std::abort();

	// Successfully looked up the path!
	if (numCharsWrittenExcludingNull != MAX_PATH &&
		// Convert the path to long path, check it's not too long or error'd
		(numCharsWrittenExcludingNull = GetLongPathName(path.data(), path.data(), path.size())) < MAX_PATH && numCharsWrittenExcludingNull > 0)
	{
		return path.substr(0, numCharsWrittenExcludingNull);
	}

	// else path is too long.

	// Only Unicode L"\\?\" prefix allows using a bigger path.
	// There's no point calling the wide version and then WideToANSI, because the result would be unusable as it would
	// be bigger than MAX_PATH in any file call, and would potentially have replacement characters if the codepage
	// couldn't represent all of it.
	// WideToUTF8 works, but if you want to support Unicode on Windows, do that in the Unicode builds of your extension,
	// don't try and force it in an ANSI build.
	// The only possible workarounds is using an 8.3 path, and that can be disabled, and also might not be long enough.
	//
	// MAX_PATH can be extended in Windows 10 b1603+, but the app/mfx requires a manifest showing it's big path aware.
	// https://docs.microsoft.com/en-gb/windows/win32/fileio/maximum-file-path-limitation#enable-long-paths-in-windows-10-version-1607-and-later

#ifndef _UNICODE
	DarkEdif::MsgBox::Error(_T("Fatal error - Path too long"),
		_T("Your application path is too long, several Fusion extensions may have difficulty loading.\n"
			"Build with the Unicode " PROJECT_NAME " to remove the limit."));
	throw std::runtime_error("Path reading error"); // Don't bother returning anything to user code.
#else
	// Max limit on path is 32,767 characters... ish. The documentation says it may go further.
	// If we assume surrogate pairs are in play, this limit can nearly double, although the folder names in the path are still capped to MAX_PATH each.
	path.resize(64 * 1024);
	numCharsWrittenExcludingNull = GetModuleFileName(hModule, path.data(), path.size());

	// We have no error handling for when this fails; there's no workaround, so we might as well give up.
	if (numCharsWrittenExcludingNull == 0 || numCharsWrittenExcludingNull == MAX_PATH)
		std::abort();

	return path.substr(0, numCharsWrittenExcludingNull);
#endif
}
std::tstring_view DarkEdif::GetRunningApplicationPath(GetRunningApplicationPathType type)
{
	// Singleton; load paths if we don't have 'em
	if (appPath.empty())
	{
		appPath = GetModulePath(NULL);

		// The user is using "compress the runtime", so we're running inside temp, not app folder
		if (EndsWith(appPath, _T(".tmp\\stdrt.exe")))
		{
			stdrtPath = appPath;

			// Ccompress the runtime runs stdrt.exe with commandline
			// "%tmp%\mrtXXX.tmp\stdrt.exe" /SF "full path to original.exe" ...
			const std::tstring_view cmdLine = GetCommandLine();

			size_t sfAt = cmdLine.find(_T(" /SF \""));
			if (sfAt == std::tstring_view::npos)
				std::abort(); // Commandline is in unrecognised format
			sfAt += sizeof(" /SF \"") - 1;

			const size_t sfEnd = cmdLine.find(_T('"'), sfAt);
			if (sfEnd == std::tstring_view::npos)
				std::abort(); // Commandline is in unrecognised format

			appPath = cmdLine.substr(sfAt, sfEnd - sfAt);
		}
	}

	std::tstring_view path = appPath;
	if (!stdrtPath.empty() && (type & GetSTDRTNotApp) == GetSTDRTNotApp)
		path = stdrtPath;

	if ((type & AppFolderOnly) == AppFolderOnly)
	{
		size_t lastSlash = std::tstring::npos;
		if ((lastSlash = path.find(_T('\\'))) == std::tstring::npos)
			lastSlash = path.find(_T('/'));
		if (lastSlash == std::tstring::npos)
			std::abort();
		return path.substr(0, lastSlash);
	}

	return path;
}
//
std::tstring_view DarkEdif::GetMFXRelativeFolder(GetFusionFolderType type)
{
	if (mfxPath.empty())
		mfxPath = GetModulePath(hInstLib);

	// Start by getting the MFX parent folder
	std::tstring_view curPath(mfxPath.c_str(), mfxPath.rfind(_T('\\')) + 1);

	// If in Extensions\Unicode, hop out to Extensions level; likewise for Data\Runtime\Unicode
	if (type != GetFusionFolderType::MFXLocation && !_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Unicode\\") - 1), _T("Unicode\\"), sizeof("Unicode\\") - 1))
		curPath.remove_suffix(sizeof("Unicode\\") - 1);

	// Go straight to root
	if (type == GetFusionFolderType::FusionRoot)
	{
		if (!_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Extensions\\") - 1), _T("Extensions\\"), sizeof("Extensions\\") - 1))
			curPath.remove_suffix(sizeof("Extensions\\") - 1);
		else if (!_tcsnicmp(curPath.data() + curPath.size() - (sizeof("Data\\Runtime\\") - 1), _T("Data\\Runtime\\"), sizeof("Data\\Runtime\\") - 1))
			curPath.remove_suffix(sizeof("Data\\Runtime") - 1);
		//else // we're not in Extensions or Data\Runtime... where are we?
		//	DarkEdif::MsgBox::Error(_T("Folder location failure"), _T("Couldn't calculate the Fusion root folder from \"%s\"; can't look for settings!"), FileToLookup);
	}

	return curPath;
}
#else
std::tstring_view DarkEdif::GetRunningApplicationPath(DarkEdif::GetRunningApplicationPathType type)
{
	throw std::runtime_error("GetRunningApplicationPath function not implemented on non-Windows.");
}
std::tstring_view DarkEdif::GetMFXRelativeFolder(GetFusionFolderType type)
{
	throw std::runtime_error("GetMFXRelativeFolder function not implemented on non-Windows.");
}
#endif // _WIN32

#if EditorBuild

// =====
// Get DarkEdif INIs and lines
// =====

static std::string sdkSettingsFileContent;
static std::atomic<bool> fileLock;
static bool fileOpened;

std::string DarkEdif::GetIniSetting(const std::string_view key)
{
	assert(DarkEdif::RunMode != DarkEdif::MFXRunMode::RunApplication && DarkEdif::RunMode != DarkEdif::MFXRunMode::BuiltEXE);

	// File locked already; wait
	while (fileLock.exchange(true))
		/* wait */;

	if (!fileOpened)
	{
		fileOpened = true; // Prevent repeatedly loading it into memory

		std::tstring iniPath(GetMFXRelativeFolder(GetFusionFolderType::FusionExtensions));
		iniPath += _T("DarkEdif.ini"sv);

		// Is the file in the directory of the MFX? (should be, languages are only needed in edittime)
		if (!FileExists(iniPath))
		{
			// Ignore DarkEdif.ini being non-existent; all other errors we report
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
				DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Error %u opening Extensions\\DarkEdif.ini."), GetLastError());

			fileLock = false;
			return std::string();
		}

		// Open DarkEdif.ini settings file in read binary, and deny other apps writing permissions.
		FILE * fileHandle = _tfsopen(iniPath.c_str(), _T("rb"), _SH_DENYWR);

		// Could not open; abort (should report error)
		if (!fileHandle)
		{
			DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Couldn't open Extensions\\DarkEdif.ini, error %d."), errno);
			fileLock = false;
			return std::string();
		}

		fseek(fileHandle, 0, SEEK_END);
		long fileSize = ftell(fileHandle);
		fseek(fileHandle, 0, SEEK_SET);

		sdkSettingsFileContent.resize(fileSize + 2);
		// Could not read all of the file properly
		if (fileSize != fread_s(&sdkSettingsFileContent.data()[1], sdkSettingsFileContent.size() - 2, 1, fileSize, fileHandle))
		{
			DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("Couldn't read Extensions\\DarkEdif.ini file, error %i."), errno);
			fclose(fileHandle);
			sdkSettingsFileContent.clear();
			fileLock = false;
			return std::string();
		}
		// Start and end with newline for easy line detection
		sdkSettingsFileContent.back() = sdkSettingsFileContent.front() = '\n';

		// Destroy any spaces to left or right of "=" - will only handle one space on the sides
		for (size_t s = sdkSettingsFileContent.find('='); s != std::string::npos; s = sdkSettingsFileContent.find('=', s))
		{
			if (sdkSettingsFileContent[s - 1] == ' ')
				sdkSettingsFileContent.erase(--s, 1);
			if (sdkSettingsFileContent[s + 1] == ' ')
				sdkSettingsFileContent.erase(++s, 1);
			if (++s > sdkSettingsFileContent.size())
			{
				DarkEdif::MsgBox::Error(_T("DarkEdif SDK error"), _T("SDK settings file is incorrectly formatted (ends with '=')."));
				break;
			}
		}

		fclose(fileHandle);
		// fall thru into unlock
	}
	// else other thread has finished loading the file, so we don't need to lock it
	fileLock = false;

	// Look for key (spaces around '=' are stripped in file open)
	std::string keyFind1;
	keyFind1 = '\n';
	keyFind1 += key;
	keyFind1 += '=';

	size_t Reading;
	if (sdkSettingsFileContent.find(keyFind1) != std::string::npos)
		Reading = sdkSettingsFileContent.find(keyFind1) + keyFind1.size();
	else // key not found in settings file
		return std::string();

	size_t lineEnd = sdkSettingsFileContent.find_first_of("\r\n", Reading);
	// Line hits end of file
	if (lineEnd == std::string::npos)
		return sdkSettingsFileContent.substr(Reading);

	return sdkSettingsFileContent.substr(Reading, lineEnd - Reading);
}

// =====
// Fusion SDK updater
// =====

#if USE_DARKEDIF_UPDATE_CHECKER

#include <WinSock2.h> // for network sockets
#include <shellapi.h> // for ShellExecuteW, opening the new ext version URL in preferred browser

static std::atomic_bool updateLock(false);
HANDLE updateThread;
static std::stringstream updateLog = std::stringstream();
static DarkEdif::SDKUpdater::ExtUpdateType pendingUpdateType;
static std::wstring pendingUpdateURL = std::wstring();
static std::wstring pendingUpdateDetails = std::wstring();

DWORD WINAPI DarkEdifUpdateThread(void * data);

void DarkEdif::SDKUpdater::StartUpdateCheck()
{
	// Can run in background, but not during startup screen. During the startup screen process, the MFX is loaded, read from, unloaded.
	// The update checker thread will try to write back to variables that have been unloaded from memory, crashing.
	// In Run Application/Built EXEs, it's useless, and shouldn't be running.
	if (DarkEdif::RunMode != DarkEdif::MFXRunMode::Editor)
		return DarkEdif::MsgBox::Error(_T("Critial error"), _T("The update checker is running during the wrong mode."));

	// Shouldn't run twice.
	if (updateThread != NULL)
		throw std::runtime_error("Using multiple update threads");

	updateThread = CreateThread(NULL, NULL, DarkEdifUpdateThread, NULL, 0, NULL);
	if (updateThread == NULL)
	{
		DarkEdif::MsgBox::Error(_T("Critial error"), _T("The update checker failed to start, error %u."), GetLastError());
		DarkEdifUpdateThread(::SDK);
	}
}

DarkEdif::SDKUpdater::ExtUpdateType DarkEdif::SDKUpdater::ReadUpdateStatus(std::string * logData)
{
	// Lock for safety
	while (updateLock.exchange(true))
		/* when holder releases, exchange() will return false */;

	if (logData)
		*logData = updateLog.str();
	ExtUpdateType extUpdateType = pendingUpdateType;

	updateLock = false; // unlock
	return extUpdateType;
}

static bool handledUpdate;
void DarkEdif::SDKUpdater::RunUpdateNotifs(mv * mV, EDITDATA * edPtr)
{
	if (handledUpdate)
		return;

	// Lock for safety
	while (updateLock.exchange(true))
		/* when holder releases, exchange() will return false */;

	ExtUpdateType extUpdateType = pendingUpdateType;
	if (extUpdateType == ExtUpdateType::CheckInProgress) {
		updateLock = false; // unlock
		return;
	}
	// Any other status indicates update thread isn't running, so no reason to watch the lock.
	updateLock = false; // unlock

	// Prevent this function running again
	handledUpdate = true;

	// Clean up the thread object
	CloseHandle(updateThread);
	updateThread = NULL;

	// Connection errors are relevant to all users
	if (extUpdateType == ExtUpdateType::ConnectionError) {
		DarkEdif::MsgBox::Error(_T("Update check error"), _T("Error occurred while checking for extension updates:\n%hs"), updateLog.str().c_str());
		return;
	}

	// Ext dev errors can only be fixed by ext developer.
	if (extUpdateType == ExtUpdateType::ExtDevError) {
		DarkEdif::MsgBox::Error(_T("Update check error"), _T("Extension developer error occurred while checking for extension updates:\n%ls"), pendingUpdateDetails.c_str());
		return;
	}

	// SDK updates can only be done by ext developer.
	if (extUpdateType == ExtUpdateType::SDKUpdate) {
		DarkEdif::MsgBox::WarningOK(_T("SDK update notice"), _T("SDK update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
		return;
	}

	if (extUpdateType != ExtUpdateType::Major && extUpdateType != ExtUpdateType::Minor && extUpdateType != ExtUpdateType::ReinstallNeeded)
		return;

	// Lots of magic numbers created by a lot of trial and error. Do not recommend.
	if (::SDK->Icon->GetWidth() != 32)
		return DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("" PROJECT_NAME "'s icon width is not 32. Contact the extension developer."));

	// If font creation fails, it's not that important; a null HFONT is replaced with a system default.
	HFONT font = CreateFontA(
		8 /* Height: 8px */,
		0 /* Width: 0; use closest match */,
		0 /* Escapement: Rotate 0 degrees */,
		0 /* Orientation:  Rotate 0 degrees */,
		FW_NORMAL /* Weight: Normal */,
		FALSE /* Italic */, FALSE /* Underline */, FALSE /* Strikeout */,
		ANSI_CHARSET /* Charset */,
		OUT_DEFAULT_PRECIS /* OUT_RASTER_PRECIS /* Out precision: pick raster font */, CLIP_DEFAULT_PRECIS /* Clip precision: default */,
		ANTIALIASED_QUALITY  /* Quality: Antialiased */,
		FF_MODERN /* Pitch and family: use fonts with constant stroke width */,
		"Small Fonts" /* Font face name */);

	auto FillBackground = [](const RECT rect, COLORREF fillColor) {
		// This is the grey background rectangle, which we'll need to both de-alpha and colour.
		if (::SDK->Icon->HasAlpha())
		{
			LPBYTE alpha = ::SDK->Icon->LockAlpha();
			if (alpha != nullptr)
			{
				for (int i = rect.top; i < rect.bottom; i++)
					memset(&alpha[rect.left + (i * 32)], 0xFF, rect.right - rect.left);
				::SDK->Icon->UnlockAlpha();
			}
		}

		::SDK->Icon->Fill(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, fillColor);
	};

	if (extUpdateType == ExtUpdateType::Major)
	{
		const RECT greyBkgdRect { 1, 8, 30, 27 };
		FillBackground(greyBkgdRect, RGB(80, 80, 80));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left + 1, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(240, 0, 0);
		::SDK->Icon->DrawTextA("MAJOR", sizeof("MAJOR") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left -= 1;
		textDrawRect.top += 6;
		::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		// It's possible to do this so the icon in the sidebar is updated too, but that causes Fusion to register that the properties have changed,
		// and causes Fusion to save the "update needed" icon into the MFA, which is no good as it'll never be restored to normal.
		// We'll attempt to avoid that, although copying the frame with an altered icon causes it to have the altered icon in sidebar too,
		// making our avoidance only an attempt.
		// mvInvalidateObject(mV, edPtr);

		if (DarkEdif::GetIniSetting("MsgBoxForMajorUpdate"sv) != "false")
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("Update notice"), _T("Major update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no.
			{
				int ret = MessageBoxW(NULL, (L"Major update for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " update notice", MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if (extUpdateType == ExtUpdateType::Minor)
	{
		const RECT greyBkgdRect{ 1, 18, 31, 31 };
		FillBackground(greyBkgdRect, RGB(60, 60, 60));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(0, 180, 180);

		::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		if (DarkEdif::GetIniSetting("MsgBoxForMinorUpdate"sv) == "true")
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("update notice"), _T("Minor update for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no.
			{
				int ret = MessageBoxW(NULL, (L"Minor update for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " update notice ", MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if (extUpdateType == ExtUpdateType::ReinstallNeeded)
	{
		const RECT greyBkgdRect{ 1, 18, 31, 31 };
		FillBackground(greyBkgdRect, RGB(60, 60, 60));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(180, 0, 0);

		::SDK->Icon->DrawTextA("REINSTL", sizeof("REINSTL") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		if (pendingUpdateDetails.size() > 3)
		{
			// No URL? Open a dialog to report it.
			if (pendingUpdateURL.empty())
				MsgBox::Info(_T("Reinstall needed"), _T("Reinstalled needed for " PROJECT_NAME ":\n%ls"), pendingUpdateDetails.c_str());
			else // URL? Request to open it. Let user say no, but default to yes.
			{
				int ret = MessageBoxW(NULL, (L"Reinstall needed for " PROJECT_NAME ":\n" + pendingUpdateDetails).c_str(), L"" PROJECT_NAME " error", MB_ICONERROR | MB_YESNO);
				if (ret == IDYES)
					ShellExecuteW(NULL, L"open", pendingUpdateURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}

	if (font)
		DeleteObject(font);
}

#pragma comment(lib,"ws2_32.lib")
#include <iomanip>
#include <functional>

std::string url_encode(const std::string & value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}

DWORD WINAPI DarkEdifUpdateThread(void *)
{
	// In order to detect it regardless of whether it as the start or end of the list,
	// we make sure the line content is wrapped in semicolons
	std::string ini = ";" + DarkEdif::GetIniSetting("DisableUpdateCheckFor"sv) + ";";

	// Remove spaces around the ';'s. We can't just remove all spaces, as some ext names have them.
	size_t semiSpace = 0;
	while ((semiSpace = ini.find("; ", semiSpace)) != std::string::npos)
		ini = ini.replace(semiSpace--, 2, ";");

	semiSpace = 0;
	while ((semiSpace = ini.find(" ;", semiSpace)) != std::string::npos)
		ini = ini.replace(semiSpace--, 2, ";");

	// Acquire the rudimentary lock, do op, and release
#define GetLockAnd(x) while (updateLock.exchange(true)) /* retry */; x; updateLock = false
#define GetLockSetConnectErrorAnd(x) while (updateLock.exchange(true)) /* retry */; x; pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError; updateLock = false

	// If the ext name is found, or the wildcard *
	if (ini.find(";" PROJECT_NAME ";") != std::string::npos || ini.find(";*;") != std::string::npos)
	{
		GetLockAnd(updateLog << "Update check was disabled."sv;
			pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::CheckDisabled);
		return 0;
	}

	std::string projConfig = STRIFY(CONFIG);
	while ((semiSpace = projConfig.find(' ')) != std::string::npos)
		projConfig.replace(semiSpace, 1, "%20");

	// Opt-in feature for tagging built EXEs with a unique key.
	// This key is unique per Fusion install, but is otherwise meaningless; it's not a hash of anything.

#if USE_DARKEDIF_UC_TAGGING
	// Due to code in the update checker caller, this MFX can be safely assumed to be in Extensions or Extensions\Unicode
	assert(DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor);

	// Find Data\Runtime MFX that will be used in built apps.
	// If ANSI runtime, it will only use ANSI MFX.
	// If Unicode runtime, it will always prefer a Data\Runtime\Unicode MFX, even when the MFA uses an ANSI Extensions MFX.
	// So in the case of a Unicode runtime, always look for Unicode MFX.

	std::tstring drMFXPath(DarkEdif::GetMFXRelativeFolder(DarkEdif::GetFusionFolderType::FusionRoot));
	drMFXPath += _T("Data\\Runtime\\"sv);
	if (mvIsUnicodeVersion(::SDK->mV))
	{
		std::tstring uniPath = drMFXPath;
		uniPath += _T("Unicode\\") PROJECT_NAME ".mfx"sv;

		if (!DarkEdif::FileExists(uniPath))
		{
			drMFXPath += _T("" PROJECT_NAME ".mfx"sv);

			// Couldn't find either; roll back to Uni for error messages
			if (!DarkEdif::FileExists(drMFXPath))
				drMFXPath = uniPath;

			// else roll with ANSI
		}
		else // else roll with Uni
			drMFXPath = uniPath;
	}
	else // ANSI runtime will only use ANSI MFX
		drMFXPath += _T("" PROJECT_NAME ".mfx"sv);

	// Stores UC tags in resources or registry.
	std::wstring resKey, regKey;
	bool resFileExists = true;
	const std::wstring_view UC_TAG_NEW_SETUP = L"New setup."sv;

	// If used, indicates the resource file didn't have it
	HKEY mainKey = NULL;

	// Use RAII to auto-close the opened registry key, when this update thread exits
	auto CloseRegHandle = [](HKEY *h) { if (*h != NULL) { RegCloseKey(*h); *h = NULL; } return; };
	std::unique_ptr<HKEY, std::function<void(HKEY*)>> registryHandleHolder(&mainKey, CloseRegHandle);

	// if Data\Runtime(\Unicode) MFX is missing.
	HMODULE readHandle = LoadLibraryEx(drMFXPath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (readHandle == NULL)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			resKey = L"Couldn't load MFX; not found"sv;
			resFileExists = false;
		}
		else if (GetLastError() == ERROR_ACCESS_DENIED)
			DarkEdif::MsgBox::Error(_T("Resource loading"), _T("Failed to set up ") PROJECT_NAME _T(" - access denied writing to Data\\Runtime MFX. Try running Fusion as admin, or re-installing the extension."));
		else // Some other error loading; we'll consider it fatal.
		{
			DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while reading Data\\Runtime MFX.\n%s"), GetLastError(), drMFXPath.c_str());
			std::abort();
		}
	}
	else
	{
		// Read key back from Data\\Runtime MFX
		HRSRC hsrcForRes = FindResourceEx(readHandle, RT_STRING, _T("UCTAG"), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		if (hsrcForRes == NULL)
			resKey = UC_TAG_NEW_SETUP;
		else
		{
			// Not actually a HGLOBAL! If you use GlobalXXX on it, you'll get an invalid handle error.
			// Use the Resource-related functions like LockResource instead.
			HGLOBAL data2 = LoadResource(readHandle, hsrcForRes);
			if (data2 == NULL)
			{
				DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading resource for reading."), GetLastError());
				FreeLibrary(readHandle);
				std::abort();
			}
			wchar_t * dataSrc = (wchar_t *)LockResource(data2);
			if (dataSrc == NULL)
			{
				DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while locking resource for reading."), GetLastError());
				FreeResource(data2);
				FreeLibrary(readHandle);
				std::abort();
			}

			unsigned short strLength = dataSrc[0];
			// We're setting to a view, but passing it to a copying std::wstring
			resKey = std::wstring_view(&dataSrc[1], strLength);
			UnlockResource(data2);
			FreeResource(data2);
		}

		FreeLibrary(readHandle);
	}

	// Attempt to open for writing
	HANDLE resHandle = NULL;
	if (resFileExists)
	{
		resHandle = BeginUpdateResource(drMFXPath.c_str(), FALSE);
		if (resHandle == NULL)
		{
			DWORD err = GetLastError();
			DarkEdif::MsgBox::Error(_T("Resource missing"), _T("UC tagging temporary failure %u. Try running your Fusion as admin."), err);
			wchar_t keyError[64];
			swprintf_s(keyError, std::size(keyError), L"Error %u tagging.", err);
			resKey = keyError;
		}
	}
	// Use RAII to auto-close the opened resource writing when this update thread exits
	auto CloseHandle = [](HANDLE *h) { EndUpdateResource(*h, TRUE); *h = NULL; return; };
	std::unique_ptr<HANDLE, std::function<void(HANDLE*)>> resHandleHolder(&resHandle, CloseHandle);

	// Users have access to registry of local machine, albeit read-only, so this should not fail.
	// Note: we always use the wide version of registry as it's always stored as UTF-16, and we
	// don't want half-baked ANSI conversions.
	if (RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Clickteam", &mainKey) != ERROR_SUCCESS)
	{
		DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading registration resource for reading."), GetLastError());
		FreeLibrary(readHandle);
		std::abort();
	}

	// Users have access to registry of local machine, albeit read-only, so this should not fail.
	regKey.resize(256);
	DWORD keySize = regKey.size() * sizeof(wchar_t), type;
	DWORD ret = RegQueryValueExW(mainKey, L"UCTag", NULL, &type, (LPBYTE)regKey.data(), &keySize);
	if (ret == ERROR_SUCCESS)
	{
		regKey.resize(keySize / sizeof(wchar_t), L'?');
		// Null terminator is usually included in the value, remove it if so
		if (regKey[regKey.size() - 1] == L'\0')
			regKey.resize(regKey.size() - 1);
	}
	else if (ret != ERROR_FILE_NOT_FOUND)
	{
		DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading registration resource for reading."), GetLastError());
		RegCloseKey(mainKey);
		FreeLibrary(readHandle);
		std::abort();
	}
	else
		regKey = UC_TAG_NEW_SETUP;

#else // !USE_DARKEDIF_UC_TAGGING
	std::wstring resKey = L"disabled"s, regKey = L"disabled"s;
#endif

	try {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			GetLockSetConnectErrorAnd(
				updateLog << "WSAStartup failed.\n"sv);
			return 1;
		}
		SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Socket == INVALID_SOCKET)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "socket() failed. Error "sv << WSAGetLastError() << ".\n"sv);
			WSACleanup();
			return 1;
		}

		// If this returns true, updateLock is still held.
		const auto handleWSAError = [&](int lastWSAError) {
			// Get lock; caller will release it at end of app
			while (updateLock.exchange(true))
				;

			// Overloading with details is unnecessary - empty the updateLog
			updateLog.str(std::string());
			updateLog.clear();

			bool reportError = true;
			// This machine doesn't have internet, so DNS lookup of nossl.dark-wire.com failed
			// Only occurs in gethostbyname()
			if (lastWSAError == WSANO_DATA) {
				reportError = false;
				updateLog << "The DNS provider couldn't find the update server; is your computer offline?\n"sv;
			}
			// Server machine is online, but refusing connection - e.g. http service is offline
			else if (lastWSAError == WSAECONNREFUSED) {
				reportError = false;
				updateLog << "The Darkwire update server is online, but not servicing HTTP requests.\n"sv;
			}
			// Server machine is offline
			else if (lastWSAError == WSAETIMEDOUT) {
				reportError = false;
				updateLog << "The Darkwire update server is offline.\n"sv;
			}
			// lastWSA is only set to this if the page content has an error
			else if (lastWSAError == EINVAL) {
				reportError = false;
				updateLog << "Page's response was unreadable.\n"sv;
			}

			// If it's not one of those errors, or user has set to report all errors, we report.
			reportError |= DarkEdif::GetIniSetting("ReportAllUpdateCheckErrors"sv) == "true"sv;

			// We also have to report if we can't read the resTag on a UC tagged file, or there's inconsistency.
			// We NEED internet for resolving that issue.
			#if USE_DARKEDIF_UC_TAGGING
				if (resFileExists && (resKey == UC_TAG_NEW_SETUP || regKey != resKey))
				{
					reportError = true;

					// Page content errors are none of their concern
					if (lastWSAError != EINVAL)
						updateLog << "This extension needs internet access for its initial setup.\n"sv;
				}
			#endif

			if (!reportError)
			{
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::None;
				updateLog << "Not reporting, as it's an error the user is unlikely to fix.\n"sv;
				updateLock = false;
				return false;
			}

			pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError;
			// Let caller fill updateLog with the error information
			return true;
		};

		// Used in IP lookup and the HTTP request content
		const char domain[] = "nossl.dark-wire.com";

		struct hostent * host;
		host = gethostbyname(domain);

		if (host == NULL)
		{
			if (handleWSAError(WSAGetLastError()))
			{
				updateLog << "DNS lookup of \""sv << domain << "\" failed, error "sv << WSAGetLastError() << '.';
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ConnectionError;
				updateLock = false;
			}

			closesocket(Socket);
			WSACleanup();
			return 1;
		}
		SOCKADDR_IN SockAddr = {};
		SockAddr.sin_port = htons(80);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr);
		GetLockAnd(
			updateLog << "Connecting...\n"sv);
		if (connect(Socket, (SOCKADDR *)(&SockAddr), sizeof(SockAddr)) != 0) {
			if (handleWSAError(WSAGetLastError()))
			{
				updateLog << "Connect failed, error number "sv << WSAGetLastError() << '.';
				updateLock = false;
			}
			closesocket(Socket);
			WSACleanup();
			return 1;
		}
		GetLockAnd(
			updateLog << "Connected to update server.\n"sv);
		// Host necessary so servers serving multiple domains know what domain is requested.
		// Connection: close indicates server should close connection after transfer.
		std::stringstream requestStream;
		requestStream << "GET /storage/darkedif_vercheck.php?ext="sv << url_encode(PROJECT_NAME)
			<< "&build="sv << Extension::Version << "&sdkBuild="sv << DarkEdif::SDKVersion
			<< "&projConfig="sv << projConfig
			<< "&tagRes="sv << url_encode(WideToUTF8(resKey)) << "&tagReg="sv << url_encode(WideToUTF8(regKey))
			<< " HTTP/1.1\r\nHost: "sv << domain << "\r\nConnection: close\r\n\r\n"sv;
		std::string request = requestStream.str();

		GetLockAnd(
			updateLog << "Sent update request for ext \"" PROJECT_NAME "\", encoded as \""sv << url_encode(PROJECT_NAME)
				<< "\", build "sv << Extension::Version << ", SDK build "sv << DarkEdif::SDKVersion << ", config "sv << projConfig
				<< ", tagging res \""sv << url_encode(WideToUTF8(resKey)) << "\", reg \""sv << url_encode(WideToUTF8(regKey)) << "\".\n"sv);
#ifdef _DEBUG
		GetLockAnd(
			updateLog << request.substr(0, request.find(' ', 4)) << '\n');
#endif

		if (send(Socket, request.c_str(), request.size() + 1, 0) == SOCKET_ERROR)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Send failed, error "sv << WSAGetLastError() << "."sv);
			closesocket(Socket);
			WSACleanup();
			return 1;
		}

		std::string fullPage;
		{
			// Used as buffer
			std::string pagePart(1024, '\0');
			std::stringstream page;
			int nDataLength;

			GetLockAnd(
				updateLog << "Result follows:\n"sv);
			while ((nDataLength = recv(Socket, pagePart.data(), pagePart.size(), 0)) > 0) {
				page << std::string_view(pagePart).substr(0, nDataLength);
				memset(pagePart.data(), 0, nDataLength);
			}
			if (nDataLength < 0)
			{
				if (handleWSAError(WSAGetLastError()))
				{
					updateLog << "Error with recv()."sv;
					updateLock = false;
				}
				closesocket(Socket);
				WSACleanup();
				return 1;
			}
			#ifdef _DEBUG
				GetLockAnd(
					updateLog << page.str() << "\nResult concluded.\n"sv;
					OutputDebugStringA(updateLog.str().c_str());
				);
			#endif
			closesocket(Socket);
			WSACleanup();

			// the c_str() ensures no null or beyond in string, by using a different constructor
			fullPage = page.str().c_str();
		}

		// In case there's an automatic error page with CRLF, we'll check for CRs after.
		size_t endIndex = fullPage.find_first_of("\r\n");

		if (endIndex == std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "End of first line not found. Full raw (non-http) response:\n"sv << fullPage);
			return 1;
		}

		const char expHttpHeader[] = "HTTP/1.1", expHttpOKHeader[] = "HTTP/1.1 200";
		std::string statusLine = endIndex == std::string::npos ? fullPage : fullPage.substr(0, endIndex);
		// Not a HTTP response
		if (endIndex == std::string::npos || strncmp(statusLine.c_str(), expHttpHeader, sizeof(expHttpHeader) - 1))
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Unexpected non-http response:\n"sv << statusLine);
			return 1;
		}

		// HTTP response, but it's not an OK
		if (strncmp(statusLine.c_str(), expHttpOKHeader, sizeof(expHttpOKHeader) - 1))
		{
			GetLockSetConnectErrorAnd(
				updateLog << "HTTP error "sv << statusLine.substr(sizeof(expHttpHeader) - 1));
			return 1;
		}

		// HTTP response has header, two CRLF, then result
		size_t headerStart = fullPage.find("\r\n\r\n");
		if ((headerStart = fullPage.find("\r\n\r\n")) == std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "Malformed HTTP response; end of HTTP header not found."sv);
			return 1;
		}

		// result should be LF line breaks only, ideally in UTF-8.
		std::string pageBody = fullPage.substr(headerStart + 4);
		if (pageBody.find('\r') != std::string::npos)
		{
			GetLockSetConnectErrorAnd(
				updateLog << "CR not permitted in update page response.\n"sv << pageBody);
			return 1;
		}

		if (!_strnicmp(pageBody.c_str(), "https://", sizeof("https://") - 1) || !_strnicmp(pageBody.c_str(), "http://", sizeof("http://") - 1))
		{
			pendingUpdateURL = UTF8ToWide(pageBody.substr(0, pageBody.find('\n')));
			pageBody = pageBody.substr(pendingUpdateURL.size() + 1);
		}

		GetLockAnd(
			updateLog << "Completed OK. Response:\n"sv << pageBody;
		);

		const char noUpdate[] = "None";
		const char sdkUpdate[] = "SDK Update:\n";
		const char majorUpdate[] = "Major Update:\n";
		const char minorUpdate[] = "Minor Update:\n";

		// introduced in DarkEdif SDK v4
		const char extDevUpdate[] = "Ext Dev Error:\n";
		// introduced in DarkEdif SDK v12
		const char noUpdateWithTag[] = "None:\nTag=";
		const char reinstallNeeded[] = "Reinstall Needed:\n";

		if (!_strnicmp(pageBody.c_str(), reinstallNeeded, sizeof(reinstallNeeded) - 1))
		{
			pageBody = pageBody.substr(sizeof(reinstallNeeded) - 1);
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ReinstallNeeded;
				pendingUpdateDetails = UTF8ToWide(pageBody));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), noUpdate, sizeof(noUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::None;
				pendingUpdateDetails = UTF8ToWide(pageBody));

			#if USE_DARKEDIF_UC_TAGGING
			// Pure "None" response, no tag
			if (!_stricmp(pageBody.c_str(), noUpdate))
				return 0; // Tagging disabled

			// None, but not recognised as plain None or None with tag.
			if (_strnicmp(pageBody.c_str(), noUpdateWithTag, sizeof(noUpdateWithTag) - 1))
				return DarkEdif::MsgBox::Error(_T("Update checker failure"), _T("Update checker failed to return a valid response.")), 0;

			const std::wstring providedKey = UTF8ToWide(pageBody.substr(sizeof(noUpdateWithTag) - 1));
			if (resKey == providedKey && regKey == providedKey)
				return 0; // our tag is up to date! woot!

			// No point trying to write resource key if there is no resource file
			if (resKey != providedKey && resFileExists)
			{
				if (providedKey.back() == L'\0')
					DarkEdif::MsgBox::Error(_T("?"), _T("Provided key is invalid."));

				if (resHandle == NULL)
				{
					if (GetLastError() == ERROR_ACCESS_DENIED)
						DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure %u. Try running Fusion as admin, or enabling write permissions for Users role on Fusion folder."), GetLastError());
				}
				else
				{
					// String resources are always UTF-16, stored with preceding uint16 size, no null terminator, and always in groups of 16
					const int groupSize = 16;
					std::wstring keyOut(1 + providedKey.size() + (groupSize - 1), L'\0');

					// Copy string size then string content in
					*(std::uint16_t *)&keyOut[0] = (std::uint16_t)providedKey.size();
					wmemcpy_s(&keyOut[1], keyOut.size() - 1, providedKey.data(), providedKey.size());

					if (!UpdateResource(resHandle, RT_STRING, _T("UCTAG"), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), keyOut.data(), keyOut.size() * sizeof(wchar_t)))
						DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; updating tag returned %u."), GetLastError());

					if (!EndUpdateResource(resHandle, FALSE))
					{
						if (GetLastError() == ERROR_ACCESS_DENIED)
							DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure. Try running Fusion as admin and dropping a " PROJECT_NAME " extension into frame editor again."));
						else
							DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; saving new tag returned %u."), GetLastError());
					}
					// Release the unique_ptr, we no longer need it to auto-run EndUpdateResource for us
					resHandleHolder.release();
				}
			}

			if (regKey != providedKey)
			{
				DWORD err = 0;

				// Users have access to registry of local machine, albeit read-only, so this should not fail.
				if (mainKey == NULL)
					err = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Clickteam", &mainKey);

				// We can try writing to the registry - we'll include null, as RegEdit uses it
				err = err ? err : RegSetValueExW(mainKey, L"UCTag", 0, REG_SZ, (LPBYTE)providedKey.c_str(), (providedKey.size() + 1) * sizeof(wchar_t));
				if (err != 0)
				{
					DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure; saving new tag returned %u.%s"), err,
						err == ERROR_ACCESS_DENIED ? "Try running Fusion as admin." : "");
				}
			}

			#endif

			return 0;
		};

		if (!_strnicmp(pageBody.c_str(), extDevUpdate, sizeof(extDevUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ExtDevError;
				pendingUpdateDetails = UTF8ToWide(pageBody.substr(sizeof(extDevUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), sdkUpdate, sizeof(sdkUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::SDKUpdate;
				pendingUpdateDetails = UTF8ToWide(pageBody.substr(sizeof(sdkUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), majorUpdate, sizeof(majorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Major;
				pendingUpdateDetails = UTF8ToWide(pageBody.substr(sizeof(majorUpdate) - 1)));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), minorUpdate, sizeof(minorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Minor;
				pendingUpdateDetails = UTF8ToWide(pageBody.substr(sizeof(minorUpdate) - 1)));
			return 0;
		}

		if (handleWSAError(EINVAL))
		{
			#ifdef _DEBUG
				updateLog << "Can't interpret type of page:\n"sv << pageBody;
				pendingUpdateDetails = UTF8ToWide(pageBody);
			#else
				updateLog << "You should report this to Phi on the Clickteam Discord."sv;
				pendingUpdateDetails.clear();
			#endif
			updateLock = false;
		}
		return 0;
	}
	catch (...)
	{
		GetLockAnd(
			updateLog << "Caught a crash. Aborting update."sv);
		OutputDebugStringA(updateLog.str().c_str());
		return 0;
	}
#undef GetLockAnd
#undef GetLockSetConnectErrorAnd
}

#endif // USE_DARKEDIF_UPDATE_CHECKER


#endif // EditorBuild

// Define it
std::tstring DarkEdif::ExtensionName(_T("" PROJECT_NAME ""s));
std::thread::id DarkEdif::MainThreadID;
WindowHandleType DarkEdif::Internal_WindowHandle;
DarkEdif::MFXRunMode DarkEdif::RunMode = DarkEdif::MFXRunMode::Unset;

// =====
// Message boxes that mostly work on all platforms
// On Android, this makes a small popup on bottom of the screen, called a biscuit?
// =====
static int Internal_MessageBox(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, va_list v, int flags)
{
	assert(titlePrefix != NULL && msgFormat != NULL);

	// This doesn't work on Windows XP in some scenarios; for an explanation, see the README file.
#if defined(_WIN32) && WINVER < 0x0600 && defined(__cpp_threadsafe_static_init)
	const static TCHAR projNameStatic[] = _T(" - " PROJECT_NAME);
	const std::tstring titleSuffix = projNameStatic;
#else
	const static std::tstring titleSuffix = _T(" - " PROJECT_NAME ""s);
#endif

	// Without this modal setting, the user will usually crash Fusion.
	// With non-modal, they can interact with the editor window of Fusion.
	// If you then switch editor from frame to event, then back to frame, then close the message box, the MB_OK gets sent back to Fusion's main
	// window unexpectedly, making it crash.
	//
	// Note: You can also fix the crash by passing HWND of NULL to MessageBox(), but this means it isn't put in front of Fusion, meaning that
	// users will often click too fast and hide the message behind the editor window... meaning they interact.. meaning when they close it, it crashes.
#if EditorBuild
	if (DarkEdif::RunMode == DarkEdif::MFXRunMode::Editor && DarkEdif::MainThreadID == std::this_thread::get_id() && (flags & (MB_SYSTEMMODAL | MB_TASKMODAL)) == 0)
		flags |= (MB_TASKMODAL | MB_SETFOREGROUND);
#endif

	std::tstring title = titlePrefix + titleSuffix;
	TCHAR msgData[4096];
	int numChars = _vstprintf_s(msgData, std::size(msgData), msgFormat, v);
	if (numChars <= 0)
	{
		MessageBox(DarkEdif::Internal_WindowHandle, _T("Failed to format a message box."), title.c_str(), MB_OK | MB_ICONERROR);
		DarkEdif::BreakIfDebuggerAttached();
		return IDCANCEL;
	}
	return MessageBox(DarkEdif::Internal_WindowHandle, msgData, title.c_str(), flags);
}
void DarkEdif::MsgBox::WarningOK(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONWARNING);
	va_end(v);
}
int DarkEdif::MsgBox::WarningYesNo(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, MB_YESNO | MB_ICONWARNING);
	va_end(v);
	return ret;
}
int DarkEdif::MsgBox::WarningYesNoCancel(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, MB_YESNOCANCEL | MB_ICONWARNING);
	va_end(v);
	return ret;
}
void DarkEdif::MsgBox::Error(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONERROR);
	va_end(v);
}
void DarkEdif::MsgBox::Info(const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	Internal_MessageBox(titlePrefix, msgFormat, v, MB_OK | MB_ICONINFORMATION);
	va_end(v);
}
int DarkEdif::MsgBox::Custom(const int flags, const TCHAR * titlePrefix, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	int ret = Internal_MessageBox(titlePrefix, msgFormat, v, flags);
	va_end(v);
	return ret;
}
void DarkEdif::Log(int logLevel, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
#ifdef _WIN32
	static TCHAR outputBuff[1024];
	_vstprintf_s(outputBuff, msgFormat, v);
	OutputDebugString(outputBuff);
#elif defined(__ANDROID__)
	__android_log_vprint(logLevel, PROJECT_NAME_UNDERSCORES, msgFormat, v);
#else // iOS
	vprintf(msgFormat, v);
#endif
	va_end(v);
}

#if defined(__ANDROID__) || defined(__APPLE__)

#if DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO
void OutputDebugStringA(const char * debugString)
{
	assert(debugString != NULL);
	// We can't get the user to remove their newlines, as Windows doesn't automatically add them in OutputDebugStringA(),
	// but __android_log_print includes automatic newlines, so strip them.
	std::string debugStringSafe(debugString);
#ifdef __ANDROID__
	if (debugStringSafe.back() == '\n')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
	if (debugStringSafe.back() == '\r')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
	if (debugStringSafe.back() == '.')
		debugStringSafe.resize(debugStringSafe.size() - 1U);
#endif

	LOGI("OutputDebugStringA: %s.", debugStringSafe.c_str());
}
#endif // DarkEdif log level INFO or higher
#endif // __ANDROID__

#ifndef _WIN32
// To get the Windows-like behaviour
void Sleep(unsigned int milliseconds)
{
	if (milliseconds == 0)
		std::this_thread::yield();
	else
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif

// Causes the produced extension to include DarkExt.PostMinify.json.
// Hat tip: https://stackoverflow.com/a/4910421
// Also note https://github.com/graphitemaster/incbin/blob/master/incbin.h

#ifdef __ANDROID__
__asm__(".section .rodata				\n\
	.global darkExtJSON					\n\
	.type   darkExtJSON, %object		\n\
	.align  4							\n\
darkExtJSON:							\n\
	.incbin \"DarkExt.PostMinify.json\"	\n\
darkExtJSON_end:						\n\
	.global darkExtJSONSize				\n\
	.type   darkExtJSONSize, %object	\n\
	.align  4							\n\
darkExtJSONSize:						\n\
	.int	darkExtJSON_end - darkExtJSON");
#elif defined(__APPLE__)
/**
 * @file incbin.h
 * @author Dale Weiler
 * @brief Utility for including binary files
 *
 * Facilities for including binary files into the current translation unit and
 * making use from them externally in other translation units.
 */

INCBIN(darkExtJSON, "DarkExt.PostMinify.json");

// See https://stackoverflow.com/a/19725269
// Note the file will NOT be transmitted to Mac unless it's set as a C/C++ header file.
/*__asm__(".const_data					\n\
	.global darkExtJSON					\n\
	.align  4							\n\
darkExtJSON:							\n\
	.incbin \"DarkExt.PostMinify.json\"	\n\
__asm__("darkExtJSON_end:						\n\
	.global darkExtJSONSize				\n\
	.align  4							\n\
darkExtJSONSize:						\n\
	.int	darkExtJSON_end - darkExtJSON");*/

#endif
// These are caused by the above ASM block. (these are also declared in the Android/iOS master header)
// char darkExtJSON[];
// unsigned darkExtJSONSize;
