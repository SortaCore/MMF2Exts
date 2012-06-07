
#include "Common.h"

char * Extension::ReplaceThisCall(char * SearchIn, char * SearchFor, char * ReplaceWith)
{
	//
	unsigned char RecursionLimit = 0;
	std::string search_in(SearchIn), search_for (SearchFor), replace_with(ReplaceWith);
	while (search_in.find(search_for) != search_in.npos && RecursionLimit++ != 255)
		search_in.replace(search_in.find(SearchFor), search_for.size(), replace_with);
	return Runtime.CopyString(search_in.c_str());
}

char * Extension::AutoReplace(char * SearchIn)
{
	std::string search_in(SearchIn), search_for, replace_with;
	
	for (unsigned int i = 0; i < Macros.size(); ++i)
	{
		unsigned char RecursionLimit = 0;	
		search_for = Macros[i].FindThis;
		replace_with = Macros[i].ReplaceWithThis;
		while (search_in.find(search_for) != search_in.npos && RecursionLimit++ != 255)
			search_in.replace(search_in.find(search_for), search_for.size(), replace_with);
	}
	MessageBoxA(NULL, search_in.c_str(), "Output:", MB_OK);
	return Runtime.CopyString(search_in.c_str());

}