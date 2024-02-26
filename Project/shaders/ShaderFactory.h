#pragma once
#include "Shader.h"
#include "Luascripts/LuaScriptRunner.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "SpirvHelper.h"

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
			if(ImGui::BeginListBox("##ShaderType"))
			{
				if (ImGui::Selectable("Vertex")) { m_ShaderType = shaderc_vertex_shader; m_ShaderExtension = ".vert"; }
				if (ImGui::Selectable("Fragment")) { m_ShaderType = shaderc_fragment_shader; m_ShaderExtension = ".frag"; }
				if (ImGui::Selectable("Geometry")) { m_ShaderType = shaderc_geometry_shader; }
				if (ImGui::Selectable("Compute")) { m_ShaderType = shaderc_compute_shader; }
				if (ImGui::Selectable("Tessellation Control")) { m_ShaderType = shaderc_tess_control_shader; }
				if (ImGui::Selectable("Tessellation Evaluation")) { m_ShaderType = shaderc_tess_evaluation_shader; }
				ImGui::EndListBox();
			}




			if (ImGui::Button("Create Shader")) 
			{
				// Execute the Lua Function to create the base shader
				sol::state& lua = LuaScriptRunner::GetInstance().GetLuaRunner();
				lua.script_file("LuaScripts/CreateShader.lua");
				const std::function<void(std::string, std::string, std::string)>& CreateShader = lua["CreateShader"];


				const std::string shaderName = m_ShaderName + m_ShaderExtension;


				//Create the actual shader based on the BaseShaders
				CreateShader("shaders/BaseShaders/BaseShader" + m_ShaderExtension, "shaders", shaderName);


				//Compile the new shader
				SpirvHelper::CompileAndSaveShader(shaderName, m_ShaderType, "shaders/" + shaderName);
			}
		}
		ImGui::End();
	}

private:
	inline const static int m_ShaderNameSize = 15;
	inline static char m_ShaderName[m_ShaderNameSize] = {"Test"};
	inline static std::string m_ShaderExtension{ ".frag" };
	inline static shaderc_shader_kind m_ShaderType{ shaderc_vertex_shader };
};