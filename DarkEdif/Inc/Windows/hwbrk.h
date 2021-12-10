#pragma once
#include <windows.h>

enum class HWBRK_TYPE : int
{
	HWBRK_TYPE_CODE,
	HWBRK_TYPE_READWRITE,
	HWBRK_TYPE_WRITE
};

enum class HWBRK_SIZE : int
{
	HWBRK_SIZE_1,
	HWBRK_SIZE_2,
	HWBRK_SIZE_4,
	HWBRK_SIZE_8
};

HANDLE SetMemoryBreakpoint(HANDLE hThread, HWBRK_TYPE Type, HWBRK_SIZE Size,void* s);

BOOL RemoveMemoryBreakpoint(HANDLE hBrk);

