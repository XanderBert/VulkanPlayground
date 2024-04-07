#pragma once
#include <chrono>

namespace GameTimer 
{
	typedef std::chrono::steady_clock::time_point TimePoint;
	typedef std::chrono::duration<float, std::chrono::seconds::period> Duration;

	inline TimePoint startTime = std::chrono::high_resolution_clock::now();
	inline TimePoint deltaStartTime = std::chrono::high_resolution_clock::now();

	inline float GetElapsedTime()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		return Duration(currentTime - startTime).count();
	}

	inline float GetDeltaTime()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		return Duration(currentTime - deltaStartTime).count();
	}

	inline void UpdateDelta()
	{
		deltaStartTime = std::chrono::high_resolution_clock::now();
	}
}