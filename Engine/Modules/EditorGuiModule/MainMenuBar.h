#pragma once

#include "../ImGuiModule/GUIObject.h"

class GUIMainMenuBar : public GUIObject
{

public:
	GUIMainMenuBar();
	virtual ~GUIMainMenuBar() = default;
	
	virtual void render() override;
};
