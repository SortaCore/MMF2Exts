#include "Common.hpp"

#ifdef _WIN32
#include <psapi.h>
#pragma comment (lib, "psapi.lib")

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string Extension::GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	if (message.back() == '\n')
		message.erase(message.end() - 1);
	if (message.back() == '\r')
		message.erase(message.end() - 1);
	if (message.back() == '.')
		message.erase(message.end() - 1);

	return message;
}
#endif
std::vector<std::string> split(const std::string& s, char seperator)
{
	std::vector<std::string> output;

	std::string::size_type prev_pos = 0, pos = 0;

	while ((pos = s.find(seperator, pos)) != std::string::npos)
	{
		std::string substring(s.substr(prev_pos, pos - prev_pos));

		output.push_back(substring);

		prev_pos = ++pos;
	}

	output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

	return output;
}

#ifdef __ANDROID__
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sys/statvfs.h>

std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::stringstream result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result << buffer.data();
		std::string t = "Inside exec: "; t += buffer.data();
		LOGI("%s", t.c_str());
	}
	return result.str();
}
#endif // __ANDROID__
extern Extension * staticExt;

void Extension::StripSpaces(std::string & str)
{
	str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char uc) { return std::isspace(uc); }), str.end());
}
void Extension::StripUnderscores(std::string & str)
{
	str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
}

void Extension::MakeStringLower(std::string & str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](const unsigned char & uc) {
		return std::tolower(uc);
	});
}
#ifdef _WIN32
void Extension::StripSpaces(std::wstring & str)
{
	str.erase(std::remove_if(str.begin(), str.end(), [](const wint_t uc) { return ::iswspace(uc); }), str.end());
}

void Extension::MakeStringLower(std::wstring & str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](const wint_t uc) {
		return towlower(uc);
	});
}
#endif

void Extension::UpdateRAMUsageInfo()
{
#ifdef _WIN32
	static constexpr std::int64_t DIV = 1024 * 1024; // 1MB

	PERFORMACE_INFORMATION ppi;
	ppi.cb = sizeof(ppi);
	if (GetPerformanceInfo(&ppi, sizeof(ppi)) == FALSE)
		return MakeError(_T("GetPerformanceInfo failed with error %u: %hs."), GetLastError(), GetLastErrorAsString().c_str());

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);

	if (GlobalMemoryStatusEx(&statex) == FALSE)
		return MakeError(_T("GetPerformanceInfo failed with error %u: %hs."), GetLastError(), GetLastErrorAsString().c_str());

	// int32 won't suffice for the multiply, so if we define page size as int64,
	// C++ will upgrade all page size math to int64.
	std::int64_t pageSize = ppi.PageSize;

	physMemTotalMB = (std::int32_t)((ppi.PhysicalTotal * pageSize) / DIV);
	physMemFreeMB = (std::int32_t)((ppi.PhysicalAvailable * pageSize) / DIV);
	// could be system max available, or process assigned max
	pageFileMemTotalMB = (std::int32_t)((ppi.CommitTotal * pageSize) / DIV);
	pageFileMemFreeMB = (std::int32_t)((ppi.CommitLimit * pageSize) / DIV);

	//virtualMemTotalMB = ?
	//virtualMemFreeMB = ?

	//physMemTotalMB = statex.ullTotalPhys / DIV;
	//physMemFreeMB = statex.ullAvailPhys / DIV;
	// could be system max available, or process assigned max
	//pageFileMemTotalMB = statex.ullTotalPageFile / DIV;
	//pageFileMemFreeMB = statex.ullAvailPageFile / DIV;
	virtualMemTotalMB = (std::int32_t)(statex.ullTotalVirtual / DIV);
	virtualMemFreeMB = (std::int32_t)(statex.ullAvailVirtual / DIV);
	// Reserved/unused, so no point
	// virtualExtendedMemFree = statex.ullAvailExtendedVirtual / DIV;
#elif defined(__ANDROID__)

	// https://stackoverflow.com/q/19104635
	// Example output:
	// TypeOfMem:(several spaces) 840868 kB
	// Due to the fixed line width, it might be possible to predict the width of the lines
	// and read by char offset, instead of searching for characters

	FILE * memInfoFile = fopen("/proc/meminfo", "rb");
	// meminfo results is around 897 chars long, but we only need the starting part anyway
	std::string memInfoStr(1024, '\0');
	if (fread((void *)memInfoStr.data(), 1, memInfoStr.size(), memInfoFile) <= 0)
	{
		fclose(memInfoFile);
		return MakeError("Failed to read /proc/meminfo, error %u: %s.", errno, strerror(errno));
	}

	// fclose error check is needed for writing files to make sure flushing works,
	// but we're only reading, so we'll ignore errors.
	fclose(memInfoFile);

	auto memInfoOutputLines = split(memInfoStr, '\n');
	std::map<std::string_view, std::int32_t> memInfoOutputHashToMB;
	OutputDebugStringA("Contents of /proc/meminfo:\n");

	auto parseMBfromLine = [&](size_t lineIndex) {
		if (lineIndex > memInfoOutputLines.size())
			return MakeError("Mem info output does not have a line at index %zu. Max index is %zu.", lineIndex, memInfoOutputLines.size()), -1;

		// TypeOfMem:<-(several spaces) 840868 kB
		auto colonIndex = memInfoOutputLines[lineIndex].find(':', 0);
		if (colonIndex == std::string::npos)
			return MakeError("Failed to find colon index in meminfo line %zu \"%s\".", lineIndex, memInfoOutputLines[lineIndex].c_str()), -1;
		// TypeOfMem:(several spaces) ->840868 kB
		auto nonSpaceIndex = memInfoOutputLines[lineIndex].find_first_not_of(' ', colonIndex + 1);
		if (nonSpaceIndex == std::string::npos)
			return MakeError("Failed to find non-space index in meminfo line %zu \"%s\".", lineIndex, memInfoOutputLines[lineIndex].c_str()), -1;

		auto nextSpaceIndex = memInfoOutputLines[lineIndex].find_first_of(' ', nonSpaceIndex + 1);
		if (nextSpaceIndex == std::string::npos)
			return MakeError("Failed to find next index in meminfo line %zu \"%s\".", lineIndex, memInfoOutputLines[lineIndex].c_str()), -1;

		std::string_view retString = memInfoOutputLines[lineIndex].substr(nonSpaceIndex, nextSpaceIndex - nonSpaceIndex);
		if (retString.size() < 1)
			return MakeError("Size after parsing in meminfo line %zu \"%s\" is too small.", lineIndex, memInfoOutputLines[lineIndex].c_str()), -1;

		std::int64_t kbInLine = strtoll(std::string(retString).c_str(), NULL, 10);
		std::int32_t res = kbInLine <= 0 ? kbInLine : (std::int32_t)(kbInLine / 1024LL);
		auto op = memInfoOutputHashToMB.insert(std::make_pair(std::string_view(memInfoOutputLines[lineIndex].data(), colonIndex), res));
		(void)op;
		LOGV("Added mem info line: \"%s\" = %d MB.\n", std::string(op.first->first).c_str(), res);
		return res;
	};

	// Skip last line, it's usually empty
	for (std::size_t i = 0; i < memInfoOutputLines.size() - 1; ++i)
		parseMBfromLine(i);

	physMemTotalMB = memInfoOutputHashToMB["MemTotal"sv];
	// Note, there is a "MemFree", but that refers to physical RAM that the kernel hasn't set up for being used.
	// MemAvailable is what's actually available to apps.
	physMemFreeMB = memInfoOutputHashToMB["MemAvailable"sv];
	pageFileMemTotalMB = memInfoOutputHashToMB["SwapTotal"sv]; // swap file total
	pageFileMemFreeMB = memInfoOutputHashToMB["SwapFree"sv];// swap file free
	virtualMemTotalMB = memInfoOutputHashToMB["VmallocTotal"sv]; // VmallocTotal
	virtualMemFreeMB = virtualMemTotalMB - memInfoOutputHashToMB["VmallocUsed"sv]; // VmallocUsed

	// https://stackoverflow.com/questions/8133417/android-get-free-size-of-internal-external-memory
	LOGI("Result of df:\n");
	std::string dfOutputStr = exec("df -h");
	auto dfOutputLines = split(dfOutputStr, '\n');
	for (std::size_t i = 0; i < dfOutputLines.size(); ++i)
		LOGI("%s", (dfOutputLines[i] + "\n").c_str());
#else
	MakeError("iOS RAM usage info not coded");
#endif
}

#include <map>

#ifdef _WIN32
#include <AclAPI.h>
#include <sddl.h>
#include <winefs.h>

bool Extension::Sub_BuildTrusteeAndAccessPerms(std::tstring & sidOrAcc, std::string & argPermList,
	PTRUSTEE trustee, PSID * trusteePSID, DWORD * accessPermissions)
{
	DWORD sidSize = SECURITY_MAX_SID_SIZE;
	*trusteePSID = LocalAlloc(LMEM_FIXED, sidSize);
	if (*trusteePSID == NULL)
		return MakeError(_T("Out of memory.")), false;

	// Successful, it's in SID format
	if (sidOrAcc._Starts_with(_T("S-1-"sv)) && ConvertStringSidToSid(sidOrAcc.c_str(), trusteePSID) == 0)
		BuildTrusteeWithSid(trustee, *trusteePSID);
	else // use account name
	{
		BuildTrusteeWithName(trustee, sidOrAcc.data()); // no idea why this is not const TCHAR * param

		// 256 recommended by https://stackoverflow.com/a/60289851
		DWORD ignoredDomainSize = MAX_SID_SIZE;
		TCHAR ignoredDomain[MAX_SID_SIZE] = {};
		SID_NAME_USE ignoredPSIDNameUse;
		if (!LookupAccountName(NULL, sidOrAcc.c_str(), *trusteePSID, &sidSize, ignoredDomain, &ignoredDomainSize, &ignoredPSIDNameUse))
		{
			*trusteePSID = NULL;
			return MakeError(_T("Account name \"%s\" wasn't mapped to a SID. Error %hs (%u)."), sidOrAcc.c_str(), GetLastErrorAsString().c_str(), GetLastError()), false;
		}
	}

	const std::map<const std::string_view, const ACCESS_MASK> accessMaskList = {
		{ "read"sv, GENERIC_READ },
		{ "write"sv, GENERIC_WRITE },
		{ "execute"sv, GENERIC_EXECUTE },
		{ "all"sv, GENERIC_ALL },
		{ "fullcontrol"sv, GENERIC_ALL /* equivalent to FILE_ALL_ACCESS? */},
		{ "delete"sv, DELETE },
		{ "writedac"sv, WRITE_DAC },
		{ "writeowner"sv, WRITE_OWNER },
		{ "synchronize"sv, SYNCHRONIZE },
	};

	errno = ENOENT;
	DWORD accessPerms = strtoul(argPermList.c_str(), nullptr, 0);
	if (errno == ENOENT && accessPerms != 0)
		*accessPermissions = accessPerms;
	else
	{
		const std::vector permList2 = split(argPermList, ',');
		if (permList2.empty())
		{
			return MakeError(_T(R"(Invalid ACL access mode list ("%hs"). Must be "read", "write", "all", etc., comma-separated. See help file )"
					"for specifics. You can also specify a numeric mask based on GENERIC_READ, or more specific "
					"number permission such as FILE_READ_EA."), argPermList.c_str()), false;
		}

		// Allow none in case you can set access to none, thereby revoking inherited perms?
		if (argPermList != "none"sv && !argPermList.empty())
		{
			for each (const std::string & p in permList2)
			{
				auto accessPermIt = accessMaskList.find(p);
				if (accessPermIt == accessMaskList.cend())
				{
					return MakeError(_T(R"(Invalid ACL access mode ("%hs"). Must be "read", "write", "all", etc., comma-separated. See help file )"
							"for specifics. You can also specify a numeric mask based on GENERIC_READ, or more specific "
							"number permission such as FILE_READ_EA."), p.c_str()), false;
				}
				accessPerms |= accessPermIt->second;
			}
			*accessPermissions = accessPerms;
		}
	}
	return true;
}

#endif // _WIN32

void Extension::ReadSystemObjectPerms(const TCHAR * itemPathPtr, const TCHAR * itemTypePtr, int includeSystemACLInt)
{
	lastReadPerms.itemPath.clear();

#if _WIN32
	std::string itemType(DarkEdif::TStringToANSI(itemTypePtr));
	MakeStringLower(itemType);

	if (includeSystemACLInt > 1 || includeSystemACLInt < 0)
		return MakeError(_T("Include system ACL is %d, not 0 or 1."), includeSystemACLInt);
	const bool includeSystemACL = includeSystemACLInt != 0;

	// Some of these will see more use than others, I expect.
	// Defined at https://docs.microsoft.com/en-us/windows/win32/api/accctrl/ne-accctrl-se_object_type
	//
	// Be aware window, access tokens, processes, threads, named/anonymous pipes, cannot be read/set
	// with GetNamedSecurityInfo, only GetSecurityInfo.
	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/access-rights-and-access-masks
	//
	// As an aside, FAT32 doesn't support file permissions, so you cannot set permissions on those sorts of drives either.
	const std::map<const std::string_view, const SE_OBJECT_TYPE> objectTypes = {
		{ "file"sv, SE_OBJECT_TYPE::SE_FILE_OBJECT },
		{ "service"sv, SE_OBJECT_TYPE::SE_SERVICE },
		{ "printer"sv, SE_OBJECT_TYPE::SE_PRINTER },
		{ "registry"sv, SE_OBJECT_TYPE::SE_REGISTRY_KEY },
		{ "lmshare"sv, SE_OBJECT_TYPE::SE_LMSHARE },
		{ "kernel"sv, SE_OBJECT_TYPE::SE_KERNEL_OBJECT },
		{ "window"sv, SE_OBJECT_TYPE::SE_WINDOW_OBJECT },
		{ "ds_object"sv, SE_OBJECT_TYPE::SE_DS_OBJECT },
		{ "ds_object_all"sv, SE_OBJECT_TYPE::SE_DS_OBJECT_ALL },
		{ "wmiguid"sv, SE_OBJECT_TYPE::SE_WMIGUID_OBJECT },
		{ "registrywow6432"sv, SE_OBJECT_TYPE::SE_REGISTRY_WOW64_32KEY },
		{ "registrywow6464"sv, (SE_OBJECT_TYPE)13 /* SE_OBJECT_TYPE::SE_REGISTRY_WOW64_64KEY, not in XP SDK */},
	};

	auto objectTypeIt = objectTypes.find(itemType);
	if (objectTypeIt == objectTypes.cend())
		return MakeError(_T("Object type \"%hs\" not recognised. Check the help file."), itemType.c_str());

	SE_OBJECT_TYPE objectType = objectTypeIt->second;
	lastReadPerms.readObjectType = objectType;
	lastReadPerms.itemPath = itemPathPtr;

	// If you want to read the SACL, note you need to enable perms. See
	// https://docs.microsoft.com/en-us/windows/win32/api/aclapi/nf-aclapi-getsecurityinfo#:~:text=to%20read%20the%20sacl%20from
	lastReadPerms.securityInfo = DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
		(includeSystemACL ? SACL_SECURITY_INFORMATION : 0);

	if (objectType == SE_REGISTRY_KEY || objectType == SE_REGISTRY_WOW64_32KEY || objectType == 13  /* SE_OBJECT_TYPE::SE_REGISTRY_WOW64_64KEY*/)
	{
		const std::tstring_view prefix = _T("\\HKEY_"sv);
		constexpr size_t prefixSize = sizeof("HKEY_") - 1;

		// You don't use HKEY_ prefix. So, for local registry:
		if (lastReadPerms.itemPath._Starts_with(prefix.substr(1)) ||
		// And remote registry
			lastReadPerms.itemPath.find(prefix) != std::tstring::npos)
		{
			lastReadPerms.itemPath.clear();
			return MakeError(_T("When specifying a registry key, don't include the HKEY_ prefix in the root key, like you did with \"%s\"."), itemPathPtr);
		}
	}

	PSECURITY_DESCRIPTOR tempSecDesc;
	DWORD d = GetNamedSecurityInfo(itemPathPtr, objectType, lastReadPerms.securityInfo,
		&lastReadPerms.owner, &lastReadPerms.primaryGroup, &lastReadPerms.dacl,
		includeSystemACL ? &lastReadPerms.sacl : NULL, &tempSecDesc);
	if (d != ERROR_SUCCESS)
	{
		SetLastError(d);
		lastReadPerms.itemPath.clear();
		return MakeError(_T("Couldn't read DACL of \"%s\". Got error %hs (%hu)."),
			itemPathPtr, GetLastErrorAsString().c_str(), GetLastError());
	}
	lastReadPerms.secDesc.reset((SECURITY_DESCRIPTOR *)tempSecDesc);

#else
	return MakeError("Reading system object perms not coded on this platform.");
#endif
}

void Extension::IterateLastReadSystemObjectDACL(const TCHAR * loopName, const TCHAR * allowDenyBothPtr, int includeInheritedInt, int includeInheritOnlyInt)
{
	if (includeInheritedInt > 1 || includeInheritedInt < 0)
		return MakeError(_T("Invalid \"include inherited\" parameter, must be 0 or 1, but you supplied %d."), includeInheritedInt);
	bool includeInherited = includeInheritedInt == 1;
	if (includeInheritOnlyInt > 1 || includeInheritOnlyInt < 0)
		return MakeError(_T("Invalid \"include inherit-only\" parameter, must be 0 or 1, but you supplied %d."), includeInheritOnlyInt);
	bool includeInheritOnly = includeInheritOnlyInt == 1;
	(void)includeInherited;
	(void)includeInheritOnly;

	std::string allowDenyBoth = DarkEdif::TStringToANSI(allowDenyBothPtr);
	MakeStringLower(allowDenyBoth);

	const bool includeAllowed = (allowDenyBoth == "allow"sv || allowDenyBoth == "both"sv);
	const bool includeDenied = (allowDenyBoth == "deny"sv || allowDenyBoth == "both"sv);

	if (!includeAllowed && !includeDenied)
		return MakeError(_T(R"(You specified an invalid ACE entry type "%s". Valid types are "allow", "deny", or "both".)"), allowDenyBothPtr);

	if (lastReadPerms.itemPath.empty())
		return MakeError(_T("Last read system object permissions is invalid, can't iterate through the permissions."));

#ifdef _WIN32
	if (currentLoopAce != nullptr)
		return MakeError(_T("Can't do a nesting of two ACL entry loops. Already doing a loop called %s."), aceLoopName);

	// GetExplicitEntriesFromAcl() ignores inherited ACE entries.
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/msmq/ms700142(v=vs.85)#top

	ACL_SIZE_INFORMATION aclsizeinfo;
	if (GetAclInformation(lastReadPerms.dacl, &aclsizeinfo, sizeof(aclsizeinfo), ACL_INFORMATION_CLASS::AclSizeInformation) == FALSE)
	{
		return MakeError(_T("Couldn't read ACL size information. Got error %hs (%u)."),
			GetLastErrorAsString().c_str(), GetLastError());
	}

	aceLoopName = loopName;
	for (std::size_t i = 0; i < aclsizeinfo.AceCount; ++i)
	{
		if (GetAce(lastReadPerms.dacl, i, (void **)&currentLoopAce) != 0)
		{
			MakeError(_T("Couldn't read DACL entry at index %i. Got error %hs (%u)."),
				i, GetLastErrorAsString().c_str(), GetLastError());
			break;
		}

		if (!includeInherited && (currentLoopAce->Header.AceFlags & INHERITED_ACCESS_ENTRY) != 0)
			continue;
		if (!includeInheritOnly && (currentLoopAce->Header.AceFlags & INHERIT_ONLY) != 0)
			continue;

		if (!includeAllowed && (
				currentLoopAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE ||
				currentLoopAce->Header.AceType == ACCESS_ALLOWED_CALLBACK_ACE_TYPE ||
				currentLoopAce->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE ||
				currentLoopAce->Header.AceType == ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE))
		{
			continue;
		}

		if (!includeDenied && (
			currentLoopAce->Header.AceType == ACCESS_DENIED_ACE_TYPE ||
			currentLoopAce->Header.AceType == ACCESS_DENIED_CALLBACK_ACE_TYPE ||
			currentLoopAce->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE ||
			currentLoopAce->Header.AceType == ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE))
		{
			continue;
		}

		Runtime.GenerateEvent(3);
	}

	aceLoopName = nullptr;
	currentLoopAce = nullptr;
	return;

#endif // _WIN32

	MakeError(_T("%s Not implemented"), _T(__FUNCTION__));
}
void Extension::AddNewDACLPermToSystemObject(const TCHAR * sidOrAccPtr, const TCHAR * allowDenyRevokePtr, const TCHAR * permListPtr, const TCHAR * inheritListPtr)
{
	// DACL canonical order:
	// Explicit Deny - Explicit Allow - Inherited (Deny - Inherited Allow)
	// https://referencesource.microsoft.com/#mscorlib/System/security/accesscontrol/acl.cs,85460059921e396e

	// Since we're adding, we don't need to sort per se, just calculate our priority, and find the first one that exceeds the priority.

	std::tstring sidOrAcc(sidOrAccPtr);
	std::string allowDenyRevoke(DarkEdif::TStringToANSI(allowDenyRevokePtr)),
		argPermList(DarkEdif::TStringToANSI(permListPtr)),
		argInheritList(DarkEdif::TStringToANSI(inheritListPtr));

	StripSpaces(allowDenyRevoke);
	MakeStringLower(allowDenyRevoke);
	StripSpaces(argPermList);
	StripUnderscores(argPermList);
	MakeStringLower(argPermList);
	StripSpaces(argInheritList);
	StripUnderscores(argInheritList);
	MakeStringLower(argInheritList);

#ifdef _WIN32
	PSID trusteePSID = NULL; // free with LocalFree?
	EXPLICIT_ACCESS access;
	SecureZeroMemory(&access, sizeof(access));

	//
	const std::map<const std::string_view, const ACCESS_MODE> accessModeList = {
		{ "grant"sv, ACCESS_MODE::GRANT_ACCESS },
		{ "deny"sv, ACCESS_MODE::DENY_ACCESS },
		{ "set"sv, ACCESS_MODE::SET_ACCESS },
		{ "revoke"sv, ACCESS_MODE::REVOKE_ACCESS },
	};

	auto accessModeIt = accessModeList.find(allowDenyRevoke);
	if (accessModeIt == accessModeList.cend())
	{
		return MakeError(_T(R"(Invalid ACL access mode ("%hs"). Must be "grant", "set", "deny", or "revoke".)"), allowDenyRevoke.c_str());
	}
	access.grfAccessMode = accessModeIt->second;

	if (!Sub_BuildTrusteeAndAccessPerms(sidOrAcc, argPermList, &access.Trustee, &trusteePSID, &access.grfAccessPermissions))
		return; // Sub will create error

	const std::map<const std::string_view, const ACCESS_MASK> inheritList = {
		// Subfolders
		{ "subcontainers"sv, SUB_CONTAINERS_ONLY_INHERIT },
		{ "subfolders"sv, SUB_CONTAINERS_ONLY_INHERIT },
		// Subobjects
		{ "subobjects"sv, SUB_OBJECTS_ONLY_INHERIT },
		{ "files"sv, SUB_OBJECTS_ONLY_INHERIT },
		// Don't apply to current folder
		{ "inheritonly"sv, INHERIT_ONLY },
		// Not grandchildren
		{ "nopropogation"sv, INHERIT_NO_PROPAGATE },
	};

	if (argInheritList != "none"sv && argInheritList != "no inheritance"sv && !argInheritList.empty())
	{
		DWORD inheritMode = NO_INHERITANCE;
		const std::vector argInheritList2 = split(argInheritList, ',');
		if (argInheritList2.empty())
		{
			return MakeError(_T(R"(Invalid ACL inheritance list ("%hs"). Must be "subcontainers", "subobjects", "inheritonly", etc., comma-separated.)"
					" See help file for specifics."), argInheritList.c_str());
		}
		// We could check for duplicates, but meh, it's a bitmask, so that'll be meaningless in practice.
		for each (const std::string & i in argInheritList2)
		{
			auto inheritIt = inheritList.find(i);
			if (inheritIt == inheritList.cend())
			{
				return MakeError(_T(R"(Invalid ACL inheritance mode ("%hs"). Must be "subcontainers", "subobjects", "inheritonly", etc., comma-separated.)"
					" See help file for specifics."), i.c_str());
			}
			inheritMode |= inheritIt->second;
		}
		access.grfInheritance = inheritMode;
	}

	// https://docs.microsoft.com/en-us/windows/win32/api/aclapi/nf-aclapi-buildexplicitaccesswithnamea
	PACL newACL = nullptr;
	DWORD res = SetEntriesInAcl(1UL, &access, lastReadPerms.dacl, &newACL);
	if (ERROR_SUCCESS != res)
	{
		return MakeError(_T("Couldn't set the new entry in the ACL. Got error %hs (%hu)."),
			GetLastErrorAsString().c_str(), GetLastError());
	}

	// Attach the new ACL as the object's DACL.
	// Be aware this does not overwrite existing ACLs of files when set to be inherited.
	// Instead, it only affects new files/folders.

	res = SetNamedSecurityInfo(lastReadPerms.itemPath.data(), lastReadPerms.readObjectType,
		  DACL_SECURITY_INFORMATION, NULL, NULL, newACL, NULL);
	if (ERROR_SUCCESS != res) {
		SetLastError(res);
		MakeError(_T("Couldn't set new ACL for \"%s\". Got error %hs (%u)."),
			lastReadPerms.itemPath.c_str(), GetLastErrorAsString().c_str(), GetLastError());

		// free newACL?
		LocalFree(newACL);
		return;
	}
	LocalFree(newACL); // yay?
#endif // _WIN32

	// https://ss64.com/nt/icacls.html#:~:text=inherited%20folder%20permissions
	// OI: Object Inherit - files inside inherit it
	// CI: Container Inherit - folders inside inherit it
	// IO: Inherit Only - doesn't apply to current file/folder
	// NP: No Propogation - applies to subfolders, but not sub-subfolders, and further levels
	//
	// Windows Security tab shows options:
	// XX XX XX This folder only
	// OI CI XX This folder, subfolders and files
	// XX CI XX This folder and subfolders
	// OI XX XX This folder and files
	// OI CI IO Subfolders and files only
	// XX CI IO Subfolders only
	// OI XX IO Files only
	//
	//
	// Note: SID such as "Users" is dependent on system language.
	// See https://docs.microsoft.com/en-US/troubleshoot/windows-server/identity/security-identifiers-in-windows to convert.
	// Also https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/81d92bba-d22b-4a8c-908a-554ab29148ab .

	// Bitfield of inheritance describing how an access control entry is inherited
	/*
	enum ACCESS_INHERITANCE : DWORD
	{
		// If set, implies this ACE entry is inherited from a parent, and does not originate from this item.
		// Set by system, you shouldn't need to do it
		// _INHERITED_ACE = INHERITED_ACE,

		_NO_INHERITANCE = NO_INHERITANCE,
		// If set, files inherit ACL
		_OBJECT_INHERIT_ACE = OBJECT_INHERIT_ACE,
		// If set, subfolders inherit ACL
		_CONTAINER_INHERIT_ACE = CONTAINER_INHERIT_ACE,
		// If set, sub-subfolders don't inherit AC
		_NO_PROPAGATE_INHERIT_ACE = NO_PROPAGATE_INHERIT_ACE,
		// If set, does not apply to the actual thing it's set on, just the children of it
		_INHERIT_ONLY_ACE = INHERIT_ONLY_ACE,
		_SUB_OBJECTS_ONLY_INHERIT = OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
		_SUB_CONTAINERS_ONLY_INHERIT = CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
		_SUB_CONTAINERS_AND_OBJECTS_INHERIT = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
	};*/
}

std::vector<int> memfind(void * startV, size_t size, void* lookFor, size_t lookForSize)
{
	std::vector<int> ret;
	char* start = (char *)startV;
	char* end = start + size;
	for (char* c = start; c < end; ++c)
	{
		if (memcmp(c, lookFor, lookForSize) == 0)
			ret.push_back((int)(c - start));
	}
	return ret;
}
#ifdef _WIN32
#include <map>
#include <iomanip>
void Dump(std::stringstream &str2, void * v, size_t s)
{
	std::map<char, char*> unprintable{
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
	char* c = (char*)v;
	for (std::size_t i = 0; i < s; ++i)
	{
		str2 << std::dec << std::setfill('0') << std::setw(2) << i << ", "sv;
		if (!c[i])
			str2 << "NUL"sv;
		else
		{
			auto a = unprintable.find(c[i]);
			if (a != unprintable.end())
				str2 << a->second;
			else
				str2 << c[i];
		}
		str2 << ", 0x"sv << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)(((unsigned char*)c)[i]) << '\n';
	}
}

#include <functional>
#include <fstream>

#define HEAP_STANDARD 0
#define HEAP_LAL 1
#define HEAP_LFH 2

int HeapDetails(char * c, size_t s, HANDLE hHeap)
{
	BOOL bResult;
	ULONG HeapInformation;

	//
	// Query heap features that are enabled.
	//
	bResult = HeapQueryInformation(hHeap,
		HeapCompatibilityInformation,
		&HeapInformation,
		sizeof(HeapInformation),
		NULL);
	if (bResult == FALSE) {
		sprintf_s(c, s, "Failed to retrieve heap features with LastError %d.\n",
			GetLastError());
		return 1;
	}

	//
	// Print results of the query.
	//
	sprintf_s(c, s, "HeapCompatibilityInformation is %d.\n", HeapInformation);
	switch (HeapInformation)
	{
	case HEAP_STANDARD:
		sprintf_s(c, s, "The default process heap is a standard heap.\n");
		break;
	case HEAP_LAL:
		sprintf_s(c, s, "The default process heap supports look-aside lists.\n");
		break;
	case HEAP_LFH:
		sprintf_s(c, s, "The default process heap has the low-fragmentation "
			"heap enabled.\n");
		break;
	default:
		sprintf_s(c, s, "Unrecognized HeapInformation reported for the default "
			"process heap.\n");
		break;
	}

	return 0;
}




#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <intsafe.h>

HANDLE GetHeaps(void * pointer, size_t expectedSize)
{
#if 0
	return GetProcessHeap();
#else
	DWORD NumberOfHeaps;
	DWORD HeapsIndex;
	HANDLE aHeaps[256];
	char str[2048];
	size_t heapSize;
	HANDLE ret = NULL;

	//
	// Retrieve handles to the process heaps and print them to stdout.
	// Note that heap functions should be called only on the default heap of the process
	// or on private heaps that your component creates by calling HeapCreate.
	//
	NumberOfHeaps = GetProcessHeaps(std::size(aHeaps), aHeaps);
	if (NumberOfHeaps == 0) {
		sprintf_s(str, "Failed to retrieve heaps with LastError %d.\n",
			GetLastError());
		OutputDebugStringA(str);
		return NULL;
	}

	LOGI(_T("Process has %ud heaps.\n"), NumberOfHeaps);
	for (HeapsIndex = 0; HeapsIndex < NumberOfHeaps; ++HeapsIndex) {
		/*
		if (HeapValidate(aHeaps[HeapsIndex], 0, pointer) == FALSE)
		{
			DebugBreak();
		}*/
		if (HeapValidate(aHeaps[HeapsIndex], 0, NULL) == FALSE)
		{
			DebugBreak();
		}
		heapSize = HeapSize(aHeaps[HeapsIndex], 0, pointer);
		//HeapDetails(str, sizeof(str), aHeaps[HeapsIndex], heapSize);
		sprintf_s(str, "Heap %d at address: %#p. Reports size as %u.\n",
			HeapsIndex,
			aHeaps[HeapsIndex],
			heapSize
		//	(void *)(((char *)(aHeaps[HeapsIndex])) + heapSize)
		);
		if (heapSize == expectedSize)
			ret = aHeaps[HeapsIndex];
		LOGI(_T("%s"), DarkEdif::UTF8ToTString(str).c_str());
	}

	return ret;
#endif
}

int objectFV = -1;
void Extension::AddBlankFramesToObject(int objectFV,
	int animNum, int dirNum,
	int numOfFrames, int insertIndexAt)
{
	::objectFV = objectFV;
	LOGI(_T("Set FV for blank frames. Now waiting.\n"));

	RunObject * runObj = Runtime.RunObjPtrFromFixed(objectFV);
	if (!runObj)
		return MakeError(_T("Selecting Fixed Value %i for adding animation failed."), objectFV);

	if (!runObj->get_roa())
	{
		//objInfoList oil = Runtime.ObjectSelection.GetOILFromOI(oc->roho.Oi);
		return MakeError(_T("Object with Fixed Value %i (%s) does not have animations."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name());
	}

	if (animNum < 0)
		return MakeError(_T("Direction param %i is invalid. Should be 0 or higher."), animNum);

	if (dirNum < 0 || dirNum > 31)
		return MakeError(_T("Direction param %i is invalid. Should be between 0 and 31."), dirNum);

	// Whether it's too large is handled in Sub_AddImages
	if (insertIndexAt < -1)
		return MakeError(_T("Insert index param %i is invalid. Should be between 0+, or -1 for appending."), insertIndexAt);

	if (numOfFrames > 100)
		return MakeError(_T("Number of frames to insert (%i) surpassed common sense limit."), numOfFrames);

#ifdef FUSION_INTERNAL_ACCESS
#if 0
	Objects_Common * oc = runObj->rHo.Common;
	HANDLE heapOfRunObj = GetHeaps(oc, oc->size);
	if (heapOfRunObj == NULL)
		return MakeError("Can't track down the object's animation in memory, unable to alter it.");
#endif

	const unsigned short blankImgNum = 3;
	// Try to turn the images into useful data
#if 0
	WORD imageID; // from image bank, yay
	CreateImageFromFileInfo info;
	info.trspColor;
	info.nSize = 0; // num byes
	info.;
	if (mvCreateImageFromFile(::SDK->mV, &imageID, filename, &info) == FALSE)
	{
		return MakeError("Could not add anim frame %s Object %s ",
	}
#endif
	std::vector<unsigned short> imgIDs(numOfFrames, blankImgNum);

	size_t s = HeapSize(GetProcessHeap(), 0, runObj);
	if (s != runObj->rHo.hoSize)
	{
		DebugBreak();
	}
	s = HeapSize(GetProcessHeap(), 0, runObj->rHo.hoCommon);
	if (s != runObj->rHo.hoCommon->size)
	{
		DebugBreak();
	}

	Sub_AddImagesAtIndex(runObj, imgIDs, animNum, dirNum, SIZE_T_MAX);
#else // FUSION_INTERNAL_ACCESS

#endif
}

void OutputAnimations(const AnimHeader * const animHead)
{
	std::stringstream str5;
	str5 << "===== ANIMATIONS:\n"sv;
	for (std::size_t lAniNum = 0; lAniNum < animHead->AnimMax; ++lAniNum)
	{
		if (animHead->OffsetToAnim[lAniNum] < 0)
			continue;

		str5 << "Animation "sv << lAniNum << " directions:\n"sv;
		Animation* lAnim = (Animation*)(((char*)animHead) + animHead->OffsetToAnim[lAniNum]);
		for (std::size_t lAnimDirNum = 0; lAnimDirNum < 32; ++lAnimDirNum)
		{
			if (lAnim->OffsetToDir[lAnimDirNum] < 0)
				continue;

			AnimDirection* lAnimDir = (AnimDirection*)(((char*)lAnim) + lAnim->OffsetToDir[lAnimDirNum]);
			str5 << "Dir "sv << lAnimDirNum << ", "sv << (int)lAnimDir->NumberOfFrame
				<< " frames, (min speed "sv << (int)lAnimDir->MinSpeed << ", max "sv << (int)lAnimDir->MaxSpeed
				<< "), loop "sv << (int)lAnimDir->Repeat << " times (back to "sv << (int)lAnimDir->RepeatFrame
				<< ")\nImage nums: "sv;

			for (std::size_t lAnimFrame = 0; lAnimFrame < lAnimDir->NumberOfFrame; ++lAnimFrame)
			{
				str5 << lAnimDir->Frame[lAnimFrame] <<
					(lAnimFrame == lAnimDir->NumberOfFrame - 1 ? "\n"sv : ", "sv);
			}

		}
		str5 << "Animation "sv << lAniNum << " directions done.\n"sv;
	}
	str5 << "===== ANIMATIONS END\n"sv;
	LOGI(_T("%s"), DarkEdif::UTF8ToTString(str5.str()).c_str());
}


void Extension::Sub_AddImagesAtIndex(RunObject * runObj, std::vector<unsigned short> imgIDs,
	size_t animNum, size_t dirNum, size_t insertAtIndex = SIZE_T_MAX)
{
#ifdef FUSION_INTERNAL_ACCESS
	if (HeapSize(GetProcessHeap(), 0, runObj) != runObj->rHo.hoSize)
	{
		DebugBreak();
	}
	if (HeapSize(GetProcessHeap(), 0, runObj->rHo.hoCommon) != runObj->rHo.hoCommon->size)
	{
		DebugBreak();
	}

	if (!runObj->get_roa())
	{
		return MakeError(_T("Object with Fixed Value %i (%s) does not have animations."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name());
	}


	Objects_Common* oc = runObj->rHo.hoCommon;
	if (oc == NULL)
	{
		return MakeError(_T("Object with Fixed Value %i (%s) does not have Common?"),
			objectFV, runObj->get_rHo()->get_OiList()->get_name());
	}

	HANDLE heapOfRunObj = GetHeaps(oc, oc->size);
	if (heapOfRunObj == NULL)
	{
		return MakeError(_T("Object with Fixed Value %i (%s): couldn't find memory heap associated with object."
			"Unable to alter animations."), objectFV, runObj->get_rHo()->get_OiList()->get_name());
	}

	if (HeapValidate(heapOfRunObj, 0, NULL) == FALSE)
	{
		DebugBreak();
	}
	if (HeapValidate(heapOfRunObj, 0, NULL) == FALSE)
	{
		DebugBreak();
	}

	// Track down the animation, then dir we want
	AnimHeader* animHead = (AnimHeader *)(((char *)oc) + oc->Animations);
	if (animHead->OffsetToAnim[animNum] < 0)
	{
		return MakeError(_T("Object with Fixed Value %i (%s) does not have animation with ID %i."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name(), animNum);
	}
	Animation* anim = (Animation *)(((char *)animHead) + animHead->OffsetToAnim[animNum]);
	if (anim->OffsetToDir[dirNum] < 0)
	{
		return MakeError(_T("Object with Fixed Value %i (%s) has anim ID %i, but no direction %i."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name(), animNum, dirNum);
	}
	AnimDirection* animDir = (AnimDirection *)(((char *)anim) + anim->OffsetToDir[dirNum]);

	if (insertAtIndex == SIZE_T_MAX)
		insertAtIndex = animDir->NumberOfFrame;
	else if (insertAtIndex > animDir->NumberOfFrame)
	{
		return MakeError(_T("Object with Fixed Value %i (%s): couldn't insert images at index %i, that's "
			"not within anim ID %i, dir %i's valid image range (of 0 to %hu, or -1 for appending)."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name(), insertAtIndex, animNum, dirNum, animDir->NumberOfFrame);
	}

	unsigned short numImagesToAdd = (unsigned short)imgIDs.size();
	unsigned short extraSizeNeeded = numImagesToAdd * sizeof(unsigned short);

	// Now we know it does exist...

	// Just in case user tries to add 65k or dumb number of images
	if (((std::size_t) animDir->NumberOfFrame) + (imgIDs.size() * sizeof (unsigned short)) > SHORT_MAX - 10)
	{
		return MakeError(_T("Object with Fixed Value %i (%s): anim ID %i, dir %i has too many frames (%i), can't add more."),
			objectFV, runObj->get_rHo()->get_OiList()->get_name(), animNum, dirNum, animDir->NumberOfFrame);
	}


	OutputAnimations(animHead);

	if (HeapValidate(heapOfRunObj, 0, NULL) == FALSE)
	{
		DebugBreak();
	}
#if 1
	const Objects_Common* const oc3 = oc; // code analysis decides oc is uninited after HeapReAlloc, so dup it before
	Objects_Common* oc2 = (Objects_Common *)HeapReAlloc(heapOfRunObj, 0, oc, oc->size + extraSizeNeeded);

	if (oc2 == NULL)
		return MakeError(_T("Failed to reallocate %u bytes for new frame."), oc->size + extraSizeNeeded);

	if (oc2 != oc3)
		LOGW(_T("WARNING: re-allocation produced entirely new memory address.\n"));
#else
	Objects_Common* oc2 = (Objects_Common *)HeapAlloc(heapOfRunObj, 0, oc->size + extraSizeNeeded);
	if (oc2 == NULL)
		return MakeError("Failed to reallocate %u bytes for new frame.", oc->size + extraSizeNeeded);

	ZeroMemory(((char *) oc2) + oc->size, extraSizeNeeded);
	MoveMemory(oc2, oc, oc->size);

	// Wipe it for fun
	ZeroMemory(oc, oc->size);
	if (HeapFree(heapOfRunObj, 0, oc) == FALSE)
	{
		DebugBreak();
	}
	if (HeapValidate(heapOfRunObj, 0, NULL) == FALSE)
	{
		DebugBreak();
	}
#endif

	// oc now contains potentially different and now freed address; do the switcheroo
	// Update the anim pointers too, as OC may be same or may be completely changed.
	oc = oc2;
	oc2 = NULL;

	animHead = (AnimHeader*)(((char*)oc) + oc->Animations);
	anim = (Animation*)(((char*)animHead) + animHead->OffsetToAnim[animNum]);
	animDir = (AnimDirection*)(((char*)anim) + anim->OffsetToDir[dirNum]);

	unsigned long ACTIVE = 'SPRI';


	/* Parameters of Animation > Load Frame
		filename
		Animation ID
		Direction
		Display frame?
		hot spot x coord (100000 for center, 110000 for right)
		hot spot y coord (100000 for center, 110000 for bottom)
		active point x coord (100000 for center, 110000 for right)
		active point y coord (100000 for center, 110000 for bottom)
		transparent colour (RGB, or -1 for first pixel)
	*/

	// Shift everything previously after the frame to a bit further ahead
	unsigned short * Frame = animDir->Frame;
	//unsigned short * origLastFrame = &animDir->Frame[insertAtIndex - 1];
	//unsigned short * insertAtFrame = &animDir->Frame[insertAtIndex];
	size_t memCpySize = ((char*)& Frame[insertAtIndex - 1]) - ((char*)oc);
	memCpySize = oc->size - memCpySize;

	OutputDebugStringA("PRE MOVE:\n");
	OutputAnimations(animHead);

	// Push all memory after last frame in this anim to slightly further along
	// Note CopyMemory() is unsuitable for overlapping source/dest memory.
	MoveMemory(&Frame[insertAtIndex + numImagesToAdd], &Frame[insertAtIndex], memCpySize);

	// Add new image in
	for (std::size_t i = 0; i < numImagesToAdd; ++i)
		animDir->Frame[insertAtIndex + i] = imgIDs[i];
	animDir->NumberOfFrame += numImagesToAdd;

	// Modify container of animation
	animHead->size += extraSizeNeeded;

	// Move all offsets for later directions in current animation
	for (std::size_t i = dirNum + 1; i < 32; ++i)
	{
		if (anim->OffsetToDir[i] < 0)
			continue;

		anim->OffsetToDir[i] += extraSizeNeeded;
	}

	// Move all offsets for animations that are placed further ahead

	// TODO: Confirm animations earlier in list have a lower offset than later
	// There's potential for a mixmatch


	for (std::size_t i = animNum + 1; i < animHead->AnimMax; ++i)
	{
		if (animHead->OffsetToAnim[i] < 0)
			continue;

		animHead->OffsetToAnim[i] += extraSizeNeeded;
		std::stringstream str4; str4 << "AnimNum moving: moved offsetToAnim from "sv <<
			(animHead->OffsetToAnim[i] - extraSizeNeeded) << " to "sv << (animHead->OffsetToAnim[i]) << ".\n"sv;
		//Animation* anim = (Animation*)(((char*)animHead) + animHead->OffsetToAnim[i]);
		//anim->OffsetToDir[0];
		//str4 << "Now points to animation with first image %i.";

		LOGI(_T("%s"), DarkEdif::UTF8ToTString(str4.str()).c_str());
	}

	LOGI(_T("PAST MOVE:\n"));
	OutputAnimations(animHead);

	// Animations updated, now update Common pointers
	runObj->rHo.hoCommon = oc;
	// oc->Animations has unchanged offset, don't update it
	oc->size += extraSizeNeeded;
	//TODO:
	runObj->roc.rcChanged = TRUE;

	// If current or later-in-memory animation is in use, update to correct pointer
	if (runObj->get_roa())
	{
		rAni& ani = *runObj->get_roa();
		if (ani.Offset >= anim)
		{
			LOGI(_T("Moved runObj->roa.Offset.\n"));
			*((char**)&ani.Offset) += extraSizeNeeded;
			if (ani.DirOffset >= animDir)
			{
				LOGI(_T("Moved runObj->roa.DirOffset.\n"));
				*((char**)& ani.DirOffset) += extraSizeNeeded;
			}
		}
	}

	runObj->get_rHo()->get_OiList()->oilOIFlags |= OILFlags::TO_RELOAD;

	short numObj = runObj->get_rHo()->get_OiList()->get_Object();
	while (numObj >= 0)
	{
		RunObjectMultiPlatPtr ro = runObj->get_rHo()->get_AdRunHeader()->GetObjectListOblOffsetByIndex(numObj);
		if (ro->rHo.hoCommon != oc)
		{
			ro->rHo.hoCommon = oc;
			(void)oc2;
			DebugBreak();
		}

		// Image is currently being displayed
		if (ro->roc.rcAnim == animNum && ro->roc.rcDir == dirNum)
		{
			LOGI(_T("Note: Forcibly boosting anim num frames.\n"));
			if (ro->get_roa())
				ro->get_roa()->NumberOfFrame += imgIDs.size();

			if ((size_t)ro->roc.rcImage >= insertAtIndex - 1)
			{
				ro->roc.rcChanged = TRUE;
				if (ro->get_rom())
					ro->get_rom()->rmReverse += 1; // ro->getMvt()->rmReverse ? 1 : 2;
				//ro->roc.rcOldDir = -1;
				CallRunTimeFunction2((&ro->rHo), RFUNCTION::REDRAW, 0, 0);
				//	Runtime.Redraw();
				LOGI(_T("Note: Forcibly re-animating.\n"));
				ro->roc.rcRoutineAnimation(&ro->rHo);
				// runObj->roc.rcOldImage = -1;
				// DebugBreak();
			}
		}

		numObj = ro->rHo.hoNumNext;
	}

	LOGI(_T("END RESULT:\n"));
	OutputAnimations(animHead);
	if (HeapValidate(heapOfRunObj, 0, NULL) == FALSE)
	{
		DebugBreak();
	}

#if 0
	std::stringstream str;
	str << "Attempts complete.\n";
	std::string mode = "RELEASE\n";
#ifdef _DEBUG
	mode = "DEBUG\n";
#endif
	std::fstream of("D:\\output.log", std::ios_base::out | std::ios_base::app);
	of << "===== " << mode << str.str() << "=====\n";
	of.close();
#endif
#else // FUSION_INTERNAL_ACCESS
#endif
}

Objects_Common* copy = NULL;
RunObject* copy2 = NULL;
HANDLE runHeap = NULL;

void Extension::StoreDetails(int objectFV)
{
	::objectFV = objectFV;
	OutputDebugStringA("Set FV for blank frames. Now waiting.\n");
	RunObject * runObj = Runtime.RunObjPtrFromFixed(objectFV);
	if (!runObj)
	{
		return MakeError(_T("Selecting Fixed Value %i for adding animation failed."), 0);
	}
//	runHeap = GetHeaps(runObj, sizeof(runObj->roHo->size));

#if 0
	WORD imageID; // from image bank, yay
	CreateImageFromFileInfo info;
	mvCreateImageFromFile(mV, &imageID, _T("C:\\yay"), &info);
#endif
	//	rdPtr->rHo.AdRunHeader->Frame->los->;
		//LevelObject* los;
#if 1

	RunObject* ro = runObj;
	static int runNum = 0;
	if (++runNum == 1)
		return;
	if (runNum > 2)
	{
		OutputDebugStringA("=== Starting diff check.\n");
		for (std::size_t i = 0; i < _msize(copy2); ++i)
		{
			if (*(((char*)ro) + i) != *(((char*)copy2) + i))
			{
				std::stringstream str;
				str << "Offset "sv << i << ", addr 0x"sv << std::hex << (long)(((char*)ro) + i)
					<< " vals "sv << std::dec << (size_t)(*(((char*)ro) + i)) << " != "sv << (size_t)(*(((char*)copy2) + i)) << ".\n"sv;
				OutputDebugStringA(str.str().c_str());
			}
		}
		OutputDebugStringA("=== Ended diff check.\n");
	}
	else // 2
	{
		free(copy2);
		copy2 = (RunObject*)malloc(sizeof(RunObject));
		if (!copy2 || memcpy_s(copy2, _msize(copy2), ro, sizeof(RunObject)) != 0)
			MakeError(_T("Copying RunHeader failed with %d."), copy2 ? errno : ENOMEM);
		OutputDebugStringA("Stored RunHeader for diff check.\n");
	}

#else // 0
	Objects_Common* oc = runObj->roHo.Address->Common;
	static int runNum = 0;
	if (++runNum == 1)
		return;
	if (runNum > 2)
	{
		OutputDebugStringA("=== Starting diff check.\n");
		for (std::size_t i = 0; i < _msize(copy); ++i)
		{
			if (*(((char*)oc) + i) != *(((char*)copy) + i))
			{
				std::stringstream str;
				str << "Offset "sv << i << ", addr 0x"sv << std::hex << (long)(((char*)oc) + i)
					<< " vals "sv << std::dec << (size_t)(*(((char*)oc) + i)) << " != "sv << (size_t)(*(((char*)copy) + i)) << ".\n"sv;
				OutputDebugStringA(str.str().c_str());
			}
		}
		OutputDebugStringA("=== Ended diff check.\n");
	}
	else // 2
	{
		free(copy);
		copy = (Objects_Common*)malloc(oc->size);
		if (memcpy_s(copy, _msize(copy), oc, oc->size) != 0)
			MakeError("Copying Objects_Common failed with %u.", errno);
		OutputDebugStringA("Stored Objects_Common for diff check.\n");
	}
#endif
}
void Extension::CheckForDiff()
{
	AddBlankFramesToObject(::objectFV, 0, 0, 1, -1);
}
#if 0
{

	RunObject* runObj = Runtime.RunObjPtrFromFixed(objectFV);
	if (!runObj)
	{
		MakeError("Selecting Fixed Value %i for adding animation failed.", 0);
	}

#if 0
	WORD imageID; // from image bank, yay
	CreateImageFromFileInfo info;
	mvCreateImageFromFile(mV, &imageID, _T("C:\\yay"), &info);
#endif
	//	rdPtr->rHo.AdRunHeader->Frame->los->;
		//LevelObject* los;

	Objects_Common* oc = runObj->roHo.Address->Common;
	OutputDebugStringA("=== Starting diff check.\n");
	for (std::size_t i = 0; i < _msize(copy); ++i)
	{
		if (*(((char*)oc) + i) != *(((char*)copy) + i))
		{
			std::stringstream str;
			str << "Offset "sv << i << ", addr 0x"sv << std::hex << (long)(((char*)oc) + i)
				<< " vals "sv << std::dec << (size_t)(*(((char*)oc) + i)) << " != "sv << (size_t)(*(((char*)copy) + i)) << ".\n"sv;
			OutputDebugStringA(str.str().c_str());
		}
	}
	OutputDebugStringA("=== Ended diff check.\n");

	if (memcpy_s(copy, _msize(copy), oc, oc->size) != 0)
		MakeError("Copying Objects_Common failed with %u.", errno);
}
#endif // 0

#else // if defined(__ANDROID__) or APPLE
void Extension::Sub_AddImagesAtIndex(RunObject * runObj, std::vector<unsigned short> imgIDs,
	size_t animNum, size_t dirNum, size_t insertAtIndex = SIZE_T_MAX)
{
	// TODO: Add images at animation index
}

void Extension::StoreDetails(int objectFV)
{
	// TODO: Probably not needed later.
}

void Extension::CheckForDiff()
{
	// TODO: Probably not needed later.
}

void Extension::AddBlankFramesToObject(int objectFV,
	int animNum, int dirNum,
	int numOfFrames, int insertIndexAt)
{

}

#endif
void Extension::AddImagesToObject(int objectFV,
	int animNum, int dirNum,
	const TCHAR * filenames, int insertIndexAt)
{
}

void Extension::CopyAltVals(RunObject *, int startIndex, int numVals, int destIndex)
{
#if _WIN32
	// Since we want the user to be able to cancel foreach loops midway, we'll do the loop ourselves.
	Runtime.CancelRepeatingObjectAction();

	if (numVals == 0)
		return;
	if (startIndex < 0 || numVals < 0 || destIndex < 0 ||
		(startIndex < destIndex ? (startIndex + numVals > destIndex) : (destIndex + numVals > startIndex)))
	{
		MakeError(_T("Can't copy alt vals, indexes are invalid or overlap"));
		return;
	}

	// oil could be -1 if valid object type, but no instances - or invalid obj type, e.g. global events but no corresponding frame obj
	short oil = Runtime.GetOIListIndexFromObjectParam(0);

	std::vector<RunObjectMultiPlatPtr> list;
	for (auto pHo : DarkEdif::ObjectIterator(rhPtr, oil, DarkEdif::Selection::Implicit))
		list.push_back(pHo);

	// No instances available
	if (list.empty())
	{
		MakeError(_T("Can't copy alt vals, no object instances available."));
		return;
	}

	for (auto a : list)
	{
		for (size_t i = 0, j = startIndex, k = destIndex; i < (size_t)numVals; ++i, ++j, ++k)
		{
			AltVals* prv = a->get_rov();
			if (!prv)
			{
				MakeError(_T("Aborting alt value copy for %s; null pointer."), list[0]->get_rHo()->get_OiList()->get_name());
				return;
			}

			// if no value set there yet, this will be null
			const auto thisAlt = prv->GetAltValueAtIndex(j);
			if (thisAlt && thisAlt->m_type == TYPE_DOUBLE)
				prv->SetAltValueAtIndex(k, thisAlt->m_double);
			else // long, possibly does not exist
				prv->SetAltValueAtIndex(k, thisAlt ? thisAlt->m_long : 0);
		}
	}
#else
	MakeError("Copying alt values is not available on this platform - ask for it to be ported!");
#endif
}

void Extension::CopyAltStrings(RunObject *, int startIndex, int numVals, int destIndex)
{
#ifdef _WIN32
	// Since we want the user to be able to cancel foreach loops midway, we'll do the loop ourselves.
	Runtime.CancelRepeatingObjectAction();

	if (numVals == 0)
		return;
	if (startIndex < 0 || numVals < 0 || destIndex < 0 ||
		(startIndex < destIndex ? (startIndex + numVals > destIndex) : (destIndex + numVals > startIndex)))
	{
		MakeError(_T("Can't copy alt strings, indexes are invalid or overlap"));
		return;
	}

	short oil = Runtime.GetOIListIndexFromObjectParam(0);

	std::vector<RunObjectMultiPlatPtr> list;
	for (auto pq : DarkEdif::ObjectIterator(rhPtr, oil, DarkEdif::Selection::Implicit))
		list.push_back(pq);

	// No instances available
	if (list.empty())
	{
		MakeError(_T("Can't copy alt vals, no object instances available."));
		return;
	}

	bool isUnicode = mvIsUnicodeApp(Edif::SDK->mV, Edif::SDK->mV->RunApp);
#if _UNICODE
	// Unicode exts can only load in Unicode
	if (!isUnicode)
	{
		MakeError(_T("Copying alt strings failed - Non-Unicode app using Unicode ext?!"));
		return;
	}
#endif

	// Unicode exts can only load in Unicode
	if (DarkEdif::IsFusion25 && !isUnicode)
	{
		MakeError(_T("Copying alt strings failed - CF2.5 not using a Unicode app?!"));
		return;
	}

	for (const auto a : list)
	{
		for (std::size_t i = 0, j = startIndex, k = destIndex; i < (std::size_t)numVals; ++i, ++j, ++k)
		{
			AltVals* prv = a->get_rov();
			if (!prv)
			{
				MakeError(_T("Aborting alt string copy for %s; null pointer."), list[0]->get_rHo()->get_OiList()->get_name());
				return;
			}

			prv->SetAltStringAtIndex(k, prv->GetAltStringAtIndex(j));
		}
	}
#else
	MakeError("Copying alt strings is not available on this platform - ask for it to be ported!");
#endif
}
