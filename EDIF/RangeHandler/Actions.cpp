
#include "Common.h"

// ID = 0
void Extension::IterateString(tchar * StringP, tchar * InputDelimiterP)
{
	tstring String = StringP;
	tstring InputDelimiter = InputDelimiterP;

	vector<tstring> Ranges = ParseStringToVector(String, InputDelimiter);

	// Sort ascendingly
	for (unsigned long i = 0; i < Ranges.size()-1; i++)
	{
		// If next in list is less in value than 
		if (_tstoi(Ranges[i].c_str()) > _tstoi(Ranges[i+1].c_str()))
		{
			tstring temp = Ranges[i+1];
			Ranges.insert(Ranges.begin()+i, temp);
			Ranges.erase(Ranges.begin()+i+2);
			i -= 2;
		}

		// If duplicate values
		if (_tstoi(Ranges[i].c_str()) == _tstoi(Ranges[i+1].c_str()))
			Ranges.erase(Ranges.begin()+i);
	}
	
	// Iterate over variables
	for (unsigned long i = 0; i < Ranges.size(); i++)
	{
		CurrentVal = _tstoi(Ranges[i].c_str());
		Runtime.GenerateEvent(0);
	}
	
}

// ID = 1
void Extension::IterateRanges(tchar * StringP, tchar * SingleDelimiter, tchar * MultipleDelimiter)
{
	// Much smaller code ftw!
	ReturnBool = true; // Tells RangeToString to not use Runtime.CopyString on the return
	IterateString(RangeToString(StringP, SingleDelimiter, MultipleDelimiter, _T("|")), _T("|"));
}
