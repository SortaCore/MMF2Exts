#include "Common.h"
#include "Public/ChromaAnimationAPI.h"

using namespace ChromaSDK;


int Extension::Add(int First, int Second)
{
	return First + Second;
}

const TCHAR * Extension::HelloWorld()
{
	return _T("Hello world!");
}

int Extension::ExpIsInitialized()
{
	return CondIsInitialized();
}

int Extension::ExpInit()
{
	return CondInit();
}
