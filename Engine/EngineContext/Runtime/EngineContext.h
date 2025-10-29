#pragma once
#include "Core/ContextLocator.h"
namespace EngineCore::Runtime {
class IEngineContext {
public:
  virtual ~IEngineContext() = default;

  virtual void initMinor(ContextLocator &locator) = 0;

  virtual void initMajor(ContextLocator &locator) = 0;

  virtual void tick(float deltaTime) = 0;
  virtual void shutdown() = 0;
}; // namespace EngineCore::Runtime
} // namespace EngineCore::Runtime
