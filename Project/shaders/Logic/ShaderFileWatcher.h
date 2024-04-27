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
		    //Check if last 3 chars are .spv
		    const std::string extension = filename.substr(filename.size() - 4, 4);
            if(extension == ".spv")
            {
                LogInfo("Shader: " + filename + " is a .spv file, skipping");
                return;
            }

			LogInfo("Shader: " + filename + " has event Modified");

			//Compile and Save the shader
			SpirvHelper::CompileAndSaveShader(filename);
		}
	}

private:

};


class ShaderFileWatcher
{
public:
	ShaderFileWatcher()
	{
		m_FileWatcher = std::make_unique<efsw::FileWatcher>();
		m_Listener = std::make_unique<ShaderListener>();

		m_FileWatcher->addWatch("shaders", m_Listener.get(), true);
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
