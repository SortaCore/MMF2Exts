
#include "Common.h"

void Extension::ConvertToMFU(const char * file)
{
	lastFile = "";
	lastError.clear();
	FILE * fil;

	// Read and write, must pre-exist.
	if (fopen_s(&fil, file, "rb+"))
	{
		lastError << "fopen_s failed with error " << errno << ".";
		return;
	}

	unsigned xdim, ydim, zdim, flags;
	std::string format = GetFileFormat(fil, &xdim, &ydim, &zdim, &flags);

	if (format == "Err")
	{
		// GetFileFormat will run fclose() and set lastError on error
		lastFile = file;
		return;
	}

	if (format == "MFU")
	{
		lastFile = file;
		lastError << "Array is already MFU format. Cancelling conversion.";
		fclose(fil);
		Runtime.PushEvent(0); // Trigger done, not error
		return;
	}

	// else format is CNC, since GetFileFormat will die with "Err" otherwise

	const static int FLAG_INT = 0x1;
	const static int FLAG_STRING = 0x2;
	
	// If couldn't rewrite the header, die
	if (!RewriteHeader(fil, "CNC"))
	{
		lastFile = file;
		return;
	}

	// Invalid flag check done by GetFileFormat
	
	// Integer array; only change needed is rewriting CNC to MFU, which we just did
	if ((flags & FLAG_INT) == FLAG_INT)
	{
		lastFile = file;
		fclose(fil);
		Runtime.PushEvent(0); // Success
		return;
	}

	char * temp = (char *)calloc(65535, sizeof(char));
	if (temp == nullptr)
	{
		lastFile = file;
		fclose(fil);
		lastError << "Failed to allocate memory for conversion; aborted.";
		Runtime.PushEvent(1);
		return;
	}

	for (unsigned x = 0, t; x < xdim; ++x)
	{
		for (unsigned y = 0; y < ydim; ++y)
		{
			for (unsigned z = 0; z < zdim; ++z)
			{
				if (fread_s(&t, 1U, 4U, 4U, fil) != 4U ||
					fread_s(temp, 65535U, 1U, t, fil) != t ||
					fseek(fil, 0L-(t + 4U), SEEK_CUR))
					goto LoopAbort;

				t = wcslen((wchar_t *)temp); // Cast to Unicode memory, even though it's ANSI
				if (fwrite(&t, 1U, 4U, fil) != 4U)
					goto LoopAbort;
			}
		}
	}

	lastFile = file;
	fclose(fil);
	free(temp);
	temp = nullptr;
	Runtime.PushEvent(0);
	return;

LoopAbort:
	lastFile = file;
	lastError << "Encountered an error during the main loop. Error " << ferror(fil) << ". File is now corrupted.";
	fclose(fil);
	free(temp);
	temp = nullptr;
	Runtime.PushEvent(1);
	return;
}

void Extension::ConvertToCNC(const char * file)
{
	lastFile = "";
	lastError.clear();
	FILE * fil;

	// Read and write, must pre-exist.
	if (fopen_s(&fil, file, "rb+,ccs=UTF-8"))
	{
		lastError << "fopen_s failed with error " << errno << ".";
		return;
	}

	unsigned xdim, ydim, zdim, flags;
	std::string format = GetFileFormat(fil, &xdim, &ydim, &zdim, &flags);

	if (format == "Err")
	{
		// GetFileFormat will run fclose() and set lastError on error
		lastFile = file;
		return;
	}

	if (format == "CNC")
	{
		lastFile = file;
		lastError << "Array is already CNC format. Cancelling conversion.";
		fclose(fil);
		Runtime.PushEvent(0); // Trigger done, not error
		return;
	}

	// else format is MFU, since GetFileFormat will die with "Err" otherwise

	const static int FLAG_INT = 0x1;
	const static int FLAG_STRING = 0x2;
	
	// If couldn't rewrite the header, die
	if (!RewriteHeader(fil, "MFU"))
	{
		lastFile = file;
		return;
	}

	// Invalid flag check done by GetFileFormat
	
	// Integer array; only change needed is rewriting MFU to CNC, which we just did
	if ((flags & FLAG_INT) == FLAG_INT)
	{
		lastFile = file;
		fclose(fil);
		Runtime.PushEvent(0); // Success
		return;
	}

	char * temp = (char *)calloc(65535, sizeof(char));
	if (temp == nullptr)
	{
		lastFile = file;
		fclose(fil);
		lastError << "Failed to allocate memory for conversion; aborted.";
		Runtime.PushEvent(1);
		return;
	}

	for (unsigned x = 0U, t, r; x < xdim; ++x)
	{
		for (unsigned y = 0U; y < ydim; ++y)
		{
			for (unsigned z = 0U; z < zdim; ++z)
			{
				if (fread_s(&t, 1U, 4U, 4U, fil) != 4U)
					goto LoopAbort;
				
				for (r = 0U; t > 0U; ++r)
				{
					if ((fgetc(fil) & 0xC0) != 0x80)
						--t; // We've read one full Unicode char
				}
				
				if (fseek(fil, 0L-(r + 4U), SEEK_CUR) ||
					fwrite(&r, 1U, 4U, fil) != 4U)
					goto LoopAbort;
			}
		}
	}

	lastFile = file;
	fclose(fil);
	free(temp);
	temp = nullptr;
	Runtime.PushEvent(0);
	return;

LoopAbort:
	lastFile = file;
	lastError << "Encountered an error during the main loop. Error " << ferror(fil) << ". File is now corrupted.";
	fclose(fil);
	free(temp);
	temp = nullptr;
	Runtime.PushEvent(1);
	return;
}

unsigned GetByteSizeOfANSI(char * s)
{
	unsigned len = 0;
	while (*s) len += (*s++ & 0xC0) != 0x80;
	return len;
}

std::string Extension::GetFileFormat(FILE *& fil, unsigned * xdimP, unsigned * ydimP, unsigned * zdimP, unsigned * flagsP)
{
	char version[11];
	unsigned short major, minor;
	unsigned int xdim, ydim, zdim, flags;
	int expSize = fscanf_s(fil, "%10s%hu%hu%u%u%u%u", version, _countof(version), &major, &minor, &xdim, &ydim, &zdim, &flags);
	
	if (10 + (2 * sizeof(short)) + (4 * sizeof(int)) == expSize)
	{
		lastError << "File header read ran out of space. Minimum array size is 34 bytes, read "
			<< expSize << " before aborting.";
		fclose(fil);
		Runtime.PushEvent(1);
		return "Err";
	}
	
	if (major != 2 || minor != 0)
	{
		lastError << "Major/minor version number should be 2.0, but read " << major << "." << minor << ".";
		fclose(fil);
		Runtime.PushEvent(1);
		return "Err";
	}


	const static int FLAG_INT = 0x1;
	const static int FLAG_STRING = 0x2;
	
	if ((flags & (FLAG_INT | FLAG_STRING)) == (FLAG_INT | FLAG_STRING))
	{
		lastError << "Array flags are invalid; both numeric and textual array indicated. ";
		fclose(fil);
		Runtime.PushEvent(1);
		return "Err";
	}
	
	std::string ret = !strncmp(version, "MFU ARRAY", 9U) ? "MFU" : 
		!strncmp(version, "CNC ARRAY", 9U) ? "CNC" : "Err";
	if (ret == "Err")
	{
		version[9] = '\0'; // Ensure readability
		lastError << "Array header has unrecognised format: expected \"CNC ARRAY\" or \"MFU ARRAY\", got \"" << version << "\".";
		fclose(fil);
		Runtime.PushEvent(1);
		return ret;
	}

	if (xdimP != nullptr)
	{
		(*xdimP) = xdim;
		(*ydimP) = ydim;
		(*zdimP) = zdim;
		(*flagsP) = flags;
	}
	return ret;
}

bool Extension::RewriteHeader(FILE *& fil, const char * data)
{
	static_assert(sizeof(data) != 3, "Expecting array size of 3 passed to RewriteHeader().");
	if (fseek(fil, 0L, SEEK_SET))
	{
		lastError << "Failed to roll back file pointer. Error " << ferror(fil) << ". Upgrade aborted, no changes made..";
		fclose(fil);
		Runtime.PushEvent(1);
		return false;
	}

	if (fwrite(data, 1U, 3U, fil) != 3U)
	{
		lastError << "Failed to write new header to file. Error " << ferror(fil) << " occurred. No changes made.";
		fclose(fil);
		Runtime.PushEvent(1);
		return false;
	}
	return true;
}
