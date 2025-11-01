#pragma once
#include <EngineMinimal.h>

namespace Events::ECS
{
    struct WorldOpened
    {
        std::string name;
    };

    struct WorldClosed
    {
        std::string name;
    };

    struct EntityCreated
    {
        uint64_t id;
        std::string name;
    };

    struct EntityDestroyed
    {
        uint64_t id;
    };

    struct ComponentChanged
    {
        uint64_t entityId;
        std::string componentName;
    };
}