#include "Common.hpp"

const bool Extension::AlwaysTrue() const
{
	return true;
}

#include <signal.h>
bool Extension::IsEqual(int a, int b) {
	return a == b;
}

const bool Extension::IsThisFrameASubApp() {
	return rhPtr->get_App()->get_ParentApp() != nullptr;
}

const bool Extension::DoesAccHaveEffectivePerm(const TCHAR * accOrSIDPtr, const TCHAR * argPermListPtr) {
	std::tstring accOrSID(accOrSIDPtr);
	std::string argPermList(DarkEdif::TStringToANSI(argPermListPtr));

	if (lastReadPerms.itemPath.empty())
		return MakeError(_T("Last read system object permissions is invalid, can't check for effective permissions.")), false;

#if _WIN32
	TRUSTEE trustee = {};
	PSID trusteePSID = NULL;
	ACCESS_MASK wantedAccessPerms, actualAccessPerms;
	if (!Sub_BuildTrusteeAndAccessPerms(accOrSID, argPermList, &trustee, &trusteePSID, &wantedAccessPerms))
	{
		if (trusteePSID)
			LocalFree(trusteePSID);
		return false; // errors reported by Sub
	}

	// May not work in scenarios such as remote computers, or for some permissions.
	// https://docs.microsoft.com/en-us/windows/win32/api/aclapi/nf-aclapi-geteffectiverightsfromacla
	bool effectiveAccessRes = Sub_GetTrueEffectiveRights(trusteePSID, &actualAccessPerms);

	LocalFree(trusteePSID);

	if (!effectiveAccessRes)
	{
		return MakeError(_T(R"(Invalid ACL access mode list ("%hs"). Must be "read", "write", "all", etc., comma-separated. See help file )"
				"for specifics. You can also specify a numeric mask based on GENERIC_READ, or more specific "
				"number permission such as FILE_READ_EA."), argPermList.c_str()), false;
	}

	// This doesn't work, expects generic perms to be mapped to specific/standard
	// AreAllAccessesGranted(actualAccessPerms, wantedAccessPerms);
	//
	// So we have to map manually, because for some reason Windows doesn't have public mappings.
	MapGenericMask(&wantedAccessPerms, &Extension::objectTypeMappings[lastReadPerms.readObjectType]);

	return (actualAccessPerms & wantedAccessPerms) == wantedAccessPerms;
#else
	//
#endif
	return false;
}

const bool Extension::OnNamedLoop(const TCHAR * loopName)
{
	return !_tcsicmp(loopName, aceLoopName);
}

bool Extension::InvalidateExplicitSelection()
{
	rhPtr->SetRH2EventCount(rhPtr->GetRH2EventCount() + 1);
	return true;
}
