#include "Common.h"
#include "Public/ChromaAnimationAPI.h"

using namespace ChromaSDK;

void Extension::ActionExample(int ExampleParameter)
{
}

void Extension::SecondActionExample()
{
}

void Extension::PlayAnimationName(TCHAR* path, int loop)
{
	OutputDebugStringA("PlayAnimationName: Path=");
	OutputDebugStringA(path);
	OutputDebugStringA(" Loop=");
	OutputDebugStringA(loop == 0 ? "false" : "true");
	OutputDebugStringA("\r\n");

	if (CondIsInitialized())
	{
		ChromaAnimationAPI::PlayAnimationName(path, loop == 1);
	}
}
