#pragma once

#include "../EngineContext/EngineContext.h"
class Editor : public IEngineContext
{
public:
	Editor() {}
	virtual ~Editor() {}

	void init() override;
	void startupEditorModules();
	void tick(float deltaTime) override;
	void shutDown() override;
	void shutDownEditorModules();
};