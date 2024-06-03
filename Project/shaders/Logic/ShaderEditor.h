#pragma once
#include <filesystem>


#include "TextEditor.h"


class ShaderEditor
{
public:
    ShaderEditor() = default;
    ~ShaderEditor() = default;

    ShaderEditor(const ShaderEditor&) = delete;
    ShaderEditor& operator=(const ShaderEditor&) = delete;
    ShaderEditor(ShaderEditor&&) = delete;
    ShaderEditor& operator=(ShaderEditor&&) = delete;


    static void Init();
    static void Render();

    static void OpenFileForEdit(const std::filesystem::path& path);
    static void SaveFile();


private:
    inline static TextEditor m_Editor{};
    inline static std::filesystem::path m_CurrentPath{};
};

