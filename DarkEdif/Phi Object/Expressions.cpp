#include "Common.hpp"

const TCHAR* Extension::Error()
{
	return Runtime.CopyString(errorList.empty() ? _T("") : errorList.front().c_str());
}

int Extension::Event_Index()
{
	return DarkEdif::GetCurrentFusionEventNum(this);
}

int Extension::Frame_IndexFromName(const TCHAR * name)
{
	if (name[0] == _T('\0'))
	{
		MakeError("Frame_IndexFromName: frame \"%s\" not found.", name);
		return -1;
	}

	auto res = std::find_if(frameNames.cbegin(), frameNames.cend(), [=](const std::tstring& n) { return !_tcsicmp(n.c_str(), name); });
	if (res == frameNames.cend())
	{
		MakeError("Frame_IndexFromName: frame \"%s\" not found.", name);
		return -1;
	}

	// 1-based
	return (int)(1 + std::distance(frameNames.cbegin(), res));
}

const TCHAR * Extension::Frame_NameFromIndex(int index)
{
	if (index < 0)
	{
		MakeError("Frame_NameFromIndex was passed a negative index (%i).", index);
		return Runtime.CopyString(_T(""));
	}
	if (index == 0)
	{
		MakeError("Frame_NameFromIndex was passed 0 as index, but the index parameter is 1-based.");
		return Runtime.CopyString(_T(""));
	}

	if ((size_t)index > frameNames.size())
	{
		MakeError("Frame_NameFromIndex was passed a index that was too high (max is %zu, you passed %i).",
			frameNames.size(), index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(frameNames[index - 1].c_str());
}


int Extension::Active_GetAnimFrameCount(int fixedValue, int animNum, int animDir)
{
	return -1; // not paid enough for this
}

int Extension::Memory_PhysicalTotal() {
	return physMemTotalMB;
}
int Extension::Memory_PhysicalFree() {
	return physMemFreeMB;
}
int Extension::Memory_PageFileTotal() {
	return pageFileMemTotalMB;
}
int Extension::Memory_PageFileFree() {
	return pageFileMemFreeMB;
}
int Extension::Memory_VirtualTotal() {
	return virtualMemTotalMB;
}
int Extension::Memory_VirtualFree() {
	return virtualMemFreeMB;
}

#ifdef _WIN32
extern std::string GetLastErrorAsString();
#elif defined(__ANDROID__)
#include <sys/statvfs.h>
#else
#include <sys/statvfs.h>
#endif
static std::uint64_t DIV = 1024 * 1024; // 1MB

std::uint32_t Extension::Disk_GetAvailableSpaceOfDriveInMB(const TCHAR* path)
{
#ifdef _WIN32
	std::tstring drive(path);
	// Both UNC and X:\ end with a '\'
	if (drive.size() < 3 || drive.back() != _T('\\'))
		goto dieearly;

	// Must be "X:\" format
	if (drive.size() == 3)
	{
		if (!_istalpha(drive[0]) || drive[1] != _T(':'))
			goto dieearly;
	}
	else // must be UNC format; e.g. "\\server\\folder\", etc.
	{
		if (drive[0] != _T('\\') || drive[1] != _T('\\'))
			goto dieearly;
	}

	// path should be OK

	ULARGE_INTEGER size;
	if (GetDiskFreeSpaceEx(drive.c_str(), &size, NULL, NULL) == FALSE)
	{
		MakeError("Couldn't look up size of drive \"%s\", error %u: %s.", path, GetLastError(), GetLastErrorAsString().c_str());
		return -1;
	}

	return (std::uint32_t)(size.QuadPart / DIV);

dieearly:
	MakeError("Couldn't look up avilable space in drive \"%s\", unexpected path format. Expecting \"X:\\\", or \"\\\\servername\\foldername\\\". Path must end with a \"\\\".", path);
	return -1;

#elif defined(__ANDROID__)

	// Worth noting API 21 is the minimum if you want 64-bit support
	// statvfs64() was added in API 20
#if __ANDROID_API__ >= 21
	struct statvfs64 data;
	int res = statvfs64(path, &data);
#else // 32-bit ARM
	struct statvfs data;
	int res = statvfs(path, &data);
#endif
	if (res == -1)
	{
		res = errno;
		MakeError("Couldn't look up avilable space of path %s, error num %d: %s.", path, res, strerror(res));
		return -1;
	}

	LOGI("Note: Result of statvfs for path \"%s\": blocks = %lu, bsize = %lu, frsize = %lu, bfree = %lu, bavail = %lu. Available: %lu bytes.\n",
		path, data.f_blocks, data.f_bsize, data.f_frsize, data.f_bfree, data.f_bavail, data.f_bfree * data.f_frsize);
	std::uint64_t freeSpace = ((((std::uint64_t)data.f_bfree) * data.f_frsize) / DIV);
	if (freeSpace > INT32_MAX)
	{
		MakeError("Couldn't look up available space of path %s; result is too large to fit in 32-bit to return to Fusion.", path);
		return -1;
	}
	return freeSpace;
#else

#if 0 &&( defined(__aarch64__) || defined(__x86_64__))
	struct statvfs64 data;
	int res = statvfs64(path, &data);
#else // 32-bit ARM
	struct statvfs data;
	int res = statvfs(path, &data);
#endif
	if (res == -1)
	{
		res = errno;
		MakeError("Couldn't look up avilable space of path %s, error num %d: %s.", path, res, strerror(res));
		return -1;
	}

	LOGI("Note: Result of statvfs for path \"%s\": blocks = %lu, bsize = %lu, frsize = %lu, bfree = %lu, bavail = %lu. Available: %lu bytes.\n",
		path, data.f_blocks, data.f_bsize, data.f_frsize, data.f_bfree, data.f_bavail, data.f_bfree * data.f_frsize);
	std::uint64_t freeSpace = ((((std::uint64_t)data.f_bfree) * data.f_frsize) / DIV);
	if (freeSpace > INT32_MAX)
	{
		MakeError("Couldn't look up available space of path %s; result is too large to fit in 32-bit to return to Fusion.", path);
		return -1;
	}
	return (int)freeSpace;
#endif
}
std::uint32_t Extension::Disk_GetTotalCapacityOfDriveInMB(const TCHAR * path)
{
#ifdef _WIN32
	std::tstring drive(path);
	// Both UNC and X:\ end with a '\'
	if (drive.size() < 3 || drive.back() != _T('\\'))
		goto dieearly;

	// Must be "X:\" format
	if (drive.size() == 3)
	{
		if (!_istalpha(drive[0]) || drive[1] != _T(':'))
			goto dieearly;
	}
	else // must be UNC format; e.g. "\\server\\folder\", etc.
	{
		if (drive[0] != _T('\\') || drive[1] != _T('\\'))
			goto dieearly;
	}

	// path should be OK

	ULARGE_INTEGER size;
	if (GetDiskFreeSpaceEx(drive.c_str(), NULL, &size, NULL) == FALSE)
	{
		MakeError("Couldn't look up total size of drive \"%s\", error %u: %s.", path, GetLastError(), GetLastErrorAsString().c_str());
		return -1;
	}

	return (std::uint32_t)(size.QuadPart / DIV);

	dieearly:
		MakeError("Couldn't look up total size of drive \"%s\", unexpected path format. Expecting \"X:\\\", or \"\\\\servername\\foldername\\\". Path must end with a \"\\\".", path);
		return -1;

#elif defined(__ANDROID__) || defined(__APPLE__)
	// 64-bit ARM, has statvfs64() as well as statvfs()
#if !defined(__APPLE__) && (defined(__aarch64__) || defined(__x86_64__))
	struct statvfs64 data;
	int res = statvfs64(path, &data);
#else // 32-bit ARM
	struct statvfs data;
	int res = statvfs(path, &data);
#endif
	if (res == -1)
	{
		res = errno;
		MakeError("Couldn't look up total size of path %s, error num %d: %s.", path, res, strerror(res));
		return -1;
	}

	LOGI("Note: Result of statvfs for path \"%s\": blocks = %lu, bsize = %lu, frsize = %lu, bfree = %lu, bavail = %lu. Total capacity: %lu.\n",
		path, data.f_blocks, data.f_bsize, data.f_frsize, data.f_bfree, data.f_bavail, data.f_blocks * data.f_frsize);
	std::uint64_t totalSpace = ((((std::uint64_t)data.f_blocks) * data.f_frsize) / DIV);
	if (totalSpace > INT32_MAX)
	{
		MakeError("Couldn't look up total capacity of path %s; result is too large to fit in 32-bit to return to Fusion.", path);
		return -1;
	}
	return (int)totalSpace;

	//std::uint64_t freeSpace = ((data.f_bfree * data.f_frsize) / DIV);
	//std::uint64_t availSpace = ((data.f_bavail * data.f_frsize) / DIV);

	//LOGI("Success: total space %lu mb, free space %lu mb, avail %lu mb.\n", totalSpace, freeSpace, availSpace);
	//LOGI("Success: bsize = %lu, frsize %lu, blocks %lu, bfree %lu, avail %lu.\n", data.f_bsize, data.f_frsize, data.f_blocks, data.f_bfree, data.f_bavail);
	//LOGI("Success: files = %lu, ffree %lu, favail %lu, fsid %lu, flag %lu, namemax %lu.\n", data.f_files, data.f_ffree, data.f_favail, data.f_fsid, data.f_flag, data.f_namemax);
#else
#error not coded
#endif
}

#ifdef _WIN32
extern AltVals* RunObject_GetVals(RunObject* ro);
#endif

#include <iomanip>

const TCHAR* Extension::GetAltValsFromObjName(const TCHAR* objectName, int altValueIndex, int numDecimalDigits)
{
	std::tstring objectNameStr(objectName);

	if (!objectNameStr[0])
	{
		MakeError("Cannot supply a blank object name as parameter.");
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (altValueIndex <= 0)
	{
		MakeError("Alterable values range from index 0+, but you supplied index %i.", altValueIndex);
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (!DarkEdif::IsFusion25 && altValueIndex > 25)
	{
		MakeError("Alterable values range from index 0-25 in Fusion 2.0, but you supplied index %i.", altValueIndex);
		return Runtime.CopyString(_T("<ERROR>"));
	}


#ifdef _WIN32
	std::tstring objectNameStrLengthLimited(objectNameStr);
	objectNameStrLengthLimited.resize(std::size(rhPtr->Frame->oiList[0].name) - 1);

	for (size_t i = 0; i < (size_t)rhPtr->NumberOi; i++)
	{
		// In case NumberOI no worky
		if (!rhPtr->Frame->oiList[i].name[0])
			break;

		// Found object by name
		if (!_tcsicmp(rhPtr->Frame->oiList[i].name, objectNameStrLengthLimited.c_str()))
		{
			std::vector<std::pair<std::int32_t, int>> longRes;
			std::vector<std::pair<std::double_t, int>> doubleRes;

			short j = rhPtr->Frame->oiList[i].Object;
			while (j >= 0)
			{
				RunObject* theObject = (RunObject *)rhPtr->ObjectList[j].oblOffset;
				if (!theObject)
					continue;

				AltVals* av = RunObject_GetVals(theObject);
				if (!av)
				{
					MakeError("Object %s doesn't have alterable values.", objectNameStr.c_str());
					return Runtime.CopyString(_T("<ERROR>"));
				}
				// theObject->roc.val
				if (DarkEdif::IsFusion25 && av->CF25.NumAltValues <= altValueIndex)
				{
					MakeError("Object %s doesn't have alterable value at index %i; max available alt value index is %i.",
						objectNameStr.c_str(), altValueIndex, av->CF25.NumAltValues - 1);
					return Runtime.CopyString(_T("<ERROR>"));
				}

				// av->Free1
				// int
				auto cValue = DarkEdif::IsFusion25 ? av->CF25.Values[altValueIndex] : av->MMF2.rvpValues[altValueIndex];

				enum types
				{
					Int = 0,
					Double = 2,
					String,
				};

				if (cValue.m_type == types::Int)
				{
					auto k = std::find_if(longRes.begin(), longRes.end(), [=](const std::pair<std::int32_t, int>& a) {
							return a.first == cValue.m_long;
					});
					if (k == longRes.end())
						longRes.push_back(std::make_pair(cValue.m_long, 1));
					else
						++k->second;
				}
				else if (cValue.m_type == types::Double)
				{
					auto k = std::find_if(doubleRes.begin(), doubleRes.end(), [=](const std::pair<double, int>& a) {
						return a.first == cValue.m_double;
					});
					if (k == doubleRes.end())
						doubleRes.push_back(std::make_pair(cValue.m_double, 1));
					else
						++k->second;
				}
				else // string, etc?
				{
					MakeError("Alt value at index %i is type %i, unexpected.", cValue.m_type);
					return Runtime.CopyString(_T("<ERROR>"));
				}

				j = theObject->roHo.NumNext;
			}

			// Got 'em all

			// No results?
			if (doubleRes.empty() && longRes.empty())
				return Runtime.CopyString(_T("<No object instances>"));

			// Promote longs to doubles
			if (!doubleRes.empty() && !longRes.empty())
			{
				for (const auto &l : longRes)
				{
					auto k = std::find_if(doubleRes.begin(), doubleRes.end(), [=](const std::pair<double, int>& a) {
						return a.first == l.first;
					});
					if (k == doubleRes.end())
						doubleRes.push_back(std::make_pair((double)l.first, l.second));
					else
						k->second += l.second;
				}
				longRes.clear();
			}

			// Should only have a set of doubles or a set of ints now

			std::tstringstream result;
			result << std::setprecision(numDecimalDigits) << std::fixed;

			if (!doubleRes.empty())
			{
				for (const auto& a : doubleRes)
				{
					result << a.first;
					if (a.second > 1)
						result << _T(" (x") << a.second << _T("), ");
					else
						result << _T(", ");
				}
			}
			else // set of ints
			{
				for (const auto& a : longRes)
				{
					result << a.first;
					if (a.second > 1)
						result << _T(" (x") << a.second << _T("), ");
					else
						result << _T(", ");
				}
			}

			std::tstring output = result.str();
			output.resize(output.size() - 2);
			return Runtime.CopyString(output.c_str());
		}
	}

	MakeError("Object name %s not found in this frame.", objectNameStr.c_str());
	return Runtime.CopyString(_T("<none found>"));
#else
	MakeError("Lookup by alt value name not created for this platform.");
	return Runtime.CopyString(_T("<not implemented>"));
#endif
}
const TCHAR* Extension::GetAltStringsFromObjName(const TCHAR* objectName, int altStringIndex, const TCHAR * delim)
{
	std::tstring objectNameStr(objectName);
	std::tstring delimStr(delim);

	if (!objectNameStr[0])
	{
		MakeError("Cannot supply a blank object name as parameter.");
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (altStringIndex <= 0)
	{
		MakeError("Alterable strings range from index 0+, but you supplied index %i.", altStringIndex);
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (!DarkEdif::IsFusion25 && altStringIndex > 25)
	{
		MakeError("Alterable strings range from index 0-25 in Fusion 2.0, but you supplied index %i.", altStringIndex);
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (delimStr.empty())
	{
		MakeError("Delimiter cannot be blank. Use Newline$, \" \", etc.");
		return Runtime.CopyString(_T("<ERROR>"));
	}
#ifdef _WIN32
	bool isUnicode = mvIsUnicodeApp(Edif::SDK->mV, Edif::SDK->mV->RunApp);
#else
	bool isUnicode = true;
#endif

#if _UNICODE
	// Unicode exts can only load in Unicode
	if (!isUnicode)
	{
		MakeError("Alterable strings reading failed - Non-Unicode app using Unicode ext?!");
		return Runtime.CopyString(_T("<ERROR>"));
	}
#else
	// While we could convert, Phi Object comes in Unicode flavour, so why are we using non-Unicode?
	if (isUnicode)
	{
		MakeError("Alterable strings reading failed - Unicode app using non-Unicode ext.");
		return Runtime.CopyString(_T("<ERROR>"));
	}
#endif

	// Unicode exts can only load in Unicode
	if (DarkEdif::IsFusion25 && !isUnicode)
	{
		MakeError("Copying alt strings failed - CF2.5 not using a Unicode app?!");
		return Runtime.CopyString(_T("<ERROR>"));
	}

#ifdef _WIN32
	std::tstring objectNameStrLengthLimited(objectNameStr);
	objectNameStrLengthLimited.resize(std::size(rhPtr->Frame->oiList[0].name) - 1);

	for (size_t i = 0; i < (size_t)rhPtr->NumberOi; i++)
	{
		// In case NumberOI no worky
		if (!rhPtr->Frame->oiList[i].name[0])
			break;

		// Found object by name
		if (!_tcsicmp(rhPtr->Frame->oiList[i].name, objectNameStrLengthLimited.c_str()))
		{
			std::vector<std::pair<std::tstring, int>> stringRes;

			short j = rhPtr->Frame->oiList[i].Object;
			while (j >= 0)
			{
				RunObject* theObject = (RunObject*)rhPtr->ObjectList[j].oblOffset;
				if (!theObject)
					continue;

				AltVals* av = RunObject_GetVals(theObject);
				if (!av)
				{
					MakeError("Object %s doesn't have alterable values.", objectNameStr.c_str());
					return Runtime.CopyString(_T("<ERROR>"));
				}
				// theObject->roc.val
				if (DarkEdif::IsFusion25 && av->CF25.NumAltStrings <= altStringIndex)
				{
					MakeError("Object %s doesn't have an alterable string at index %i; max available alt string index is %i.",
						objectNameStr.c_str(), altStringIndex, av->CF25.NumAltStrings - 1);
					return Runtime.CopyString(_T("<ERROR>"));
				}

				const TCHAR* strOrNull = DarkEdif::IsFusion25 ? av->CF25.Strings[altStringIndex] : av->MMF2.rvStrings[altStringIndex];
				std::tstring str(strOrNull ? strOrNull : _T(""));

				if (str.empty())
				{
					auto k = std::find_if(stringRes.begin(), stringRes.end(), [&](const std::pair<std::tstring, int>& a) {
						return a.first == str;
					});
					if (k == stringRes.end())
						stringRes.push_back(std::make_pair(str, 1));
					else
						++k->second;
				}

				j = theObject->roHo.NumNext;
			}
			// Got 'em all

			// No results?
			if (stringRes.empty())
				return Runtime.CopyString(_T("<No object instances>"));

			std::tstringstream result;

			for (const auto& a : stringRes)
			{
				result << a.first;
				if (a.second > 1)
					result << _T(" (x") << a.second << _T(")");
				result << delimStr;
			}

			std::tstring output = result.str();
			output.resize(output.size() - delimStr.size());
			return Runtime.CopyString(output.c_str());
		}
	}

	MakeError("Object name %s not found in this frame.", objectNameStr.c_str());
	return Runtime.CopyString(_T("<none found>"));
#else
	MakeError("Lookup by alt value name not created for this platform.");
	return Runtime.CopyString(_T("<not implemented>"));
#endif
}
const TCHAR* Extension::GetFlagsFromObjName(const TCHAR* objectName, int flagIndex)
{
	std::tstring objectNameStr(objectName);

	if (!objectNameStr[0])
	{
		MakeError("Cannot supply a blank object name as parameter.");
		return Runtime.CopyString(_T("<ERROR>"));
	}
	if (flagIndex < 0 || flagIndex > 31)
	{
		MakeError("Internal flags range from index 0 to 31, but you supplied index %i.", flagIndex);
		return Runtime.CopyString(_T("<ERROR>"));
	}

#ifdef _WIN32
	std::tstring objectNameStrLengthLimited(objectNameStr);
	objectNameStrLengthLimited.resize(std::size(rhPtr->Frame->oiList[0].name) - 1);

	for (size_t i = 0; i < (size_t)rhPtr->NumberOi; i++)
	{
		// In case NumberOI no worky
		if (!rhPtr->Frame->oiList[i].name[0])
			break;

		// Found object by name
		if (!_tcsicmp(rhPtr->Frame->oiList[i].name, objectNameStrLengthLimited.c_str()))
		{
			size_t numTrueFlags = 0, numFalseFlags = 0;

			short j = rhPtr->Frame->oiList[i].Object;
			while (j >= 0)
			{
				RunObject* theObject = (RunObject*)rhPtr->ObjectList[j].oblOffset;
				if (!theObject)
					continue;

				AltVals* av = RunObject_GetVals(theObject);
				if (!av)
				{
					MakeError("Object %s doesn't have internal flags.", objectNameStr.c_str());
					return Runtime.CopyString(_T("<ERROR>"));
				}

				if ((DarkEdif::IsFusion25 ? av->CF25.InternalFlags : av->MMF2.rvValueFlags) & (1 << flagIndex))
					++numTrueFlags;
				else
					++numFalseFlags;

				j = theObject->roHo.NumNext;
			}
			// Got 'em all

			// No results?
			if (numTrueFlags == 0 && numFalseFlags == 0)
				return Runtime.CopyString(_T("<No object instances>"));

			std::tstringstream result;

			if (numFalseFlags > 0)
			{
				result << _T("false");
				if (numFalseFlags > 1)
					result << _T(" (x") << numFalseFlags << _T(")");
			}

			if (numTrueFlags > 0)
			{
				if (numFalseFlags > 0)
					result << _T(", ");
				result << _T("true");
				if (numTrueFlags > 1)
					result << _T(" (x") << numTrueFlags << _T(")");
			}

			return Runtime.CopyString(result.str().c_str());
		}
	}
#else // ANDROID
	// TODO: Android implementation
	MakeError("Function not implemented in Android/iOS.");
	return Runtime.CopyString(_T("<not implemented>"));
#endif

	MakeError("Object name %s not found in this frame.", objectNameStr.c_str());
	return Runtime.CopyString(_T("<none found>"));
}

const TCHAR * Extension::GetAltValsFromFixedValue(int fixedValue, int altValueIndex, int numDecimalDigits)
{
	// refer to copying alt vals/string action, it has a nicer list method than above
	return Runtime.CopyString(_T("<not implemented>"));
}
const TCHAR * Extension::GetAltStringsFromFixedValue(int fixedValue, int altStringIndex, const TCHAR * delim)
{
	return Runtime.CopyString(_T("<not implemented>"));
}
const TCHAR * Extension::GetFlagsFromFixedValue(int fixedValue, int flagIndex)
{
	return Runtime.CopyString(_T("<not implemented>"));
}
int Extension::GetCPUTemp()
{
	// usually returned in celsius, see OpenHardwareMonitor or something
	return -1;
}
#ifdef _WIN32
#include <Sddl.h>
#include <winefs.h>
#endif

const TCHAR * Extension::GetLoopedACLEntry_AccountName()
{
#ifdef _WIN32
	if (!currentLoopAce)
		return Runtime.CopyString(_T("<no ACL loop>"));

	// Most ACE types have SidStart at same location as ACCESS_ALLOWED_ACE struct, bar these two.
	size_t offset = offsetof(ACCESS_ALLOWED_ACE, SidStart);
	if (currentLoopAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE ||
		currentLoopAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)
	{
		offset = offsetof(ACCESS_ALLOWED_OBJECT_ACE, SidStart);
	}
	PSID sidPtr = (PSID)(((char *)&currentLoopAce) + offset);

	// 256 recommended by https://stackoverflow.com/a/60289851
	DWORD ignoredDomainSize = MAX_SID_SIZE;
	TCHAR ignoredDomain[MAX_SID_SIZE] = {};
	SID_NAME_USE ignoredPSIDNameUse;

	DWORD accNameSize = MAX_SID_SIZE + 1;
	TCHAR accNameBuff[MAX_SID_SIZE + 1];
	if (!LookupAccountSid(NULL, sidPtr, accNameBuff, &accNameSize, ignoredDomain, &ignoredDomainSize, &ignoredPSIDNameUse))
	{
		MakeError("SID couldn't be mapped to an account name. Error %hs (%u).", GetLastErrorAsString().c_str(), GetLastError());
		return Runtime.CopyString(_T("Unknown account>"));
	}

	return Runtime.CopyString(accNameBuff);

#endif
	return Runtime.CopyString(_T("<not implemented on this platform>"));
}
const TCHAR * Extension::GetLoopedACLEntry_SID()
{
#ifdef _WIN32
	if (!currentLoopAce)
		return Runtime.CopyString(_T("<no ACL loop>"));

	// Most ACE types have SidStart at same location as ACCESS_ALLOWED_ACE struct, bar these two.
	size_t offset = offsetof(ACCESS_ALLOWED_ACE, SidStart);
	if (currentLoopAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE ||
		currentLoopAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)
	{
		offset = offsetof(ACCESS_ALLOWED_OBJECT_ACE, SidStart);
	}

	LPTSTR sidStr;
	if (!ConvertSidToStringSid((PSID)(((char *)&currentLoopAce) + offset), &sidStr))
		return Runtime.CopyString(_T("<SID convert error>"));

	std::tstring copy = sidStr;
	LocalFree(sidStr);
	return Runtime.CopyString(copy.c_str());
#endif
	return Runtime.CopyString(_T("<not implemented on this platform>"));
}
int Extension::GetLoopedACLEntry_AccessMask()
{
#ifdef _WIN32
	return currentLoopAce ? currentLoopAce->Mask : -1;
#endif
	return -1;
}

#include <signal.h>
float Extension::TestParamsFunc(int a, float b, const TCHAR* c, float d, int e, float f, const TCHAR* g, int h, float i,
	const TCHAR* j, int k, float l, const TCHAR* m, float n, int o, const TCHAR* p)
{
	raise(SIGINT);
	return a + b;
}

float Extension::ProximitySensor()
{
#ifdef _WIN32
	MakeError("Proximity sensor is only available in Android and iOS.");
	return 0.0f;
#else
	MakeError("Proximity sensor should be implemented in native-code wrapper in Android and iOS.");
	return 0.0f;
#endif
}
