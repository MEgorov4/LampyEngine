#pragma once

#include <EngineMinimal.h>

namespace ScriptModule
{
	class LuaScriptModule;
}

namespace RenderModule
{
	class RenderModule;
}

namespace InputModule
{
	class InputModule;
}

namespace FilesystemModule
{
	class FilesystemModule;
}

namespace ECSModule
{
	class ECSModule;
}

namespace ProjectModule
{
	class ProjectModule;
}

namespace Logger
{
	class Logger;
}

namespace ImGUIModule
{
	class ImGUIModule;
}


class EditorGUIModule : public IModule
{
	ImGUIModule::ImGUIModule* m_imGuiModule;
	ProjectModule::ProjectModule* m_projectModule;
	FilesystemModule::FilesystemModule* m_filesystemModule;
	ECSModule::ECSModule* m_ecsModule;
	RenderModule::RenderModule* m_renderModule;
	InputModule::InputModule* m_inputModule;
	ScriptModule::LuaScriptModule* m_luaScriptModule;
public:
	void startup() override;
	void shutdown() override;

	void render(float deltaTime) const;
};
