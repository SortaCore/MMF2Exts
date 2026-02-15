#include "Common.hpp"

int Extension::Add(int First, int Second)
{
	return First + Second;
}

const TCHAR * Extension::HelloWorld()
{
	return Runtime.CopyString(_T("Hello world!"));
}
