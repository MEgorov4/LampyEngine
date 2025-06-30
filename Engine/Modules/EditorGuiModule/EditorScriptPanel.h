#pragma once
#include <memory>

#include "../ImGuiModule/GUIObject.h"

namespace FilesystemModule
{
	class FilesystemModule;
}

class GUIEditorScriptPanel : ImGuiModule::GUIObject
{
	std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
public:
	GUIEditorScriptPanel(const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule);
	~GUIEditorScriptPanel() override = default;

	void render() override;
};
