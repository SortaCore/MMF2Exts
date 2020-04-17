#include "Common.h"

void Extension::UTF16StrToUTF16Mem(const wchar_t * utf16Str, int addr, int removeNull)
{
	if (IsBadMemoryAddress((void *)addr))
	{
		MessageBoxA(NULL, "Error, bad memory address", "UTF-16 Object Message", MB_OK | MB_ICONERROR);
		return;
	}
	if (removeNull < 0 || removeNull > 1)
	{
		MessageBoxA(NULL, "Error, bad 'exclude null' setting", "UTF-16 Object Message", MB_OK | MB_ICONERROR);
		return;
	}

	size_t size = (wcslen(utf16Str) + (1 - removeNull)) * sizeof(wchar_t);
	memcpy((wchar_t *)addr, utf16Str, size);
}
