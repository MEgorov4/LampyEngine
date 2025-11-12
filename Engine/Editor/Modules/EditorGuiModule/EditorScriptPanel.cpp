#include "EditorScriptPanel.h"

#include "imgui.h"

GUIEditorScriptPanel::GUIEditorScriptPanel(const std::shared_ptr<FilesystemModule::FilesystemModule> &filesystemModule)
    : m_filesystemModule(filesystemModule)
{
    std::string resourcesPath = Fs::currentPath() + "\\..\\Resources\\Scripts\\";
    std::vector<std::string, ProfileAllocator<std::string>> items = Fs::getDirectoryContents(
        resourcesPath,
        SearchFilter{DirContentType::All, std::vector<std::string, ProfileAllocator<std::string>>({".lua"})});
}

void GUIEditorScriptPanel::render(float deltaTime)
{
    ZoneScopedN("GUIObject::ScriptPanel");
    if (ImGui::Begin("ScriptComponent Panel"))
    {
        if (ImGui::BeginChild("Scripts"))
        {
            ImGui::EndChild();
        }
    }

    ImGui::End();
}
