#include "Common.h"
#include "Public/ChromaAnimationAPI.h"

using namespace ChromaSDK;

bool Extension::AreTwoNumbersEqual(int First, int Second)
{
	return First == Second;
}

bool Extension::CondIsInitialized()
{
	return _mInitResult == RZRESULT_SUCCESS;
}

bool Extension::CondInit()
{
	if (CondIsInitialized())
	{
		return true;
	}
	if (ChromaAnimationAPI::IsInitializedAPI())
	{
		_mInitResult = ChromaAnimationAPI::Init();
	}
	return _mInitResult == RZRESULT_SUCCESS;
}
