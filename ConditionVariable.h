#pragma once

#include "windows.h"

namespace Sys
{
	class ConditionVariable
	{
	public:
		ConditionVariable()
		{
			InitializeCriticalSection(&cs);
			InitializeConditionVariable(&cv);
		}

		~ConditionVariable() {}

		void aquire()
		{
			EnterCriticalSection(&cs);
		}

		void release()
		{
			LeaveCriticalSection(&cs);
		}

		void wait()
		{
			SleepConditionVariableCS(&cv, &cs, INFINITE);
		}

		void waitFor(size_t ms)
		{
			SleepConditionVariableCS(&cv, &cs, (DWORD)ms);
		}

		void signal()
		{
			WakeConditionVariable(&cv);
		}

		void signalAll()
		{
			WakeAllConditionVariable(&cv);
		}
	private:
		CRITICAL_SECTION cs;
		CONDITION_VARIABLE cv;
	};
}