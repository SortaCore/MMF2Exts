
#include "Common.h"


// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	return UTF8ToWide(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str)
{
	return WideToUTF8(str);
}



int Extension::UTF16CharToUTF16Int(const wchar_t * utf16Char)
{
	return *((unsigned short *)utf16Char);
}

const wchar_t * Extension::UTF16IntToUTF16Char(unsigned int utf16int)
{
	wchar_t * wc = (wchar_t *)Runtime.Allocate(4U);
	wc[0] = (unsigned short)utf16int;
	wc[1] = 0;
	return wc;
}

const wchar_t * Extension::UTF16StrFromUTF16Mem(long addr, int numChars)
{
	// obvious bad pointer is obvious
	if (addr == 0xDDDDDDDL || addr == 0x0000000L || addr == 0xFFFFFFFL)
		return Runtime.CopyString(L"<err, bad pointer>");
	// numChars cannot be below -1. Bad juju.
	if (numChars < -1)
		return Runtime.CopyString(L"<err, numChars is below -1>");
	// 0-char string is easy.
	if (numChars == 0)
		return Runtime.CopyString(L"");

	if (numChars == -1)
		return Runtime.CopyString((wchar_t *)addr);

	// NB: wstring ctor copies the memory.
	return Runtime.CopyString(std::wstring((wchar_t *)addr, (size_t)numChars).c_str());
}
const wchar_t * Extension::UTF16StrFromUTF8Mem(long addr, int numChars)
{
	// obvious bad pointer is obvious
	if (addr == 0xDDDDDDDL || addr == 0x0000000L || addr == 0xFFFFFFFL)
		return Runtime.CopyString(L"<err, bad pointer>");
	// numChars cannot be below -1. Bad juju.
	if (numChars < -1)
		return Runtime.CopyString(L"<err, numChars is below -1>");
	// 0-char string is easy.
	if (numChars == 0)
		return Runtime.CopyString(L"");

	std::string str;
	if (numChars == -1)
		str = (char *)addr;
	else
		str = std::string((char *)addr, (size_t)numChars);

	std::wstring str2 = utf8_to_wstring(str);
	// NB: wstring ctor copies the memory.
	return Runtime.CopyString(str2.c_str());
}
#if 0==1
int Extension::UTF16CharToUTF8Int(const wchar_t * utf16Char)
{
	unsigned short byte1 = *((unsigned short *)utf16Char);
	unsigned int cp = 0;
	// U+0000 to U+D7FF and U+E000 to U+FFFF
	// Code point bits 2 bytes, exact numerical equivalent
	if (byte1 <= 0xD7FF || (byte1 >= 0xE000 && cp <= 0xFFFF))
		cp = byte1;
	else if (byte1)
}
const wchar_t * Extension::UTF8IntToUTF16Char(unsigned int byte1)
{
	unsigned __int64 cp = 0;

	// Code point bits 1 byte
	if (byte1 & 0b10000000 == 0)
		cp = (byte1 & 0b01111111) << 32;
	// Code point bits 2 bytes
	else if ((byte1 & 0b11000000) == 0b11000000 && (byte1 & 0b00100000) == 0)
		cp = (byte1 & 0b0001111100111111) << 32;
	// Code point bits 3 bytes
	else if ((byte1 & 0b11100000) == 0b11100000 && (byte1 & 0b00010000) == 0)
		cp = (byte1 & 0b000011110011111100111111) << 32;
	// Code point bits 4 bytes
	else if ((byte1 & 0b11110000) == 0b11110000 && (byte1 & 0b00001000) == 0)
		cp = (byte1 & 0b00000111001111110011111100111111) << 32;
	// In November 2003, UTF-8 was restricted by RFC 3629 to end at U+10FFFF, in order to match the
	// constraints of the UTF-16 character encoding. This removed all five- and six-byte sequences,
	// and 983,040 four-byte sequences.
	
	// Code point bits 5 bytes
	else if ((byte1 & 0b11111000) == 0b11111000 && (byte1 & 0b00000100) == 0)
		//cp = ((byte1 & 0b00000111001111110011111100111111) << 32) | (byte2 & 0b00000000000000000011111100111111);
		return Runtime.CopyString(L"<deprecated 5-byte UTF-8>");
	// Code point bits 6 bytes
	else if ((byte1 & 0b11111100) == 0b11111100 && (byte1 & 0b00000010) == 0)
		//cp = ((byte1 & 0b00000111001111110011111100111111) << 32) | (byte2 & 0b00000000000000000000000000111111);
		return Runtime.CopyString(L"<deprecated 6-byte UTF-8>");
	else
		return Runtime.CopyString(L"<Not Valid>");

	wchar_t * wc = (wchar_t *)Runtime.Allocate(6U);
	*((unsigned int *)wc) = cp;
	wc[2] = 0;

	return wc;
}

#endif
