#pragma once
#include <iostream>
#include <string>
#include <signal.h>
#include "imgui.h"



#define VulkanCheck(result, message) \
if (result != VK_SUCCESS) \
{\
	VulkanLogger::LogMessage(VulkanLogger::LogLevel::LOGERROR, message);\
}\



#define LogAssert(condition, message, isError) \
if (!(condition)) \
{\
    if(isError) \
	{\
       	VulkanLogger::LogMessage(VulkanLogger::LogLevel::LOGERROR, message);\
    }\
   	else\
   	{\
   	    VulkanLogger::LogMessage(VulkanLogger::LogLevel::WARNING, message);\
   	}\
}\




#define LogInfo(message) VulkanLogger::LogMessage(VulkanLogger::LogLevel::INFO, message)
#define LogWarning(message) VulkanLogger::LogMessage(VulkanLogger::LogLevel::WARNING, message)
#define LogError(message) VulkanLogger::LogMessage(VulkanLogger::LogLevel::LOGERROR, message)

namespace VulkanLogger
{
    enum class LogLevel : uint8_t
    {
        INFO,
        WARNING,
        LOGERROR,
    };

    struct ImguiLogging
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.

        ImguiLogging()
        {
            ClearWindow();
        }

        void ClearWindow()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (const int new_size = Buf.size(); old_size < new_size; old_size++)
            {
				if (Buf[old_size] == '\n')
				{
					LineOffsets.push_back(old_size + 1);
				}
					
            }
              
        }

        void Render(const char* title, bool* p_open = NULL)
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }

            // Main window
            const bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            const bool copy = ImGui::Button("Copy");
            ImGui::SameLine();

            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (clear) ClearWindow();
                if (copy) ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                const char* buf = Buf.begin();
                const char* buf_end = Buf.end();
                if (Filter.IsActive())
                {
                    for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        if (Filter.PassFilter(line_start, line_end)) ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                else
                {
                    ImGuiListClipper clipper;
                    clipper.Begin(LineOffsets.Size);
                    while (clipper.Step())
                    {
                        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                        {
                            const char* line_start = buf + LineOffsets[line_no];
                            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                            ImGui::TextUnformatted(line_start, line_end);
                        }
                    }
                    clipper.End();
                }
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
            ImGui::End();
        }
    };
    inline ImguiLogging Log;


    inline void LogMessage(LogLevel level, const std::string& message)
    {
        std::string prefix;
        std::string colorCode;

        switch (level)
        {
        case LogLevel::INFO:
            prefix = "[INFO]: ";
            colorCode = "\033[1;32m";
            break;
        case LogLevel::WARNING:
            prefix = "[WARNING]: ";
            colorCode = "\033[1;33m";
            break;
        case LogLevel::LOGERROR:
            prefix = "[ERROR]: ";
            colorCode = "\033[1;31m";
            break;
        }


        Log.AddLog("%s %s\n", prefix.c_str(), message.c_str());
        std::cout << colorCode << prefix << " " << message << "\033[0m" << std::endl;

#ifdef _DEBUG
		if (level == LogLevel::LOGERROR) __debugbreak();
#endif
    }
}