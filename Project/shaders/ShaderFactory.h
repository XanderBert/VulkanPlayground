#pragma once
#include "imgui.h"
#include "Luascripts/LuaScriptRunner.h"
#include "Patterns/ServiceLocator.h"


class ShaderFactory final
{
public:
	ShaderFactory() = default;
	~ShaderFactory() = default;
	ShaderFactory(const ShaderFactory&) = delete;
	ShaderFactory& operator=(const ShaderFactory&) = delete;
	ShaderFactory(ShaderFactory&&) = delete;
	ShaderFactory& operator=(ShaderFactory&&) = delete;

	static void Render()
	{

		ImGui::Begin("Shader Factory");
		{
			ImGui::InputText("Shader Name", m_ShaderName, m_ShaderNameSize);

			
			//Create dropdown for shader type
			ImGui::Text("Shader Type");
			if(ImGui::BeginListBox("##ShaderType"))
			{
				if (ImGui::Selectable("Vertex")) { m_ShaderExtension = ".vert"; }
				if (ImGui::Selectable("Fragment")) { m_ShaderExtension = ".frag"; }
				if (ImGui::Selectable("Geometry")) { m_ShaderExtension = ".geom"; }
				if (ImGui::Selectable("Compute")) { m_ShaderExtension = ".comp"; }
				if (ImGui::Selectable("Tessellation Control")) {  m_ShaderExtension = ".tesc";}
				if (ImGui::Selectable("Tessellation Evaluation")) {m_ShaderExtension = ".tese";}
				ImGui::EndListBox();
			}


			if (ImGui::Button("Create Shader")) 
			{
				// Execute the Lua Function to create the base shader
				sol::state& lua = ServiceLocator::GetService<LuaScriptRunner>()->GetLuaRunner();
				lua.script_file("LuaScripts/CreateShader.lua");
				const std::function<void(std::string, std::string, std::string)>& CreateShader = lua["CreateShader"];

				const std::string shaderName = m_ShaderName + m_ShaderExtension;

				//Create the actual shader based on the BaseShaders
				CreateShader("shaders/BaseShaders/BaseShader" + m_ShaderExtension, "shaders", shaderName);
			}
		}
		ImGui::End();
	}

private:
	inline const static int m_ShaderNameSize = 15;
	inline static char m_ShaderName[m_ShaderNameSize] = {"Test"};
	inline static std::string m_ShaderExtension{ ".frag" };
};
