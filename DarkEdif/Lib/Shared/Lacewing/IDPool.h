/* vim: set et ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.
 * Copyright (C) 2012-2022 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/licenses/mit-license.php
*/
#include <vector>
#include <set>

#ifndef LacewingIDPool
#define LacewingIDPool

/// <summary> An ID number list, ensures no duplicate IDs and lowest
/// 		  available ID numbers used first, etc. </summary>
class IDPool
{

protected:

	std::set<lw_ui16> releasedIDs;	// A sorted list of all released IDs.
	lw_ui16 nextID;					// The next ID to use, not within releasedIDs.
	lw_i32 borrowedCount;			// The number of IDs currently in use.
	lacewing::readwritelock lock;

public:

	/// <summary> Creates an ID pool. First ID returned is 0. </summary>
	IDPool()
	{
		nextID = 0;
		borrowedCount = 0;
	}

	/// <summary> Gets the next ID available from the pool. </summary>
	/// <returns> New ID to use. </returns>
	lw_ui16 borrow()
	{
		lacewing::writelock writeLock = lock.createWriteLock();

		++borrowedCount;
		lw_trace("Borrowed Client ID. %i IDs borrowed so far.", borrowedCount);

		// More than can be stored in an ID list are in use. JIC.
		if (borrowedCount > 0xFFFE)
			throw std::runtime_error("Exceeded limit of ID pool. Please contact the developer.");

		if (!releasedIDs.empty())
		{
			lw_ui16 freshID = *releasedIDs.cbegin(); // .front() not in std::set
			releasedIDs.erase(releasedIDs.cbegin());
			return freshID;
		}

		return nextID ++;
	}

	/// <summary> Returns the given identifier. </summary>
	/// <param name="ID"> The identifier to return. </param>
	void returnID(lw_ui16 ID)
	{
		lacewing::writelock writeLock = lock.createWriteLock();
		// No IDs in use at all: empty list
		if ((-- borrowedCount) == 0)
		{
			releasedIDs.clear();
			nextID = 0;
		}
		else
		{
			// Was the last one, just roll back the next ID to this one
			if (nextID == ID + 1)
				--nextID;
			else // Add ID back for re-using
				releasedIDs.emplace(ID);
		}
	}
};

#endif

