#pragma once

#include <EngineMinimal.h>
#include "../ImGuiModule/GUIObject.h"


class GUIMainMenuBar : public ImGUIModule::GUIObject
{
public:
	GUIMainMenuBar();
	virtual ~GUIMainMenuBar();

	Event<std::string> OnOpenClicked;
	
	virtual void render(float deltaTime) override;
};
