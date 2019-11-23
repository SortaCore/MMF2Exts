
#include "Common.h"

const TCHAR * Extension::GetLastHash()
{
	return LastHash;
}
unsigned int Extension::GetLastHashAddress()
{
	return (unsigned int)LastHash;
}
