#include "Common.h"

const char * Extension::FullCommand(void)
{
	return _strdup(Data->ConsoleReceived.c_str());
}

const char * Extension::CommandMinusName(void)
{
	return _strdup((Data->ConsoleReceived.find(' ') == std::string::npos ? "" : 
				Data->ConsoleReceived.c_str() + Data->ConsoleReceived.find(' ') + 1));
}