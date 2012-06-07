
#include "Common.h"

#ifdef _DEBUG
#define DebugBoxAF() MessageBoxA(NULL, FromBuffer, "Failure: From buffer:", MB_OK); \
					 MessageBoxW(NULL, ToBuffer, L"Failure: To buffer:", MB_OK);
#define DebugBoxAS() MessageBoxA(NULL, FromBuffer, "Success: From buffer:", MB_OK); \
					 MessageBoxW(NULL, ToBuffer, L"Success: To buffer:", MB_OK);

#define DebugBoxUF() MessageBoxW(NULL, FromBuffer, L"Failure: From buffer:", MB_OK); \
					 MessageBoxA(NULL, ToBuffer, "Failure: To buffer:", MB_OK);
#define DebugBoxUS() MessageBoxW(NULL, FromBuffer, L"Success: From buffer:", MB_OK); \
					 MessageBoxA(NULL, ToBuffer, "Success: To buffer:", MB_OK);
#else
#define DebugBoxAF()
#define DebugBoxAS()
#define DebugBoxUF()
#define DebugBoxUS()
#endif
#define RAW_CONVERSION
//#define MakeNull()
//#define MakeLNull()
#define MakeNull() ToBuffer[(Length*2)-1] = '\0';
#define MakeLNull() ToBuffer[(Length/2)] = L'\0';
char * Extension::GetASCIIStringFromUnicodeString(wchar_t * Unicode, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = wcslen(Unicode);
	else
		Length = Size;
	wchar_t * FromBuffer = new wchar_t [Length+1];
	char * ToBuffer = new char [(Length+1)*2];
	wcscpy_s(FromBuffer, Length+1, Unicode);

#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (WideCharToMultiByte(cp,          // Code page (UTF8 or ASCII)
						   NULL,        // Modification flags (unused here)
						   FromBuffer,  // Address of source (wide char)
						   Length,      // Size of source (-1 for guess, but no need for that!)
						   ToBuffer,    // ASCII conversion
						   Length,      // Size of destination
						   NULL,        // Default char (NULL uses system default)
						   NULL) == 0)  // Whether it was used (NULL ignores)
	{

		DebugBoxUF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxUS()
		string ASCIIConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return Runtime.CopyString(ASCIIConv.c_str());
	}
#else
	for (unsigned int i = 0; i < Length; i++)
	{
		ToBuffer[i*2] = (char)(LOBYTE(FromBuffer[i]));
		ToBuffer[i*2+1] = (char)(HIBYTE(FromBuffer[i]));
	}
	MakeNull()
	DebugBoxUS()
	string ASCIIConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return Runtime.CopyString(ASCIIConv.c_str());
#endif
}

wchar_t * Extension::GetUnicodeStringFromASCIIString(char * ASCII, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = strlen(ASCII)+1;
	else
		Length = Size+1;
	char * FromBuffer = new char [Length];
	wchar_t * ToBuffer = new wchar_t [Length];
	strcpy_s(FromBuffer, Length, ASCII);

#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (MultiByteToWideChar(cp,				// Code page (UTF8 or ASCII)
						   NULL,			// Modification flags (unused here)
						   FromBuffer,		// Address of source (wide char)
						   Length,			// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,		// ASCII conversion
						   Length) == 0)	// Size of destination
	{
		DebugBoxAF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxAS()
		wstring UnicodeConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return Runtime.CopyString(UnicodeConv.c_str());
	}
#else
	for (unsigned int i = 0; i < (Length/2); i++)
	{
		ToBuffer[i] = MAKEWORD(FromBuffer[i*2], FromBuffer[i*2+1]);
	}
	MakeLNull()
	DebugBoxAS()
	wstring UnicodeConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return Runtime.CopyString(UnicodeConv.c_str());
#endif
}

char * Extension::GetASCIIStringFromUnicodeMemory(int Address, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = wcslen((wchar_t *)Address)+1;
	else
		Length = Size+1;

	wchar_t * FromBuffer = new wchar_t [Length];
	char * ToBuffer = new char [Length*2];
	wcscpy_s(FromBuffer, Length, (wchar_t *)Address);
#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (WideCharToMultiByte(cp,			// Code page (UTF8 or ASCII)
						   NULL,		// Modification flags (unused here)
						   FromBuffer,	// Address of source (wide char)
						   Length,		// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,	// ASCII conversion
						   Length,		// Size of destination
						   NULL,		// Default char (NULL uses system default)
						   NULL) == 0)	// Whether it was used (NULL ignores)
	{
		DebugBoxUF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxUS()
		string ASCIIConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return Runtime.CopyString(ASCIIConv.c_str());
	}
#else
#if 0
	for (unsigned int i = 0; i < Length; i++)
	{
		ToBuffer[(i*2)] = (char)(LOBYTE(FromBuffer[i]));
		ToBuffer[(i*2+1)] = (char)(HIBYTE(FromBuffer[i]));
	}
#else
	for (unsigned int i = 0; i < Length; i++)
	{
		ToBuffer[(i*2)] = (char) ((uchar)((ushort)FromBuffer[i] & 0xff));
		ToBuffer[((i*2)+1)] = (char) ((uchar)(((ushort)FromBuffer[i] >> 8) & 0xff));
	}
#endif
	MakeNull()
	DebugBoxUS()
	string ASCIIConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return Runtime.CopyString(ASCIIConv.c_str());
#endif
}

wchar_t * Extension::GetUnicodeStringFromASCIIMemory(int Address, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = strlen((char *)Address);
	else
		Length = Size;
	wstringstream ss;
	if (Length % 2 == 5)
	{
		ss << "Length is odd; -1\n";
		MsgBox("Length is odd...");
		Length--;
	}
	char * FromBuffer = new char [Length+1];
	wchar_t * ToBuffer = new wchar_t [(Length/2)+1];
	strcpy_s(FromBuffer, Length+1, (char *)Address);
	ss << "FROM:\n[" << FromBuffer << "]\n";

#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (MultiByteToWideChar(cp,				// Code page (UTF8 or ASCII)
						   NULL,			// Modification flags (unused here)
						   FromBuffer,		// Address of source (wide char)
						   Length,			// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,		// ASCII conversion
						   Length) == 0)	// Size of destination
	{
		DebugBoxAF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from multibyte/ASCII to Unicode.");
		return 0;
	}
	else
	{
		DebugBoxAS()
		wstring UnicodeConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return Runtime.CopyString(UnicodeConv.c_str());
	}
#else
#define REPLACERCHAR SCHAR_MAX
#define HaveFun(a,b) (wchar_t)((((uchar)(a) & 0xff)) | ((((uchar)(b)) & 0xff) << 8))
	for (unsigned int i = 0; i < (Length/2); i++)
	{
			 if (FromBuffer[(i*2)] == REPLACERCHAR && FromBuffer[((i*2)+1)] != REPLACERCHAR)
			ToBuffer[i] = HaveFun('\0', FromBuffer[((i*2)+1)]);
		else if (FromBuffer[(i*2)] != REPLACERCHAR && FromBuffer[((i*2)+1)] == REPLACERCHAR)
			ToBuffer[i] = HaveFun(FromBuffer[(i*2)], '\0');
		else if (FromBuffer[(i*2)] == REPLACERCHAR && FromBuffer[((i*2)+1)] == REPLACERCHAR)
			ToBuffer[i] = L'\0';
		else
			ToBuffer[i] = HaveFun(FromBuffer[(i*2)], FromBuffer[((i*2)+1)]);
//		FromBuffer[(i*2)] == REPLACERCHAR
	}
	MakeLNull()
	wstring UnicodeConv (ToBuffer,(Length/2)+1);
	ss << "\n\nTO:\n[" << UnicodeConv.c_str() << "]\n\0";
	wstring s2(ss.str());
	char Fun [10]("\0");
	_itoa(s2.length(), Fun, 10);
	MsgBox(Fun);
	if (s2.length() > 1024)
		FatalBox()
	else
	{
		
	MsgBox("Line 262");
	//! ||||
	//! ||||
	//! \/\/
	
	MessageBoxW(NULL, s2.c_str(), L"A -> U : Debug information:", MB_OK);
	}
	MsgBox("Line 267");
	free(FromBuffer);
	free(ToBuffer);
	MsgBox("Line 270");
	return Runtime.CopyString(UnicodeConv.c_str());
#endif

}


///
size_t Extension::GetASCIIMemoryFromUnicodeString(wchar_t * Unicode, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = wcslen(Unicode);
	else
		Length = Size;
	wchar_t * FromBuffer = new wchar_t [(Length+1)];
	char * ToBuffer = new char [((Length*2)+1)];
	wcscpy_s(FromBuffer, Length+1, Unicode);
	
#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (WideCharToMultiByte(cp,			// Code page (UTF8 or ASCII)
						   NULL,		// Modification flags (unused here)
						   FromBuffer,	// Address of source (wide char)
						   Length,		// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,	// ASCII conversion
						   Length,		// Size of destination
						   NULL,		// Default char (NULL uses system default)
						   NULL) == 0)	// Whether it was used (NULL ignores)
	{
		DebugBoxUF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxUS()
		string ASCIIConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return (size_t)Runtime.CopyString(ASCIIConv.c_str());
	}

#else
	wstringstream ss;
	ss << "FROM:\n[" << FromBuffer << "]\n";
	for (unsigned int i = 0; i < Length; i++)
	{
		if (FromBuffer[i] == L'\0')
		{
			ToBuffer[(i*2)] = REPLACERCHAR;
			ToBuffer[((i*2)+1)] = REPLACERCHAR;
		}
		else
		{
			#if 0
			ToBuffer[(i*2)] = (char)(LOBYTE(FromBuffer[i]));
			ToBuffer[(i*2+1)] = (char)(HIBYTE(FromBuffer[i]));
			#else
			ToBuffer[(i*2)] = (char) ((uchar)((ushort)FromBuffer[i] & 0xff));
			ToBuffer[((i*2)+1)] = (char) ((uchar)(((ushort)FromBuffer[i] >> 8) & 0xff));
			#endif
			if (ToBuffer[(i*2)] == '\0')
				ToBuffer[(i*2)] = REPLACERCHAR;
			if (ToBuffer[((i*2)+1)] == '\0')
				ToBuffer[((i*2)+1)] = REPLACERCHAR;
		}
		wchar_t a = ToBuffer[(i*2)];
		wchar_t b = ToBuffer[((i*2)+1)];
		
		ss << "\nIndex [" << i << "] of [" << Length << "], which should be [" << a << "] + [" << b << "], appended for [" << FromBuffer[i] <<"].";
	}

	MakeNull()
	
	string ASCIIConv (ToBuffer, Length*2);
	ss << "\n\nTO:\n[" << ASCIIConv.c_str() << "]";
	wstring s2 = ss.str();
	MessageBoxW(NULL, s2.c_str(), L"U -> A : Debug information:", MB_OK);	
	free(FromBuffer);
	free(ToBuffer);

	return (size_t)Runtime.CopyString(ASCIIConv.c_str());
#endif
}

size_t Extension::GetUnicodeMemoryFromASCIIString(char * ASCII, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = strlen(ASCII);
	else
		Length = Size;
	char * FromBuffer = new char [Length];
	wchar_t * ToBuffer = new wchar_t [Length];
	strcpy_s(FromBuffer, Length, ASCII);
#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (MultiByteToWideChar(cp,				// Code page (UTF8 or ASCII)
						   NULL,			// Modification flags (unused here)
						   FromBuffer,		// Address of source (wide char)
						   Length,			// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,		// ASCII conversion
						   Length) == 0)	// Size of destination
	{
		DebugBoxAF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxAS()
		wstring UnicodeConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return (size_t)Runtime.CopyString(UnicodeConv.c_str());
	}
#else
	for (unsigned int i = 0; i < (Length/2); i++)
	{
		ToBuffer[i] = HaveFun(FromBuffer[(i*2)], FromBuffer[((i*2)+1)]);
	}
	MakeLNull()
	DebugBoxAS()
	wstring UnicodeConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return (size_t)Runtime.CopyString(UnicodeConv.c_str());
#endif
}

size_t Extension::GetASCIIMemoryFromUnicodeMemory(int Address, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = wcslen((wchar_t *)Address)+1;
	else
		Length = Size+1;
	wchar_t * FromBuffer = new wchar_t [Length];
	char * ToBuffer = new char [Length*2];
	wcscpy_s(FromBuffer, Length, (wchar_t *)Address);

#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (WideCharToMultiByte(cp,			// Code page (UTF8 or ASCII)
						   NULL,		// Modification flags (unused here)
						   FromBuffer,	// Address of source (wide char)
						   Length,		// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,	// ASCII conversion
						   Length,		// Size of destination
						   NULL,		// Default char (NULL uses system default)
						   NULL) == 0)	// Whether it was used (NULL ignores)
	{
		DebugBoxUF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from wide char to multibyte/ASCII.");
		return 0;
	}
	else
	{
		DebugBoxUS()
		string ASCIIConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return (size_t)Runtime.CopyString(ASCIIConv.c_str());
	}
#else
	for (unsigned int i = 0; i < Length; i++)
	{
		ToBuffer[i*2] = (char)(LOBYTE(FromBuffer[i]));
		ToBuffer[i*2+1] = (char)(HIBYTE(FromBuffer[i]));
	}
	MakeNull()
	DebugBoxUS()
	string ASCIIConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return (size_t)Runtime.CopyString(ASCIIConv.c_str());
#endif
}

size_t Extension::GetUnicodeMemoryFromASCIIMemory(int Address, int Size, int ASCIIOrUTF8)
{
	unsigned long Length = 0;
	if (Size <= 0)
		Length = strlen((char *)Address)+1;
	else
		Length = Size+1;

	char * FromBuffer = new char [Length];
	wchar_t * ToBuffer = new wchar_t [Length];
	strcpy_s(FromBuffer, Length, (char *)Address);
#ifndef RAW_CONVERSION
	// Default to ASCII
	int cp = CP_ACP;
	// Or go to UTF8 if needed
	if (ASCIIOrUTF8)	cp = CP_UTF8;

	if (MultiByteToWideChar(cp,				// Code page (UTF8 or ASCII)
						   NULL,			// Modification flags (unused here)
						   FromBuffer,		// Address of source (wide char)
						   Length,			// Size of source (-1 for guess, but no need for that!)
						   ToBuffer,		// ASCII conversion
						   Length) == 0)	// Size of destination
	{
		DebugBoxAF()
		free(FromBuffer);
		free(ToBuffer);
		MsgBox("Could not convert the string from multibyte/ASCII to Unicode.");
		return 0;
	}
	else
	{
		DebugBoxAS()
		wstring UnicodeConv = ToBuffer;
		free(FromBuffer);
		free(ToBuffer);
		return (size_t)Runtime.CopyString(UnicodeConv.c_str());
	}
#else
	for (unsigned int i = 0; i < (Length/2); i++)
	{
		ToBuffer[i] = MAKEWORD(FromBuffer[i*2], FromBuffer[i*2+1]);
	}
	ToBuffer[Length-1] = L'\0';
	DebugBoxAS()
	wstring UnicodeConv = ToBuffer;
	free(FromBuffer);
	free(ToBuffer);
	return (size_t)Runtime.CopyString(UnicodeConv.c_str());
#endif
}


