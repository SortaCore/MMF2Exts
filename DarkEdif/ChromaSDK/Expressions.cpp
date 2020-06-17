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
	return _mInitResult == RZRESULT_SUCCESS;
}

int Extension::ExpInit()
{
	if (ChromaAnimationAPI::IsInitializedAPI())
	{
		return ChromaAnimationAPI::Init();
	}
	else
	{
		return false;
	}
}
