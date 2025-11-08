#pragma once

#include <EngineMinimal.h>
#include "../ImGuiModule/GUIObject.h"

namespace FilesystemModule
{
	class FilesystemModule;
}

class GUIEditorScriptPanel : ImGUIModule::GUIObject
{
	std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
public:
	GUIEditorScriptPanel(const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule);
	~GUIEditorScriptPanel() override = default;

	void render(float deltaTime) override;
};
