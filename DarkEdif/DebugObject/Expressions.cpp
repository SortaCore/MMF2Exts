#include "Common.hpp"

const TCHAR * Extension::FullCommand(void)
{
	return Runtime.CopyString(data->consoleReceived.c_str());
}

const TCHAR * Extension::CommandMinusName(void)
{
	return Runtime.CopyString((data->consoleReceived.find(_T(' ')) == std::tstring::npos ? _T("") :
				data->consoleReceived.c_str() + data->consoleReceived.find(_T(' ')) + 1));
}
