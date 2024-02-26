#pragma once
#include <vulkanbase/vulkanUtil.h>
#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"

//https://zandrofargnoli.co.uk/posts/2021/03/real-time-shader-programming/
struct SpirvHelper
{
    static std::vector<uint32_t> CompileShader(const std::string& sourceName, shaderc_shader_kind kind, const std::string& source, bool optimize = false) 
    {
		//Read the file
		std::string shaderString = readFileStr(source);

		//Create a compiler and its options
	    const shaderc::Compiler compiler{};
        shaderc::CompileOptions options{};

        //Set the optimization level
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);


        // preprocess
        const shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(shaderString, kind, sourceName.c_str(), options);

        if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            throw std::runtime_error(preprocessed.GetErrorMessage());
        }

        shaderString = { preprocessed.cbegin(), preprocessed.cend() };


        //Compile the shader
        const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString, kind, sourceName.c_str(), options);

        //Check for compilation errors
        if (module.GetCompilationStatus() != shaderc_compilation_status_success) 
        {
            throw std::runtime_error(module.GetErrorMessage());
        }


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