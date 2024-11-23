#pragma once
#include <chrono>

namespace GameTimer 
{
	using TimePoint = std::chrono::steady_clock::time_point;
	//using Duration = std::chrono::duration<float>;
	using Duration = std::chrono::duration<float, std::chrono::seconds::period>;

	inline auto startTime = std::chrono::steady_clock::now();
	inline auto deltaStartTime = std::chrono::steady_clock::now();



	inline float GetElapsedTime()
	{
		const auto currentTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<Duration>(currentTime - startTime).count();
	}

	inline float GetDeltaTime()
	{
		const auto currentTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<Duration>(currentTime - deltaStartTime).count();
	}

	inline void UpdateDelta()
	{
		deltaStartTime = std::chrono::steady_clock::now();
	}
}