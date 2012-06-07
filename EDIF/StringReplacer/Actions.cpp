#include "Common.h"

void Extension::AddNewMacro(char * ReplaceThisP, char * WithThisP)
{
	Macros.push_back(Macro(ReplaceThisP, WithThisP));
}

void Extension::ClearMacros()
{
	Macros.empty();
}
