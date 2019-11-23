
#include "Common.h"

void Extension::UTF16StrToUTF8Mem(const wchar_t * utf16Str, int addr, int excludeNull)
{
	if (addr == 0xDDDDDDDL || addr == 0x0000000L || addr == 0xFFFFFFFL)
	{
		MessageBoxA(NULL, "Error, bad memory address", "UTF-16 Object Message", MB_OK | MB_ICONERROR);
		return;
	}
	if (excludeNull < 0 || excludeNull > 1)
	{
		MessageBoxA(NULL, "Error, bad 'exclude null' setting", "UTF-16 Object Message", MB_OK | MB_ICONERROR);
		return;
	}

	size_t size = wcslen(utf16Str) + (1 - excludeNull);
	memcpy((wchar_t *)addr, utf16Str, size);
}