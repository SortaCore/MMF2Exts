/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * Copyright (C) 2020-2025 Darkwire Software.
 * All rights reserved.
 *
 * https://opensource.org/license/mit
*/

#include "Lacewing.h"

bool lacewing::readlock::isEnabled() const
{
	return locked;
}
bool lacewing::writelock::isEnabled() const
{
	return locked;
}
#if defined(COXSDK)
	#if defined(__ANDROID__)
		#include "../Inc/Android/MMFAndroidMasterHeader.hpp"
	#elif defined(__APPLE__)
		#if MacBuild==0
			#include "../Inc/iOS/MMFiOSMasterHeader.hpp"
		#else
			#include "../Inc/Mac/MMFMacMasterHeader.hpp"
		#endif
	#endif
#endif

lacewing::readlock::~readlock()
{
	if (locked)
	{
#ifdef _DEBUG
		lock.closeReadLock(*this, "<unknown>", "readlock::~readlock()", 0);
#else
		lock.closeReadLock(*this);
#endif
	}
#ifdef _DEBUG
	if (supercededByWriter)
	{
		// If writelock is previously opened on this thread, make sure it's not closed when this read lock is closed.
		bool foundWriter = false;

		// Obtain meta lock so holders can be safely accessed
		while (this->lock.metaLock.exchange(true))
			;

		for (auto i = lock.holders.cbegin(); i != lock.holders.cend(); ++i)
		{
			if (i->threadID == std::this_thread::get_id() && i->isWrite == true &&
				!strcmp(i->file, writerOpenFile) && i->line == writerOpenLine)
			{
				foundWriter = true;
				break;
			}
		}

		// Drop meta lock
		this->lock.metaLock = false;
		if (!foundWriter)
		{
			assert(false && "Closing readlock failure: Writer not found.");
		}
	}
#endif
}
lacewing::writelock::~writelock()
{
	if (locked)
	{
#ifdef _DEBUG
		lock.closeWriteLock(*this, "<unknown>", "writelock::~writelock()", 0);
#else
		lock.closeWriteLock(*this);
#endif
	}
}

#ifdef _DEBUG
lacewing::readlock::readlock(readwritelock &lock, lw_rwlock_debugParamNames)
	: lock(lock), locker(lock.lock, std::defer_lock)
{
	lock.openReadLock(*this, file, func, line);
}

void lacewing::readlock::relockDebug(lw_rwlock_debugParamNames)
{
	if (locked)
		throw std::runtime_error("ReadLock: Locking when it's already locked");
	lock.openReadLock(*this, file, func, line);
	locked = true;
}
void lacewing::readlock::unlockDebug(lw_rwlock_debugParamNames)
{
	if (!locked)
		throw std::runtime_error("ReadLock: Unlocking when it's already unlocked");
	lock.closeReadLock(*this, file, func, line);
	locked = false;
}

#ifdef LW_ESCALATION
lacewing::writelock lacewing::readlock::upgrade(lw_rwlock_debugParamNames)
{
	if (!locked)
		throw std::runtime_error("Uhhhhh...");

	lacewing::writelock wl = this->lock.createWriteLock(false, file, func, line);
	// Switch wl as read for a bit, then lock over as write.
	lock.openWriteLock(*this, file, func, line);
	wl.lock.lock.lock_shared();
	lock.closeReadLock(*this, file, func, line);

	if (wl.isEnabled())
		lock.write
	lacewing::writelock wl(lock, false, file, func, line);
}
void lacewing::readlock::upgrade(lw_rwlock_debugParamNames, lacewing::writelock &wl)
{

}
#endif
#else
lacewing::readlock::readlock(readwritelock &lock)
	: lock(lock), locker(lock.lock, std::defer_lock) {
	lock.openReadLock(*this);
}
void lacewing::readlock::relock()
{
	assert(!locked && "ReadLock: Locking when it's already locked");
	lock.openReadLock(*this);
}
void lacewing::readlock::unlock()
{
	assert(locked && "ReadLock: Unlocking when it's already unlocked");
	lock.closeReadLock(*this);
}
#endif


#ifdef _DEBUG
lacewing::writelock::writelock(readwritelock &lock, const char * file, const char * func, int line)
	: lock(lock), locker(lock.lock, std::defer_lock)
{
	lock.openWriteLock(*this, file, func, line);
}

void lacewing::writelock::relockDebug(const char * file, const char * func, int line)
{
	if (locked)
		throw std::runtime_error("WriteLock: Locking when it's already locked");
	else if (!lock.checkHoldsWrite(false))
	{
		// Not a meaningful relock?
		//LacewingFatalErrorMsgBox();
	}
	locked = true;
	lock.openWriteLock(*this, file, func, line);
}
void lacewing::writelock::unlockDebug(const char * file, const char * func, int line)
{
	if (!locked)
		return;
		// throw std::exception("WriteLock: Unlocking when it's already unlocked");
	lock.closeWriteLock(*this, file, func, line);
	locked = false;
}
#else
lacewing::writelock::writelock(readwritelock &lock)
	: lock(lock), locker(lock.lock, std::defer_lock) {
	lock.openWriteLock(*this);
}
void lacewing::writelock::relock()
{
	assert(!locked && "WriteLock: Locking when it's already locked");
	lock.openWriteLock(*this);
}
void lacewing::writelock::unlock()
{
	if (!locked)
		return;
		// throw std::runtime_error("WriteLock: Unlocking when it's already unlocked");
	lock.closeWriteLock(*this);
}
#endif


lacewing::readwritelock::readwritelock()
{
	readers = writers = read_waiters = write_waiters = 0;
}
lacewing::readwritelock::~readwritelock() noexcept(false)
{
	if (readers || writers || read_waiters || write_waiters)
		LacewingFatalErrorMsgBox();
}
// Debug breakpoint if writelock is not held by current thread.
bool lacewing::readwritelock::checkHoldsWrite(bool excIfNot /* = true */) const
{
	// Writers might be incrementing/decrementing, but if one is held on this thread, this'll definitely be > 0.
	if (writers == 0)
		goto nope;

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	for (auto &i : holders)
	{
		if (i.threadID == std::this_thread::get_id() && i.isWrite)
		{
			// Drop meta lock
			this->metaLock = false;
			return true;
		}
	}

	// Drop meta lock
	this->metaLock = false;

	nope:
	assert(!excIfNot && "Undefined behaviour; write lock not held when expected. Please attach debugger now.");
	return false;
}
// Debug breakpoint if readlock is not held by current thread.
bool lacewing::readwritelock::checkHoldsRead(bool excIfNot /* = true */) const
{
	// Readers might be coming and going, but if one is held on this thread, this'll definitely be > 0.
	if (readers == 0)
		goto nope;

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	for (auto &i : holders)
	{
		if (i.threadID == std::this_thread::get_id() && !i.isWrite)
		{
			// Drop meta lock
			this->metaLock = false;
			return true;
		}
	}

	// Drop meta lock
	this->metaLock = false;

nope:
	assert(!excIfNot && "Undefined behaviour; read lock not held when expected. Please attach debugger now.");
	return false;
}

// https://stackoverflow.com/questions/244316/reader-writer-locks-in-c/244376#244376

#ifdef _DEBUG
void lacewing::readwritelock::openReadLock(readlock &rl, lw_rwlock_debugParamNames)
#else
void lacewing::readwritelock::openReadLock(readlock &rl)
#endif
{
	// Read lock held by current thread will produce a deadlock
	if (checkHoldsWrite(false))
	{
#ifdef _DEBUG
		for (auto& i : holders)
		{
			if (i.threadID == std::this_thread::get_id() && i.isWrite)
			{
				rl.supercededByWriter = true;
				rl.writerOpenFile = i.file;
				rl.writerOpenLine = i.line;
			}
		}
#endif

		rl.locked = false; // set to true by ctor
		return;
	}
	// Recursive mutex, hence locked stays false
	if (checkHoldsRead(false))
	{
		rl.locked = false; // set to true by ctor
		return;
	}

	read_waiters++;
	rl.locker.lock();
	rl.locked = true;

	if (writers)
		LacewingFatalErrorMsgBox(); // Shouldn't have a writer still locking while we're reading

	//do read.wait(lock);
	// while (writers || write_waiters);
	read_waiters--;
	readers++;

	holder dd;
#ifdef _DEBUG
	dd.file = file;
	dd.func = func;
	dd.line = line;
#endif
	dd.isWrite = false;
	dd.threadID = std::this_thread::get_id();

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

#ifdef _DEBUG
	for (auto& i : holders)
	{
		if (i.threadID == std::this_thread::get_id())
			LacewingFatalErrorMsgBox(); // multiple readers on same thread, first one should've been closed
	}
#endif

	this->holders.push_back(dd);

	// Drop meta lock
	this->metaLock = false;
}

#ifdef _DEBUG
void lacewing::readwritelock::openWriteLock(writelock &wl, const char * file, const char * func, int line)
#else
void lacewing::readwritelock::openWriteLock(writelock &wl)
#endif
{
#ifdef _DEBUG
	// Read lock held by current thread will produce a deadlock
	if (checkHoldsRead(false))
	{
		char debugInfo[1024];
		sprintf(debugInfo, "Deadlock - opened new write lock with read lock already held by same thread.\nNew writer opened from file [%s], func [%s] line %i.",
			file, func, line);
		#ifdef _WIN32
			MessageBoxA(NULL, debugInfo, "Deadlock failure.", MB_ICONERROR);
		#endif
		throw std::runtime_error("Deadlock");
	}
#endif

	// Recursive mutex, hence locked stays false
	if (checkHoldsWrite(false))
	{
		wl.locked = false; // set to true by ctor
		return;
	}

	++write_waiters;
	wl.locker.lock();
	wl.locked = true;

	// No extra writers pls
	if (writers || readers)
		LacewingFatalErrorMsgBox();

	--write_waiters;
	++writers;


	holder dd;
#ifdef _DEBUG
	dd.file = file;
	dd.func = func;
	dd.line = line;
#endif
	dd.isWrite = true;
	dd.threadID = std::this_thread::get_id();

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	this->holders.push_back(dd);

	// Drop meta lock
	this->metaLock = false;
}

#ifdef _DEBUG
void lacewing::readwritelock::closeReadLock(readlock &rl, const char * file, const char * func, int line)
#else
void lacewing::readwritelock::closeReadLock(readlock &rl)
#endif
{
#ifdef _DEBUG
	if (readers == 0)
		LacewingFatalErrorMsgBox(); // don't own this lock!

	if (writers)
		LacewingFatalErrorMsgBox(); // writer shouldn't be accessing while a read lock is open...

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	// not found
	auto ourData = holders.cend();
	for (auto i = holders.cbegin(); i != holders.cend(); ++i)
	{
		if (i->threadID == std::this_thread::get_id() && i->isWrite == false)
		{
			if (ourData == holders.cend())
				ourData = i; // found us
			else // Found us twice... nani
				LacewingFatalErrorMsgBox();
		}
	}

	// Us not found, nani
	if (ourData == holders.cend())
		LacewingFatalErrorMsgBox();
	else
		holders.erase(ourData);

#else
	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	// not found
	auto ourData = std::find_if(holders.cbegin(), holders.cend(),
		[=](const auto &i) { return i.threadID == std::this_thread::get_id() && i.isWrite == false; });
	holders.erase(ourData);
#endif

	// Drop meta lock
	this->metaLock = false;

	--readers;
	rl.locker.unlock();
	rl.locked = false;
}

#ifdef _DEBUG
void lacewing::readwritelock::closeWriteLock(writelock &wl, const char * file, const char * func, int line)
#else
void lacewing::readwritelock::closeWriteLock(writelock &wl)
#endif
{
	// Recursive and not held
	if (!wl.locked)
		return;

#ifdef _DEBUG

	if (writers == 0)
		LacewingFatalErrorMsgBox(); // don't own this lock!

	if (readers)
		LacewingFatalErrorMsgBox(); // readers shouldn't be accessing while a write lock is open...

	if (writers != 1)
		LacewingFatalErrorMsgBox(); // readers shouldn't be accessing while a write lock is open...

	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	// Should only be one thing in holders, but eh.
	auto ourData = holders.cend();
	for (auto i = holders.cbegin(); i != holders.cend(); ++i)
	{
		if (i->threadID == std::this_thread::get_id() && i->isWrite == true)
		{
			if (ourData == holders.cend())
				ourData = i; // found us
			else // Found us twice... nani
				LacewingFatalErrorMsgBox();
		}
	}

	// Us not found, nani
	if (ourData == holders.cend())
		LacewingFatalErrorMsgBox();
	else
		holders.erase(ourData);

#else
	// Obtain meta lock so holders can be safely accessed
	while (this->metaLock.exchange(true))
		;

	// not found
	auto ourData = std::find_if(holders.cbegin(), holders.cend(),
		[=](const auto &i) { return i.threadID == std::this_thread::get_id() && i.isWrite == true; });
	holders.erase(ourData);
#endif

	// Drop meta lock
	this->metaLock = false;

	--writers;
#ifdef _MSC_VER
	// This function won't hold this lock, other functions will, so suppress the "caller doesn't hold lock" warning
	#pragma warning (suppress: 26110)
#endif
	wl.locker.unlock();
	wl.locked = false;
}

#ifdef _DEBUG
#undef createReadLock
#undef createWriteLock
[[nodiscard]]
lacewing::readlock lacewing::readwritelock::createReadLock(const char *file, const char * func, int line) {
	return lacewing::readlock(*this, file, func, line);
}
#else
[[nodiscard]]
lacewing::readlock lacewing::readwritelock::createReadLock() {
	return lacewing::readlock(*this);
}
#endif

#ifdef _DEBUG
[[nodiscard]]
lacewing::writelock lacewing::readwritelock::createWriteLock(const char *file, const char * func, int line) {
	return lacewing::writelock(*this, file, func, line);
}
#else
[[nodiscard]]
lacewing::writelock lacewing::readwritelock::createWriteLock() {
	return lacewing::writelock(*this);
}
#endif
