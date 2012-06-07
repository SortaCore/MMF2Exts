
#include "Common.h"

const char * Extension::GetLastHash()
{
    return Extension::LastHash;
}
unsigned int Extension::GetLastHashAddress()
{
	return (unsigned int)Extension::LastHash;
}