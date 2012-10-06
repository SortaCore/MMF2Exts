#include "Common.h"

// Handles Ctrl+C, Ctrl+Break, and console's close button
bool WINAPI HandlerRoutine(DWORD ControlType)
{
	if (!GlobalExt)
		return true;

	// Note: always return true, to indicate this function has handled it.
	switch (ControlType)
	{
		case CTRL_C_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+C hit. No response set up...");
			return true;
		case CTRL_BREAK_EVENT:
			GlobalExt->OutputNow(5, -1, "Ctrl+Break hit. No response set up...");
			return true;
		case CTRL_CLOSE_EVENT:
			GlobalExt->OutputNow(5, -1, "Close console hit. No response set up...");
			return true;
		default:
			GlobalExt->OutputNow(5, -1, "Unknown control type sent to console. No response.");
			return true;
	}
}

// Receives input text from console
void WINAPI ReceiveConsoleInput()
{
	char InputText[256];
	
	// Continue until shutdown is enabled
	while (Data && Data->ConsoleEnabled)
	{
		// memset() infinite-loops if > 255 characters
		for (unsigned char c = 0; c < 255; ++c)
			InputText[c] = 0;
		
		// Read console
		if (std::cin.good()) 
			std::cin.getline(InputText, 255);
		else // Jump to next loop
			continue;

		Data->ReleaseConsoleInput = false;
		Data->ConsoleReceived = InputText;
		GlobalExt->Runtime.PushEvent(12);

		while (!Data->ReleaseConsoleInput)
			Sleep(10);
	}
}