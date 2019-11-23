
#include "Common.h"

bool Extension::Compare(TCHAR * a, TCHAR * b)
{
	bool okay = true;
	if (!a)
	{
		MessageBoxA(NULL, "Was supplied NULL in first parameter.", "Comparison error", MB_OK);
		okay = false;
	}
	if (!b)
	{
		MessageBoxA(NULL, "Was supplied NULL in first parameter.", "Comparison error", MB_OK);
		okay = false;
	}
	if (a && !_tcslen(a))
	{
		MessageBoxA(NULL, "Was supplied 0-length string in first parameter.", "Comparison error", MB_OK);
		okay = false;
	}
	if (b && !_tcslen(b))
	{
		MessageBoxA(NULL, "Was supplied 0-length string in second parameter.", "Comparison error", MB_OK);
		okay = false;
	}

	return okay ? (!_tcsicmp(a,b)) : false;
}
