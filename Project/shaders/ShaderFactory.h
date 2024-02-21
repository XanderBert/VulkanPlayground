#pragma once
#include "Shader.h"
#include "Luascripts/LuaScriptRunner.h"

class ShaderFactory final
{
public:
	ShaderFactory() = default;
	~ShaderFactory() = default;
	ShaderFactory(const ShaderFactory&) = delete;
	ShaderFactory& operator=(const ShaderFactory&) = delete;
	ShaderFactory(ShaderFactory&&) = delete;
	ShaderFactory& operator=(ShaderFactory&&) = delete;

	void Render() 
	{
		ImGui::Begin("Shader Factory");
		{
			ImGui::InputText("Shader Name", m_ShaderName, m_ShaderNameSize);

			
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
				sol::state& lua = LuaScriptRunner::GetInstance().GetLuaRunner();
				lua.script_file("LuaScripts/CreateShader.lua");

				// Execute the Lua Function
				const std::function<void(std::string, std::string, std::string)>& CreateShader = lua["CreateShader"];
				CreateShader("shaders/shader.frag", "shaders", m_ShaderName);
			}
		}
		ImGui::End();
	}
private:
	const static int m_ShaderNameSize = 15;
	char m_ShaderName[m_ShaderNameSize] = {"Test"};
};