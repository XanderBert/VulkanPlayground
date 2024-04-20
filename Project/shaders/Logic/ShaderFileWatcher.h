#pragma once
#include "efsw/include/efsw/efsw.hpp"
#include "SpirvHelper.h"
#include "efsw/System.hpp"
#include "Core/GraphicsPipeline.h"

class ShaderListener : public efsw::FileWatchListener
{
public:

	ShaderListener() = default;

	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action,	std::string oldFilename) override
	{
		//Wait for a bit to make sure the file is done being written (as sometimes it is not done yet)
		efsw::System::sleep(100);

		//Check if the file was created/modified
		if(action == efsw::Actions::Modified || action == efsw::Actions::Moved)
		{
			const shaderc_shader_kind kind = GetShaderKind(filename);

			//Check if its a shader file
			if (kind == shaderc_glsl_infer_from_source) return;

			LogInfo("Shader: " + filename + " has event Modified");

			//Compile and Save the shader
			SpirvHelper::CompileAndSaveShader(filename, kind, "shaders/" + filename);
		}
	}

private:
	static shaderc_shader_kind GetShaderKind(const std::string& filename)
	{
		// pick the last 4 characters of the filename
		const std::string extension = filename.substr(filename.size() - 5, 5);

		//return the kind based on the extionsion
		if(extension == ".vert") return shaderc_glsl_vertex_shader;
		if(extension == ".frag") return shaderc_glsl_fragment_shader;
		if(extension == ".geom") return shaderc_glsl_geometry_shader;
		if(extension == ".comp") return shaderc_glsl_compute_shader;
		if(extension == ".tesc") return shaderc_glsl_tess_control_shader;
		if(extension == ".tese") return shaderc_glsl_tess_evaluation_shader;

		return shaderc_glsl_infer_from_source;
	}
};


class ShaderFileWatcher
{
public:
	ShaderFileWatcher()
	{
		m_FileWatcher = std::make_unique<efsw::FileWatcher>();
		m_Listener = std::make_unique<ShaderListener>();

		efsw::WatchID watchID = m_FileWatcher->addWatch("shaders", m_Listener.get(), true);
		m_FileWatcher->watch();
	}

	~ShaderFileWatcher() = default;

	ShaderFileWatcher(const ShaderFileWatcher&) = delete;
	ShaderFileWatcher& operator=(const ShaderFileWatcher&) = delete;
	ShaderFileWatcher(ShaderFileWatcher&&) = delete;
	ShaderFileWatcher& operator=(ShaderFileWatcher&&) = delete;

private:
	inline static std::unique_ptr<efsw::FileWatcher> m_FileWatcher;
	inline static std::unique_ptr<ShaderListener> m_Listener;
};
