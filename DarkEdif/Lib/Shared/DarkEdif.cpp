#include "Common.hpp"
#include <atomic>
#include "DarkEdif.hpp"
#include <math.h>
#include "Extension.hpp"

#ifdef _WIN32
extern HINSTANCE hInstLib;
#endif
namespace Edif {
	class SDK;
	extern class SDK * SDK;
}

#if EditorBuild

static const _json_value * StoredCurrentLanguage = &json_value_none;
static std::tstring StoredCurrentLanguageName = _T("<default>"s);

static const _json_value * DefaultLanguageIndex()
{
	// Misuse of function; called before ::SDK is valid
	if (!Edif::SDK)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif error"), _T("Premature function call!\n  Called DefaultLanguageIndex() before ::SDK was a valid pointer."));
		return &json_value_none;
	}

	for (unsigned int i = 0; i < Edif::SDK->json.u.object.length; ++i)
	{
		auto& lang = Edif::SDK->json.u.object.values[i];
		if ((*lang.value).type == json_object
			&& (*lang.value)["About"]["Name"].type == json_string)
		{
			StoredCurrentLanguageName = DarkEdif::UTF8ToTString(lang.name);
			return lang.value;
		}
	}

	// Fallback on last object
	auto& lang = Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1];
	StoredCurrentLanguageName = DarkEdif::UTF8ToTString(lang.name);
	return lang.value;
}

const json_value & DarkEdif::JSON::LanguageJSON()
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
		for (unsigned int i = 0; i < Edif::SDK->json.u.object.length; ++i)
		{
			// Return index
			if ((*Edif::SDK->json.u.object.values[i].value).type == json_object &&
				!_stricmp(Edif::SDK->json.u.object.values[i].name, langItem.c_str()))
			{
				StoredCurrentLanguage = Edif::SDK->json.u.object.values[i].value;
				StoredCurrentLanguageName = DarkEdif::UTF8ToTString(Edif::SDK->json.u.object.values[i].name);
				return *StoredCurrentLanguage;
			}
		}
		readIndex += langItem.size()+1;
		if (langList.at(readIndex-1) == '\r')
			break;
	}

	return *DefaultLanguageIndex();
}
const TCHAR* const DarkEdif::JSON::LanguageName()
{
	return StoredCurrentLanguageName.c_str();
}
#else // not EditorBuild
#define CurLang (*Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].value)

// In runtime, only one langauge is included, so we'll use a single string to contain it
static std::tstring jsonLangName;
const TCHAR* const DarkEdif::JSON::LanguageName()
{
	if (jsonLangName.empty())
		jsonLangName = UTF8ToTString(Edif::SDK->json.u.object.values[Edif::SDK->json.u.object.length - 1].name);
	return jsonLangName.c_str();
}

#endif // EditorBuild

ACEInfo * Edif::ACEInfoAlloc(unsigned int NumParams)
{
	// Allocate space for ACEInfo struct, plus Parameter[NumParams] so it has valid memory
	return (ACEInfo *)calloc(sizeof(ACEInfo) + (NumParams * sizeof(short) * 2), 1);	// The *2 is for reserved variables
}
ExpReturnType Edif::ReadExpressionReturnType(const char * Text);

bool Edif::CreateNewActionInfo()
{
	// Get ID and thus properties by counting currently existing actions.
	const json_value & UnlinkedAction = CurLang["Actions"][(std::int32_t)Edif::SDK->ActionInfos.size()];

	// Invalid JSON reference
	if (UnlinkedAction.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Invalid JSON reference for action ID %zu, expected object."), Edif::SDK->ActionInfos.size()), false;

	const json_value & Param = UnlinkedAction["Parameters"];

	// Num of parameters is beyond number of bits in FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading action JSON"), _T("Too many parameters in action ID %zu."), Edif::SDK->ActionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ActInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ActInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating action info"), _T("Could not allocate memory for action ID %zu return."), Edif::SDK->ActionInfos.size()), false;

	ActInfo->ID = (short)Edif::SDK->ActionInfos.size();
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
	Edif::SDK->ActionInfos.push_back(ActInfo);
	return true;
}

bool Edif::CreateNewConditionInfo()
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Condition = CurLang["Conditions"][(std::int32_t)Edif::SDK->ConditionInfos.size()];

	// Invalid JSON reference
	if (Condition.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Invalid JSON reference for condition ID %zu, expected a JSON object."), Edif::SDK->ConditionInfos.size()), false;

	const json_value & Param = Condition["Parameters"];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags) *CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading condition JSON"), _T("Too many parameters in condition ID %zu."), Edif::SDK->ConditionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * CondInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!CondInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating condition info"), _T("Could not allocate memory for condition ID %zu return."), Edif::SDK->ConditionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	CondInfo->ID = (short)Edif::SDK->ConditionInfos.size();
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
	Edif::SDK->ConditionInfos.push_back(CondInfo);
	return true;
}

bool Edif::CreateNewExpressionInfo()
{
	// Get ID and thus properties by counting currently existing conditions.
	const json_value & Expression = CurLang["Expressions"][(std::int32_t)Edif::SDK->ExpressionInfos.size()];

	// Invalid JSON reference
	if (Expression.type != json_object)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Invalid JSON reference for expression ID %zu, expected a JSON object."), Edif::SDK->ExpressionInfos.size()), false;

	const json_value & Param = Expression["Parameters"];

	// Num of parameters is beyond size of FloatFlags
	if (sizeof(ACEInfo::FloatFlags)*CHAR_BIT < Param.u.object.length)
		return DarkEdif::MsgBox::Error(_T("Error reading expression JSON"), _T("Too many JSON parameters in expression ID %zu."), Edif::SDK->ExpressionInfos.size()), false;

	// Parameters checked; allocate new info
	ACEInfo * ExpInfo = ACEInfoAlloc(Param.u.object.length);

	// Could not allocate memory
	if (!ExpInfo)
		return DarkEdif::MsgBox::Error(_T("Error creating expression info"), _T("Could not allocate memory for expression ID %zu return."), Edif::SDK->ExpressionInfos.size()), false;

	// If a non-triggered condition, set the correct flags
	ExpInfo->ID = (short)Edif::SDK->ExpressionInfos.size();
	ExpInfo->NumOfParams = Param.u.object.length;
	ExpInfo->Flags.ef = Edif::ReadExpressionReturnType(Expression["Returns"]);

	if (ExpInfo->NumOfParams > 0)
	{
		// Set up each parameter
		bool IsFloat;
		for (std::uint8_t c = 0; c < ExpInfo->NumOfParams; ++c)
		{
			IsFloat = false;
			ExpInfo->Parameter[c].ep = Edif::ReadExpressionParameterType(Param[c][0], IsFloat);	// Store parameter type
			ExpInfo->FloatFlags |= (IsFloat << c);											// Store whether it is a float or not with a single bit
		}

		// For some reason in Edif an extra short is provided, initialized to 0, so duplicate that
		memset(&ExpInfo->Parameter[ExpInfo->NumOfParams], 0, ExpInfo->NumOfParams * sizeof(short));
	}

	// Add to table
	Edif::SDK->ExpressionInfos.push_back(ExpInfo);
	return true;
}

using namespace DarkEdif;

#ifdef NOPROPS
namespace DarkEdif { namespace Properties { struct Data; static constexpr bool DebugProperties = false; } }
#endif

static void DebugProp_OutputString(PrintFHintInside const TCHAR* msgFormat, ...)
{
	if constexpr (!DarkEdif::Properties::DebugProperties)
		return;
	va_list v;
	va_start(v, msgFormat);
	DarkEdif::LogV(DARKEDIF_LOG_INFO, msgFormat, v);
	va_end(v);
}

// Any type that has no changeable value (excludes checkbox)
static bool IsUnchangeablePropExclCheckbox(const char* curStr)
{
	return !_stricmp(curStr, "Text") || !_strnicmp(curStr, "Folder", sizeof("Folder") - 1)
		|| !_stricmp(curStr, "Edit button") || !_stricmp(curStr, "Group");
}
// Any type that has no changeable value (includes checkbox, JSON statics, and buttons)
static bool IsUnchangeablePropInclCheckbox(const char* curStr)
{
	return !_stricmp(curStr, "Checkbox") || IsUnchangeablePropExclCheckbox(curStr);
}

#pragma pack(push, 1)
struct DarkEdif::Properties::Data
{
	// Size of property's data relative from start pos in dataForProps.
	std::uint32_t sizeBytes; // for jumping past in scrolling; includes this entire Data
	// Property type ID - not Unicode incremented!
	std::uint16_t propTypeID;

	// Data includes:
	// 1) Prop name as retrieved from JSON, but in lowercase for faster string compare.
	// 2) Actual data for property (prop value)
	std::uint8_t propNameSize;
	std::uint8_t data[];

	inline std::string ReadPropName() const {
		// Must be nicely padded to 1 byte, or there be shenanigans
		static_assert((sizeof(Data) - sizeof(sizeBytes) - sizeof(propTypeID) - sizeof(propNameSize)) == 0, "data[] is not 0 bytes");

		std::string str((const char*)data, (size_t)propNameSize);
		DebugProp_OutputString(_T("ReadPropName result: type %hu, %s.\n"), propTypeID, UTF8ToTString(str).c_str());
		return str;
	}

	inline const std::uint8_t* const ReadPropValue() const {
		return data + propNameSize;
	}
	inline std::uint8_t* ReadPropValue() {
		return data + propNameSize;
	}
	inline size_t ReadPropValueSize() const {
		return sizeBytes - sizeof(Data) - propNameSize;
	}

	Data* Next() const {
		return (Data*)(((char*)this) + sizeBytes);
	}
};
#pragma pack(pop)
#if EditorBuild

// Hash function; we can't use std::hash as it returns size_t, which varies in size per platform
static std::uint32_t fnv1a(std::string_view const& text)
{
	// 32 bit params
	constexpr std::uint32_t fnv_prime = 16777619U;
	constexpr std::uint32_t fnv_offset_basis = 2166136261U;

	const std::basic_string_view<std::uint8_t> text2((std::uint8_t*)text.data(), text.size());
	std::uint32_t hash = fnv_offset_basis;
	for (const std::uint8_t& c : text2) {
		hash ^= c;
		hash *= fnv_prime;
	}

	return hash;
}
#endif // EditorBuild

#ifndef NOPROPS

#if defined(_WIN32) && EditorBuild

static size_t GetPropRealID(size_t fromFusion)
{
	if (fromFusion < PROPID_EXTITEM_CUSTOM_FIRST)
		return std::numeric_limits<size_t>::max();

	// Ext properties start from 0x80000, PROPID_EXTITEM_CUSTOM_FIRST
	return fromFusion - PROPID_EXTITEM_CUSTOM_FIRST;
}
static DarkEdif::Properties & GetRealPropsAddress(EDITDATA * edPtr)
{
	size_t propOffset = edPtr->eHeader.extPrivateData;
	if (propOffset < sizeof(edPtr->eHeader))
	{
		DarkEdif::MsgBox::WarningOK(_T("Old DarkEdif props"), _T("No offset found. Falling back to default. Resetting properties recommended."));
		propOffset = ((std::uint8_t *)&edPtr->Props) - ((std::uint8_t *)edPtr);
	}
	if (propOffset > edPtr->eHeader.extSize - sizeof(DarkEdif::Properties))
	{
		DarkEdif::MsgBox::WarningOK(_T("DarkEdif offset invalid"), _T("Offset is too high!"));
		propOffset = ((std::uint8_t*)&edPtr->Props) - ((std::uint8_t*)edPtr);
	}
	return *(DarkEdif::Properties *)(((std::uint8_t *)edPtr) + propOffset);
}


namespace DarkEdif {
	namespace DLL
	{
		struct ConverterReturnAccessor
		{
			Properties::ConvReturnType convRetType;
			const void * data;
			size_t dataSize;
			int checkboxState; // 0, 1, or -1 if unset

			// If non-null, a pointer to a freeing function.
			void (*freeData)(const void *);
		};
	}
}

// Return a value with optional deleter, and optional checkbox state
void Properties::ConverterReturn::Return_OK(const void * data, size_t dataSize,
	void (*freeData)(const void *) /* = nullptr */, int checkboxState /* = -1 */)
{
	this->data = data;
	this->dataSize = dataSize;
	this->freeData = freeData;
	this->checkboxState = checkboxState;

	if (checkboxState < -1 || checkboxState > 1)
		MsgBox::Error(_T("Checkbox error"), _T("Checkbox state of %i is invalid."), checkboxState);

	convRetType = ConvReturnType::OK;

	// This happens when upgrading from an old smart property version that didn't store any data.
	// Can't just modify GetPropValue() because it might be used to get offset to write in Properties.
	if (data && dataSize == 0)
		data = nullptr;
}

// Failed to read, but it should be okay to use converter after
// Passing to next level of converter is requested
void Properties::ConverterReturn::Return_Pass()
{
	convRetType = ConvReturnType::Pass;
	//
}

// Was okay to return it, but had a third-party error
// Report to user
static TCHAR outputBuff[2048];
void Properties::ConverterReturn::Return_Error(PrintFHintInside const TCHAR * err, ...)
{
	convRetType = ConvReturnType::Error;
	va_list v;
	va_start(v, err);
	assert(_vsntprintf_s(outputBuff, std::size(outputBuff), (size_t)-1, err, v) > 0);
	va_end(v);
	data = outputBuff;
	DebugProp_OutputString(_T("%s"), outputBuff);
}

// The source data doesn't have this, and clearly the source is unreliable
// This converter should no longer be used
void Properties::ConverterReturn::Return_ConverterUnsuitable()
{
	convRetType = ConvReturnType::ConverterUnsuitable;
}

Properties::ConverterReturn::ConverterReturn()
{
	this->data = nullptr;
	this->dataSize = 0;
	this->freeData = nullptr;
	this->checkboxState = -1;
	this->convRetType = ConvReturnType::Unset;
}
#endif // _WIN32

DarkEdif::DLL::PropAccesser & Elevate(const DarkEdif::Properties &p)
{
	return *(DarkEdif::DLL::PropAccesser *)&p;
}

// Reads DarkEdif::Properties without the fanfare.
struct DarkEdif::DLL::PropAccesser
{
	NO_DEFAULT_CTORS_OR_DTORS(PropAccesser);
	// Type of DarKEdif::Properties
	decltype(Properties::propVersion) propVersion;

	// Used to read JSON properties to see if a change has been made.
	decltype(Properties::hash) hash; // fnv1a hash of property titles and types (with NOT applied)
	decltype(Properties::hashTypes) hashTypes; // fnv1a hash of property types only (with NOT applied)
	// Number of properties
	decltype(Properties::numProps) numProps;
	// VS decided to pad this struct, so let's continue the idiocy officially
	decltype(Properties::pad) pad;
	// Size of DataForProps - including EDITDATA (and thus EDITDATA::Properties)
	decltype(Properties::sizeBytes) sizeBytes;
	// The actual data for properties, merged together
	// Starts with checkboxes, then data, which is Data struct: type ID followed by binary.
	decltype(DarkEdif::Properties::dataForProps) dataForProps; /* [], inherited from decltype*/;

	// Note: There is a single bit for each checkbox.
	// Use numProps / 8 for num of bytes used by checkboxes.
	static_assert(std::is_same<decltype(DarkEdif::DLL::PropAccesser::dataForProps), decltype(DarkEdif::Properties::dataForProps)>::value, "Invalid type");

	const DarkEdif::Properties::Data * Internal_FirstData() const
	{
		return ((DarkEdif::Properties *)this)->Internal_FirstData();
	}
	const DarkEdif::Properties::Data * Internal_DataAt(unsigned int ID) const
	{
		return ((DarkEdif::Properties *)this)->Internal_DataAt(ID);
	}
#if EditorBuild
	DarkEdif::Properties::Data* Internal_FirstData()
	{
		return ((DarkEdif::Properties*)this)->Internal_FirstData();
	}
	DarkEdif::Properties::Data * Internal_DataAt(unsigned int ID)
	{
		return ((DarkEdif::Properties *)this)->Internal_DataAt(ID);
	}

	Prop * GetProperty(size_t ID)
	{
		return ((DarkEdif::Properties *)this)->GetProperty(ID);
	}
	static void Internal_PropChange(mv * mV, EDITDATA * &edPtr, unsigned int PropID, const void * newData, size_t newSize)
	{
		DarkEdif::Properties::Internal_PropChange(mV, edPtr, PropID, newData, newSize);
	}
#endif
};

#if EditorBuild

void DarkEdif::Properties::Internal_PropChange(mv* mV, EDITDATA*& edPtr, unsigned int PropID,
	const void* newPropValue, size_t newPropValueSize)
{
	const char* curTypeStr = CurLang["Properties"][PropID]["Type"];
	Data* oldPropValue = edPtr->Props.Internal_DataAt(PropID);
	if (!oldPropValue)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("Couldn't find property %s (type %hs), failed to edit it."),
			UTF8ToTString((const char*)CurLang["Properties"][PropID]["Title"]).c_str(), curTypeStr);
		return;
	}
	bool rearrangementRequired = false;
	const size_t oldPropValueSize = oldPropValue->ReadPropValueSize();

	// Fusion sends combo box changes as UInts, item index, but we store them as the item text.
	if (oldPropValue->propTypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOX ||
		oldPropValue->propTypeID == Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN)
	{
		assert(newPropValueSize == sizeof(unsigned int));
		const std::string_view newValue = (const char*)CurLang["Properties"][PropID]["Items"][*(unsigned int*)newPropValue];
		newPropValue = newValue.data(); // string_view doesn't copy underlying memory, so this is safe
		newPropValueSize = newValue.size();
	}

	if (newPropValueSize == oldPropValueSize)
	{
		// The Data is unchangeable, but the container EDITDATA isn't, so really the Data is changeable.
		memcpy(oldPropValue->ReadPropValue(), newPropValue, newPropValueSize);
		return;
	}

	// Even an empty string should be 1 (null char). Warn if not.
	if (newPropValueSize == 0)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("Property size of property %s (type %hs) is 0!"),
			UTF8ToTString((const char*)CurLang["Properties"][PropID]["Title"]).c_str(), curTypeStr);
	}

	// Shouldn't happen unless user starts messing.
	if (edPtr->eHeader.extSize != edPtr->Props.sizeBytes)
	{
		if (MsgBox::WarningYesNo(_T("DarkEdif property warning"), _T("Property block size is different to actual size. "
			"Press Yes to set it to Fusion's size (may corrupt properties).\n"
			"Press No to cancel property change."),
			UTF8ToTString((const char*)CurLang["Properties"][PropID]["Title"]).c_str(), curTypeStr) == IDYES)
		{
			edPtr->Props.sizeBytes = edPtr->eHeader.extSize;
		}
		else
			return;
	}

	// Where P is value of data. Note the metadata of Properties::Data is included in beforeOldSize.
	const size_t beforeOldSize = oldPropValue->ReadPropValue() - ((std::uint8_t*)edPtr);	// Pointer to O|<P|O
	const size_t afterOldSize = edPtr->Props.sizeBytes - beforeOldSize - oldPropValueSize;	// Pointer to O|P>|O

	// Duplicate memory to another buffer (if new arragement is smaller - we can't just copy from old buffer after realloc)
	const size_t newEdPtrSize = edPtr->Props.sizeBytes - oldPropValueSize + newPropValueSize;
	std::unique_ptr<std::uint8_t[]> newEdPtr = std::make_unique<std::uint8_t[]>(newEdPtrSize);

	if (!newEdPtr)
	{
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("Out of memory attempting to rewrite properties!"));
		return;
	}

	if (newEdPtrSize > UINT16_MAX)
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("Need %zu bytes, but EDITDATA can only store %u bytes!"), newEdPtrSize, UINT16_MAX);

	// Copy the part before new data into new address
	memcpy(newEdPtr.get(), edPtr, beforeOldSize);

	// Copy the new data into new address
	memcpy(newEdPtr.get() + beforeOldSize, newPropValue, newPropValueSize);

	// Copy the part after new data into new address
	memcpy(newEdPtr.get() + beforeOldSize + newPropValueSize,
		oldPropValue->ReadPropValue() + oldPropValueSize,
		afterOldSize);

	// Update EDITDATA size
	((EDITDATA*)newEdPtr.get())->Props.sizeBytes = newEdPtrSize;

	// Update the new property's sizeBytes.
	// To get newEdPtr -> sizeBytes, we can re-use the offset of oldEdPtr -> prop::sizeBytes,
	// as obviously no properties before it in the queue were shifted, which is faster than using Internal_DataAt which jumps through Datas.
	// ((EDITDATA *)newEdPtr.get())->Props.Internal_DataAt(PropID)->sizeBytes = VV;
	*(std::uint32_t*)(newEdPtr.get() + (((std::uint8_t*)&oldPropValue->sizeBytes) - (std::uint8_t*)edPtr))
		= (std::uint32_t)(sizeof(DarkEdif::Properties::Data) + oldPropValue->propNameSize + newPropValueSize);

	static_assert(std::is_same_v<decltype(((EDITDATA*)newEdPtr.get())->Props.Internal_DataAt(PropID)->sizeBytes), std::uint32_t>, "Types invalid");

	// Reallocate edPtr
	EDITDATA* fusionNewEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, newEdPtrSize);
	if (!fusionNewEdPtr)
	{
		DarkEdif::MsgBox::Error(_T("Property Error"), _T("NULL returned from EDITDATA reallocation. Property changed cancelled."));
		return;
	}

	// Copy into edPtr (copy everything after eHeader, leave eHeader alone)
	// eHeader::extSize and such will be changed by Fusion by mvReAllocEditData,
	// so should not be considered ours to interact with
	memcpy(&GetRealPropsAddress(fusionNewEdPtr),
		&GetRealPropsAddress((EDITDATA*)newEdPtr.get()),
		newEdPtrSize - ((EDITDATA*)newEdPtr.get())->eHeader.extPrivateData);

	edPtr = fusionNewEdPtr; // Inform caller of new address
}


LPARAM DarkEdif::DLL::DLL_GetPropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	// CreateParams are not used by DarkEdif currently. These dummy functions are here so
	// General CPP files won't need replacing if DarkEdif does start using them.
	// Note that CreateParam is called when PROPOPT_PARAMREQUIRED is specified, but NULL is passed as the parameter.
	return NULL;
}
void DarkEdif::DLL::DLL_ReleasePropCreateParam(mv * mV, EDITDATA * edPtr, unsigned int PropID, LPARAM lParam)
{
	// do nothing
}

int DarkEdif::DLL::DLL_CreateObject(mv * mV, LevelObject * loPtr, EDITDATA * edPtr)
{
	// To make it easier, we delegate to UpdateEditStructure. That returns a HGLOBAL, so we'll pull the original pointer back out.
	HGLOBAL gblPtr = DLL_UpdateEditStructure(mV, nullptr);
	if (gblPtr == NULL)
		return -1;

	EDITDATA * gblEdPtr = (EDITDATA *)GlobalLock(gblPtr);
	if (gblEdPtr == NULL)
	{
		GlobalFree(gblPtr);
		return -1;
	}

	// The one thing gbl can't have set yet, because edPtr wasn't passed.
	// WARNING: If this is not set properly, Fusion will not display object properties correctly!
	gblEdPtr->eHeader.extID = edPtr->eHeader.extID;

	// Do we need more size now? We shouldn't, but check.
	size_t numBytes = Elevate(gblEdPtr->Props).sizeBytes;
	EDITDATA* fusionEdPtr = edPtr;
	errno_t err = 0;

	if (numBytes != edPtr->eHeader.extSize)
	{
		MsgBox::Error(_T("EDITDATA size mismatch"), _T("Fusion provided an EDITDATA of %zu bytes, but GetEDITDATASizeFromJSON calculation was %hu bytes expected.\nThis is a DarkEdif bug!"),
			edPtr->eHeader.extSize, Internal_GetEDITDATASizeFromJSON());
		fusionEdPtr = (EDITDATA*)mvReAllocEditData(mV, edPtr, numBytes);

		if (fusionEdPtr == NULL)
			err = ENOMEM;
	}
	else
	{
		// Copy out any extra EDITDATA variables between eHeader and Props
		size_t extraDataSize = ((char*)&edPtr->Props) - (((char*)&edPtr->eHeader) + sizeof(edPtr->eHeader));
		char* extraData = NULL;
		if (extraDataSize > 0)
		{
			extraData = (char*)malloc(extraDataSize);
			err = extraData ? memcpy_s(extraData, extraDataSize, ((char*)&edPtr->eHeader) + sizeof(edPtr->eHeader), extraDataSize) : ENOMEM;
		}

		if (err == 0)
			err = memcpy_s(fusionEdPtr, fusionEdPtr->eHeader.extSize, gblEdPtr, numBytes);

		// Copy back extra EDITDATA variables
		if (err == 0 && extraData)
		{
			err = memcpy_s(((char *)fusionEdPtr)+sizeof(fusionEdPtr->eHeader),
				fusionEdPtr->eHeader.extSize - sizeof(fusionEdPtr->eHeader),
				extraData, extraDataSize);
			free(extraData);
		}
	}
	GlobalUnlock(gblPtr);
	GlobalFree(gblPtr);

	if (err)
	{
		MsgBox::Error(_T("Initialisation failure"), _T("Initialising extension failed: error %d."), err);
		return -1;
	}

	return 0;
}

BOOL DarkEdif::DLL::DLL_GetProperties(mv * mV, EDITDATA * edPtr, bool masterItem)
{
	DebugProp_OutputString(_T("Call to GetProperties with edPtr version %u.\n"), edPtr->eHeader.extVersion);

	if (Edif::SDK->EdittimeProperties == nullptr)
		MsgBox::Error(_T("Property error"), _T("Call to DLL_GetProperties without valid EdittimeProperties."));

	mvInsertProps(mV, edPtr, Edif::SDK->EdittimeProperties.get(), PROPID_TAB_GENERAL, TRUE);
	mvInvalidateObject(mV, edPtr);
	return TRUE; // OK
}
void DarkEdif::DLL::DLL_ReleaseProperties(mv * mV, EDITDATA * edPtr, BOOL bMasterItem)
{
	// do nothing because SDK is static and properties are re-used
}

Prop * DarkEdif::Properties::GetProperty(size_t IDParam)
{
	size_t ID = GetPropRealID(IDParam);

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= ID)
		return nullptr;

	// Premature call
	if (sizeBytes == 0)
	{
		MsgBox::Error(_T("Premature function call"), _T("GetProperty() called without edPtr->DarkEdif::Properties being valid."));
		return nullptr;
	}

	const json_value &propJSON = CurLang["Properties"][ID];
	const char * curStr = propJSON["Type"];
	Prop * ret = nullptr;
	bool allConv;

	// Unchanging contents, so read the text from JSON.
	if (!_stricmp(curStr, "Text") || !_stricmp(curStr, "Edit button"))
	{
		ret = new Prop_Str(UTF8ToTString((const char *)propJSON["DefaultState"], &allConv).c_str());
		if (!allConv)
		{
			MsgBox::WarningOK(_T("DarkEdif property error"), _T("Warning: The property's Unicode string \"%hs\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."), (const char *)propJSON["DefaultState"]);
		}
		return ret;
	}

	// Unchanging contents, null is fine for data.
	if (IsUnchangeablePropInclCheckbox(curStr))
		return nullptr;

	const Data * Current = Internal_DataAt(ID);
	if (!Current)
	{
		MsgBox::WarningOK(_T("DarkEdif property error"), _T("Warning: Returned null Data. Properties are corrupt; "
			"replace the existing " PROJECT_NAME ", by creating a new one and using \"Replace by another object\" in event editor."));
		return nullptr;
	}

	if (!_strnicmp(curStr, "Editbox String", sizeof("Editbox String") - 1) ||
		!_stricmp(curStr, "Editbox File") || !_stricmp(curStr, "Editbox Folder"))
	{
		ret = new Prop_Str(UTF8ToTString(std::string_view((const char *)Current->ReadPropValue(), Current->ReadPropValueSize()), &allConv).c_str());
		if (!allConv)
		{
			MsgBox::WarningOK(_T("DarkEdif property error"), _T("GetProperty warning: The property's Unicode string \"%hs\" couldn't be converted to ANSI. "
				"Characters will be replaced with filler."), std::string((const char*)Current->ReadPropValue(), Current->ReadPropValueSize()).c_str());
		}
	}
	else if (!_stricmp(curStr, "Editbox Number") || !_stricmp(curStr, "Edit Spin") || !_stricmp(curStr, "Edit Slider") || !_stricmp(curStr, "Color") || !_stricmp(curStr, "Edit direction"))
		ret = new Prop_SInt(*(const int *)Current->ReadPropValue());
	else if (!_stricmp(curStr, "Editbox Float") || !_stricmp(curStr, "Edit spin float")) {
		ret = new Prop_Float(*(const float *)Current->ReadPropValue());
	}
	else if (!_strnicmp(curStr, "Combo Box", sizeof("Combo Box") - 1))
	{
		// Combo box is stored as its item text, so items can be altered between versions.
		// Fusion expects the returned property to be the index of the item in combo box, so we have to do a lookup.
		std::string str((const char *)Current->ReadPropValue(), Current->ReadPropValueSize());
		const json_value & propJSONItems = propJSON["Items"];
		if (propJSONItems.type != json_array)
		{
			MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON Items not valid in Combo Box property \"%s\", ID %i, language %s."),
				ID, (const char *)propJSON["Title"], DarkEdif::JSON::LanguageName());
			return nullptr;
		}
		size_t itemIndex = -1;

		for (size_t i = 0; i < propJSON["Items"].u.array.length; i++)
		{
			assert(propJSONItems[i].type == json_string);
			if (str == (const char *)propJSONItems[i])
			{
				itemIndex = i;
				break;
			}
		}
		if (itemIndex == -1)
		{
			MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON Items does not contain given value %s (Combo Box property \"%s\", ID %i, JSON language %s)."),
				UTF8ToTString(str).c_str(), (const char *)propJSON["Title"], ID, DarkEdif::JSON::LanguageName());
			return nullptr;
		}
		ret = new Prop_UInt(itemIndex);
	}
	else
		MsgBox::Error(_T("Property error"), _T("GetProperty error: JSON property type \"%hs\" is unexpected."), curStr);

	return ret;
}

void * DarkEdif::DLL::DLL_GetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
	if (edPtr->eHeader.extVersion != Extension::Version)
	{
		// MFA is older than current ext - we should've upgraded already
		if (edPtr->eHeader.extVersion < Extension::Version)
		{
			MsgBox::Error(_T("Version upgrade failure"), _T("Extension version mismatch (MFA is %lu, real ext is %i). "
				"Fusion should have called UpdateEditStructure, but didn't."), edPtr->eHeader.extVersion, Extension::Version);
		}
		else // MFA is newer than current version - old ext MFX?
		{
			MsgBox::Error(_T("Version upgrade failure"), _T("Extension version mismatch (MFA is %lu, real ext is %i). "
				"You should update the %s extension immediately, WITHOUT saving the MFA first."), edPtr->eHeader.extVersion, Extension::Version, _T("" PROJECT_NAME));
		}
		return nullptr;
	}
	if (edPtr->eHeader.extPrivateData == 0)
	{
		MsgBox::Error(_T("No smart properties"), _T("MFA ext version %lu was saved WITHOUT smart properties, but current extension has same version %d WITH smart properties.\n\n"
			"Did the extension developer upgrade the extension to smart properties without incrementing Extension::Version?"), edPtr->eHeader.extVersion, Extension::Version);
		return nullptr;
	}
	auto& Props = Elevate(edPtr->Props);
	return Props.GetProperty(PropID_);
}

void DarkEdif::DLL::DLL_SetPropValue(mv * mV, EDITDATA * edPtr, unsigned int PropID_, void * Param)
{
	Prop * prop = (Prop *)Param;
	std::uint32_t i = prop->GetClassID(), PropID = GetPropRealID(PropID_);

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
	{
		DebugProp_OutputString(_T("Accessed property ID %u, outside of custom extension range; ignoring it.\n"), PropID);
		return;
	}

	auto& Props = Elevate(edPtr->Props);

	switch (i)
	{
		case 'DATA': // Buffer or string
		{
			const json_value & propjson = CurLang["Properties"][PropID];
			// Buff can be used for a string property
			if (!_strnicmp(propjson["Type"], "Editbox String", sizeof("Editbox String") - 1))
			{
				std::string utf8Str = DarkEdif::TStringToUTF8(((Prop_Str *)prop)->String);
				Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			}
			// If we get a Buff and it's not a string property, DarkEdif doesn't know how to handle it.
			else
			{
				MsgBox::Error(_T("DarkEdif property error"), _T("Got Buff type for non-string property \"%hs\"."),
					(const char *)propjson["Title"]);
			}
			break;
		}
		case 'STRA': // ANSI string
		{
			std::string utf8Str = DarkEdif::ANSIToUTF8(((Prop_AStr *)prop)->String);
			Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			break;
		}
		case 'STRW': // Unicode string
		{
			std::string utf8Str = DarkEdif::WideToUTF8(((Prop_WStr *)prop)->String);
			Props.Internal_PropChange(mV, edPtr, PropID, utf8Str.c_str(), utf8Str.size());
			break;
		}
		case 'INT ': // 4-byte signed int
		{
			Prop_SInt * prop2 = (Prop_SInt *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(int));
			break;
		}
		case 'DWRD': // 4-byte unsigned int
		{
			Prop_UInt * prop2 = (Prop_UInt *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(unsigned int));
			break;
		}
		case 'INT2': // 8-byte signed int
		{
			Prop_Int64 * prop2 = (Prop_Int64 *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(std::int64_t));
			break;
		}
		case 'DBLE': // 8-byte floating point var
		{
			Prop_Double * prop2 = (Prop_Double *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(double));
			break;
		}
		case 'FLOT': // 4-byte floating point var
		{
			Prop_Float * prop2 = (Prop_Float *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->Value, sizeof(float));
			break;
		}
		case 'SIZE': // Two ints depicting a size
		{
			Prop_Size * prop2 = (Prop_Size *)prop;
			Props.Internal_PropChange(mV, edPtr, PropID, &prop2->X, sizeof(int) * 2);
			break;
		}
		default: // Custom property
		{
			Prop_Custom * prop2 = (Prop_Custom *)prop;
			//Internal_PropChange(mV, edPtr, PropID, prop2->GetPropValue(), prop2->GetPropValueSize());

			MsgBox::Info(_T("Custom property"), _T("Assuming class ID %i is custom - but no custom code yet written."), i);
			break;
		}
	}
}
BOOL DarkEdif::DLL::DLL_GetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_)
{
	unsigned int PropID = GetPropRealID(PropID_);

	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return FALSE;

	return edPtr->Props.IsPropChecked(PropID);
}
void DarkEdif::DLL::DLL_SetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int PropID_, BOOL checked)
{
	unsigned int PropID = GetPropRealID(PropID_);
	// Not our responsibility; ID unrecognised
	if (CurLang["Properties"].type == json_null || CurLang["Properties"].u.array.length <= PropID)
		return;

	// The dataForProps consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	int byteIndex = PropID / 8, bitIndex = PropID % 8;

	if (checked != FALSE)
		Elevate(edPtr->Props).dataForProps[byteIndex] |= 1 << bitIndex;
	else
		Elevate(edPtr->Props).dataForProps[byteIndex] &= ~(1 << bitIndex);
}
BOOL DarkEdif::DLL::DLL_IsPropEnabled(mv * mV, EDITDATA * edPtr, unsigned int PropID)
{
	// Possibly an upgrade to do here with JSON properties in later DarkEdif.
	// It's not too hard to write an enabler expression evaluator; something that reads the other property values
	// using an conditional expression kept in the JSON file.
	return TRUE;
}

struct Properties::PreSmartPropertyReader : Properties::PropertyReader
{
	//int numProps;
	const std::uint8_t * at;
	size_t atID;
	const std::uint8_t * endPos;
	const Properties::ConverterState * convState;
	size_t maxChkboxID;
	const std::uint8_t * chkBoxAt;


	// Start the reader. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialise(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;
		if (!convState.oldEdPtr || convState.oldEdPtr->eHeader.extPrivateData != 0)
		{
			atID = 0;
			endPos = nullptr;
			if (!convState.oldEdPtr)
				DebugProp_OutputString(_T("PreSmartPropertyReader: No old edPtr, not initing.\n"));
			else
				DebugProp_OutputString(_T("PreSmartPropertyReader: Private ext data is non-null, assuming smart properties.\n"));
			return Abort(convRet);
		}

		int sizeOfProps = *(int *)(&convState.oldEdPtr->Props);
		if (sizeOfProps < 0 || sizeOfProps < sizeof(EDITDATA::eHeader))
		{
			DebugProp_OutputString(_T("PreSmartPropertyReader: size in bytes of props is %i, not %u, expecting wrong converter.\n"),
				sizeOfProps, convState.oldEdPtr->eHeader.extPrivateData);
			return Abort(convRet);
		}

		sizeOfProps -= sizeof(EDITDATA::eHeader);

		at = ((const std::uint8_t *)&convState.oldEdPtr->Props) + sizeof(sizeOfProps);

		// Add offset for skipping checkbox data
		// note this is based on current JSON's property count, not when data was stored,
		// so it's going to be invalid if user appended too many properties.
		// Since num properties isn't stored, best guess is all we can hope for anyway.

		size_t numChkBoxBytes = (size_t)ceil(convState.jsonProps.u.array.length / 8.0f);
		maxChkboxID = (numChkBoxBytes * 8U) - 1;
		chkBoxAt = at;
		at += numChkBoxBytes;

		atID = 0;
		endPos = ((const std::uint8_t *)&convState.oldEdPtr->Props) + sizeOfProps;

		DebugProp_OutputString(_T("PreSmartPropertyReader: Initialised OK. %i bytes in properties, Fusion reports %u bytes without eHeader.\n"),
			sizeOfProps, convState.oldEdPtr->eHeader.extSize - sizeof(EDITDATA::eHeader));
		return convRet->Return_OK(nullptr, 0);
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(size_t id, ConverterReturn * const convRet)
	{
		// Converter unavailable already
		if (at == nullptr)
			return Abort(convRet);

		// Some to skip - don't pass convRet, just null
		while (id > atID + 1)
		{
			if (Next(nullptr) == false)
			{
				maxChkboxID = atID - 1;
				return Abort(convRet);
			}

			++atID;
		}

		// convRet will be set regardless. False means no more data, which means this ID is inaccessible,
		// making last readable prop ID atID - 1, so chkbox max is atID - 1 too.
		if (Next(convRet) == false)
			maxChkboxID = atID - 1;

		++atID;
	}


	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		if (id > maxChkboxID)
			return convRet->Return_ConverterUnsuitable();

		convRet->Return_OK(nullptr, 0U, nullptr, (chkBoxAt[id / CHAR_BIT] >> (id % CHAR_BIT)) & 1);
	}


	void Abort(ConverterReturn * const convRet)
	{
		at = nullptr;
		endPos = nullptr;
		convRet->Return_ConverterUnsuitable();
	}
	bool Next(ConverterReturn * const convRet)
	{
		//OutputString("Prop ID %i: read from %p [%u], offset %u.\n", atID, at, at[0],
		//	((unsigned long)at) - ((unsigned long)&convState->oldEdPtr->Props));
		bool b = Next2(convRet);
		//OutputString("Prop ID %i: read end at %p [%u], offset %u.\n", atID, at, at ? at[0] : 'X',
		//	at ? ((unsigned long)at) - ((unsigned long)& convState->oldEdPtr->Props) : 0UL);
		return b;
	}
	// Return false if converter unsuitable.
	bool Next2(ConverterReturn * const convRet)
	{
		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[atID].Type_ID % 1000;

		// look up checkbox if we think it's in it
		int chkState = -1;
		if (convRet && atID <= maxChkboxID)
			chkState = (chkBoxAt[atID / CHAR_BIT] >> (atID % CHAR_BIT)) & 1;

		// Hit the end, exactly last property
		if (at >= endPos)
		{
			// Checkboxes are stored outside the queue, so we can still read those
			if (propTypeID == IDs::PROPTYPE_LEFTCHECKBOX)
			{
				if (convRet)
				{
					if (chkState != -1)
						convRet->Return_OK(nullptr, 0U, nullptr, chkState);
					else
					{
						Abort(convRet);
						return false;
					}
				}
				return true;
			}

			maxChkboxID = atID;
			if (convRet)
				Abort(convRet);
			return false;
		}

		switch (propTypeID)
		{
			case IDs::PROPTYPE_LEFTCHECKBOX:
			{
				// Checkbox have no data
				if (convRet)
					convRet->Return_OK(nullptr, 0U, nullptr, chkState);
				return true;
			}
			case IDs::PROPTYPE_STATIC:
			case IDs::PROPTYPE_FOLDER:
			case IDs::PROPTYPE_FOLDER_END:
			case IDs::PROPTYPE_EDITBUTTON:
			{
				// No data for these. Should not be asked for.
				DebugProp_OutputString(_T("PreSmartPropertyReader: Got static type %s (ID %i) for ID %zu. Delegating.\n"),
					UTF8ToTString(Edif::Properties::Names[propTypeID]).c_str(), propTypeID, atID);
				if (convRet)
					convRet->Return_Pass();
				return true;
			}
			case IDs::PROPTYPE_EDIT_STRING:
			case IDs::PROPTYPE_EDIT_MULTILINE:
			case IDs::PROPTYPE_FILENAME:
			case IDs::PROPTYPE_PICTUREFILENAME:
			case IDs::PROPTYPE_DIRECTORYNAME:
			case IDs::PROPTYPE_URLBUTTON:
			{
				size_t maxSize = endPos - at;
				size_t sizeOfStr = maxSize <= 0 ? INT32_MIN : strnlen((const char *)at, maxSize);
				if (sizeOfStr == maxSize)
				{
					// string went past end of properties.
					DebugProp_OutputString(_T("PreSmartPropertyReader: Couldn't find end of string for ID %zu. Killing use of converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(sizeOfStr ? at : nullptr, sizeOfStr, nullptr, chkState);
				at += sizeOfStr + 1; // add null byte
				return true;
			}
			// Stores index of item (smart properties store item as string, in case of Items reordering)
			case IDs::PROPTYPE_COMBOBOX:
			case IDs::PROPTYPE_COMBOBOXBTN:
			case IDs::PROPTYPE_ICONCOMBOBOX:
			{
				// ID stored, read applicable string from JSON
				const json_value& itemsJSON = convState->jsonProps[atID]["Items"];
				if (itemsJSON.type != json_array || atID >= itemsJSON.u.array.length)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Can't find stored ID %zu in JSON Items array. Delegating.\n"), atID);
					if (convRet)
						convRet->Return_Pass();
					return true; // Item ID no longer exists.
				}
				if (at + sizeof(int) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu. Killing converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
				{
					std::string res = (const char *)itemsJSON[*(int*)at];
					if (res.size() > 0)
						convRet->Return_OK(_strdup(res.c_str()), res.size(), [](const void* v) { free((void *)v); }, chkState);
					else
						convRet->Return_OK(nullptr, 0, nullptr, chkState);
				}

				at += sizeof(int);
				return true;
			}
			case IDs::PROPTYPE_SIZE:
			{
				if (at + (sizeof(int) * 2) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(at, sizeof(int) * 2, nullptr, chkState);

				at += sizeof(int) * 2;
				return true;
			}
			case IDs::PROPTYPE_EDIT_NUMBER:
			case IDs::PROPTYPE_COLOR:
			case IDs::PROPTYPE_SLIDEREDIT:
			case IDs::PROPTYPE_SPINEDIT:
			case IDs::PROPTYPE_DIRCTRL:
			case IDs::PROPTYPE_EDIT_FLOAT:
			case IDs::PROPTYPE_SPINEDITFLOAT:
			{
				// int and float have same size

				// ID stored, read applicable string from JSON
				if (at + sizeof(int) > endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Exceeded sizeof data %zu. Killing converter.\n"), atID);
					return false;
				}

				// If there is a return, send it back
				if (convRet)
					convRet->Return_OK(at, sizeof(int), nullptr, chkState);

				at += sizeof(int);
				return true;
			}
			// Null property at end of SDK::Properties
			case 0:
			case IDs::PROPTYPE_CUSTOM:
			default:
			{
				// End position is important
				if (propTypeID == 0 || at >= endPos)
				{
					DebugProp_OutputString(_T("PreSmartPropertyReader: Hit end of property data (prop ID %zu, type ID %i). Killing converter.\n"),
						atID, propTypeID);
					if (convRet)
						Abort(convRet);
					return false;
				}

				const json_value & jsonTitle = CurLang["Properties"][atID]["Title"];
				std::tstring jsonTitleStr = jsonTitle.type == json_none ? _T("<not in JSON>"s) : UTF8ToTString((const char *)jsonTitle);
				DebugProp_OutputString(_T("PreSmartPropertyReader: Custom or unrecognised property name %s, ID %zu, language %s, type ID %i. Delegating.\n"),
					jsonTitleStr.c_str(), atID, DarkEdif::JSON::LanguageName(), propTypeID);
				if (convRet)
					convRet->Return_Pass();
				return true;
			}
		}
	}
};
static Properties::PreSmartPropertyReader preSmartPropertyReader;

struct Properties::SmartPropertyReader : Properties::PropertyReader
{
	std::vector<const Properties::Data *> data;
	const Properties::ConverterState * convState;

	// Resets the reader for a new run. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialise(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;
		data.clear();

		if (!convState.oldEdPtr)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: No old edPtr, not initing.\n"));
			return Abort(convRet);
		}

		if (convState.oldEdPtr->eHeader.extPrivateData == 0 ||
			convState.oldEdPtr->eHeader.extPrivateData >= convState.oldEdPtr->eHeader.extSize)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: offset to Properties is invalid.\n"));
			return Abort(convRet);
		}

		const DLL::PropAccesser& oldProps = Elevate(*convState.oldEdPtrProps);

		const std::uint32_t curVer = 'DAR1';
		if (oldProps.propVersion != curVer)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: smart property is implemented with DarkEdif::Properties struct %.4s, but this reader is %.4s.\n"), &oldProps.propVersion, &curVer);
			return Abort(convRet);
		}

		// Old version of properties pre-smart properties won't have this.
		// (there's not much chance they have extPrivateData set, but JIC)
		if (oldProps.numProps < 0 || oldProps.sizeBytes > std::numeric_limits<std::uint32_t>::max())
		{
			DebugProp_OutputString(_T("SmartPropertyReader: num properties too low, or size of properties is too high.\n"));
			return Abort(convRet);
		}

		if (oldProps.numProps == 0)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: num properties is zero. No Data reading.\n"));
			return convRet->Return_OK(nullptr, 0U);
		}

		const Data * d = oldProps.Internal_FirstData();
		for (size_t i = 0; i < oldProps.numProps; i++)
		{
			data.push_back(d);
			d = d->Next();

			// None or overread
			if (d == nullptr || (const std::uint8_t *)d > ((const std::uint8_t *)convState.oldEdPtr) + convState.oldEdPtr->eHeader.extSize)
			{
				return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu / %zu.\n"),
					i, oldProps.numProps);
			}
		}
		return convRet->Return_OK(nullptr, 0U);
	}
	// Returns true if the type of Data can be upgraded to the type of second parameter.
	bool IsSimilar(const std::uint16_t propTypeOld, const std::uint16_t propTypeNew)
	{
		// Same type
		if (propTypeOld == propTypeNew)
			return true;

		using IDs = Edif::Properties::IDs;
		// Text-based types are similar enough (we don't allow multiline -> single-line, but we do allow vice versa)
		if ((propTypeOld == IDs::PROPTYPE_DIRECTORYNAME || propTypeOld == IDs::PROPTYPE_EDIT_STRING || propTypeOld == IDs::PROPTYPE_FILENAME || propTypeOld == IDs::PROPTYPE_PICTUREFILENAME) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_MULTILINE || propTypeNew == IDs::PROPTYPE_DIRECTORYNAME || propTypeNew == IDs::PROPTYPE_EDIT_STRING || propTypeNew == IDs::PROPTYPE_FILENAME || propTypeNew == IDs::PROPTYPE_PICTUREFILENAME))
		{
			return true;
		}
		// Combo box are the same - we'll only store the item text, so the presence of icons is irrelevant
		if ((propTypeOld == IDs::PROPTYPE_COMBOBOX || propTypeOld == IDs::PROPTYPE_COMBOBOXBTN || propTypeOld == IDs::PROPTYPE_ICONCOMBOBOX) &&
			(propTypeNew == IDs::PROPTYPE_COMBOBOX || propTypeNew == IDs::PROPTYPE_COMBOBOXBTN || propTypeNew == IDs::PROPTYPE_ICONCOMBOBOX))
		{
			return true;
		}
		// All int selection are swappable
		if ((propTypeOld == IDs::PROPTYPE_EDIT_NUMBER || propTypeOld == IDs::PROPTYPE_SPINEDIT || propTypeOld == IDs::PROPTYPE_SLIDEREDIT) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_NUMBER || propTypeNew == IDs::PROPTYPE_SPINEDIT || propTypeNew == IDs::PROPTYPE_SLIDEREDIT))
		{
			return true;
		}
		// Allow int -> float, but not vice versa
		if ((propTypeOld == IDs::PROPTYPE_EDIT_FLOAT || propTypeOld == IDs::PROPTYPE_SPINEDITFLOAT ||
			propTypeOld == IDs::PROPTYPE_EDIT_NUMBER || propTypeOld == IDs::PROPTYPE_SPINEDIT || propTypeOld == IDs::PROPTYPE_SLIDEREDIT) &&
			(propTypeNew == IDs::PROPTYPE_EDIT_FLOAT || propTypeNew == IDs::PROPTYPE_SPINEDITFLOAT))
		{
			return true;
		}

		// Later, we can expand this function by checking lengths are within bounds and max/min, etc.

		return false;
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(size_t id, ConverterReturn * const convRet)
	{
		const json_value & propJ = convState->jsonProps[id];

		std::string jsonPropName = (const char *)propJ["Title"];

		// If a property has "OldTitle", we'll check for that first
		// Worth noting if you rename a -> b and b -> a, a will simply be put as a.
		std::string jsonPropOldName = propJ["OldTitle"].type == json_none ? "" : propJ["OldTitle"];

		int jsonPropTypeID = -1;

		// Loop through Parameter names and compareth them.
		for (size_t j = Edif::Properties::IDs::PROPTYPE_FIRST_ITEM; j < Edif::Properties::IDs::PROPTYPE_LAST_ITEM; ++j)
		{
			if (!_stricmp(propJ["Type"], Edif::Properties::Names[j]))
			{
				jsonPropTypeID = j;
				break;
			}
		}

		if (jsonPropTypeID == -1)
		{
			return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu. Type ID in JSON %s could not be converted to ID.\n"),
				id, UTF8ToTString((const char*)propJ["Type"]).c_str());
		}

		auto f = data.cend();
		// Find property by old name
		if (!jsonPropOldName.empty())
		{
			f = std::find_if(data.cbegin(), data.cend(), [&](const Data* d) {
				return !_stricmp(d->ReadPropName().c_str(), jsonPropOldName.c_str()) && IsSimilar(d->propTypeID, jsonPropTypeID);
			});
			if (f != data.cend())
			{
				DebugProp_OutputString(_T("SmartPropertyReader: found property %zu by old name %s.\n"),
					id, UTF8ToTString(jsonPropOldName).c_str());
			}
		}
		if (f == data.cend())
		{
			// Find property by name (the "smart" part)
			f = std::find_if(data.cbegin(), data.cend(), [&](const Data* d) {
				return !_stricmp(d->ReadPropName().c_str(), jsonPropName.c_str()) && IsSimilar(d->propTypeID, jsonPropTypeID);
			});
		}

		if (f != data.cend())
		{
			DebugProp_OutputString(_T("SmartPropertyReader: found property %zu by name %s.\n"),
				id, UTF8ToTString(jsonPropName).c_str());
			const size_t foundIndex = std::distance(data.cbegin(), f);
			return convRet->Return_OK(data[id]->ReadPropValue(), data[id]->ReadPropValueSize(), nullptr, convState->oldEdPtrProps->IsPropChecked((int)foundIndex));
		}

#if 0
		// Name not found? fall back on looking up by ID
		// I've disabled this because it would lead to returning any property that matched. I want it smarter than that.

		if (data.size() <= id)
		{
			return convRet->Return_Error(_T("SmartPropertyReader: failed to read Data for property %zu, only %zu Datas are stored.\n"),
				id, data.size());
		}

		if (data[id]->propTypeID != jsonPropTypeID)
		{
			DebugProp_OutputString(_T("SmartPropertyReader: failed to read Data for property %zu (json name %s). Name and ID was not found in old data.\n"),
				id, UTF8ToTString(jsonPropName).c_str());
			return convRet->Return_Pass();
		}

		convRet->Return_OK(data[id]->ReadPropValue(), data[id]->ReadPropValueSize(), nullptr, convState->oldEdPtrProps->IsPropChecked(id));
#else
		DebugProp_OutputString(_T("SmartPropertyReader: property ID %zu couldn't be found by title \"%s\" or old title \"%s\". Passing it on for reset.\n"),
			id, UTF8ToTString(jsonPropName).c_str(), UTF8ToTString(jsonPropOldName).c_str());
		convRet->Return_Pass();
#endif
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		const auto &p = GetRealPropsAddress((EDITDATA *)convState->oldEdPtr);
		return convRet->Return_OK(NULL, 0, NULL, p.IsPropChecked(id));
	}

	void Abort(ConverterReturn * const convRet)
	{
		data.clear();
		return convRet->Return_ConverterUnsuitable();
	}
};
static Properties::SmartPropertyReader smartPropertyReader;

struct Properties::JSONPropertyReader : Properties::PropertyReader
{
	Properties::ConverterState * convState;

	// Start the reader. Return ConverterUnsuitable if the converter isn't necessary.
	void Initialise(ConverterState &convState, ConverterReturn * const convRet)
	{
		this->convState = &convState;

		// Are JSON Properties item an array?
		if (convState.jsonProps.type != json_array || convState.jsonProps.u.array.length > 1000)
		{
			// There is no Properties item in the JSON file. Consider the JSON valid, but with no properties.
			if (convState.jsonProps.type == json_none)
				DebugProp_OutputString(_T("JSONPropertyReader: No Properties item. Going to initialize with 0 properties.\n"));
			else // Properties is not an array - nani?
			{
				MsgBox::Error(_T("Property error"), _T("JSONPropertyReader: Properties is the wrong type, expected an array.\n"));
				return Abort(convRet);
			}
		}

		DebugProp_OutputString(_T("JSONPropertyReader: Initialised OK. %u properties.\n"),
			convState.jsonProps.u.array.length);
		return convRet->Return_OK(nullptr, 0U);
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetProperty() as if IDs can be skipped.
	void GetProperty(size_t id, ConverterReturn * const convRet)
	{
		if (convState->jsonProps.u.array.length <= id)
		{
			return convRet->Return_Error(_T("JSONPropertyReader: no JSON item for property %zu, only %u items are stored"),
				id, convState->jsonProps.u.array.length);
		}

		const json_value &prop = *convState->jsonProps.u.array.values[id];
		if (prop.type != json_object)
			return convRet->Return_Error(_T("JSDNPropertyReader: Index %zu is not a json_object (but within Properties array)"), id);
		std::string title = prop["Title"].type == json_string ? (const char *)prop["Title"] : "<missing title>";

		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[id].Type_ID % 1000;
		switch (propTypeID)
		{
		case IDs::PROPTYPE_STATIC:
		case IDs::PROPTYPE_FOLDER:
		case IDs::PROPTYPE_FOLDER_END:
		case IDs::PROPTYPE_EDITBUTTON:
		case IDs::PROPTYPE_LEFTCHECKBOX:
		case IDs::PROPTYPE_URLBUTTON:
		case IDs::PROPTYPE_GROUP:
		{
			// No data for these. We store just metadata so getting property by index has consistent indexes
			DebugProp_OutputString(_T("JSDNPropertyReader: Got static type %i (%hs) for property title %s, ID %zu. Storing just metadata.\n"),
				propTypeID, Edif::Properties::Names[propTypeID], UTF8ToTString(title).c_str(), id);
			return convRet->Return_OK(nullptr, 0U);
		}
		case IDs::PROPTYPE_EDIT_STRING:
		case IDs::PROPTYPE_EDIT_MULTILINE:
		case IDs::PROPTYPE_FILENAME:
		case IDs::PROPTYPE_PICTUREFILENAME:
		case IDs::PROPTYPE_DIRECTORYNAME:
		{
			const size_t maxSize = 4096;
			size_t sizeOfStr = strnlen(prop["DefaultState"], maxSize);
			if (sizeOfStr == maxSize)
			{
				// string went past end of properties.
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't find end of string for property %s, ID %zu (language %s). Will "
					"delegate this property and any further properties."), UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
				return Abort(convRet);
			}

			// convState->resetPropertiesStream << title << " = \"" << (const char *)prop["DefaultState"] << "\"\n";
			convState->resetPropertiesStream << title << '\n';
			++convState->numPropsReset;
			std::string data = (const char*)prop["DefaultState"];
			if (!_stricmp(prop["Case"], "Lower"))
				std::transform(data.begin(), data.end(), data.begin(), [](std::uint8_t c) { return std::tolower(c); });
			else if (!_stricmp(prop["Case"], "Upper"))
				std::transform(data.begin(), data.end(), data.begin(), [](std::uint8_t c) { return std::toupper(c); });
			return convRet->Return_OK(_strdup(data.c_str()), sizeOfStr, [](const void* v) { free((void *)v); });
		}
		// Stores text of item, but checks it's in Items
		case IDs::PROPTYPE_COMBOBOX:
		case IDs::PROPTYPE_COMBOBOXBTN:
		case IDs::PROPTYPE_ICONCOMBOBOX:
		{
			const json_value& itemsJSON = prop["Items"];
			if (itemsJSON.type != json_array)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Can't find Items for JSON item %s, ID %zu, language %s. Passing onto next converter."),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
				return convRet->Return_Pass(); // Item ID no longer exists.
			}

			const size_t maxSize = 4096;
			size_t sizeOfStr = strnlen(prop["DefaultState"], maxSize);
			if (sizeOfStr == maxSize)
			{
				// string went past end of properties.
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't find end of string for JSON item %s, ID %zu, language %s. Will "
					"pass on this property and any further properties."), UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
				return Abort(convRet);
			}

			// convState->resetPropertiesStream << title << " = \"" << (const char *)prop["DefaultState"] << "\"\n";
			convState->resetPropertiesStream << title << '\n';
			++convState->numPropsReset;

			return convRet->Return_OK((const char *)prop["DefaultState"], sizeOfStr);
		}
		case IDs::PROPTYPE_SIZE:
		{
			if (prop["DefaultState"].type != json_array || prop["DefaultState"].u.array.length != 2)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Couldn't read default state for JSON item %s, ID %zu, language %s. Erroring."),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
				return Abort(convRet);
			}

			static int both[2] = { 0, 0 };
			both[0] = (int)(json_int_t)prop["DefaultState"][0];
			both[1] = (int)(json_int_t)prop["DefaultState"][1];

			// convState->resetPropertiesStream << title << " = (" << both[0] << ", " << both[1] << ")\n";
			convState->resetPropertiesStream << title << '\n';
			++convState->numPropsReset;

			return convRet->Return_OK(both, sizeof(both));
		}
		case IDs::PROPTYPE_EDIT_NUMBER:
		case IDs::PROPTYPE_COLOR:
		case IDs::PROPTYPE_SLIDEREDIT:
		case IDs::PROPTYPE_SPINEDIT:
		case IDs::PROPTYPE_DIRCTRL:
		{
			if (prop["DefaultState"].type != json_integer)
			{
				// Wrong data type for this property.
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has no default value."),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
			}

			// JSON stores integers as long long (64-bit signed), but Fusion only allows 32-bit (signed).
			std::int64_t intDataAsLong = prop["DefaultState"].u.integer;
			if (intDataAsLong > INT32_MAX || intDataAsLong < INT32_MIN)
			{
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has default value that can't be stored in 32-bit int (value is %lld)."),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName(), intDataAsLong);
			}

			// convState->resetPropertiesStream << title << " = " << intDataAsLong << "\n";
			convState->resetPropertiesStream << title << '\n';
			++convState->numPropsReset;

			static int intData2;
			intData2 = (int)intDataAsLong;

			return convRet->Return_OK(&intData2, sizeof(int));
		}
		case IDs::PROPTYPE_EDIT_FLOAT:
		case IDs::PROPTYPE_SPINEDITFLOAT:
		{
			if (prop["DefaultState"].type != json_double)
			{
				// Wrong data type for this property.
				return convRet->Return_Error(_T("JSONPropertyReader: JSON item %s, ID %zu, language %s has no default value."),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
			}

			static float f;
			f = (float)prop["DefaultState"].u.dbl;

			// convState->resetPropertiesStream << title << " = " << std::setprecision(3) << f << "\n";
			convState->resetPropertiesStream << title << "\n";
			++convState->numPropsReset;

			return convRet->Return_OK(&f, sizeof(float));
		}

		case IDs::PROPTYPE_CUSTOM:
		default:
		{
			if (propTypeID >= IDs::PROPTYPE_CUSTOM && propTypeID <= IDs::PROPTYPE_CUSTOM + 9)
			{
				DebugProp_OutputString(_T("JSONPropertyReader: Custom property name %s, ID %zu, language %s, type ID %d. Passing on, hopefully to user converter.\n"),
					UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName(), propTypeID);
				return convRet->Return_Pass();
			}

			DebugProp_OutputString(_T("JSONPropertyReader: Property ID %zu, name %s, language %s, type ID %u. Delegating.\n"),
				id, UTF8ToTString(title).c_str(), DarkEdif::JSON::LanguageName(), propTypeID);
			return convRet->Return_Pass();
		}
		}
	}

	// Get property by ID.
	// Note that IDs will always be increasing, but you should program GetPropertyCheckbox() as if IDs can be skipped.
	void GetPropertyCheckbox(size_t id, ConverterReturn * const convRet)
	{
		if (convState->jsonProps.u.array.length <= id)
		{
			return convRet->Return_Error(_T("JSONPropertyReader: no JSON item for property %zu, only %u items are stored.\n"),
				id, convState->jsonProps.u.array.length);
		}

		const json_value &prop = *convState->jsonProps.u.array.values[id];
		if (prop.type != json_object)
			return convRet->Return_Error(_T("JSDNPropertyReader: Index %zu is not a json_object (but within Properties array).\n"), id);
		std::string title = prop["Title"].type == json_string ? (const char *)prop["Title"] : "<missing title>";

		using IDs = Edif::Properties::IDs;
		int propTypeID = Edif::SDK->EdittimeProperties[id].Type_ID % 1000;
		if (propTypeID == IDs::PROPTYPE_LEFTCHECKBOX)
		{
			if (prop["DefaultState"].type == json_boolean)
			{
				// convState->resetPropertiesStream << title << " = " << (prop["DefaultState"] ? "true" : "false") << "\n";
				convState->resetPropertiesStream << title << "\n";
				++convState->numPropsReset;
				return convRet->Return_OK(nullptr, 0U, nullptr, ((bool)prop["DefaultState"]) ? 1 : 0);
			}

			return convRet->Return_Error(_T("JSONPropertyReader: DefaultState is not set for checkbox property. Property %s, ID %zu (language %s). Erroring.\n"),
				UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
		}

		if (prop["CheckboxDefaultState"].type != json_none)
		{
			if (prop["CheckboxDefaultState"].type == json_boolean)
			{
				// convState->resetPropertiesStream << title << " = " << (prop["DefaultState"] ? "true" : "false") << "\n";
				convState->resetPropertiesStream << title << "\n";
				++convState->numPropsReset;
				return convRet->Return_OK(nullptr, 0U, nullptr, ((bool)prop["CheckboxDefaultState"]) ? 1 : 0);
			}

			return convRet->Return_Error(_T("JSONPropertyReader: the CheckboxDefaultState is not set for property with checkbox flag. Property %s, ID %u (language %s). Erroring.\n"),
				UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
		}

		DebugProp_OutputString(_T("JSONPropertyReader: Property %s, ID %u (language %s) is not a checkbox-related property. Delegating.\n"),
			UTF8ToTString(title).c_str(), id, DarkEdif::JSON::LanguageName());
		return convRet->Return_Pass();
	}

	void Abort(ConverterReturn * const convRet)
	{
		return convRet->Return_ConverterUnsuitable();
	}
};
static Properties::JSONPropertyReader jsonPropertyReader;

// Default fallback for test_has_UserConverter when EDITDATA can't be used because it has no matching function: false
template <class T, typename = void>
struct test_has_UserConverter : std::false_type { };

// test_has_UserConverter when EDITDATA has a matching function: true!
template <class T>
struct test_has_UserConverter <T, std::enable_if_t<std::is_invocable_r<DarkEdif::Properties::PropertyReader*, decltype(T::UserConverter)>::value>>
	: std::true_type { };

// Wrapper for calling UserConverter; if it doesn't exist, this stops a compile-time error
template <typename T>
	typename std::enable_if_t<!test_has_UserConverter<T>::value, DarkEdif::Properties::PropertyReader *>
	UserConverterWrap()
{
	// This function is a dummy so the compiler thinks UserConverterWrap always has a definition, but it shouldn't be run,
	// because in the scenario where UserConverterWrap is executed, the below override should be what is compiled into the code instead.
	LOGF(_T("Running dummy UserConverterWrap(). Should not happen."));
	return nullptr;
};
// Wrapper when EDITDATA has a usable UserConverter function: just call it!
template <typename T>
	typename std::enable_if_t<test_has_UserConverter<T>::value, DarkEdif::Properties::PropertyReader*>
		UserConverterWrap() {
	return T::UserConverter();
}

HGLOBAL FusionAPI UpdateEditStructure(mv * mV, void * OldEdPtr) {
#pragma DllExportHint
	return DarkEdif::DLL::DLL_UpdateEditStructure(mV, (EDITDATA*)OldEdPtr);
}

HGLOBAL DarkEdif::DLL::DLL_UpdateEditStructure(mv * mV, EDITDATA * oldEdPtr)
{
	// Note: GetRunObjectInfos provides the current Extension::Version to Fusion,
	// which causes UpdateEditStructure to be called by Fusion if it doesn't match
	// eHeader.extVersion .

	std::uint32_t oldExtVersion = oldEdPtr ? oldEdPtr->eHeader.extVersion : 0;

	DebugProp_OutputString(_T("UpdateEditStructure called; MFA/old version %u, current/new version %i.\n"),
		oldExtVersion, Extension::Version);

	// Fusion shouldn't call it for this scenario.
	if (oldEdPtr && oldExtVersion == Extension::Version)
	{
		DebugProp_OutputString(_T("UpdateEditStructure aborting: versions are equivalent.\n"));

		return nullptr;
	}

	// Fusion may not call it for this scenario either. We'll see.
	if (oldExtVersion > Extension::Version)
	{
		DebugProp_OutputString(_T("UpdateEditStructure warning: MFA version is greater than current version.\n"));

#ifdef _DEBUG
		// If a developer, let them pick what to do. They might've screwed up the MFA version number.
		int msgRet = MsgBox::WarningYesNoCancel(_T("Property update warning"), _T("UpdateEditStructure has current version %i and MFA has a version of %i. "
			"Select an action to take:\n"
			"Yes = run updater.\n"
			"No = set the EDITDATA's version number to earlier version %i.\n"
			"Cancel = make no changes and ignore the difference."),
			Extension::Version, oldEdPtr->eHeader.extVersion, Extension::Version);
		if (msgRet != IDYES)
		{
			if (msgRet == IDNO)
				oldEdPtr->eHeader.extVersion = Extension::Version;
			return nullptr;
		}
#else
		MsgBox::WarningOK(_T("Property update warning"), _T("%s MFA extension version %u is greater than current %s ext version %d. Is the installed extension out of date?"),
			_T("" PROJECT_NAME), oldEdPtr->eHeader.extVersion, Extension::Version);
		return nullptr;
#endif
	}

	// oldEdPtr->eHeader.extVersion < Extension::Version at this point, or conversion was requested.

	// While our properties haven't changed, the EDITDATA has.
	long oldOffset = oldEdPtr ? (long)oldEdPtr->eHeader.extPrivateData : 0L;
	const long newOffset = offsetof(EDITDATA, Props);

	const static int PROPID_ITEM_NAME = 21; // user-defined object taken from a Props.h file
	Prop_Str * ps = (Prop_Str *)(oldEdPtr ? mvGetPropValue(mV, oldEdPtr, PROPID_ITEM_NAME) : NULL);
	std::tstring objName = ExtensionName;
	if (ps != nullptr)
	{
		objName = ps->String;
		if (objName != ExtensionName)
			objName += _T(" (") + ExtensionName + _T(")");
		ps->Delete();
	}

	// Reading pre-smart failed. Only feasible option is to reset.
	//DarkEdif::MsgBox::Info("Resetting properties", "Warning: Forced to reset properties for object \"%s\" (%s), "
	//	"due to extension upgrade. Will use default property settings.",
	//	objName.c_str(), ExtensionName.c_str());

	std::ostringstream dataToWriteStream;

	const size_t numPropsIncludingStatics = CurLang["Properties"].u.array.length;
	// 1 byte per 8 properties, rounded up
	auto chkboxes = std::make_unique<std::uint8_t[]>((size_t)ceil((double)numPropsIncludingStatics / 8.0));
	memset(chkboxes.get(), 0, (size_t)ceil((double)numPropsIncludingStatics / 8.0));

	// smart properties
	if (oldOffset != 0L)
	{
		auto& oldProps = Elevate(GetRealPropsAddress(oldEdPtr));

		DebugProp_OutputString(_T("Note: oldOffset = %i, newOffset is %i.\n"), oldOffset, newOffset);

		size_t newEdPtrSize = 0;

		// Names and types match, both old and new version have same properties.
		// The upgrade of Extension::Version did not involve a property change.
		if (oldProps.hash == ~Edif::SDK->jsonPropsNameAndTypesHash)
		{
			// No changes to EDITDATA outside properties, so Extension::Version change had no change to EDITDATA at all.
			// UpdateEditStructure() thus has nothing to do.
			if (oldOffset == newOffset)
			{
				DebugProp_OutputString(_T("Prop offsets are same. Replacing extVersion as %i and ignoring difference."), Extension::Version);
				oldEdPtr->eHeader.extVersion = Extension::Version;
				return NULL; // No change
			}

			// Prop offsets change: properties are same, but EDITDATA changed.
			// Bulk copy all the properties over
			dataToWriteStream.write(((const char *)oldEdPtr) + oldOffset, oldProps.sizeBytes - ((int)oldOffset));
			DebugProp_OutputString(_T("Prop offsets changed, but hashes are the same. Jumping to output.\n"));
			goto ReadyToOutput;
		}
		// Names changed, but types didn't. Perhaps a typo was fixed.
		// All the data we could copy was copied, so now add defaults.
		else if (oldProps.hashTypes == ~Edif::SDK->jsonPropsTypesHash)
			DebugProp_OutputString(_T("Note: Types the same arrangement, but names changed. Assuming cosmetic change.\n"));
		else
			DebugProp_OutputString(_T("Types and names are both different.\n"));
	}
	else // non-smart properties
	{
		// If extPrivateData is 0, the old version was stored without smart JSON properties.
		// Since extVersion is set based on Extension::Version, that's very possible.
		DebugProp_OutputString(_T("Non-smart properties: skipping hash check, since the upgrade to smart properties is needed.\n"));
		oldOffset = newOffset;
	}


	// so goto is happy
	{
		Properties::ConverterState convState(oldEdPtr, CurLang["Properties"]);

		struct reader {
			Properties::PropertyReader * ptr;	// singleton only, no garbage collect
			const char * name;					// string literal only, no garbage collect
		};
		std::vector<reader> readers;

		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor & retStateAdmin = *(DLL::ConverterReturnAccessor *) & retState;

			// Detect if UserConverter function exists in EDITDATA at compile-time, and init if so
			// This uses template fallback magic so the code will compile into no-op, rather than being unable to compile,
			// if EDITDATA has no matching function
			if constexpr (test_has_UserConverter<EDITDATA>::value)
			{
				Properties::PropertyReader * userConv = UserConverterWrap<EDITDATA>();
				userConv->Initialise(convState, &retState);
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					DebugProp_OutputString(_T("User converter init OK; will be used.\n"));
					readers.push_back({ userConv, "UserConverter" });
				}
				else
				{
					DebugProp_OutputString(_T("User converter init failed; will be excluded.\n"));
				}
			}
			else
				DebugProp_OutputString(_T("User converter does not exist; will be excluded.\n"));

			preSmartPropertyReader.Initialise(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &preSmartPropertyReader, "PreSmartPropertyReader" });
			else
				DebugProp_OutputString(_T("Pre-smart property reader init failed; will be excluded.\n"));

			smartPropertyReader.Initialise(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &smartPropertyReader, "SmartPropertyReader" });
			else
				DebugProp_OutputString(_T("Smart property reader init failed; will be excluded.\n"));

			jsonPropertyReader.Initialise(convState, &retState);
			if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				readers.push_back({ &jsonPropertyReader, "JSONPropertyReader" });
			else
				DebugProp_OutputString(_T("JSON property reader init failed; will be excluded.\n"));

			// EDITDATA makes no sense
			if (readers.empty())
			{
				if (DarkEdif::RunMode == MFXRunMode::SplashScreen)
					MsgBox::Error(_T("Property reading error"), _T("Setting up %s object properties from JSON failed."), _T("" PROJECT_NAME));
				else
					MsgBox::Error(_T("Property conversion error"), _T("All %s property readers have failed. Your property data is corrupt beyond recovery or reset. Please re-add the object to frame."), _T("" PROJECT_NAME));
				return NULL;
			}
		}

		std::vector<int> chkboxIndexesToRead(numPropsIncludingStatics);
		for (size_t i = 0; i < numPropsIncludingStatics; i++)
			chkboxIndexesToRead[i] = i;

		std::vector<reader> chkReaders = readers;
		std::string title;

		DebugProp_OutputString(_T("Starting to read data. Accum data size is now %zu.\n"), std::max(0u, (size_t)dataToWriteStream.tellp()));

		for (size_t i = 0; i < numPropsIncludingStatics; i++)
		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor& retStateAdmin = *(DLL::ConverterReturnAccessor *)&retState;
			if (readers.empty())
			{
				return NULL;
			}

			// Don't include title for static text types
			if (IsUnchangeablePropExclCheckbox(convState.jsonProps[i]["Type"]))
				title.clear();
			else
				title = convState.jsonProps[i]["Title"];


			// Note: Unchangeable props are included for IDs to be consistent.
			// So if there's a label at ID 0, then a editbox, and property ID 1 is requested, property ID 0 won't be ignored
			// The JSON property parser will OK the properties.
			// if (IsUnchangeableProp(convState.jsonProps[i]["Type"]))
			// 	continue;

			bool ok = false;
			for (size_t j = 0; j < readers.size(); j++)
			{
				readers[j].ptr->GetProperty(i, &retState);

				// Converted OK, no reason for other readers
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					ok = true;

					std::uint16_t propTypeID = Edif::SDK->EdittimeProperties[i].Type_ID % 1000;

					assert(title.size() <= UINT8_MAX - 1); // can't store in titleLen
					std::uint8_t titleLen = (std::uint8_t)title.size();

					DebugProp_OutputString(_T("Prop index %zu (%s) was read by reader %s (%s checkbox).\n"),
						i, UTF8ToTString(title).c_str(), UTF8ToTString(readers[j].name).c_str(),
						retStateAdmin.checkboxState == -1 ? _T("without") : _T("with"));

					std::uint32_t propSize2 = sizeof(Properties::Data) + titleLen + (std::uint16_t)retStateAdmin.dataSize;
					static_assert(sizeof(propTypeID) == sizeof(Properties::Data::propTypeID), "Properties::Data does not match the stream's sizes.");
					static_assert(sizeof(propSize2) == sizeof(Properties::Data::sizeBytes), "Properties::Data does not match the stream's sizes.");
					static_assert(sizeof(titleLen) == sizeof(Properties::Data::propNameSize), "Properties::Data does not match the stream's sizes.");

					// embedded nulls upset the << operator, so write() is preferred.
					dataToWriteStream.write((char *)& propSize2, sizeof(propSize2));
					dataToWriteStream.write((char *)& propTypeID, sizeof(propTypeID));
					dataToWriteStream.write((char *)& titleLen, sizeof(titleLen));
					dataToWriteStream.write(title.c_str(), titleLen);

					if (retStateAdmin.data != nullptr)
					{
						// assert(retStateAdmin.dataSize > 0);
						dataToWriteStream.write((const char *)retStateAdmin.data, retStateAdmin.dataSize);

						if (retStateAdmin.freeData != nullptr)
							retStateAdmin.freeData(retStateAdmin.data);
					}

					if (retStateAdmin.checkboxState != -1)
					{
						assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);
						if (retStateAdmin.checkboxState == 1)
							chkboxes[i / CHAR_BIT] |= 1 << (i % CHAR_BIT);

						auto f = std::find(chkboxIndexesToRead.cbegin(), chkboxIndexesToRead.cend(), i);
						assert(f != chkboxIndexesToRead.cend());
						chkboxIndexesToRead.erase(f);
					}

					DebugProp_OutputString(_T("Added property %d - accum data size is now %zu.\n"), i, dataToWriteStream.tellp());
					break;
				}

				// Conversion failed and converter is no longer useable - drop converter
				if (retStateAdmin.convRetType == Properties::ConvReturnType::ConverterUnsuitable)
				{
					readers.erase(readers.cbegin() + j--);
					continue;
				}
				// Converter should've worked, but failed?
				else if (retStateAdmin.convRetType == Properties::ConvReturnType::Error)
				{
					MsgBox::WarningOK(_T("Property conversion error"), _T("%hs converter encountered a failure: %s. Delegating."), readers[j].name,
						retStateAdmin.data ? (const TCHAR *)retStateAdmin.data : _T("unknown error"));
					if (retStateAdmin.freeData)
						retStateAdmin.freeData(retStateAdmin.data);
					continue;
				}
				//else if (retStateAdmin.convRetType == Properties::ConvReturnType::Delegate)
				//	continue;
			}

			// JSON errors are reported elsewhere, and aren't relevant for upgrading anyway
			if (!ok && RunMode != MFXRunMode::SplashScreen)
			{
				MsgBox::Error(_T("Property conversion error"), _T("All converters have failed for property %i (%hs). Your property data is corrupt "
					"beyond recovery or reset. Please re-add the object to frame."), i, (const char *)convState.jsonProps[i]["Title"]);
			}
		}

		readers = chkReaders;
		for (size_t k = 0; k < chkboxIndexesToRead.size(); k++)
		{
			Properties::ConverterReturn retState;
			DLL::ConverterReturnAccessor & retStateAdmin = *(DLL::ConverterReturnAccessor *) & retState;
			size_t i = chkboxIndexesToRead[k];

			if (IsUnchangeablePropExclCheckbox(convState.jsonProps[i]["Type"]))
				title.clear();
			else
				title = convState.jsonProps[i]["Title"];

			for (size_t j = 0; j < readers.size(); j++)
			{
				readers[j].ptr->GetPropertyCheckbox(i, &retState);

				// Converted OK, no reason for other readers
				if (retStateAdmin.convRetType == Properties::ConvReturnType::OK)
				{
					assert(retStateAdmin.checkboxState == 0 || retStateAdmin.checkboxState == 1);

					if (retStateAdmin.checkboxState != -1)
					{
						DebugProp_OutputString(_T("Prop index %zu (%s) checkbox read by reader %s.\n"),
							i, UTF8ToTString(title).c_str(), UTF8ToTString(readers[j].name).c_str());

						if (retStateAdmin.checkboxState == 1)
							chkboxes[i / 8] |= 1 << (i % 8);

						auto f = std::find(chkboxIndexesToRead.cbegin(), chkboxIndexesToRead.cend(), i);
						assert(f != chkboxIndexesToRead.cend());
						chkboxIndexesToRead.erase(f);
						--k;
					}

					break;
				}

				// Conversion failed and converter is no longer useable - drop converter
				if (retStateAdmin.convRetType == Properties::ConvReturnType::ConverterUnsuitable)
				{
					readers.erase(readers.cbegin() + j--);
					continue;
				}
				else if (retStateAdmin.convRetType == Properties::ConvReturnType::Error)
				{
					MsgBox::Error(_T("Property conversion error"), _T("Couldn't read check box of  some properties. Delegating."));
					continue;
				}
				//else if (retStateAdmin.convRetType == Properties::ConvReturnType::Delegate)
				//	continue;
			}
		}

		// If it's an upgrade, make a message box
		if (oldEdPtr != nullptr)
		{
			std::string upgradeBox = DarkEdif::GetIniSetting("SmartPropertiesUpgradeBox"sv);
			if (upgradeBox == ""sv)
				upgradeBox = "never"sv;
			#if _DEBUG
			upgradeBox = "always"sv;
			#endif

			if (upgradeBox != "for reset only" && upgradeBox != "always"sv && upgradeBox != "never"sv)
			{
				MsgBox::Error(_T("DarkEdif INI error"), _T("Couldn't understand \"SmartProperiesUpgradeBox\" setting of \"%s\". Valid values are \"always\", \"new only\" and \"never\"."), UTF8ToTString(upgradeBox).c_str());
				upgradeBox = "always"sv;
			}

			if (upgradeBox != "never"sv)
			{
				std::string resetPropStr = convState.resetPropertiesStream.str();
				if (resetPropStr.empty())
				{
					if (upgradeBox != "new only"sv)
					{
						MsgBox::Info(_T("Upgraded properties"), _T("Successfully upgraded all %u of %s object properties from ext version %lu to version %i."),
							convState.jsonProps.u.array.length, objName.c_str(), oldExtVersion, Extension::Version);
					}
				}
				else
				{
					resetPropStr.resize(resetPropStr.size() - 1U); // remove last line's ending newline
					MsgBox::Info(_T("Upgraded properties"), _T("Successfully copied %u of %s object properties from ext version %lu to %i.\n\nAlso reset %zu %s to default settings, namely:\n%s"),
						convState.jsonProps.u.array.length - convState.numPropsReset, objName.c_str(),
						oldExtVersion, Extension::Version,
						convState.numPropsReset, convState.numPropsReset == 1 ? _T("property") : _T("properties"), UTF8ToTString(resetPropStr).c_str());
				}
			}
		}
	}

ReadyToOutput:
	std::string dataToWriteString = std::string((const char *)chkboxes.get(), (size_t)ceil(numPropsIncludingStatics / 8.0f)) + dataToWriteStream.str();
	size_t newEdPtrSize = sizeof(EDITDATA) + dataToWriteString.size() + (newOffset - oldOffset);

	if (!oldEdPtr && newEdPtrSize >= UINT16_MAX)
	{
		MsgBox::Error(_T("Properties too big"), _T("New EDITDATA was too big to store all the initial properties; needed %zu bytes, but %d is the maximum possible."), newEdPtrSize, UINT16_MAX);
		return NULL;
	}

	// Allocate memory and zero-fill
	HGLOBAL globalPtr = GlobalAlloc(GPTR, newEdPtrSize);
	if (globalPtr == NULL)
	{
		MsgBox::Error(_T("GlobalAlloc failed"), _T("Failed to allocate %zu bytes for data during upgrade."), newEdPtrSize);
		return NULL;
	}

	EDITDATA * newEdPtr = (EDITDATA *)GlobalLock(globalPtr);
	newEdPtr->eHeader.extPrivateData = newOffset;
	newEdPtr->eHeader.extVersion = Extension::Version;
	newEdPtr->eHeader.extSize = newEdPtrSize;
	newEdPtr->eHeader.extID = oldEdPtr ? oldEdPtr->eHeader.extID : 0;
	newEdPtr->eHeader.extMaxSize = std::max(newEdPtrSize, oldEdPtr ? oldEdPtr->eHeader.extMaxSize : 0);

	DLL::PropAccesser &newEdPtrProps = Elevate(newEdPtr->Props);

	newEdPtrProps.propVersion = 'DAR1';
	newEdPtrProps.hash = ~Edif::SDK->jsonPropsNameAndTypesHash;
	newEdPtrProps.hashTypes = ~Edif::SDK->jsonPropsTypesHash;
	newEdPtrProps.sizeBytes = newEdPtrSize;
	newEdPtrProps.numProps = (std::uint16_t)numPropsIncludingStatics;

	// Pre-properties data copy over.
	if (oldOffset < newOffset &&
		memcpy_s(newEdPtr, newEdPtrSize, oldEdPtr, oldOffset))
	{
		MsgBox::Error(_T("memcpy failed"), _T("Failed to copy data during conversion. Error %u. Attempted to copy %u bytes to space for %u."),
			errno, oldOffset, newEdPtrSize - newOffset);
		GlobalUnlock(globalPtr);
		GlobalFree(globalPtr);
		return NULL;
	}

	// Margin between pre-old properties and new properties is left zeroed.

	// Properties data.
	if (memcpy_s(newEdPtrProps.dataForProps, dataToWriteString.size(), dataToWriteString.data(), dataToWriteString.size()))
	{
		MsgBox::Error(_T("memcpy failed"), _T("Failed to copy data during conversion. Error %u. Attempted to copy %u bytes to space for %u."),
			errno, dataToWriteString.size(), newEdPtrSize - newOffset);
		GlobalUnlock(globalPtr);
		GlobalFree(globalPtr);
		return NULL;
	}

#if 0
	FILE * fil = fopen("D:\\writeData.txt", "wb");
	assert(fil != NULL);
	unsigned char utf8Bom[] = { 0xEF, 0xBB, 0xBF };
	assert(fwrite(utf8Bom, 1, 3, fil) == 3);
	assert(fwrite(newEdPtr, 1, newEdPtrSize, fil) == newEdPtrSize);
	char sep[] = "\r\n\r\nByte data:\r\n";
	assert(fwrite(sep, 1, sizeof(sep) - 1, fil) == sizeof(sep) - 1);

	std::stringstream str2;
	auto Dump = [&](void * v, size_t s) {
		std::map<char, char *> unprintable{
			{ '\0', "NUL"},
			{ 1, "SOH", },
			{ 2, "STX", },
			{ 3, "ETX", },
			{ 4, "EOT", },
			{ 5, "ENQ", },
			{ 6, "ACK" },
			{ 7, "BEL", },
			{ 8, "BS", },
			{ '\t', "TAB", },
			{ '\n', "LF"},
			{ 11, "VT" },
			{ 12, "FF"},
			{ '\r', "CR"},
			{ 14, "SO"},
			{ 15, "SI"},
			{ 16, "DLE"},
			{ 17, "DC1"},
			{ 18, "DC2"},
			{ 19, "DC3"},
			{ 20, "DC4"},
			{ 21, "NAK"},
			{ 22, "SYN"},
			{ 23, "ETB"},
			{ 24, "CAN"},
			{ 25, "EM"},
			{ 26, "SUB"},
			{ 27 , "ESC"},
			{ 28, "FS"},
			{ 29, "GS"},
			{ 30, "RS"},
			{ 31, "US"},
			{ ' ', "space"},
			{ 127, "DEL"},
		};
		char * c = (char *)v;
		for (size_t i = 0; i < s; i++)
		{
			str2 << std::dec << std::setfill('0') << std::setw(2) << i << ", ";
			if (!c[i])
				str2 << "NUL";
			else
			{
				auto a = unprintable.find(c[i]);
				if (a != unprintable.end())
					str2 << a->second;
				else
					str2 << c[i];
			}
			str2 << ", 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)(((unsigned char *)c)[i]) << "\n";
		}
	};
	Dump(newEdPtr, newEdPtrSize);
	std::string str = str2.str();
	assert(fwrite(str.c_str(), 1, str.size(), fil) == str.size());
	fclose(fil);
#endif

	GlobalUnlock(globalPtr);

	return globalPtr;


#if 0

	bool reset = false, warning = false;

	// New object and defaults aren't in EDITDATA yet
	if (edPtr->Props.sizeBytes == 0)
	{
		reset = true;
	}

	// EDITDATA version is out of date. Let's convert!
	if (edPtr->eHeader.extVersion > Extension::Version)
	{
		// Um, slight wrench in the plans...
		auto localConverter = edPtr->Props.converter;
		if (localConverter == NULL)
			localConverter = SmartExtConvert;

		EDITDATA * newEdPtr = NULL;
		if (localConverter(mV, &edPtr->Props, newEdPtr))
		{
			void * v = mvReAllocEditData(mV, edPtr, _msize(newEdPtr));
			if (v == NULL)
			{
				DarkEdif::MsgBox::Error(_T("Upgrading properties failed"), _T("Out of memory."));
				return FALSE;
			}
			edPtr = (EDITDATA *)v;
			return TRUE;
		}

		return FALSE;
	}
	// EDITDATA has data, but JSON properties have changed, data probably needs porting
	else if (SDK->jsonPropsNameAndTypesHash != edPtr->Props.hash)
	{


		// JSON properties have changed since MFA was saved, yet Extension::Version remains the same.
		// This is potentially a bad scenario:
		// 1) Ext dev corrected some typos in Properties, no rearranging or type changes.
		// ---> In this scenario, the property data remains the same, so no change is needed.
		// 2) Ext dev DID change things, but forgot to increment Extension::Version.
		// ---> This is bad, because the converter function won't be run unless version has changed.
		bool jsonIDsSame = ::SDK->jsonPropsTypesHash == edPtr->Props.hashTypes;

		// Welp, it's definitely not scenario #1.
		// We need to convert the properties.
		if (!jsonIDsSame)
		{
			DarkEdif::MsgBox::WarningOK(_T("Version Upgrade Failure"), _T("MFA Extension Version is %i, but JSON properties have changed.\n")
				_T("Please change Extension::Version in Extension.h to %i, or save the MFA to reset the properties to defaults."),
				edPtr->eHeader.extVersion, edPtr->eHeader.extVersion + 1);
			DarkEdif::BreakIfDebuggerAttached();
		}


		if (edPtr->Props.converter == NULL)
		{
			reset = true;
			warning = true;
		}
	}
	else if (edPtr->eHeader.extVersion < Extension::Version)
	{
		MessageBoxA(mV->HEditWin, "Had to reset properties for object.", "DarkEdif - Warning", MB_ICONWARNING);
		LoadDefaultsFromJSON(mV, edPtr);
	}

	// Call this when the property affects the display of the object.
	mvInvalidateObject(mV, edPtr);
#endif

	return NULL;
}

DarkEdif::Properties::ConverterState::ConverterState(EDITDATA* edPtr, const json_value& json)
	: jsonProps(json), oldEdPtr(edPtr), numPropsReset(0U)
{
	if (edPtr && edPtr->eHeader.extPrivateData != 0)
		oldEdPtrProps = &GetRealPropsAddress(edPtr);
	else
		oldEdPtrProps = nullptr;
}

#endif // EditorBuild


// Returns property checked or unchecked from property name.
bool DarkEdif::Properties::IsPropChecked(std::string_view propName) const
{
	const auto& p = Elevate(*this);
	const Properties::Data* data = p.Internal_FirstData();
	int index = 0;
	while (data && index < p.numProps)
	{
		if (data->ReadPropName() == propName)
			goto found;
		data = data->Next();
		++index;
	}

	MsgBox::Error(_T("DarkEdif property error"), _T("IsPropChecked() error; property name \"%s\" does not exist."), UTF8ToTString(propName).c_str());
	return false;

found:
#if EditorBuild
	if (data->propTypeID != Edif::Properties::IDs::PROPTYPE_LEFTCHECKBOX &&
		CurLang["Properties"][index]["CheckboxDefaultState"].type == json_none)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("IsPropChecked() name = \"%s\" does not have a checkbox in the JSON."), UTF8ToTString(propName).c_str());
		return false;
	}
#endif
	return IsPropChecked(index);
}
// Returns property checked or unchecked from property ID.
bool DarkEdif::Properties::IsPropChecked(int propID) const
{
	if (propID >= this->numProps)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("Can't read property checkbox for property ID %d, the valid ID range is 0 to %hu."), propID, this->numProps);
		return false;
	}
	// The dataForProps consists of a set of chars, whereby each bit in the char is the "checked"
	// value for the Prop ID specified. Thus each char supports 8 properties.
	int byteIndex = propID / CHAR_BIT, bitIndex = propID % CHAR_BIT;
	return (Elevate(*this).dataForProps[byteIndex] >> bitIndex) & 1;
}
// Returns std::tstring property setting from property name.
std::tstring DarkEdif::Properties::GetPropertyStr(std::string_view propName) const
{
	const auto& p = Elevate(*this);
	const Properties::Data* data = p.Internal_FirstData();
	int index = 0;
	while (data && index < p.numProps)
	{
		if (data->ReadPropName() == propName)
			goto found;
		data = data->Next();
		++index;
	}

	MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyStr() error; property name \"%s\" does not exist."), UTF8ToTString(propName).c_str());
	return std::tstring();

found:
	std::tstring ret;
	switch (data->propTypeID)
	{
	case Edif::Properties::IDs::PROPTYPE_EDIT_STRING:
	case Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE:
	case Edif::Properties::IDs::PROPTYPE_FILENAME:
	case Edif::Properties::IDs::PROPTYPE_PICTUREFILENAME:
	case Edif::Properties::IDs::PROPTYPE_DIRECTORYNAME:
	case Edif::Properties::IDs::PROPTYPE_COMBOBOX:
	case Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN:
	case Edif::Properties::IDs::PROPTYPE_ICONCOMBOBOX:
		ret = UTF8ToTString(std::string_view((const char*)data->ReadPropValue(), data->ReadPropValueSize()));
#ifndef _WIN32
		// On non-Windows, the multiline editbox is stored as CRLF, and we want LF newlines instead
		if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE)
			ret.erase(std::remove(ret.begin(), ret.end(), _T('\r')), ret.end());
#endif
		return ret;
	default:
		MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyStr(name = \"%s\") is not a string property."), UTF8ToTString(propName).c_str());
		return std::tstring();
	}
}
// Returns std::tstring property string from property ID.
std::tstring DarkEdif::Properties::GetPropertyStr(int propID) const
{
	const auto& p = Elevate(*this);
	const Properties::Data * data = p.Internal_DataAt(propID);
	if (!data)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyStr() error; property ID %d does not exist."), propID);
		return std::tstring();
	}
	std::tstring ret;
	switch (data->propTypeID)
	{
	case Edif::Properties::IDs::PROPTYPE_EDIT_STRING:
	case Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE:
	case Edif::Properties::IDs::PROPTYPE_FILENAME:
	case Edif::Properties::IDs::PROPTYPE_PICTUREFILENAME:
	case Edif::Properties::IDs::PROPTYPE_DIRECTORYNAME:
	case Edif::Properties::IDs::PROPTYPE_COMBOBOX:
	case Edif::Properties::IDs::PROPTYPE_COMBOBOXBTN:
	case Edif::Properties::IDs::PROPTYPE_ICONCOMBOBOX:
		ret = UTF8ToTString(std::string_view((const char*)data->ReadPropValue(), data->ReadPropValueSize()));
#ifndef _WIN32
		// On non-Windows, the multiline editbox is stored as CRLF, and we want LF newlines instead
		if (data->propTypeID == Edif::Properties::IDs::PROPTYPE_EDIT_MULTILINE)
			ret.erase(std::remove(ret.begin(), ret.end(), _T('\r')), ret.end());
#endif
		return ret;
	default:
		MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyStr(ID = %d) is not a string property."), propID);
		return std::tstring();
	}
}

// Returns a float property setting from property name.
float DarkEdif::Properties::GetPropertyNum(std::string_view propName) const
{
	const auto& p = Elevate(*this);
	const Properties::Data* data = p.Internal_FirstData();
	while (data)
	{
		if (data->ReadPropName() == propName)
			goto found;
		data = data->Next();
	}

	MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum() error; property name \"%s\" does not exist."), UTF8ToTString(propName).c_str());
	return 0.f;

found:
	float ret;
	switch (data->propTypeID)
	{
	case Edif::Properties::IDs::PROPTYPE_EDIT_NUMBER:
	case Edif::Properties::IDs::PROPTYPE_COLOR:
	case Edif::Properties::IDs::PROPTYPE_SLIDEREDIT:
	case Edif::Properties::IDs::PROPTYPE_SPINEDIT:
	case Edif::Properties::IDs::PROPTYPE_DIRCTRL:
		// Integer prop; but since they have a low precision, we'll return as float
		if (data->ReadPropValueSize() != 4)
			MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(name = \"%s\") is a numeric property, but has an unexpected size."), UTF8ToTString(propName).c_str());
		ret = (float)*(int*)data->ReadPropValue();
		return ret;
	case Edif::Properties::IDs::PROPTYPE_EDIT_FLOAT:
	case Edif::Properties::IDs::PROPTYPE_SPINEDITFLOAT:
		if (data->ReadPropValueSize() != 4)
			MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(name = \"%s\") is a numeric property, but has an unexpected size."), UTF8ToTString(propName).c_str());
		ret = *(float*)data->ReadPropValue();
		return ret;
	default:
		DarkEdif::MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(name = \"%s\") is not a numeric property."), UTF8ToTString(propName).c_str());
		return 0.f;
	}
}
// Returns float property setting from a property ID.
float DarkEdif::Properties::GetPropertyNum(int propID) const
{
	const auto& p = Elevate(*this);
	const Properties::Data* data = p.Internal_DataAt(propID);
	if (!data)
	{
		MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum() error; property ID %d does not exist."), propID);
		return 0.f;
	}
	float ret;
	switch (data->propTypeID)
	{
	case Edif::Properties::IDs::PROPTYPE_EDIT_NUMBER:
	case Edif::Properties::IDs::PROPTYPE_COLOR:
	case Edif::Properties::IDs::PROPTYPE_SLIDEREDIT:
	case Edif::Properties::IDs::PROPTYPE_SPINEDIT:
	case Edif::Properties::IDs::PROPTYPE_DIRCTRL:
		// Integer prop; but since they have a low precision, we'll return as float
		if (data->ReadPropValueSize() != 4)
			MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(ID = %d) is a numeric property, but has an unexpected size."), propID);
		ret = (float)*(int *)data->ReadPropValue();
		return ret;
	case Edif::Properties::IDs::PROPTYPE_EDIT_FLOAT:
	case Edif::Properties::IDs::PROPTYPE_SPINEDITFLOAT:
		if (data->ReadPropValueSize() != 4)
			MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(ID = %d) is a numeric property, but has an unexpected size."), propID);
		ret = *(float *)data->ReadPropValue();
		return ret;
	default:
		MsgBox::Error(_T("DarkEdif property error"), _T("GetPropertyNum(ID = %d) is not a numeric property."), propID);
		return 0.f;
	}
}


const Properties::Data * DarkEdif::Properties::Internal_FirstData() const
{
	return (const Data *)(dataForProps + (int)std::ceil(((float)numProps) / 8.0f));
}
const Properties::Data * DarkEdif::Properties::Internal_DataAt(int ID) const
{
	const json_value& j = CurLang["Properties"];
	if (j.type != json_array)
	{
		MsgBox::Error(_T("Premature function call"), _T("Internal_DataAt() const called for prop ID %u without JSON properties being valid."), ID);
		return nullptr;
	}

	// If it's an unchangable property, no data to return anyway
	// Note there is a Data made for unchangeable, but they have a name length of 0, and no data.
	if (IsUnchangeablePropInclCheckbox((const char *)j[ID]["Type"]))
		return nullptr;

	const Data * data = Internal_FirstData();
	for (size_t i = 0; i < (size_t)ID; i++)
	{
		DebugProp_OutputString(_T("Locating ID %d, at %zu: type %s, title: %s.\n"), ID, i, UTF8ToTString((const char *)j[ID]["Type"]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
		data = data->Next();
	}

	DebugProp_OutputString(_T("DataAt ID %d type %s, title: %s.\n"), ID, UTF8ToTString((const char *)j[ID]["Type"]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
	return data;
}
#if EditorBuild
Properties::Data * DarkEdif::Properties::Internal_FirstData()
{
	return (Data *)(dataForProps + (int)ceil(((float)numProps) / 8.0f));
}
Properties::Data * DarkEdif::Properties::Internal_DataAt(int ID)
{
	if (ID >= this->numProps)
	{
		DebugProp_OutputString(_T("DataAt ID %u was not found.\n"), ID);
		return nullptr;
	}

	const json_value& j = CurLang["Properties"];
	if (j.type != json_array)
	{
		MsgBox::Error(_T("Premature function call"), _T("Internal_DataAt() called for prop ID %u without DarkEdif_Props being valid."),
			ID, DarkEdif::JSON::LanguageName());
		return nullptr;
	}

	// If it's an unchangable property, no data to return anyway
	// Note there is a Data made for unchangeable, but they have a name length of 0, and no data.
	if (IsUnchangeablePropInclCheckbox((const char *)j[ID]["Type"]))
		return nullptr;

	Data * data = Internal_FirstData();
	for (size_t i = 0; i < (size_t)ID && data; i++)
	{
		DebugProp_OutputString(_T("Locating ID %u, at %u: type %s, title: %s.\n"), ID, i, UTF8ToTString((const char *)j[ID]["Type"]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
		data = data->Next();
	}

	DebugProp_OutputString(_T("DataAt ID %u type %s, title: %s.\n"), ID, UTF8ToTString((const char *)j[ID]["Type"]).c_str(), UTF8ToTString(data->ReadPropName()).c_str());
	return data;
}
#endif

#endif // NOPROPS

#if EditorBuild

void DarkEdif::DLL::GeneratePropDataFromJSON()
{
	std::vector <PropData> VariableProps;
	PropData* CurrentProperty;
	const TCHAR* errorPrefix = _T("DarkEdif JSON property parser");

	const json_value& props = CurLang["Properties"];

	std::vector<const char*> openFolderList;
	for (size_t i = 0; i < props.u.array.length; ++i)
	{
		const json_value& Property = props[i];
		CurrentProperty = nullptr;

		// Category shift. Currently not supported.
		// When implemented, the JSON will look like
		// "Properties": {
		//   "General",
		//   { ... }, // general props
		//   "Display",
		//   { ... }, // display props
		// }
		if (Property.type == json_string)
		{
			DarkEdif::MsgBox::Error(errorPrefix, _T("Properties contains a string \"%s\" instead of an object. ")
				_T("Multiple categories of properties not currently implemented. (%s)"),
				DarkEdif::UTF8ToTString((const char*)Property).c_str(), JSON::LanguageName());
			continue;
		}
		const char* PropertyType = Property["Type"];

		// Reserved/invalid property types are marked with ! at the start.
		// If some muppet attempts to use 'em, throw an error.
		if (PropertyType[0] == '!')
		{
			DarkEdif::MsgBox::Error(errorPrefix,
				_T("You have specified an invalid Parameter type \"%s\". These types are reserved. (%s)"),
				DarkEdif::UTF8ToTString(PropertyType).c_str(), DarkEdif::JSON::LanguageName());
			continue;
		}
#define SetAllProps(opt,lParams) CurrentProperty->SetAllProperties(Options|(opt), (LPARAM)(lParams)); break
		using namespace Edif::Properties;

		// Custom Parameter: Read the number CustomXXX and use that.
		if (!_strnicmp(PropertyType, "Custom", 6))
		{
			DarkEdif::MsgBox::Info(errorPrefix, _T("Detected a custom property."));
			CurrentProperty = new PropData(VariableProps.size(), i + PROPTYPE_LAST_ITEM);
		}
		else if (!_stricmp(PropertyType, Names[PROPTYPE_FOLDER_END]))
		{
			CurrentProperty = new PropData(-1, PROPTYPE_FOLDER_END);
			if (openFolderList.empty())
				DarkEdif::MsgBox::Error(errorPrefix, _T("Too many FolderEnd properties! Extra one found at index %i."), i);
			else
				openFolderList.erase(openFolderList.cend() - 1);
		}
		else // Regular Parameter
		{

			// Loop through Parameter names and compareth them.
			for (size_t j = PROPTYPE_FIRST_ITEM; j < PROPTYPE_LAST_ITEM; ++j)
			{
				if (!_stricmp(PropertyType, Names[j]))
				{
					// Unicode Properties have IDs 1000 greater than their ANSI equivalents.
#ifdef _UNICODE
					CurrentProperty = new PropData(VariableProps.size(), j + 1000);
#else
					CurrentProperty = new PropData(VariableProps.size(), j);
#endif
					break;
				}
			}

			if (!CurrentProperty)
			{
				DarkEdif::MsgBox::Error(errorPrefix, _T("Property index %zu \"%hs\" has an unrecognised property type \"%hs\" in the JSON (under %s language).\n")
					_T("Check your spelling of the \"Type\" Parameter."), i, (const char*)Property["Title"], PropertyType, JSON::LanguageName());
				continue;
			}
			// If checkbox is enabled, pass that as flags as well.
			unsigned int Options =
				(Property["CheckboxDefaultState"].type != json_none ? PROPOPT_CHECKBOX : 0)		// Checkbox enabled by property option in JSON
				| (bool(Property["Bold"]) ? PROPOPT_BOLD : 0)				// Bold enabled by property option in JSON
				| (bool(Property["Removable"]) ? PROPOPT_REMOVABLE : 0)		// Removable enabled by property option in JSON
				| (bool(Property["Renameable"]) ? PROPOPT_RENAMEABLE : 0)	// Renamable enabled by property option in JSON
				| (bool(Property["Moveable"]) ? PROPOPT_MOVABLE : 0)		// Movable enabled by property option in JSON
				| (bool(Property["List"]) ? PROPOPT_LIST : 0)				// List enabled by property option in JSON
				| (bool(Property["SingleSelect"]) ? PROPOPT_SINGLESEL : 0);	// Single-select enabled by property option in JSON
			bool EnableLParams = false;

			CurrentProperty->Title = Edif::ConvertString((const char*)Property["Title"]);
			CurrentProperty->Info = Edif::ConvertString((const char*)Property["Info"]);

			switch (CurrentProperty->Type_ID % 1000)
			{
				// Simple static text
			case PROPTYPE_STATIC:
				SetAllProps(0, NULL);

				// Folder
			case PROPTYPE_FOLDER:
			{
				openFolderList.push_back(Property["Title"]);
				SetAllProps(0, NULL);
			}
			// FolderEnd handled outside of this switch, in the if() above

			// Edit button, Params1 = button text, or nullptr if Edit
			case PROPTYPE_EDITBUTTON:
			{
				if (Property["DefaultState"] == "") {
					SetAllProps(0, NULL);
				}
				else {
					SetAllProps(PROPOPT_PARAMREQUIRED, Edif::ConvertString((const char*)Property["DefaultState"]));
				}
			}

			// Edit box for strings, Parameter = max length
			case PROPTYPE_EDIT_STRING:
			case PROPTYPE_EDIT_MULTILINE:
			case PROPTYPE_DIRECTORYNAME:
			{
				if (CurrentProperty->Type_ID % 1000 == PROPTYPE_EDIT_STRING)
				{
					Options |= ((!_stricmp(Property["Case"], "Lower")) ? PROPOPT_EDIT_LOWERCASE : 0)
						| ((!_stricmp(Property["Case"], "Upper")) ? PROPOPT_EDIT_UPPERCASE : 0)
						| ((Property["Password"]) ? PROPOPT_EDIT_PASSWORD : 0);
				}
				else
				{
					if (Property["Case"].type != json_none)
						DarkEdif::MsgBox::WarningOK(_T("DarkEdif JSON property"), _T("Property \"%hs\" is set to \"Case\"=\"%s\", but that won't work with property type %hs."), (const char*)Property["Title"], (const char*)Property["Case"], Names[CurrentProperty->Type_ID % 1000]);
					if (Property["Password"].type != json_none)
						DarkEdif::MsgBox::WarningOK(_T("DarkEdif JSON property"), _T("Property \"%hs\" is set to password mask, but that won't work with property type %hs."), (const char*)Property["Title"], Names[CurrentProperty->Type_ID % 1000]);
				}
				if (Property["MaxLength"].type != json_integer) {
					SetAllProps(0, NULL);
				}
				else
				{
					int* textLength = new int;
					*textLength = (int)(json_int_t)Property["MaxLength"];
					SetAllProps(PROPOPT_PARAMREQUIRED, textLength);
				}
			}

			// Edit box for numbers (does not support min/max)
			case PROPTYPE_EDIT_NUMBER:
			case PROPTYPE_COLOR:
				SetAllProps(0, NULL);

			// Edit box for slider or spin control, Parameters = min value, max value
			case PROPTYPE_SPINEDIT:
			case PROPTYPE_SLIDEREDIT:
			{
				int* temp = new int[2];
				temp[0] = ((json_int_t)Property["Minimum"]) & 0xFFFFFFFF;
				temp[1] = ((json_int_t)Property["Maximum"]) & 0xFFFFFFFF;
				SetAllProps(PROPOPT_PARAMREQUIRED, temp);
			}

			// Combo box, Parameters = list of strings, options (sorted, etc)
			case PROPTYPE_COMBOBOX:
			case PROPTYPE_COMBOBOXBTN:
			{
				if (Property["Items"].type != json_array || Property["Items"].u.array.length == 0)
					DarkEdif::MsgBox::Error(_T("DarkEdif JSON property"), _T("No Items detected in combobox property %hs."), (const char*)Property["Title"]);

				const TCHAR** Fixed = new const TCHAR * [Property["Items"].u.array.length + 2];

				// NULL is required at start of array
				Fixed[0] = Fixed[Property["Items"].u.array.length + 1] = nullptr;

				// Use incrementation and copy to fixed list.
				for (unsigned int index = 1; index < Property["Items"].u.array.length + 1; ++index)
					Fixed[index] = Edif::ConvertString((const char*)Property["Items"][index - 1]);

				// Pass fixed list as Parameter
				SetAllProps(PROPOPT_PARAMREQUIRED, (LPARAM)Fixed);
			}

			// Size
			case PROPTYPE_SIZE:
				SetAllProps(0, NULL);

			// Checkbox
			case PROPTYPE_LEFTCHECKBOX:
				// Enforce option to show it is a checkbox
				SetAllProps(PROPOPT_CHECKBOX, NULL);

			// Direction Selector
			case PROPTYPE_DIRCTRL:
				SetAllProps(0, NULL);

			// Group
			case PROPTYPE_GROUP:
				SetAllProps(0, NULL);

			// Edit box + browse file button, Parameter = FilenameCreateParams
			case PROPTYPE_FILENAME:
			case PROPTYPE_PICTUREFILENAME:
				SetAllProps(0, NULL);

			// Font dialog box
			case PROPTYPE_FONT:
				SetAllProps(0, NULL);

			// Edit box for floating point numbers (does not support min/max)
			case PROPTYPE_EDIT_FLOAT:
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

				// Edit + Spin, value = floating point number, Parameters = min value, max value
			case PROPTYPE_SPINEDITFLOAT:
				SetAllProps(0, NULL);

				// Unrecognised
			default:
				DarkEdif::MsgBox::Error(errorPrefix, _T("The Parameter type \"%hs\" was unrecognised."),
					(const char*)Property["Type"]);
				SetAllProps(0, NULL);
			}
		}

		// Add to properties
		VariableProps.push_back(*CurrentProperty);
	}

	Edif::SDK->EdittimeProperties = std::make_unique<PropData[]>(VariableProps.size() + 1);
	// Use incrementation and copy to fixed list.
	for (size_t l = 0; l < VariableProps.size(); ++l)
		Edif::SDK->EdittimeProperties[l] = VariableProps[l];

	// End with completely null PropData
	memset(&Edif::SDK->EdittimeProperties[VariableProps.size()], 0, sizeof(PropData));

	if (!openFolderList.empty())
	{
		DarkEdif::MsgBox::Error(_T("JSON verification error"), _T("There was a Folder [%hs] that does not have a FolderEnd property."),
			openFolderList.back());
	}

	// Generate hash of JSON props and IDs, in case JSON is changed and
	// EDITDATA properties are now invalid.
	std::stringstream hashNamesAndTypes, hashTypes;
	for (size_t i = 0; i < props.u.array.length; i++)
	{
		const json_value& p = *props.u.array.values[i];

		// Don't include unchangeable props in the hash. Note there are still Data for those.
		if (IsUnchangeablePropExclCheckbox(p["Type"]))
			continue;

		// Lowercase the name in case there's a simple typo.
		std::string propName = (const char*)p["Title"];
		std::transform(propName.begin(), propName.end(), propName.begin(), ::tolower);

		size_t propTypeID = Edif::SDK->EdittimeProperties[i].Type_ID % 1000;

		hashNamesAndTypes << propName << '|' << propTypeID << " | "sv;
		hashTypes << propTypeID << '|';
	}

	Edif::SDK->jsonPropsNameAndTypesHash = fnv1a(hashNamesAndTypes.str());
	Edif::SDK->jsonPropsTypesHash = fnv1a(hashTypes.str());
}
#endif

// Returns size of EDITDATA and all properties if they were using their default values from JSON
std::uint16_t DarkEdif::DLL::Internal_GetEDITDATASizeFromJSON()
{
	const json_value& JSON = CurLang["Properties"];
	size_t fullSize = sizeof(EDITDATA);
	// Store one bit per property, for checkboxes
	fullSize += (int)std::ceil(JSON.u.array.length / 8.0f);

	DebugProp_OutputString(_T("Calulation: started with EDITDATA size %zu, checkbox size %d, accumulative size is now %zu.\n"), sizeof(EDITDATA), (int)std::ceil(JSON.u.array.length / 8.0f), fullSize);

	for (auto i = 0u; i < JSON.u.array.length; ++i)
	{
		const json_value& propjson = *JSON.u.array.values[i];
		const char* curPropType = propjson["Type"];

		// Metadata for all properties
		fullSize += sizeof(Properties::Data);

		// No title for unchangeable props, we won't need to smart rearrange those properties
		// by title, so storing the title is unnecessary.
		if (IsUnchangeablePropExclCheckbox(curPropType))
		{
			DebugProp_OutputString(_T("Adding property %hs (type %hs) accumulative size is now %zu.\n"), (const char*)propjson["Title"], curPropType, fullSize);
			continue;
		}

		fullSize += strnlen(propjson["Title"], 254);
		// Don't bother checking title length. Other funcs will do that.

		// Checkboxes store title, but not data
		if (IsUnchangeablePropInclCheckbox(curPropType))
		{
			DebugProp_OutputString(_T("Adding property %hs (type %hs) accumulative size is now %zu.\n"), (const char*)propjson["Title"], curPropType, fullSize);
			continue;
		}

		// Stores text
		if (!_strnicmp(curPropType, "Editbox String", sizeof("Editbox String") - 1) || !_strnicmp(curPropType, "Combo Box", sizeof("Combo Box") - 1) ||
			!_stricmp(curPropType, "Editbox File") || !_stricmp(curPropType, "Editbox Image File") ||
			!_stricmp(curPropType, "URL button") || !_stricmp(curPropType, "Editbox Folder"))
		{
			const char* defaultText = propjson["DefaultState"];
			fullSize += (defaultText ? strlen(defaultText) : 0); // UTF-8
		}
		// Stores a number
		else if (!_stricmp(curPropType, "Editbox Number") || !_stricmp(curPropType, "Edit spin") || !_stricmp(curPropType, "Edit slider") ||
			!_stricmp(curPropType, "Color") || !_stricmp(curPropType, "Edit direction") || !_stricmp(curPropType, "Editbox Float") || !_stricmp(curPropType, "Edit spin float"))
		{
			fullSize += sizeof(int); // Floats are same size as int
		}
		// Stores two numbers
		else if (!_stricmp(curPropType, "Size"))
			fullSize += sizeof(int) * 2;
		else
		{
			DarkEdif::MsgBox::Error(_T("GetEDITDATASizeFromJSON failed"), _T("Calculation of edittime property size can't understand property type \"%s\". (%s)"),
				UTF8ToTString(curPropType).c_str(), DarkEdif::JSON::LanguageName());
		}
		DebugProp_OutputString(_T("Adding property %hs (type %s) accumulative size is now %zu.\n"), (const char*)propjson["Title"], UTF8ToTString(curPropType).c_str(), fullSize);
	}
	DebugProp_OutputString(_T("Accumulative size finalized at %zu.\n"), fullSize);

#if !defined(NOPROPS) && EditorBuild
	// This function estimates the size of a fresh edPtr without generating one.
	// To check it's working, generate an actual fresh edPtr and see if it's different
	if constexpr (DarkEdif::Properties::DebugProperties)
	{
		HGLOBAL data = DLL_UpdateEditStructure(Edif::SDK->mV, nullptr);
		if (!data)
			return std::numeric_limits<std::uint16_t>::max();
		EDITDATA* edPtr = (EDITDATA*)GlobalLock(data);
		if (!edPtr)
		{
			GlobalFree(data);
			return std::numeric_limits<std::uint16_t>::max();
		}
		std::uint32_t actualResult = edPtr->eHeader.extSize;
		GlobalUnlock(data);
		GlobalFree(data);

		if (fullSize != actualResult)
		{
			DarkEdif::MsgBox::Error(_T("GetEDITDATASizeFromJSON failed"), _T("Mismatch of calculation and actual: calculated %zu bytes, but the actual new EDITDATA was %zu bytes."),
				actualResult, fullSize);
		}
	}
#endif
	if (fullSize >= UINT16_MAX)
	{
		DarkEdif::MsgBox::Error(_T("EDITDATA too large!"), _T("The JSON extension properties use more than %u bytes to store, and are too large for Fusion. (%s)"),
			UINT16_MAX, DarkEdif::JSON::LanguageName());
	}

	DebugProp_OutputString(_T("GetEDITDATASizeFromJSON: result is %zu bytes.\n"), fullSize);

	return (std::uint16_t)fullSize;
}
// =====
// Get event number (CF2.5+ feature)
// =====


// Static definition; set during SDK::SDK()
#ifdef _WIN32
bool DarkEdif::IsFusion25;
#else
//constexpr bool DarkEdif::IsFusion25 = true;
#endif

// Returns the Fusion event number for this group. Works in CF2.5 and MMF2.0
std::uint16_t DarkEdif::GetEventNumber(eventGroup * evg) {
	// Windows may be 2.0 or 2.5; if 2.5, the local SDK's evgInhibit is where the identifier is.
	// Android/iOS is assumed to be 2.5 and should work directly.
#ifdef _WIN32
	if (DarkEdif::IsFusion25) {
		return evg->get_evgInhibit();
	}
#endif
	return evg->get_evgIdentifier();
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
	if (!ext->rhPtr->get_EventGroup())
		return -1;

	int eventNum = GetEventNumber(ext->rhPtr->get_EventGroup());
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
	return DarkEdifObjCFunc(PROJECT_NAME_RAW, getCurrentFusionEventNum)(ext->objCExtPtr);
#endif
}

// Returns path as-is if valid, an extracted binary file's path, or an error; error indicated by returning '>' at start of text
std::tstring DarkEdif::MakePathUnembeddedIfNeeded(const Extension * ext, const std::tstring_view filePath)
{
	if (filePath.empty())
		return _T(">File path was blank"s);
#if _WIN32
	std::tstring truePath(MAX_PATH + 1, _T(' '));
#ifdef _UNICODE
	if (Edif::SDK->mV->GetFileW(std::tstring(filePath).c_str(), truePath.data(), 0) == FALSE)
#else
	if (Edif::SDK->mV->GetFileA(std::tstring(filePath).c_str(), truePath.data(), 0) == FALSE)
#endif
		return _T(">mvGetFile returned false"s);
#elif defined(__ANDROID__)
	// Call `String darkedif_jni_makePathUnembeddedIfNeeded(String)` Java function
	static jmethodID getEventIDMethod;
	if (getEventIDMethod == nullptr)
	{
		jclass javaExtClass = threadEnv->GetObjectClass(ext->javaExtPtr);
		getEventIDMethod = threadEnv->GetMethodID(javaExtClass, "darkedif_jni_makePathUnembeddedIfNeeded", "(Ljava/lang/String;)Ljava/lang/String;");

		// This is a Java wrapper implementation failure and so its absence should be considered fatal
		if (getEventIDMethod == nullptr)
			LOGF("Failed to find CRun" PROJECT_NAME_UNDERSCORES "'s darkedif_jni_makePathUnembeddedIfNeeded method in Java wrapper file.\n");
	}

	jstring pathJava = CStrToJStr(std::string(filePath).c_str());
	JavaAndCString str;
	str.ctx = (jstring)threadEnv->CallObjectMethod(ext->javaExtPtr, getEventIDMethod, pathJava);
	str.ptr = mainThreadJNIEnv->GetStringUTFChars((jstring)str.ctx, NULL);
	const std::string truePath = str.ptr ? str.ptr : "";

	threadEnv->DeleteLocalRef(pathJava);
	threadEnv->DeleteLocalRef((jobject)str.ctx);
#else
	const std::string truePath = DarkEdifObjCFunc(PROJECT_NAME_RAW, makePathUnembeddedIfNeeded)(ext->objCExtPtr, std::string(filePath).c_str());
#endif
	if (filePath != truePath)
		LOGV(_T("File path extracted from \"%s\" to \"%s\".\n"), std::tstring(filePath).c_str(), truePath.c_str());
	else
		LOGV(_T("File path extraction of \"%s\" produced no change.\n"), truePath.c_str());
	return truePath;
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

std::tstring DarkEdif::ANSIToTString(const std::string_view input) {
	return WideToTString(ANSIToWide(input));
}
std::string DarkEdif::ANSIToUTF8(const std::string_view input) {
	return WideToUTF8(ANSIToWide(input));
}
std::wstring DarkEdif::ANSIToWide(const std::string_view input) {
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
std::string DarkEdif::UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return WideToANSI(UTF8ToWide(input), allValidChars);
}
std::tstring DarkEdif::UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true; // UTF-8 and UTF-16 share all chars
	return UTF8ToWide(input);
#else
	return UTF8ToANSI(input, allValidChars);
#endif
}
std::wstring DarkEdif::UTF8ToWide(const std::string_view input)
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
std::string DarkEdif::WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
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
std::tstring DarkEdif::WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	if (allValidChars)
		*allValidChars = true;
	return std::wstring(input);
#else
	return WideToANSI(input, allValidChars);
#endif
}
std::string DarkEdif::WideToUTF8(const std::wstring_view input)
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
std::string DarkEdif::TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
#ifdef _UNICODE
	return WideToANSI(input, allValidChars);
#else
	if (allValidChars)
		*allValidChars = true;
	return std::string(input);
#endif
}
std::string DarkEdif::TStringToUTF8(const std::tstring_view input) {
#ifdef _UNICODE
	return WideToUTF8(input);
#else
	return ANSIToUTF8(input);
#endif
}
std::wstring DarkEdif::TStringToWide(const std::tstring_view input) {
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

std::tstring DarkEdif::ANSIToTString(const std::string_view input) {
	return UTF8ToTString(input);
}
std::string DarkEdif::ANSIToUTF8(const std::string_view input) {
	return std::string(input);
}
std::wstring DarkEdif::ANSIToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string DarkEdif::UTF8ToANSI(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::string(input);
}
std::tstring DarkEdif::UTF8ToTString(const std::string_view input, bool * const allValidChars /* = nullptr */) {
	return std::tstring(input);
}
std::wstring DarkEdif::UTF8ToWide(const std::string_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}
std::string DarkEdif::WideToANSI(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::tstring DarkEdif::WideToTString(const std::wstring_view input, bool * const allValidChars /* = nullptr */) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::tstring();
}
std::string DarkEdif::WideToUTF8(const std::wstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::string();
}
std::string DarkEdif::TStringToANSI(const std::tstring_view input, bool * const allValidChars /* = nullptr */) {
	return TStringToUTF8(input);
}
std::string DarkEdif::TStringToUTF8(const std::tstring_view input) {
	return std::string(input);
}
std::wstring DarkEdif::TStringToWide(const std::tstring_view input) {
	assert(false && "Linux-based Wide not programmed yet.");
	return std::wstring();
}

#endif

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
int DarkEdif::MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	jclass toast = threadEnv->FindClass("android/widget/Toast");
	jobject globalContext = getGlobalContext();
	jmethodID methodMakeText = threadEnv->GetStaticMethodID(toast, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
	if (methodMakeText == NULL) {
		LOGE("toast.makeText not Found\n");
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

void DarkEdif::LOGFInternal(PrintFHintInside const TCHAR * x, ...)
{
	TCHAR buf[2048];
	va_list va;
	va_start(va, x);
	_vstprintf_s(buf, std::size(buf), x, va);
	va_end(va);
	DarkEdif::MsgBox::Error(_T("Fatal error"), _T("%s"), buf);
	std::abort();
}
#else // APPLE
void DarkEdif::BreakIfDebuggerAttached()
{
	__builtin_trap();
}

int DarkEdif::MessageBoxA(WindowHandleType hwnd, const TCHAR * text, const TCHAR * caption, int iconAndButtons)
{
	::DarkEdif::Log(iconAndButtons, "Message box \"%s\" absorbed: \"%s\".", caption, text);
	DarkEdif::BreakIfDebuggerAttached();
	return 0;
}

void DarkEdif::LOGFInternal(PrintFHintInside const TCHAR * x, ...)
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
	return FusionDebugAdmin.GetDebugTree(((RunObject*)rdPtr)->GetExtension());
}

// This routine returns the text of a given item.
void FusionAPI GetDebugItem(TCHAR *pBuffer, RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.GetDebugItem(((RunObject *)rdPtr)->GetExtension(), pBuffer, id);
}

// This routine allows the user to edit the debugger's editable items.
void FusionAPI EditDebugItem(RUNDATA *rdPtr, int id)
{
#pragma DllExportHint
	FusionDebugAdmin.StartEditForItemID(((RunObject*)rdPtr)->GetExtension(), id);
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
		if ((lastSlash = path.rfind(_T('\\'))) == std::tstring::npos)
			lastSlash = path.rfind(_T('/'));
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
		DarkEdifUpdateThread(Edif::SDK);
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
	if (Edif::SDK->Icon->GetWidth() != 32)
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
		if (Edif::SDK->Icon->HasAlpha())
		{
			LPBYTE alpha = Edif::SDK->Icon->LockAlpha();
			if (alpha != nullptr)
			{
				for (int i = rect.top; i < rect.bottom; i++)
					memset(&alpha[rect.left + (i * 32)], 0xFF, rect.right - rect.left);
				(Edif::SDK)->Icon->UnlockAlpha();
			}
		}

		(Edif::SDK)->Icon->Fill(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, fillColor);
	};

	if (extUpdateType == ExtUpdateType::Major)
	{
		const RECT greyBkgdRect { 1, 8, 30, 27 };
		FillBackground(greyBkgdRect, RGB(80, 80, 80));

		// For some reason there are margins added in by the font drawing technique;
		// we have to counter it by positioning each line manually.
		RECT textDrawRect = { greyBkgdRect.left + 1, greyBkgdRect.top - 1, 32, 32 };
		COLORREF textColor = RGB(240, 0, 0);
		Edif::SDK->Icon->DrawTextA("MAJOR", sizeof("MAJOR") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left -= 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
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

		Edif::SDK->Icon->DrawTextA("UPDATE", sizeof("UPDATE") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
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

		Edif::SDK->Icon->DrawTextA("REINSTL", sizeof("REINSTL") - 1,
			&textDrawRect, DT_NOPREFIX, textColor, font, BMODE_TRANSP, BOP_COPY, 0L, 1);

		textDrawRect.left += 1;
		textDrawRect.top += 6;
		Edif::SDK->Icon->DrawTextA("NEEDED", sizeof("NEEDED") - 1,
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
	if (mvIsUnicodeVersion(Edif::SDK->mV))
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
		{
			DarkEdif::MsgBox::Error(_T("Resource loading"), _T("Failed to set up ") PROJECT_NAME _T(" - access denied writing to Data\\Runtime MFX. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%20Fix%20Tool.exe"));
			std::abort();
		}
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
	DWORD ret;
	if ((ret = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Clickteam", &mainKey)) != ERROR_SUCCESS)
	{
		DarkEdif::MsgBox::Error(_T("Resource loading"), _T("UC tagging resource load failed. Error %u while loading registration resource for reading."), ret);
		FreeLibrary(readHandle);
		std::abort();
	}

	// Users have access to registry of local machine, albeit read-only, so this should not fail.
	regKey.resize(256);
	DWORD keySize = regKey.size() * sizeof(wchar_t), type;
	ret = RegQueryValueExW(mainKey, L"UCTag", NULL, &type, (LPBYTE)regKey.data(), &keySize);
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
	else // reg key is missing
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
			// Get lock; caller will release it at end of update check
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

		// Suppress warning C4996 about using getaddrinfo instead; we don't need a newer function
		// for Darkwire's simple DNS, and it'll be a lot more code for no obvious benefit.
#pragma warning (push)
#pragma warning (disable: 4996)
		struct hostent * host = gethostbyname(domain);
#pragma warning (pop)

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

		// Socket was closed without any content or error - can happen if Darkwire is down from power cut
		// We only allow ignoring errors of server being down and just report no update
		if (fullPage.empty())
		{
			if (handleWSAError(WSAETIMEDOUT))
			{
				updateLog << "Page result was empty."sv;
				updateLock = false;
			}
			return 1;
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
			pendingUpdateURL = DarkEdif::UTF8ToWide(pageBody.substr(0, pageBody.find('\n')));
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
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), noUpdate, sizeof(noUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::None;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody));

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
						DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure %u. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%%20Fix%%20Tool.exe"), GetLastError());
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
							DarkEdif::MsgBox::Error(_T("Tag failure"), _T("UC tagging failure. Try running the UCT Fix Tool, or run Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%%20Fix%%20Tool.exe"));
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
						err == ERROR_ACCESS_DENIED ? _T(" Try running the UCT Fix Tool, or running Fusion as admin.\n\nUCT fix tool:\nhttps://dark-wire.com/storage/UCT%20Fix%20Tool.exe") : _T(""));
				}
			}

			#endif

			return 0;
		};

		if (!_strnicmp(pageBody.c_str(), extDevUpdate, sizeof(extDevUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::ExtDevError;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(extDevUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), sdkUpdate, sizeof(sdkUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::SDKUpdate;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(sdkUpdate) - 1)));
			return 0;
		}
		if (!_strnicmp(pageBody.c_str(), majorUpdate, sizeof(majorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Major;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(majorUpdate) - 1)));
			return 0;
		}

		if (!_strnicmp(pageBody.c_str(), minorUpdate, sizeof(minorUpdate) - 1))
		{
			GetLockAnd(
				pendingUpdateType = DarkEdif::SDKUpdater::ExtUpdateType::Minor;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody.substr(sizeof(minorUpdate) - 1)));
			return 0;
		}

		if (handleWSAError(EINVAL))
		{
			#ifdef _DEBUG
				updateLog << "Can't interpret type of page:\n"sv << pageBody;
				pendingUpdateDetails = DarkEdif::UTF8ToWide(pageBody);
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
// On Android, this makes a small popup on bottom of the screen, called a toast notification
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
		MessageBox(DarkEdif::Internal_WindowHandle, _T("Failed to format a message box. Contact extension developer."), title.c_str(), MB_OK | MB_ICONERROR);
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
extern int aceIndex;
void DarkEdif::Log(int logLevel, PrintFHintInside const TCHAR * msgFormat, ...)
{
	va_list v;
	va_start(v, msgFormat);
	DarkEdif::LogV(logLevel, msgFormat, v);
	va_end(v);
}
void DarkEdif::LogV(int logLevel, PrintFHintInside const TCHAR* msgFormat, va_list v)
{
#ifdef _WIN32
	static TCHAR outputBuff[2048];
	int didTrunc = _vsntprintf_s(outputBuff, std::size(outputBuff), _TRUNCATE, msgFormat, v);

	// Truncated the log; trim it
	if (didTrunc == -1)
	{
		_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("... [truncated]")) - 2], _T("... [truncated]"));
		// add newline if format had it
		if (msgFormat[_tcslen(msgFormat) - 2] == _T('\r'))
			_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("\r\n"))], _T("\r\n"));
		else if (msgFormat[_tcslen(msgFormat) - 1] == _T('\n'))
			_tcscpy(&outputBuff[std::size(outputBuff) - std::size(_T("\n"))], _T("\n"));
	}

	OutputDebugString(outputBuff);
#elif defined(__ANDROID__)
	std::string msgFormatT = std::string(aceIndex, '>');
	msgFormatT += ' ';
	msgFormatT += msgFormat;
	__android_log_vprint(logLevel, PROJECT_NAME_UNDERSCORES, msgFormatT.c_str(), v);
#else // iOS
	static const char* logLevels[] = {
		"", "", "verbose", "debug", "info", "warn", "error", "fatal"
	};
	printf("%-9s", logLevels[logLevel]);
	vprintf(msgFormat, v);
#endif
}

#if (defined(__ANDROID__) || defined(__APPLE__)) && DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_INFO
void DarkEdif::OutputDebugStringAInternal(const char * debugString)
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
#endif // Android, iOS, and DarkEdif log level INFO or lower level

#ifndef _WIN32
// To get the Windows-like behaviour
void DarkEdif::Sleep(unsigned int milliseconds)
{
	if (milliseconds == 0)
		std::this_thread::yield();
	else
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif



// =====
// Embed the minified JSON file in Android and iOS
// =====

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

INCBIN(PROJECT_NAME_RAW, _darkExtJSON, "DarkExt.PostMinify.json");

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
