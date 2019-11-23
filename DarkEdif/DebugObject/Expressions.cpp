#include "Common.h"

const TCHAR * Extension::FullCommand(void)
{
	return _tcsdup(data->consoleReceived.c_str());
}

const TCHAR * Extension::CommandMinusName(void)
{
	return _tcsdup((data->consoleReceived.find(_T(' ')) == std::tstring::npos ? _T("") : 
				data->consoleReceived.c_str() + data->consoleReceived.find(_T(' ')) + 1));
}
