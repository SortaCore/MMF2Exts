
#include "Common.h"

void Extension::SelectFile(TCHAR * FileToHashP, int PureBytes)
{
	std::basic_string<TCHAR> FileToHash(FileToHashP);

	CSHA1 SHA1;
	if (!SHA1.HashFile(FileToHash.c_str()))
	{
		_tcscpy_s(LastHash, _T("Error with HashFile()."));
		return;
	}
	SHA1.Final();

	TCHAR dest[50];
	if (!SHA1.ReportHash(dest, CSHA1::REPORT_HEX_SHORT))
		_tcscpy_s(LastHash, _T("Error with ReportHash()."));
	else
	{
		if (PureBytes)
		{
			const TCHAR * readdest = dest;
			TCHAR temp[20];

			for (unsigned int count = 0; count < 20; ++count)
			{
				_tscanf_s(readdest, _T("%2hhx"), &temp[count]);
				readdest += 2 * sizeof(TCHAR);
			}

			if (memcpy_s(dest, sizeof(dest), temp, sizeof(temp)))
				_tcscpy_s(LastHash, _T("Error copying pure bytes."));
		}
		memcpy_s(LastHash, sizeof(LastHash), dest, sizeof(dest));
	}
}
