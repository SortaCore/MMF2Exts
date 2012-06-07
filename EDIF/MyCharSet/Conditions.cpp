
#include "Common.h"

bool Extension::IsUnicode()
{
#if defined(UNICODE) || defined(_UNICODE)
	return true;
#else
	return false;
#endif
}
