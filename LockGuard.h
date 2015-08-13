#pragma once
#ifndef __LOCK_GUARD_H
#define __LOCK_GUARD_H

#include "windows.h"
#include <exception>

namespace Sys
{
	class CriticalSection
	{
	public:
		CriticalSection()
		{
			if (!InitializeCriticalSectionAndSpinCount(&cs, 0x00000400))
				throw std::exception("Can't initialize critical section");
		}

		~CriticalSection()
		{
			DeleteCriticalSection(&cs);
		}

		void aquire()
		{
			EnterCriticalSection(&cs);
		}

		void release()
		{
			LeaveCriticalSection(&cs);
		}
	private:
		CRITICAL_SECTION cs;
	};

	template<typename ThreadPrimitiveType>
	class LockGuard
	{
	public:
		LockGuard(ThreadPrimitiveType& _cs) : cs(_cs)
		{
			cs.aquire();
		}

		~LockGuard()
		{
			cs.release();
		}
	private:
		ThreadPrimitiveType& cs;
	};
}

#endif