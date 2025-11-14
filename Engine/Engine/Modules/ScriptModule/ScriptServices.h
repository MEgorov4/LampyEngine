#pragma once

#include <Foundation/Event/Event.h>

#include <SDL3/SDL_events.h>

namespace AudioModule
{
class AudioModule;
}

class EntityWorld;

namespace ScriptModule
{
class IInputScriptService
{
  public:
    virtual ~IInputScriptService() = default;
    virtual EngineCore::Foundation::Event<SDL_KeyboardEvent>& keyboardEvent() = 0;
};

class IAudioScriptService
{
  public:
    virtual ~IAudioScriptService() = default;
    virtual void playSoundAsync() = 0;
};

class IECSWorldScriptService
{
  public:
    virtual ~IECSWorldScriptService() = default;
    virtual EntityWorld* currentWorld() = 0;
};
} // namespace ScriptModule

