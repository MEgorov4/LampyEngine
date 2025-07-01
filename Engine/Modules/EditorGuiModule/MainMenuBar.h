#pragma once

#include "../ImGuiModule/GUIObject.h"
#include "../EventModule/Event.h"


class GUIMainMenuBar : public ImGUIModule::GUIObject
{
public:
	GUIMainMenuBar();
	virtual ~GUIMainMenuBar();

	Event<std::string> OnOpenClicked;
	
	virtual void render(float deltaTime) override;
};
