#include "ShaderEditor.h"

#include "vulkanbase/VulkanUtil.h"


void ShaderEditor::Init()
{
    const TextEditor::LanguageDefinition langDef = TextEditor::LanguageDefinition::GLSL();
    m_Editor.SetLanguageDefinition(langDef);

    m_Editor.SetShowWhitespaces(false);
}

void ShaderEditor::Render() {
    ImGui::Begin("Shader Editor");
    if (ImGui::Button("Save")) SaveFile();
    m_Editor.Render("title of file");
    ImGui::End();
}
void ShaderEditor::OpenFileForEdit(const std::filesystem::path &path)
{
    m_CurrentPath = path;
    m_Editor.SetText(tools::readFileStr(path.generic_string()));
}

void ShaderEditor::SaveFile()
{
    //Check if a file is open
    if(m_CurrentPath.empty())
    {
        LogWarning("No file open to save");
        return;
    }

    tools::writeFileStr(m_CurrentPath.generic_string(), m_Editor.GetText());
}



