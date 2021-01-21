
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
			throw std::exception("Exceeded limit of ID pool. Please contact the developer.");

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

