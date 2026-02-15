/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.
 * Copyright (C) 2012-2026 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/
#pragma once
#ifndef lw_idpool_h
#define lw_idpool_h

#include "Lacewing.h"
#include "src/common.h"
#include <chrono>
#include <map>
#include <stdexcept> // else no runtime_error

// An ID number list, ensures no duplicate IDs and lowest available ID numbers used first, etc.
// IDs returned are reusable after 5 seconds of continued no-use.
class IDPool
{
protected:
	using idclock = std::chrono::system_clock;

	// Last use of ID before it is freed. IDs may be used within a few seconds after disconnect,
	// and some buggy clients keep hammering IDs after being disconnected, including Connect Denied,
	// and some channel messages may be delivered strangely.
	static constexpr idclock::duration safetyMargin =
		std::chrono::duration_cast<idclock::duration>(std::chrono::seconds(5));

	// IDs that were returned for re-use, with their times for when they are usable.
	// Returning an ID creates an entry here with default timeout of safetyMargin.
	// Any attempt to use a ID, while no ID owner has been assigned it,
	// also results an entry here, or the expiry being extended if already exists.
	std::map<lw_ui16, idclock::time_point> releasedIDs;
	// The next ID to use, always beyond releasedIDs' end.
	lw_ui16 nextID;
	// The number of IDs currently in use as far as container is concerned.
	// This number may not relate to nextID or releasedID sizes.
	lw_i32 borrowedCount;
	// Could use a lw_sync but this creates logging.
	lacewing::readwritelock lock;

public:
	// Creates an ID pool. First ID returned is 0.
	IDPool()
	{
		nextID = 0;
		borrowedCount = 0;
	}

	// Gets the next ID available from the pool, picking the lowest unused.
	lw_ui16 borrow()
	{
		lacewing::writelock writeLock = lock.createWriteLock();

		++borrowedCount;
		lwp_trace("Borrowing Client ID. %i IDs borrowed so far.", borrowedCount);

		// More than can be stored in an ID list are in use. JIC.
		if (borrowedCount > 0xFFFE)
			throw std::runtime_error("Exceeded limit of ID pool. Please contact the developer.");

		// Find lowest ID available that has also expired its block time window
		if (!releasedIDs.empty())
		{
			// We'll always use first entry in release list that has expired
			const auto now = idclock::now();
			const auto freeIDIt = std::find_if(releasedIDs.cbegin(), releasedIDs.cend(),
				[&now](const auto& idReleaseTime) {
					return idReleaseTime.second < now;
				});

			// If we found an expiry at all...
			if (freeIDIt != releasedIDs.cend())
			{
				const lw_ui16 freshID = freeIDIt->first;
				releasedIDs.erase(freeIDIt);
				lwp_trace("Found expired ID %hu, assigning it. NextID is %i, borrow count %i.", freshID, nextID, borrowedCount);

				// Now we've found the first expired ID, we can decrement nextID to use expired IDs lower than it.
				// nextID is always past the end of releasedIDs by some amount, but that amount is more than 1,
				// then that gap is IDs currently in use, so we cannot erase.

				// Check end of list backwards to see if we have expiry there too.
				for (auto it = releasedIDs.crbegin(); it != releasedIDs.crend();)
				{
					// Should never happen
					assert(nextID >= it->first);

					// Not at end, so first->nextID range is in use
					// or not expired, so we can't erase
					if (it->first < nextID - 1 || now < it->second)
					{
						lwp_trace("Cannot free released ID %hu.", nextID);
						break;
					}

					// This is not a cast, it's a reverse_iterator ctor taking forward iterator,
					// as erase only allows forward iterator.
					// The next part is a strange implementation detail due to how reverse vs forward iterators
					// are one index apart.
					it = decltype(it)(releasedIDs.erase(std::next(it).base()));
					--nextID;
				}

				// The loop won't free something earlier than freshID, as freshID is absent from the loop, so the loop sees it as in use.
				assert(freshID < nextID);
				return freshID;
			}
			// else fall thru
		}
		// else no recently released ID, so we assign ID by incrementation

		// Shouldn't fail, but if it does, we're double-assigning an ID, one by releasing, one by nextID
		assert(releasedIDs.find(nextID) == releasedIDs.end());

		return nextID ++;
	}

	// Blocks the given ID. Returns true if ID was recently disconnected and pending a release.
	// If true is returned, the use of the bad ID is latency issues.
	// If false, the use of bad ID may be considered an error or a malicious act.
	bool releasedIDWasUsed(const lw_ui16 ID)
	{
		lacewing::writelock writeLock = lock.createWriteLock();
		const auto newReleaseTime = idclock::now() + safetyMargin;
		auto IDIt = releasedIDs.find(ID);

		// ID used, and was recently released: extend the block period
		if (IDIt != releasedIDs.end())
		{
			IDIt->second = newReleaseTime;
			return true;
		}

		// ID used but not a released ID, random user? we don't want a legit user trying to use that ID,
		// so we have to block it out too
		releasedIDs.emplace(ID, newReleaseTime);

		// If we set nextID so nextID = ID + 1, we end up with orig nextID -> new set nextID range being unreachable,
		// as a malicious user can request user ID 0xFFFE and lock all IDs 0-0xFFFE for a server.
		// If we don't set it, when the malicious entry expires, the releasedID of 0xFFFE is used.
		// For now, our only possible fix is to set nextID anyway, and add fake entries so releasedIDs will find an entry
		// earlier than nextID. This is not a great fix in terms of performance, but it's usable.
		// Since nextID + 1 is checked against 0xFFFE, and 0xFFFF is invalid ID, we stop at 0xFFFD.
		if (ID >= nextID && ID < 0xFFFD)
		{
			for (lw_ui16 fakeID = nextID; fakeID < ID; ++fakeID)
				releasedIDs.emplace(fakeID, decltype(safetyMargin)::zero());
			always_log("Note: Unexpected ID %hu was used, causing reservation entries for IDs %hu to %hu.\n",
				ID, nextID);
			nextID = static_cast<lw_ui16>(ID + 1);
		}

		return false;
	}

	// Returns the given identifier back to the pool for re-use
	void returnID(const lw_ui16 ID)
	{
		lacewing::writelock writeLock = lock.createWriteLock();
		assert(releasedIDs.find(ID) == releasedIDs.cend());
		assert(nextID > ID);
		--borrowedCount;

		// Free ID for reuse, once it's safe
		releasedIDs.emplace(ID, idclock::now() + safetyMargin);
	}
};

#endif // lw_idpool_h - include guard
