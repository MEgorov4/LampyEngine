#pragma once
#include <flecs.h>
#include "../ImGuiModule/GUIObject.h"


class GUIWorldInspector : public GUIObject
{
	flecs::world& m_world;
	flecs::entity m_selectedEntity;
public:
	GUIWorldInspector();
	virtual ~GUIWorldInspector() override = default;

	virtual void render() override;
	void renderEntityList();
	void renderSelectedEntityDefaults();
};