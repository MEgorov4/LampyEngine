#pragma once

#include <EngineMinimal.h>
#include <optional>

class RuntimeApplication : public Application
{
  public:
    ~RuntimeApplication() override = default;

    void startup();
    void setProjectFileOverride(const std::string& path);

  protected:
    void onStartupMinor(ContextLocator *locator) override;
    void onStartupMajor(ContextLocator *locator) override;
    void onShutdown() override;

    void render() override;
    void tick(float deltaTime) override;

    void configureModules(ModuleConfigRegistry& registry) override;

  private:
    std::optional<std::string> m_projectFileOverride;
};

