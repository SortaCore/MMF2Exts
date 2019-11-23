#include "Common.h"

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

// ID = 0
tchar * Extension::GetError(int clear)
{
	ThreadSafe_Start();
	tstring temp = LastError;
	if (clear)
		LastError = _T("");	//Clear error if user chose to
	ThreadSafe_End();
	return Runtime.CopyString(temp.c_str());
}

// ID = 1
tchar * Extension::GetReport(int clear)
{
	ThreadSafe_Start();
	tstring temp = CompleteStatus;
	if (clear)
		CompleteStatus = _T("");	//Clear report if user chose to
	ThreadSafe_End();
	return Runtime.CopyString(temp.c_str());
}
