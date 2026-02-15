#include "Common.hpp"

const bool Extension::AlwaysTrue() const {
	return true;
}

bool Extension::OnSpecificConsoleInput(TCHAR * command)
{
	if (!command || command[0] == _T('\0'))
		GlobalExt->OutputNow(5, -1, "Blank command given as parameter to \"On specific console input\".");
	return _tcsnicmp(command, data->consoleReceived.c_str(), _tcsnlen(command, 255)) == 0;
}
