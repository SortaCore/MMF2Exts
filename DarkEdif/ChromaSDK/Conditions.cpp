#include "Common.h"
#include "Public/ChromaAnimationAPI.h"

using namespace ChromaSDK;

bool Extension::AreTwoNumbersEqual(int First, int Second)
{
	return First == Second;
}

bool Extension::CondIsInitialized()
{
	OutputDebugStringA("CondIsInitialized: Result=");
	OutputDebugStringA(_mInitResult == RZRESULT_SUCCESS ? "SUCCESS" : "FAILED");
	OutputDebugStringA("\r\n");
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
		APPINFOTYPE appInfo = {};

		_tcscpy_s(appInfo.Title, 256, _T("ChromaSDK Extension CF2.5"));
		_tcscpy_s(appInfo.Description, 1024, _T("ChromaSDK for ClickTeam Fusion 2.5"));
		_tcscpy_s(appInfo.Author.Name, 256, _T("Razer"));
		_tcscpy_s(appInfo.Author.Contact, 256, _T("https://developer.razer.com/chroma"));

		//appInfo.SupportedDevice = 
		//    0x01 | // Keyboards
		//    0x02 | // Mice
		//    0x04 | // Headset
		//    0x08 | // Mousepads
		//    0x10 | // Keypads
		//    0x20   // ChromaLink devices
		appInfo.SupportedDevice = (0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20);
		appInfo.Category = 1;

		_mInitResult = ChromaAnimationAPI::InitSDK(&appInfo);
	}
	return CondIsInitialized();
}
