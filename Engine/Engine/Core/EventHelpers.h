#pragma once
#include "CoreLocator.h"

/// Глобальный доступ к EventBus ядра
#define GCEB() ::EngineCore::Base::CoreLocator::GetEventBus()
