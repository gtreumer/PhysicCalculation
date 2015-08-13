#pragma once
#include "Windows.h"

namespace Sys
{
	class Time
	{
	public:
		typedef long long Value;

		static Value now()
		{
			static LARGE_INTEGER frequency;
			static BOOL use_gpc = QueryPerformanceFrequency(&frequency);
			if (use_gpc)
			{
				LARGE_INTEGER now;
				QueryPerformanceCounter(&now);
				return (1000LL * now.QuadPart) / frequency.QuadPart;
			}
			else
			{
				return GetTickCount();
			}
		}
	};
}
