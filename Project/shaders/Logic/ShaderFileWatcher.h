#pragma once
#include <efsw/System.hpp>
#include <efsw/include/efsw/efsw.hpp>
#include <thread>

#include "SpirvHelper.h"


class ShaderListener : public efsw::FileWatchListener
{
public:

	ShaderListener() = default;

	void handleFileAction(efsw::WatchID id, const std::string& str, const std::string& filename, efsw::Action action,	std::string oldFilename) override
	{
		//Check if the file was created/modified
		if(action == efsw::Actions::Modified)
		{
		    //Check if last 3 chars are .spv
		    const std::string extension = filename.substr(filename.size() - 4, 4);
            if(extension == ".spv") return;
		    
			LogInfo("Shader: " + filename + " has event Modified");

		    //Wait a little bit, As some programs write the file in chunks (meaning more then one modified event will be triggered)(e.g. Visual Studio Code)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
			SpirvHelper::CompileAndSaveShader(filename);
		}
	}
};


class ShaderFileWatcher
{
public:
	ShaderFileWatcher()
	{
		m_FileWatcher = std::make_unique<efsw::FileWatcher>();
		m_Listener = std::make_unique<ShaderListener>();

		m_FileWatcher->addWatch("shaders", m_Listener.get(), false);
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
