
#include "Common.h"

const char * Extension::GetError()
{
	return Runtime.CopyString(error.str().c_str());
}

const char * Extension::GetActionMenu2()
{
	return Runtime.CopyString(menuStr[0].c_str());
}

const char * Extension::GetConditionMenu2()
{
	return Runtime.CopyString(menuStr[1].c_str());
}

const char * Extension::GetExpressionMenu2()
{
	return Runtime.CopyString(menuStr[2].c_str());
}

typedef void (DLLExport * GetXXXStringFunc)(mv *mV, short actionID, TCHAR * strBuf, short maxLen);
typedef void * (DLLExport * GetXXXInfosFunc)(mv * mV, short actionID);// Actually ACEInfo *, ID param
typedef void (DLLExport * GetXXXTitleFunc)(mv *mV, short actionID, short paramID, TCHAR * strBuf, short maxLen);
typedef void (DLLExport * GetExpressionParamFunc)(mv *mV, short actionID, short paramID, TCHAR * strBuf, short maxLen);



#include <unordered_map>
static std::unordered_map<short, std::string> actionAndConditionParamTypes
{
	// Taken from "Params" struct
	{ 1, "object" },
	{ 5, "int" },
	{ 22, "int" }, // Expression
	{ 28, "text number" },
	{ 40, "filename" },
	{ 41, "string" },
	{ 45, "string" },
	{ 63, "filename" }
};

static std::unordered_map<short, std::string> expressionParamTypes
{
	{ 1, "int" },
	{ 3, "string"},
};

const char * Extension::GetAllActionInfos()
{
	error.clear();
	error.str("");
	if (filename.empty())
	{
		error << "Select a filename first, ya plank.";
		return Runtime.CopyString("");
	}

	HINSTANCE hGetProcIDDLL = LoadLibraryA(filename.c_str());

	if (!hGetProcIDDLL) {
		error << "could not load the dynamic library";
		return Runtime.CopyString("");
	}

	GetXXXStringFunc getActionStringFunc = (GetXXXStringFunc)GetProcAddress(hGetProcIDDLL, "GetActionString");
	GetXXXInfosFunc getActionInfosFunc = (GetXXXInfosFunc)GetProcAddress(hGetProcIDDLL, "GetActionInfos");
	GetXXXTitleFunc getActionTitleFunc = (GetXXXTitleFunc)GetProcAddress(hGetProcIDDLL, "GetActionTitle");

	if (!getActionStringFunc) {
		error << "Could not locate function GetActionString";
		return Runtime.CopyString("");
	}
	if (!getActionInfosFunc) {
		error << "Could not locate function GetActionInfos";
		return Runtime.CopyString("");
	}
	if (!getActionTitleFunc) {
		error << "Could not locate function GetActionTitle";
		return Runtime.CopyString("");
	}

	std::stringstream output;
	const size_t bufferSize = 512U;
	char * buffer = (char *)malloc(bufferSize);
	if (!buffer) {
		error << "Could not allocate memory";
		return Runtime.CopyString("");
	}

	//	short DLLExport GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
	//	returns in infoPtr the number of actions
	for (size_t i = 0; ; i++)
	{
		try {
			getActionStringFunc(SDK->mV, i, buffer, bufferSize);
			if (buffer[0] == '\0')
				break;

			if (i < 10)
				output << "0";
			output << i << ": " << buffer << " (";
			// Lazy man's pointer decrementation, see ACEInfo->MMFPtr()
			// fun->FloatFlags is not valid and should not be read
			ACEInfo * fun = (ACEInfo *)(((char *)getActionInfosFunc(SDK->mV, i)) - 2); // don't free
			for (size_t j = 0; j < fun->NumOfParams; j++)
			{
				if (actionAndConditionParamTypes.find(fun->Parameter[j]) == actionAndConditionParamTypes.cend())
					output << "<UNKNOWN TYPE Params val " << fun->Parameter[j] << "> ";
				else
					output << actionAndConditionParamTypes[fun->Parameter[j]] << " ";
				getActionTitleFunc(SDK->mV, i, j, buffer, bufferSize);
				output << "\"" << buffer << ((j == fun->NumOfParams - 1) ? "\" " : "\", ");
			}
			output << ")\r\n";
			buffer[0] = '\0';
		}
		catch (...)
		{
			break;
		}
	}
	free(buffer);
	return Runtime.CopyString(output.str().c_str());
}

const char * Extension::GetAllConditionInfos()
{
	error.clear();
	error.str("");
	if (filename.empty())
	{
		error << "Select a filename first, ya plank.";
		return Runtime.CopyString("");
	}

	HINSTANCE hGetProcIDDLL = LoadLibraryA(filename.c_str());

	if (!hGetProcIDDLL) {
		error << "could not load the dynamic library";
		return Runtime.CopyString("");
	}

	GetXXXStringFunc getConditionStringFunc = (GetXXXStringFunc)GetProcAddress(hGetProcIDDLL, "GetConditionString");
	GetXXXInfosFunc getConditionInfosFunc = (GetXXXInfosFunc)GetProcAddress(hGetProcIDDLL, "GetConditionInfos");
	GetXXXTitleFunc getConditionTitleFunc = (GetXXXTitleFunc)GetProcAddress(hGetProcIDDLL, "GetConditionTitle");

	if (!getConditionStringFunc) {
		error << "Could not locate function GetConditionString";
		return Runtime.CopyString("");
	}
	if (!getConditionInfosFunc) {
		error << "Could not locate function GetConditionInfos";
		return Runtime.CopyString("");
	}
	if (!getConditionTitleFunc) {
		error << "Could not locate function GetConditionTitle";
		return Runtime.CopyString("");
	}

	std::stringstream output;
	const size_t bufferSize = 512U;
	char * buffer = (char *)malloc(bufferSize);
	if (!buffer) {
		error << "Could not allocate memory";
		return Runtime.CopyString("");
	}

	//	short DLLExport GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
	//	returns in infoPtr the number of conditions
	for (size_t i = 0; ; i++)
	{
		try {
			getConditionStringFunc(SDK->mV, i, buffer, bufferSize);
			if (buffer[0] == '\0')
				break;

			if (i < 10)
				output << "0";
			output << i << ": " << buffer << " (";
			// Lazy man's pointer decrementation, see ACEInfo->MMFPtr()
			// fun->FloatFlags is not valid and should not be read
			ACEInfo * fun = (ACEInfo *)(((char *)getConditionInfosFunc(SDK->mV, i)) - 2); // don't free
			for (size_t j = 0; j < fun->NumOfParams; j++)
			{
				if (actionAndConditionParamTypes.find(fun->Parameter[j]) == actionAndConditionParamTypes.cend())
					output << "<UNKNOWN TYPE Params val " << fun->Parameter[j] << "> ";
				else
					output << actionAndConditionParamTypes[fun->Parameter[j]] << " ";
				getConditionTitleFunc(SDK->mV, i, j, buffer, bufferSize);
				output << "\"" << buffer << ((j == fun->NumOfParams - 1) ? "\" " : "\", ");
			}
			output << ")\r\n";
			buffer[0] = '\0';
		}
		catch (...)
		{
			break;
		}
	}
	free(buffer);
	return Runtime.CopyString(output.str().c_str());
}

const char * Extension::GetAllExpressionInfos()
{
	error.clear();
	error.str("");
	if (filename.empty())
	{
		error << "Select a filename first, ya plank.";
		return Runtime.CopyString("");
	}

	HINSTANCE hGetProcIDDLL = LoadLibraryA(filename.c_str());

	if (!hGetProcIDDLL) {
		error << "could not load the dynamic library";
		return Runtime.CopyString("");
	}

	GetXXXStringFunc getExpressionStringFunc = (GetXXXStringFunc)GetProcAddress(hGetProcIDDLL, "GetExpressionString");
	GetXXXInfosFunc getExpressionInfosFunc = (GetXXXInfosFunc)GetProcAddress(hGetProcIDDLL, "GetExpressionInfos");
	GetXXXTitleFunc getExpressionTitleFunc = (GetXXXTitleFunc)GetProcAddress(hGetProcIDDLL, "GetExpressionTitle");
	GetExpressionParamFunc getExpressionParamFunc = (GetExpressionParamFunc)GetProcAddress(hGetProcIDDLL, "GetExpressionParam");
	
	if (!getExpressionStringFunc) {
		error << "Could not locate function GetExpressionString";
		return Runtime.CopyString("");
	}
	if (!getExpressionInfosFunc) {
		error << "Could not locate function GetExpressionInfos";
		return Runtime.CopyString("");
	}
	if (!getExpressionTitleFunc) {
		error << "Could not locate function GetExpressionTitle";
		return Runtime.CopyString("");
	}

	std::stringstream output;
	const size_t bufferSize = 512U;
	char * buffer = (char *)malloc(bufferSize);
	if (!buffer) {
		error << "Could not allocate memory";
		return Runtime.CopyString("");
	}

	//	short DLLExport GetRunObjectInfos(mv * mV, kpxRunInfos * infoPtr)
	//	returns in infoPtr the number of expressions
	for (size_t i = 0; ; i++)
	{
		try {
			getExpressionStringFunc(SDK->mV, i, buffer, bufferSize);
			if (buffer[0] == '\0')
				break;

			if (i < 10)
				output << "0";
			output << i << ": " << buffer; // ( at end of buffer
			// Lazy man's pointer decrementation, see ACEInfo->MMFPtr()
			// fun->FloatFlags is not valid and should not be read
			ACEInfo * fun = (ACEInfo *)(((char *)getExpressionInfosFunc(SDK->mV, i)) - 2); // don't free

			for (size_t j = 0; j < fun->NumOfParams; j++)
			{
				if (expressionParamTypes.find(fun->Parameter[j]) == expressionParamTypes.cend())
					output << "<UNKNOWN TYPE ExpParams val " << fun->Parameter[j];
				else
					output << "<" << expressionParamTypes[fun->Parameter[j]];
				getExpressionParamFunc(SDK->mV, i, j, buffer, bufferSize);
				output << " " << buffer << ((j == fun->NumOfParams - 1) ? ">" : ">, ");
			}
			output << ")\r\n";
			buffer[0] = '\0';
		}
		catch (...)
		{
			break;
		}
	}
	free(buffer);
	return Runtime.CopyString(output.str().c_str());
}
