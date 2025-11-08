#pragma once

#include "Context.h"
#include "ContextLocator.h"
#include "Core.h"
#include "CoreLocator.h"
#include "IModule.h"
#include "ModuleEventBinder.h"

/// GCM (Get Core Module)
#define GCM(type) (&Core::Locator().get<type>())

/// GCM(Get ConteXt Module)
#define GCXM(type) (&Context::Locator().get<type>())

/// GCEB (Get Core Event Bus)
#define GCEB() ::EngineCore::Base::CoreLocator::GetEventBus()

using namespace EngineCore::Base;
