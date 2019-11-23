
#include "Common.h"
bool HasFound(tstring InHere, tstring FindThis)
{
	MsgBox(FindThis.c_str());
	MsgBox(InHere.c_str());
	for (size_t i = 0; i < FindThis.length(); i++)
	{
		if (FindThis.substr(i, 1) == InHere)
		//if (_tcscmp((tchar *)InHere.c_str(), (tchar *)((size_t)((size_t)FindThis.c_str())+i*ts)) == 0)
			return true;
	}
	return false;
}
// ID = 0
tchar * Extension::StringToRange(tchar * StringP, tchar * InputDelimiterP, tchar * SingleItemDelimiterP, tchar * MultipleItemDelimiterP)
{
	tstring String = StringP;
	tstring InputDelimiter = InputDelimiterP;
	tstring SingleItemDelimiter = SingleItemDelimiterP;
	tstring MultipleItemDelimiter = MultipleItemDelimiterP;
	
	tstring Return = _T("");
	vector<tstring> Ranges = ParseStringToVector(String, InputDelimiter);

	if (Ranges.size() == 1)
		Return = Ranges[0];
	else if (Ranges.size() > 1)
	{
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
		// Run ranges scan
		long startpos = -1;
		for (unsigned long i = 0; i < Ranges.size()-1; i++)
		{
			// Start position not defined
			if (startpos == -1)
			{
				// If this number is 1 less than next (part of range)
				if (_tstoi(Ranges[i+1].c_str()) - _tstoi(Ranges[i].c_str()) == 1)
				{
					startpos = i;
				}
				else if (_tstoi(Ranges[i+1].c_str()) - _tstoi(Ranges[i].c_str()) > 1)
				{
					Return += Ranges[i];
					Return += SingleItemDelimiter;
				}
			}
			else
			{
				if (_tstoi(Ranges[i+1].c_str()) - _tstoi(Ranges[i].c_str()) > 1)
				{
					Return += Ranges[startpos];
					Return += MultipleItemDelimiter;
					Return += Ranges[i];
					Return += SingleItemDelimiter;
					startpos = -1;
				}
			}
		}
		// Handle last value
		if (startpos != -1)
		{
			Return += Ranges[startpos];
			Return += MultipleItemDelimiter;
			Return += Ranges[Ranges.size()-1];
			// No last element (see lines 74-5)
			startpos = -1;
		}
		else
		{
			Return += Ranges[Ranges.size()-1];
			// No last element (see lines 74-5)
		}
	}
	// Remove last delimiter:
	// Return.erase(Return.length()-SingleItemDelimiter.length());
	
	return Runtime.CopyString(Return.c_str());
}


// ID = 1
tchar * Extension::RangeToString(tchar * StringP, tchar * SingleItemDelimiterP, tchar * MultipleItemDelimiterP, tchar * OutputDelimiterP)
{ 
	tstring String = StringP;
	// Join the two delimiters for ParseStringToVector()
	tstring SingleItemDelimiter = SingleItemDelimiterP;
	tstring MultipleItemDelimiter = MultipleItemDelimiterP;
	tstring OutputDelimiter = OutputDelimiterP;
	
	tstring Return = _T("");
	vector<tstring> Ranges = ParseStringToVector(String, SingleItemDelimiter+MultipleItemDelimiter);
	//Return.resize(MAXSIZE_T);
	if (Ranges.size() == 1)
		Return = Ranges[0];
	else if (Ranges.size() > 1)
	{
		unsigned long alreadyscanned = 0;
		// Run ranges scan
		for (unsigned long i = 0; i < Ranges.size()-1; i++)
		{
			alreadyscanned += Ranges[i].length();
			// Scan the first character after the value:
			// Is single-number character?
			//if (String.find_first_of(SingleItemDelimiter.c_str(), alreadyscanned) != string::npos)
			if (HasFound(String, SingleItemDelimiter))
			{
				Return += Ranges[i];
				Return += OutputDelimiter;
				// Append data
				alreadyscanned += SingleItemDelimiter.length();
			}
			// Is multiple-number character?
			//else if (String.find_first_of(MultipleItemDelimiter.c_str(), alreadyscanned) != string::npos)
			else if (HasFound(String, MultipleItemDelimiter))
			{
				unsigned long diff = _tstoi(Ranges[i+1].c_str())-_tstoi(Ranges[i].c_str())+1;
			
				// Iterate all values and add to list
				for (unsigned int k = 0; k < diff; k++)
				{
					tchar temp[10] = {0};
					_stprintf_s(temp, 10, _T("%i"), _tstoi(Ranges[i].c_str())+k);
					Return += temp;
					Return += OutputDelimiter;
				}
				// Append data
				alreadyscanned += MultipleItemDelimiter.length();
				alreadyscanned += Ranges[i+1].length();
				alreadyscanned += SingleItemDelimiter.length();
				// Append last value as well
				i++;
			}
			MsgBox(Return.c_str());
		}
		// Last delimiter
		Return += Ranges[Ranges.size()-1];
		MsgBox(Return.c_str());
	}

	// Remove last delimiter:
	// Return.erase(Return.length()-OutputDelimiter.length());
	if (ReturnBool)
	{
		ReturnBool = false;
		return (tchar *)Return.c_str();
	}
	else
	{
		return Runtime.CopyString(Return.c_str());
	}
}


// ID = 2
int Extension::CurrentIteratedValue()
{
	return CurrentVal;
}