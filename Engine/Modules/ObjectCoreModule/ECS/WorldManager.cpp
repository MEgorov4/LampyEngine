#include "WorldManager.h"

#include "Events.h"
#include "Foundation/Profiler/ProfileAllocator.h"

#include <EngineMinimal.h>

using namespace ECSModule;

EntityWorld &WorldManager::createWorld(const std::string &name)
{
    if (m_worlds.contains(name))
    {
        LT_LOGW("ECSModule", "World already exists: " + name);
        return *m_worlds[name];
    }

    auto world = std::make_unique<EntityWorld>(m_resources, m_scripts, m_physics);
    world->init();
    EntityWorld &ref = *world;
    m_worlds[name] = std::move(world);

    LT_LOGI("ECSModule", "Created world: " + name);
    GCEB().emit(Events::ECS::WorldOpened{name});
    return ref;
}

void WorldManager::destroyWorld(const std::string &name)
{
    auto it = m_worlds.find(name);
    if (it == m_worlds.end())
        return;

    GCEB().emit(Events::ECS::WorldClosed{name});
    m_worlds.erase(it);

    if (m_activeWorld == name)
        m_activeWorld.clear();
}

EntityWorld *WorldManager::getWorld(const std::string &name)
{
    auto it = m_worlds.find(name);
    return (it != m_worlds.end()) ? it->second.get() : nullptr;
}

EntityWorld *WorldManager::getActiveWorld()
{
    if (m_activeWorld.empty())
        return nullptr;
    auto it = m_worlds.find(m_activeWorld);
    return (it != m_worlds.end()) ? it->second.get() : nullptr;
}

void WorldManager::setActiveWorld(const std::string &name)
{
    if (m_worlds.contains(name))
    {
        m_activeWorld = name;
        LT_LOGI("ECSModule", "Active world switched to: " + name);
    }
}

void WorldManager::tickActive(float dt)
{
    if (auto *active = getActiveWorld())
        active->tick(dt);
}

void WorldManager::clear()
{
    m_worlds.clear();
    m_activeWorld.clear();
}

std::vector<std::string, ProfileAllocator<std::string>> WorldManager::getLoadedWorlds() const
{
    std::vector<std::string, ProfileAllocator<std::string>> names;
    names.reserve(m_worlds.size());
    for (auto &[n, _] : m_worlds)
        names.push_back(n);
    return names;
}
