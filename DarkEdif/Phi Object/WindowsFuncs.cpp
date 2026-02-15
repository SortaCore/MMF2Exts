#include "Common.hpp"
#include <aclapi.h>
#include <authz.h>
#include <WinSpool.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "authz.lib")

// SE_OBJECT_TYPE to GENERIC_MAPPING

//
/* SE_UNKNOWN_OBJECT_TYPE = 0,
SE_FILE_OBJECT,
SE_SERVICE,
SE_PRINTER,
SE_REGISTRY_KEY,
SE_LMSHARE,
SE_KERNEL_OBJECT,
SE_WINDOW_OBJECT,
SE_DS_OBJECT,
SE_DS_OBJECT_ALL,
SE_PROVIDER_DEFINED_OBJECT,
SE_WMIGUID_OBJECT,
SE_REGISTRY_WOW64_32KEY,*/

GENERIC_MAPPING Extension::objectTypeMappings[] = {
	// SE_UNKNOWN_OBJECT_TYPE
	// Read, Write, Execute, ALL
	// Most of these mappings lifted from
	// https://flylib.com/books/en/4.420.1.83/1/#:~:text=generic%20mappings%20for%20common%20securable%20objects
	{ 0, 0, 0, 0 },
	// SE_FILE_OBJECT
	{ FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS },
	// SE_SERVICE
	{ STANDARD_RIGHTS_READ | SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS | SERVICE_INTERROGATE | SERVICE_USER_DEFINED_CONTROL,
		STANDARD_RIGHTS_WRITE | SERVICE_CHANGE_CONFIG,
		STANDARD_RIGHTS_EXECUTE | SERVICE_START | SERVICE_STOP | SERVICE_PAUSE_CONTINUE | SERVICE_INTERROGATE | SERVICE_USER_DEFINED_CONTROL,
		SERVICE_ALL_ACCESS },
	// SE_PRINTER
	{ PRINTER_READ, PRINTER_WRITE, PRINTER_EXECUTE, PRINTER_ALL_ACCESS },
	// SE_REGISTRY_KEY
	{ KEY_READ, KEY_WRITE, KEY_EXECUTE, KEY_ALL_ACCESS },
	// SE_LMSHARE; may be LMShare.h, PERM_FILE_READ, PERM_FILE_WRITE, etc.
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_KERNEL_OBJECT
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_WINDOW_OBJECT
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_DS_OBJECT
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_DS_OBJECT_ALL
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_PROVIDER_DEFINED_OBJECT
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_WMIGUID_OBJECT
	{ STANDARD_RIGHTS_READ, STANDARD_RIGHTS_WRITE, STANDARD_RIGHTS_EXECUTE, STANDARD_RIGHTS_ALL },
	// SE_REGISTRY_WOW64_32KEY
	{ KEY_READ, KEY_WRITE, KEY_EXECUTE, KEY_ALL_ACCESS },
	// SE_REGISTRY_WOW64_64KEY
	{ KEY_READ, KEY_WRITE, KEY_EXECUTE, KEY_ALL_ACCESS },
};

bool Authz_GetAccess(Extension * ext, AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClient, PSECURITY_DESCRIPTOR psd,
	PACCESS_MASK actualAccess)
{
	AUTHZ_ACCESS_REQUEST AccessRequest = { };
	AUTHZ_ACCESS_REPLY AccessReply = { };
	BYTE Buffer[1024] = {};
	BOOL bRes = FALSE;  // assume error

	//  Do AccessCheck.
	AccessRequest.DesiredAccess = MAXIMUM_ALLOWED;

	RtlZeroMemory(Buffer, sizeof(Buffer));
	AccessReply.ResultListLength = 1;
	AccessReply.GrantedAccessMask = (PACCESS_MASK)(Buffer);
	AccessReply.Error = (PDWORD)(Buffer + sizeof(ACCESS_MASK));

	if (!AuthzAccessCheck(0, hAuthzClient, &AccessRequest, NULL, psd, NULL, 0, &AccessReply, NULL)) {
		ext->MakeError(_T("AuthzAccessCheck failed with %hs (code %u)"), Extension::GetLastErrorAsString().c_str(), GetLastError());
		return false;
	}

	*actualAccess = *(PACCESS_MASK)(AccessReply.GrantedAccessMask);
	return true;
}

bool Extension::Sub_GetTrueEffectiveRights(PSID pSid, PACCESS_MASK pAccessRights)
{
	*pAccessRights = 0; // for warning

	AUTHZ_RESOURCE_MANAGER_HANDLE hManager;
	BOOL bResult = AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT, NULL, NULL, NULL, NULL, &hManager);
	if (!bResult)
		return MakeError(_T("AuthzInitializeResourceManager failed with %hs (code %u)"), GetLastErrorAsString().c_str(), GetLastError()), false;

	LUID unusedId = { };
	AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext = NULL;

	bResult = AuthzInitializeContextFromSid(0, pSid, hManager, NULL, unusedId, NULL, &hAuthzClientContext);
	if (bResult)
	{
		bResult = Authz_GetAccess(this, hAuthzClientContext, lastReadPerms.secDesc.get(), pAccessRights);
		AuthzFreeContext(hAuthzClientContext);
	}
	else // error, don't return for cleanup
		MakeError(_T("AuthzInitializeContextFromSid failed with %hs (code %u)"), GetLastErrorAsString().c_str(), GetLastError());

	AuthzFreeResourceManager(hManager);
	return bResult != FALSE;
}

