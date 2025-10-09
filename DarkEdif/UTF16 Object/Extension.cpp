#include "Common.hpp"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this)
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/

	LinkExpression(0, UTF16CharToUTF16Int);
	LinkExpression(1, UTF16IntToUTF16Char);
	LinkExpression(2, UTF16StrFromUTF16Mem);
	LinkExpression(3, UTF16StrFromUTF8Mem);

	LinkAction(0, UTF16StrToUTF16Mem);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/

}

bool Extension::IsBadMemoryAddress(const void* const addrP)
{
	long addr = (long)addrP;
	return (addr == 0xDDDDDDDL || addr == 0xCCCCCCCL || addr == 0xCDCDCDCDL || addr == 0x0000000L || addr == 0xFFFFFFFL);
}

Extension::~Extension()
{

}


REFLAG Extension::Handle()
{
	// Will not be called next loop
	return REFLAG::ONE_SHOT;
}

// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
