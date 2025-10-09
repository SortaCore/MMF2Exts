/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * Copyright (C) 2021-2025 Darkwire Software.
 * All rights reserved.
 *
 * https://opensource.org/license/mit
*/

#include "Lacewing.h"
#include "deps/utf8proc.h"
#ifdef _MSC_VER
	// suppress complaints about utf8proc C enums not being C++ enum classes
	#pragma warning (push)
	#pragma warning (disable: 26812)
#endif

static std::string CPALMakeError(lacewing::codepointsallowlist * that, lacewing::codepointsallowlist & acTemp, const char * str, ...)
{
	va_list v, v2;
	va_start(v, str);
	va_copy(v2, v);

	size_t numChars = vsnprintf(nullptr, 0, str, v);
	std::string error(numChars, ' ');
	vsprintf(error.data(), str, v2);

	va_end(v);
	va_end(v2);
	*that = acTemp; // restore old
	return error;
};

std::string lacewing::codepointsallowlist::setcodepointsallowedlist(std::string acStr)
{
	if (acStr.empty())
	{
		codePointCategories.clear();
		specificCodePoints.clear();
		codePointRanges.clear();
		allAllowed = true;
		list = acStr;
		return std::string();
	}

	codepointsallowlist acTemp = *this;

	// Reset to blank
	codePointCategories.clear();
	specificCodePoints.clear();
	codePointRanges.clear();
	allAllowed = false;
	list = acStr;

	// String should be format:
	// 2 letters, or 1 letter + *, or an integer number that is the UTF32 number of char
	if (acStr.front() == ',')
		return CPALMakeError(this, acTemp, "The acceptable code point list \"%hs...\" starts with a comma.", acStr.c_str());

	acStr.erase(std::remove(acStr.begin(), acStr.end(), ' '), acStr.end());
	if (acStr.empty())
		return CPALMakeError(this, acTemp, "The acceptable code point list \"%hs\" is all spaces.", acStr.c_str());

	if (acStr.back() == ',')
		return CPALMakeError(this, acTemp, "The acceptable code point list \"%hs...\" ends with a comma.", acStr.c_str());

	if (acStr.find(",,"sv) != std::string::npos)
		return CPALMakeError(this, acTemp, "The acceptable code point list \"%hs...\" contains \",,\".", acStr.c_str());

	acStr += ','; // to make sure when cur is +='d and passes end of string, it'll end with remaining = 0

	const char * cur = acStr.data();
	size_t remaining = acStr.size();
	while (true)
	{
		remaining = (acStr.data() + acStr.size()) - cur;
		if (cur >= acStr.data() + acStr.size() - 1)
			break;

		// Two-letter category, or letter + * for all categories
		if (remaining >= 2 &&
			std::isalpha(cur[0]) && (std::isalpha(cur[1]) || cur[1] == '*'))
		{
			// more than two letters
			if (remaining > 2 && cur[2] != ',')
			{
				return CPALMakeError(this, acTemp, "The acceptable code point list \"%hs\" has a 3+ letter category \"%hs\". Categories are 2 letters.",
					acStr.c_str(), cur);
			}

			// See utf8proc.cpp for these defined under utf8proc_category_t
			static const char categoryList[][3] = { "Cn","Lu","Ll","Lt","Lm","Lo","Mn","Mc","Me","Nd","Nl","No","Pc","Pd","Ps","Pe","Pi","Pf","Po","Sm","Sc","Sk","So","Zs","Zl","Zp","Cc","Cf","Cs","Co" };
			static const char wildcardCategory[] = { 'C', 'L', 'M', 'N', 'P','S','Z' };

			// Wildcard
			if (cur[1] == '*')
			{
				char firstCharUpper = (char)std::toupper(cur[0]);
				for (std::size_t i = 0; i < sizeof(wildcardCategory); ++i)
				{
					if (firstCharUpper == wildcardCategory[i])
					{
						// Wildcard category found, yay
						for (std::size_t j = 0; j < std::size(categoryList); ++j)
						{
							if (firstCharUpper == categoryList[j][0])
								codePointCategories.push_back((lw_i32)j);
						}

						cur += 3;
						goto nextChar;
					}
				}

				return CPALMakeError(this, acTemp, "Wildcard category \"%.2hs\" not recognised. Check the help file.", cur);
			}

			for (std::size_t i = 0; i < std::size(categoryList); ++i)
			{
				if (std::toupper(cur[0]) == categoryList[i][0] && std::tolower(cur[1]) == categoryList[i][1])
				{
					// Category found, is it already added?
					if (std::find(codePointCategories.cbegin(), codePointCategories.cend(), i) != codePointCategories.cend())
						return CPALMakeError(this, acTemp, "Category \"%.2hs\" was added twice in list \"%hs\".", cur, acStr.c_str());

					codePointCategories.push_back((lw_i32)i);
					cur += 3;
					goto nextChar;
				}
			}

			return CPALMakeError(this, acTemp, "Category \"%.2hs\" not recognised. Check the help file.", cur);
		}

		// Numeric, or numeric range expected
		if (std::isdigit(cur[0])) {
			char * endPtr;
			unsigned long codePointAllowed = std::strtoul(cur, &endPtr, 0);
			if (codePointAllowed == 0 || codePointAllowed > INT32_MAX) // error in strtoul, or user has put in 0 and approved null char, either way bad
				return CPALMakeError(this, acTemp, "Specific codepoint %hs not a valid codepoint.", cur, acStr.c_str());

			// Single code point, after this it's a new Unicode list, or it's end of string
			cur = endPtr;
			if (cur[0] == '\0' || cur[0] == ',')
			{
				if (std::find(specificCodePoints.cbegin(), specificCodePoints.cend(), codePointAllowed) != specificCodePoints.cend())
					return CPALMakeError(this, acTemp, "Specific codepoint %lu was added twice in list \"%hs\".", codePointAllowed, acStr.c_str());

				specificCodePoints.push_back((int)codePointAllowed);
				if (cur[0] == ',')
					++cur;
				goto nextChar;
			}

			// Range of code points
			if (cur[0] == '-')
			{
				++cur;
				unsigned long lastCodePointNum = std::strtoul(cur, &endPtr, 0);
				if (lastCodePointNum == 0 || lastCodePointNum > INT32_MAX) // error in strtoul, or user has put in 0 and approved null char, either way bad
					return CPALMakeError(this, acTemp, "Ending number in codepoint range %lu to \"%.15hs...\" could not be read.", codePointAllowed, cur, cur);
				// Range is reversed
				if (lastCodePointNum < codePointAllowed)
					return CPALMakeError(this, acTemp, "Range %lu to %lu is backwards.", codePointAllowed, lastCodePointNum);

				// Allow range overlaps - we could search by range1 max > range2 min, but we won't.
				// We will check for an exact match in range, though.

				auto range = std::make_pair((std::int32_t)codePointAllowed, (std::int32_t)lastCodePointNum);
				if (std::find(codePointRanges.cbegin(), codePointRanges.cend(), range) != codePointRanges.cend())
					return CPALMakeError(this, acTemp, "Range %lu to %lu is in the list twice.", codePointAllowed, lastCodePointNum);

				codePointRanges.push_back(range);
				cur = endPtr + 1; // skip the ','
				goto nextChar;
			}

			// fall through
		}

		return CPALMakeError(this, acTemp, "Unrecognised character list starting at \"%.15hs\".", cur);

	nextChar:
		/* go to next char */;
	}

	return std::string();
}

int lacewing::codepointsallowlist::checkcodepointsallowed(const std::string_view toTest, int * const rejectedUTF32CodePoint /* = NULL */) const
{
	if (allAllowed)
		return -1;

	const utf8proc_uint8_t * str = (const utf8proc_uint8_t *)toTest.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainingBytes = toTest.size();
	int codePointIndex = 0;
	utf8proc_category_t category;
	while (remainingBytes > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainingBytes, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			goto badChar;

		if (std::find(specificCodePoints.cbegin(), specificCodePoints.cend(), thisChar) != specificCodePoints.cend())
			goto goodChar;
		if (std::find_if(codePointRanges.cbegin(), codePointRanges.cend(),
			[=](const std::pair<std::int32_t, std::int32_t> & range) {
				return thisChar >= range.first && thisChar <= range.second;
			}) != codePointRanges.cend())
		{
			goto goodChar;
		}
		category = utf8proc_category(thisChar);
		if (std::find(codePointCategories.cbegin(), codePointCategories.cend(), category) != codePointCategories.cend())
			goto goodChar;

		// ... fall through from above
	badChar:
		if (rejectedUTF32CodePoint != NULL)
			*rejectedUTF32CodePoint = thisChar;
		return codePointIndex;

		// Loop around
	goodChar:
		++codePointIndex;
		str += numBytesInCodePoint;
		remainingBytes -= numBytesInCodePoint;
	}

	return -1; // All good
}

#ifdef _MSC_VER
	#pragma warning (pop)
#endif
