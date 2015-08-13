#pragma once

#include "windows.h"
#include "process.h"

namespace Sys
{
	class Thread
	{
	public:
		class Runnable
		{
		public:
			Runnable() {}
			virtual ~Runnable() {}
			virtual void run() = 0;
		};

		Thread(Runnable* runnable) :
			mThreadId(0),
			mRunnable(runnable)
		{
			(HANDLE) ::_beginthreadex(0, 0, (_beginthreadex_proc_type)(&Thread::dispatch), this, 0, &mThreadId);
		}

		void suspend()
		{
			SuspendThread(mThread);
		}

		void resume()
		{
			ResumeThread(mThread);
		}

		bool join()
		{
			DWORD res = ::WaitForSingleObjectEx(mThread, INFINITE, false);
			return res == WAIT_OBJECT_0;
		}

		static size_t selfId()
		{
			return (size_t) GetCurrentThreadId();
		}
	private:
		static unsigned dispatch(const void* args)
		{
			Thread* self = (Thread*)args;
			self->mRunnable->run();
			return 0;
		}

		HANDLE mThread;
		unsigned int mThreadId;
		Runnable* mRunnable;
	};
}