#pragma once
#include <fstream>
#include <optional>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <vulkanbase/vulkanUtil.h>
#include "Core/Logger.h"
#include "Patterns/Delegate.h"

struct SpirvHelper
{
    inline static Delegate<const std::string&> OnCompilingFinished;

    struct includer : public shaderc::CompileOptions::IncluderInterface {

         struct result_t : shaderc_include_result
         {
             std::string filepath;
             std::vector<char> code;
         };

         shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type, const char* requesting_source, size_t) override {
             auto& result = *(new result_t);
             auto& filepath = result.filepath;
             auto& code = result.code;


            filepath = "shaders/";
            filepath += requested_source;

             size_t pos = filepath.rfind('/');
             if (pos == -1)
                 pos = filepath.rfind('\\');
             filepath.replace(pos + 1 + filepath.begin(), filepath.end(), requested_source);

             code = tools::readFile(filepath.c_str());


             static_cast<shaderc_include_result&>(result) =
             {
                 filepath.c_str(),
                 filepath.length(),
                 code.data(),
                 code.size(),
                 this
             };

             return &result;
         }

         void ReleaseInclude(shaderc_include_result* data) override
         {
             delete static_cast<result_t*>(data);
         }
     };

    static std::optional<std::vector<uint32_t>> CompileShader(const std::string& sourceName, shaderc_shader_kind kind, const std::string& source, bool& didCompileSuccessfully, bool optimize = false)
    {
        LogInfo("Compiling shader: " + sourceName);

		//Read the file
		std::string shaderString = tools::readFileStr(source);

		//Create a compiler and its options
	    const shaderc::Compiler compiler{};

        shaderc::CompileOptions options{};
        options.SetIncluder(std::move(std::make_unique<includer>()));
        options.SetSourceLanguage(shaderc_source_language_glsl);

#ifdef _DEBUG
        options.SetGenerateDebugInfo();
#endif
        //Set the optimization level
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);



        // preprocess
        const shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(shaderString, kind, sourceName.c_str(), options);
        shaderString = { preprocessed.cbegin(), preprocessed.cend() };

        //Compile the shader
        const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString, kind, sourceName.c_str(), options);

        didCompileSuccessfully =
            preprocessed.GetCompilationStatus() == shaderc_compilation_status_success &&
            module.GetCompilationStatus() == shaderc_compilation_status_success &&
            module.cend() - module.cbegin() > 0 &&
            preprocessed.cend() - preprocessed.cbegin() > 0;

        if(!didCompileSuccessfully)
        {
            LogError("Failed to compile shader: " + sourceName);
            LogError(module.GetErrorMessage());
            return std::nullopt;
        }

        //Return the compiled binary
        auto binary = std::optional<std::vector<uint32_t>>{};
        //set the value
        binary = { module.cbegin(), module.cend() };
        return binary;
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

        bool didCompileSuccessfully = false;

    	//Compile the shader
		auto shaderBinary = CompileShader(sourceName, GetShaderKind(sourceName), "shaders/" + sourceName, didCompileSuccessfully, optimize);
        if(!shaderBinary.has_value()) return;

        std::vector<uint32_t> binaryData = shaderBinary.value();

		//Store the shader binary to a file
		std::ofstream file("shaders/" + sourceName + ".spv", std::ios::binary);
		file.write(reinterpret_cast<char *>(binaryData.data()), binaryData.size() * sizeof(uint32_t));
		file.close();

        if(didCompileSuccessfully)
        {
            //Notify the listeners
            OnCompilingFinished.Broadcast(sourceName);
        }
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