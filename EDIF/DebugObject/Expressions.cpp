#include "Common.h"

const char * Extension::FullCommand(void)
{
	return Data->ConsoleReceived.c_str();
}

const char * Extension::CommandMinusName(void)
{
	return (Data->ConsoleReceived.find(' ') == std::string::npos ? "" : 
				Data->ConsoleReceived.c_str() + Data->ConsoleReceived.find(' '));
}