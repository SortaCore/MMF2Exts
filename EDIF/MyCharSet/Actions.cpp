#include "Common.h"

void Extension::SetBOMMarkASC(const char * FileToAddTo, int TypeOfBOM, int IgnoreCurrentBOM)
{
	// Check parameters	
	if (!FileToAddTo ||									// Invalid file name supplied
		TypeOfBOM > 2 || TypeOfBOM < 0 ||				// Invalid BOM choice (0 = none (ANSI), 1 = UTF-8, 2 = UTF-16)
		IgnoreCurrentBOM < 0 || IgnoreCurrentBOM > 1)	// Invalid RemoveCurrentBOM choice (yes, no)
		return;

	// Too long or ""
	if (strnlen(FileToAddTo, MAX_PATH+1) == MAX_PATH || FileToAddTo[0] == '\0')
		return;

	// Duplicate file name (MMF2 will run off with orignal char *s)
	const char * Copy = _strdup(FileToAddTo);
	if (!Copy)
		return;

	// The entire file will have to stream, Windows doesn't support prepending
	BOMThreadData * Data = new BOMThreadData(Copy, TypeOfBOM, IgnoreCurrentBOM == 1);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&SetBOMMarkASCThread, Data, NULL, NULL);
}