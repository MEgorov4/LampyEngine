#include "ECSModule.h"

#include "Events.h"

#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <ResourceModule/RWorld.h>
#include <glm/fwd.hpp>

namespace ECSModule
{
void ECSModule::startup()
{
    LT_PROFILE_SCOPE("ECSModule::startup");
    LT_LOGI("ECSModule", "Startup");

    auto* res     = GCM(ResourceModule::ResourceManager);
    auto* scripts = GCM(ScriptModule::LuaScriptModule);
    auto* physics = GCM(PhysicsModule::PhysicsModule);

    m_worldManager = std::make_unique<WorldManager>(res, scripts, physics);

    createWorld("Default");
    openBasicWorld();
}

void ECSModule::shutdown()
{
    LT_PROFILE_SCOPE("ECSModule::shutdown");
    LT_LOGI("ECSModule", "Shutdown");
    m_worldManager->clear();
    m_worldManager.reset();
}

EntityWorld* ECSModule::getCurrentWorld()
{
    LT_PROFILE_SCOPE("ECSModule::getCurrentWorld");
    if (!m_worldManager)
        return nullptr;
    return m_worldManager->getActiveWorld();
}

EntityWorld& ECSModule::createWorld(const std::string& name)
{
    LT_PROFILE_SCOPE("ECSModule::createWorld");
    auto& world = m_worldManager->createWorld(name);
    GCEB().emit(Events::ECS::WorldOpened{name});
    return world;
}

void ECSModule::closeWorld(const std::string& name)
{
    LT_PROFILE_SCOPE("ECSModule::closeWorld");
    m_worldManager->destroyWorld(name);
    GCEB().emit(Events::ECS::WorldClosed{name});
}

void ECSModule::openBasicWorld()
{
    LT_PROFILE_SCOPE("ECSModule::openBasicWorld");
    auto& world = createWorld("EditorWorld");
    // world.deserialize(Fs::readTextFile("../Resources/World/Templates/basic_world.lworld"));
    m_worldManager->setActiveWorld("EditorWorld");
}

void ECSModule::openWorldByResource(const std::string& guid)
{
    LT_PROFILE_SCOPE("ECSModule::openWorldByResource");
    auto worldRes = GCM(ResourceModule::ResourceManager)->loadBySource<ResourceModule::RWorld>(guid);
    if (!worldRes)
    {
        LT_LOGE("ECSModule", "Failed to load world resource: " + guid);
        return;
    }

    m_worldManager->createWorld(guid);
    m_worldManager->setActiveWorld(guid);
    m_worldManager->getActiveWorld()->reset();
    m_worldManager->getActiveWorld()->deserialize(worldRes->getJsonData().c_str());
}

void ECSModule::ecsTick(float dt)
{
    LT_PROFILE_SCOPE("ECSModule::ecsTick");
    if (m_inSimulate)
        m_worldManager->tickActive(dt);
}

void ECSModule::simulate(bool state)
{
    LT_PROFILE_SCOPE("ECSModule::simulate");
    if (state && !m_inSimulate)
    {
        m_inSimulate = true;
        GCEB().emit(Events::ECS::WorldOpened{"Simulation"});
    }
    else if (!state && m_inSimulate)
    {
        m_inSimulate = false;
        GCEB().emit(Events::ECS::WorldClosed{"Simulation"});
    }
}
} // namespace ECSModule
