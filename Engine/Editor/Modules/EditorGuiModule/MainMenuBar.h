#pragma once

#include <EngineMinimal.h>
#include <Modules/ImGuiModule/GUIObject.h> 


class GUIMainMenuBar : public ImGUIModule::GUIObject
{
public:
	GUIMainMenuBar();
	virtual ~GUIMainMenuBar();
	
	virtual void render(float deltaTime) override;

private:
	void renderViewMenu();
};
