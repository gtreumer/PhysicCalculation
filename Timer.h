#pragma once
#ifndef __TIMER_H
#define __TIMER_H

#include "windows.h"

namespace Test
{
	void print(const char* str)
	{
		OutputDebugStringA(str);
	}

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	typedef void(*OnTickCallback) (const void* param);

	class TimerQ
	{
	public:
		TimerQ() :
			mDoneEvent(NULL),
			mTimer(NULL),
			mTimerQueue(NULL),
			mCallback(NULL),
			mParam(NULL),
			mIsProcessing(false),
			mDuration(0)
		{
		}

		~TimerQ()
		{
		}

		void start(size_t offset, size_t period, size_t duration)
		{
			mDuration = duration;
			mPeriod = period;
			mDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (NULL == mDoneEvent)
			{
				//std::cout << "Create event failed" << GetLastError() << std::endl;
				return;
			}

			mTimerQueue = CreateTimerQueue();
			if (NULL == mTimerQueue)
			{
				//std::cout << "CreateTimerQueue failed " << GetLastError() << std::endl;
				return;
			}

			if (!CreateTimerQueueTimer(&mTimer, mTimerQueue,
				(WAITORTIMERCALLBACK)TimerRoutine, this, offset, period, 0))
			{
				//std::cout << "CreateTimerQueueTimer failed " << GetLastError() << std::endl;
				return;
			}
		}

		void stop()
		{
			CloseHandle(mDoneEvent);

			if (!DeleteTimerQueue(mTimerQueue))
				return;
				//std::cout << "DeleteTimerQueue failed " << GetLastError() << std::endl;
		}

		void processIfNeed()
		{
			if (WaitForSingleObject(mDoneEvent, INFINITE) != WAIT_OBJECT_0)
			{
				return;
				//std::cout << "WaitForSingleObject failed " << GetLastError() << std::endl;
			}
			if (!mNeedToStop)
				ResetEvent(mDoneEvent);
			else
				stop();
		}

		void setOnTickCallback(OnTickCallback callback, const void* param)
		{
			mCallback = callback;
			mParam = param;
		}
	private:
		static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
		{
			TimerQ* timer = (TimerQ*)lpParam;
			if (!timer->mIsProcessing)
			{
				QueryPerformanceFrequency(&Frequency);
				QueryPerformanceCounter(&StartingTime);
				timer->mIsProcessing = true;
			}

			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
			ElapsedMicroseconds.QuadPart /= 1000;

			size_t elapsed = (size_t)ElapsedMicroseconds.QuadPart;
			if (elapsed >= (timer->mDuration))
			{
				timer->mNeedToStop = true;
				timer->mIsProcessing = false;
				SetEvent(timer->mDoneEvent);
				timer->stop();
			}

			if (timer->mCallback)
				timer->mCallback(timer->mParam);

			SetEvent(timer->mDoneEvent);
		}

		HANDLE mDoneEvent;
		HANDLE mTimer;
		HANDLE mTimerQueue;
		OnTickCallback mCallback;
		const void* mParam;
		bool mIsProcessing;
		bool mNeedToStop;
		size_t mDuration;
		size_t mPeriod;
	};
}

#endif