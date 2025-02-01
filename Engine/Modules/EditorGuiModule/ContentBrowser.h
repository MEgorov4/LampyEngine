#pragma once 
#include "../ImGuiModule/GUIObject.h"
class GUIContentBrowser : public GUIObject
{
public:
	GUIContentBrowser();
	~GUIContentBrowser() override = default;
	
	void render() override;
};