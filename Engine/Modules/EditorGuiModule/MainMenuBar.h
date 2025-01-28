#pragma once

#include "../ImGuiModule/GUIObject.h"
#include "../EventModule/Event.h"
class GUIMainMenuBar : public GUIObject
{
public:
	GUIMainMenuBar();
	virtual ~GUIMainMenuBar() = default;

	Event<std::string> OnOpenClicked;
	
	virtual void render() override;
};
