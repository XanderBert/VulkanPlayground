#pragma once
#include "Shader.h"
#include "lua.hpp"
#include "sol/sol.hpp"
#include <filesystem>

class ShaderFactory final
{
public:
	ShaderFactory()
	{
		lua.open_libraries(sol::lib::base, sol::lib::io);
		//Load the Lua Script

	}


	~ShaderFactory() = default;
	ShaderFactory(const ShaderFactory&) = delete;
	ShaderFactory& operator=(const ShaderFactory&) = delete;
	ShaderFactory(ShaderFactory&&) = delete;
	ShaderFactory& operator=(ShaderFactory&&) = delete;

	void Render() 
	{
		ImGui::Begin("Shader Factory");
		{
			ImGui::InputText("Shader Name", m_ShaderName, 13);

			
			//Create dropdown for shader type
			ImGui::Text("Shader Type");
			ImGui::SameLine();

	
			//ImGui::BeginListBox("##ShaderType");
			//{
			//	if (ImGui::Selectable("Vertex")) { m_ShaderStage = VK_SHADER_STAGE_VERTEX_BIT; }
			//	if (ImGui::Selectable("Fragment")) { m_ShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT; }
			//	if (ImGui::Selectable("Geometry")) {}
			//	if (ImGui::Selectable("Tessellation Control")) {}
			//	if (ImGui::Selectable("Tessellation Evaluation")) {}
			//}
			//ImGui::EndListBox();

			if (ImGui::Button("Create Shader")) 
			{
				//Load the Lua Script
				lua.script_file("CreateShader.lua");

				// Execute the Lua Function
				const std::function<void(std::string, std::string, std::string)>& CreateShader = lua["CreateShader"];
				CreateShader("shaders/shader.frag", "shaders", m_ShaderName);
			}
		}
		ImGui::End();
	}
private:
	//create char* of size 15 for shader name
	char m_ShaderName[15] = {"Test"};

	VkShaderStageFlagBits m_ShaderStage;

	//Lua
	sol::state lua{};
};