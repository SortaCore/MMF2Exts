#include "common.h"
// ============================================================================
//
// UNREFENCED FUNCTIONS
// 
// ============================================================================

// Report - For non-errors
void Extension::Unreferenced_Report(tchar * report = _T("Unknown report..."), int ThreadID = -1)
{
	// Not from a thread
	if (ThreadID == -1)
	{
		ThreadSafe_Start();
		CompleteStatus += report;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
			MessageBox(NULL, report, _T("DarkSocket - Latest Report:"), MB_OK);
	}
	else
	// From a thread
	{
		tchar text [255];
		_stprintf_s(text, 255, _T("Socket ID = %i >> %s."), ThreadID, report);
		ThreadSafe_Start();
		CompleteStatus += text;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
		{
			tchar title [255];
			_stprintf_s(title, 255, _T("DarkSocket - Latest Report from %i:"), ThreadID);
			MessageBox(NULL, report, title, MB_OK);
		}
	}

	Runtime.PushEvent(1);
}

// Explode - For errors
void Extension::Unreferenced_Error(tchar * error = _T("Unknown error..."), int ThreadID = -1)
{
	// Not from a thread
	if (ThreadID == -1)
	{
		ThreadSafe_Start();
		LastError += error;
		LastError += _T("\r\n");
		CompleteStatus += error;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
			MessageBox(NULL, error, _T("DarkSocket - Latest Error:"), MB_OK);
	}
	else
	// From a thread
	{
		tchar text [255];
		_stprintf_s(text, 255, _T("Socket ID = %i >> %s."), ThreadID, error);
		ThreadSafe_Start();
		LastError += text;
		LastError += _T("\r\n");
		CompleteStatus += text;
		CompleteStatus += _T("\r\n");
		ThreadSafe_End();
		if (UsePopups)
		{
			tchar title [255];
			_stprintf_s(title, 255, _T("DarkSocket - Latest Error from %i:"), ThreadID);
			MessageBox(NULL, error, title, MB_OK);
		}
	}
	Runtime.PushEvent(0);
	Runtime.PushEvent(1);
}