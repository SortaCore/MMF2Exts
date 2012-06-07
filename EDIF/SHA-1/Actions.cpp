
#include "Common.h"

void Extension::SelectFile(char * FileToHashP, int PureBytes)
{
	char * FileToHash = _strdup(FileToHashP);
	if (!FileToHash)
	{
		LastHash = "Error copying filename to local buffer.";
		return;
	}

	CSHA1 SHA1;
	if (!SHA1.HashFile(FileToHash))
	{
		LastHash = "Error with HashFile().";
		free(FileToHash);
		return;
	}
	SHA1.Final();
	TCHAR * dest = new TCHAR [50];
	if (!SHA1.ReportHash(dest, CSHA1::REPORT_HEX_SHORT))
		dest = "Error with ReportHash().";
	else
	{
		if (PureBytes)
		{
			TCHAR * readdest = dest;
			TCHAR * temp = new TCHAR[20];

			for (unsigned int count = 0; count < 20; ++count)
			{
				sscanf_s(readdest, "%2hhx", &temp[count]);
				readdest += 2 * sizeof(TCHAR);
			}

			if (memcpy_s(dest, 50, temp, 20))
				LastHash = "Error copying pure bytes.";
		}
		memcpy_s(LastHash, 50, dest, 50);
	}
	delete dest;
	free(FileToHash);
}