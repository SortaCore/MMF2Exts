
#include "Common.h"

const char * Extension::GetFormat(const char * file)
{
	lastFile = "";
	lastError.clear();
	FILE * fil;

	// Read and write, must pre-exist.
	if (fopen_s(&fil, file, "rb+"))
	{
		lastError << "fopen_s failed with error " << errno << ".";
		Runtime.PushEvent(1);
		return Runtime.CopyString("Err");
	}

	std::string format = GetFileFormat(fil, nullptr, nullptr, nullptr, nullptr);
	if (format != "Err")
	{
		fclose(fil);
		Runtime.PushEvent(1);
		return Runtime.CopyString("Err");
	}

	return Runtime.CopyString(format.c_str());
}

const char * Extension::GetLastConverted()
{
	return Runtime.CopyString(lastFile.c_str());
}

const char * Extension::GetLastError()
{
	return Runtime.CopyString(lastError.str().c_str());
}