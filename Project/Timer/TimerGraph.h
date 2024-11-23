#pragma once
#include <implot.h>

#include "GameTimer.h"
#include "Core/Logger.h"
#include "Types/CircularBuffer.h"



class TimerGraph
{
public:
	static void OnImGui(float ms, float fps)
	{
		ImGui::Begin("Info");
		ImGui::Text("%.1f FPS", fps);
		ImGui::SameLine();
		ImGui::Text("%.3f ms", ms);

		msToPush -= GameTimer::GetDeltaTime();
		if(msToPush <= 0)
		{
			//Update FrameTimes every 0.3 seconds
			msToPush = 0.01f;
			frameTimes.Push(ms);
		}
		if (ImPlot::BeginPlot("Frame Times", ImVec2(-1, 0), ImPlotFlags_NoInputs | ImPlotFlags_NoTitle))
		{
			ImPlot::SetupAxes("time", "ms", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
			ImPlot::PlotLine("Frame Times", frameTimes.Data(), static_cast<int>(frameTimes.Size()), 0.001, 0, ImPlotLineFlags_Shaded);
			ImPlot::EndPlot();
		}

		ImGui::End();
	}
private:
	inline static float msToPush{0.01f};
	inline static CircularBuffer<500> frameTimes{};
};
