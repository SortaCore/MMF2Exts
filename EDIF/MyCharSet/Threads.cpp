#include "Common.h"

// Change BOM mark in a file
void WINAPI SetBOMMarkASCThread(void * Param)
{
	// Invalid parameter (We'll assume all the struct variables are valid if Param is valid)
	if(!Param)
		return;

	BOMThreadData * Data = (BOMThreadData *)Param;
	std::string ReadFromFilePath = Data->FileToAddTo, 
				WriteToFilePath = Data->FileToAddTo;
	WriteToFilePath += ".tmp";
	bool IgnoreCurrentBOM = Data->IgnoreCurrentBOM;
	delete Param;
	
	// Open file
	FILE * ReadFromFile = NULL, * WriteToFile = NULL;
	if (fopen_s(&ReadFromFile, ReadFromFilePath.c_str(), "rb") || !ReadFromFile)
		return;

	if (fopen_s(&WriteToFile, WriteToFilePath.c_str(), "ab") || !WriteToFile)
		return;

	fseek(ReadFromFile, 0, SEEK_END);
	long FileSize = ftell(ReadFromFile);
	if (Data->IgnoreCurrentBOM == false && FileSize >= 3)
	{
		fseek(ReadFromFile, 0, SEEK_SET);
		char Front [3] = {0};
		
		// Read 3 bytes (UTF-8 BOM is 3 bytes)
		size_t s = fread_s(Front, 3, 1, 3, ReadFromFile);
		if (s < 2)
		{
			// Errorz
			fclose(ReadFromFile);
			fclose(WriteToFile);
			return;
		}

		// UTF-16 BOM in original file (0xFF, 0xFE, N/A)
		if (s >= 2 && Data->TypeOfBOM != 2 &&
			Front[0] == 0xFF && Front[1] == 0xFE)
		{
			fseek(ReadFromFile, 2, SEEK_SET); // UTF-16 BOM is 2 bytes, not 3
		}
		else // !UTF-8 (0xEF, 0xBB, 0xBF), so ANSI file 
		{
			if (!(s >= 3 && Data->TypeOfBOM != 1 &&
				Front[0] == 0xEF &&	Front[1] == 0xBB && Front[2] == 0xBF))
			{
				fseek(ReadFromFile, 0, SEEK_SET); // ANSI doesn't need a BOM, go back
			}
			/*	else UTF-8, 3-byte BOM, fread() skipped over it already */
		}
	}
		
	fseek(ReadFromFile, 0, SEEK_SET);

	// UTF-8
	if (Data->TypeOfBOM == 1)
	{
		char UTF8_BOM[2] = { 0xFF, 0xFE };
		if (fwrite(UTF8_BOM, sizeof(char), 2, WriteToFile) != 2)
		{
			fclose(ReadFromFile);
			fclose(WriteToFile);
			remove(WriteToFilePath.c_str());
			return;
		}
	}
	// UTF-16
	else if (Data->TypeOfBOM == 2)
	{
		char UTF16_BOM[3] = { 0xEF, 0xBB, 0xBF };
		if (fwrite(UTF16_BOM, sizeof(char), 3, WriteToFile) != 3)
		{
			fclose(ReadFromFile);
			fclose(WriteToFile);
			remove(WriteToFilePath.c_str());
			return;
		}
	}

	// Stream the original file to end of new file
	char Buffer [255];
	size_t BytesRead;
	while ((BytesRead = fread_s(Buffer, 255, sizeof(char), 255, ReadFromFile)) > 0)
	{
		//
		if (fwrite(Buffer, sizeof(char), BytesRead, WriteToFile) != BytesRead)
		{
			fclose(ReadFromFile);
			fclose(WriteToFile);
			remove(WriteToFilePath.c_str());
			return;
		}
	}

	// Cleanup
	fclose(ReadFromFile);
	fclose(WriteToFile);
	remove(ReadFromFilePath.c_str());
	rename(WriteToFilePath.c_str(), ReadFromFilePath.c_str());
	return;
}
