#pragma once
#include <fstream>
#include <vulkanbase/vulkanUtil.h>
#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"
#include "Core/Logger.h"

struct SpirvHelper
{
    static std::vector<uint32_t> CompileShader(const std::string& sourceName, shaderc_shader_kind kind, const std::string& source, bool optimize = false) 
    {
		//Read the file
		std::string shaderString = tools::readFileStr(source);

		LogInfo("Compiling shader: " + sourceName);
		LogInfo("Shader source: "+ shaderString);


		//Create a compiler and its options
	    const shaderc::Compiler compiler{};
        shaderc::CompileOptions options{};

        //Set the optimization level
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

        // preprocess
        const shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(shaderString, kind, sourceName.c_str(), options);

		LogAssert(preprocessed.GetCompilationStatus() == shaderc_compilation_status_success, preprocessed.GetErrorMessage(), true)

        shaderString = { preprocessed.cbegin(), preprocessed.cend() };

        //Compile the shader
        const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString, kind, sourceName.c_str(), options);

        //Check for compilation errors
		LogAssert(module.GetCompilationStatus() == shaderc_compilation_status_success, module.GetErrorMessage(), true)

        //Return the compiled binary
        return { module.cbegin(), module.cend() };
    }
	static void CompileAndSaveShader(const std::string& sourceName, shaderc_shader_kind kind, const std::string& source, bool optimize = false)
    {
    	//Compile the shader
		auto shaderBinary = CompileShader(sourceName, kind, source, optimize);

		//Store the shader binary to a file
		std::ofstream file(source + ".spv", std::ios::binary);
		file.write((char*)shaderBinary.data(), shaderBinary.size() * sizeof(uint32_t));
		file.close();
	}
};