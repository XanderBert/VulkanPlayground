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
        LogInfo("Compiling shader: " + sourceName);

		//Read the file
		std::string shaderString = tools::readFileStr(source);

		//Create a compiler and its options
	    const shaderc::Compiler compiler{};
        shaderc::CompileOptions options{};
        options.SetSourceLanguage(shaderc_source_language_glsl);
#ifdef _DEBUG
        options.SetGenerateDebugInfo();
#endif
        //Set the optimization level
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

        // preprocess
        const shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(shaderString, kind, sourceName.c_str(), options);

		LogAssert(preprocessed.GetCompilationStatus() == shaderc_compilation_status_success, preprocessed.GetErrorMessage(), false)

        shaderString = { preprocessed.cbegin(), preprocessed.cend() };

        //Compile the shader
        const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString, kind, sourceName.c_str(), options);

        //Check for compilation errors
		LogAssert(module.GetCompilationStatus() == shaderc_compilation_status_success, module.GetErrorMessage(), false)


        //Return the compiled binary
        return { module.cbegin(), module.cend() };
    }

    static void CompileAndSaveShader(const std::string& sourceName, bool optimize = false)
    {
        //Check the extension of the file
        if(GetShaderKind(sourceName) == shaderc_glsl_infer_from_source)
        {
            LogWarning("Unknown shader extension: " + sourceName);
            LogWarning("Shader will not be compiled!");
            return;
        }

    	//Compile the shader
		auto shaderBinary = CompileShader(sourceName, GetShaderKind(sourceName), "shaders/" + sourceName, optimize);

		//Store the shader binary to a file
		std::ofstream file("shaders/" + sourceName + ".spv", std::ios::binary);
		file.write((char*)shaderBinary.data(), shaderBinary.size() * sizeof(uint32_t));
		file.close();
	}

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