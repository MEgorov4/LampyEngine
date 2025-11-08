#pragma once

#include <EngineMinimal.h>

class RuntimeApplication : public Application
{
  public:
    ~RuntimeApplication() override = default;

    void startup();

  protected:
    void onStartupMinor(ContextLocator *locator) override;
    void onStartupMajor(ContextLocator *locator) override;
    void onShutdown() override;

    void render() override;
    void tick(float deltaTime) override;
};

