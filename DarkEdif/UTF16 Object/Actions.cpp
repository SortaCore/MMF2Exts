#include "Common.h"

void Extension::UTF16StrToUTF16Mem(const wchar_t * utf16Str, int addr, int removeNull)
{
	if (IsBadMemoryAddress((void *)addr))
		return DarkEdif::MsgBox::Error(_T("Bad memory address"), _T("Badad memory address %p provided."), (void *)addr);
	if (removeNull < 0 || removeNull > 1)
		return DarkEdif::MsgBox::Error(_T("Invalid action parameter"), _T("Error, bad 'exclude null' setting %i provided. 1 or 0 only."), removeNull);

	size_t size = (wcslen(utf16Str) + (1 - removeNull)) * sizeof(wchar_t);
	memcpy((wchar_t *)addr, utf16Str, size);
}
