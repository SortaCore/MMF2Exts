// Include guard
#ifndef COMMON_H
#define COMMON_H

#include <tchar.h>
#include <string>

namespace std
{
	using tstring = basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
	using tstringstream = basic_stringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR>>;
}

// General includes
#include "..\Inc\TemplateInc.h"

// Specific to this extension
#include "Resource.h"
#include "FlagsPrefs.h"
#include "Information.h"
#include "Data.h"
#include "..\Lib\Chilkat\CkCrypt2.h"

// https://www.chilkatsoft.com/downloads_vcpp.asp
#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "dnsapi.lib")
#ifdef _DEBUG
#pragma comment (lib, "..\\Lib\\Chilkat\\ChilkatDbg.lib")
#else
#pragma comment (lib, "..\\Lib\\Chilkat\\ChilkatRel.lib")
#endif

// rTemplate include
#include "..\Inc\rTemplate.h"

// Globals and prototypes
extern HINSTANCE hInstLib;
extern short * conditionsInfos;
extern short * actionsInfos;
extern short * expressionsInfos;
extern long (WINAPI ** ConditionJumps)(LPRDATA rdPtr, long param1, long param2);
extern short (WINAPI ** ActionJumps)(LPRDATA rdPtr, long param1, long param2);
extern long (WINAPI ** ExpressionJumps)(LPRDATA rdPtr, long param);
extern PropData Properties[];
extern WORD DebugTree[];

// End include guard
#endif


/* Pages that might be useful:
http://en.wikipedia.org/wiki/PBKDF2
http://msdn.microsoft.com/en-us/library/aa380261(VS.85).aspx
http://www.openbsd.org/cgi-bin/cvsweb/src/sbin/bioctl/pbkdf2.c?rev=HEAD&content-type=text%2Fplain
http://www.google.co.uk/search?hl=en&source=hp&q=C%2B%2B+PBKDF2&aq=f&aqi=g-sx10&aql=&oq=&gs_rfai=
http://pastebin.com/f5ba89711 (C code of PBKDF2)
http://www.xs4all.nl/~rjoris/wpapsk.html
http://anandam.name/pbkdf2/
http://www.example-code.com/vcpp/crypt2_pbkdf2.asp
http://www.chilkatsoft.com/refdoc/vcCkCrypt2Ref.html
http://www.koders.com/cpp/fid1ED0FF03C1C6033A5B0A38128AECAE389C7951AD.aspx?s=crc
*/
