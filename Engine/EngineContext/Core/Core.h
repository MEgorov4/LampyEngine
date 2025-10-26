#pragma once
#include "CoreLocator.h"

namespace EngineCore::Base
{
    class Core final
    {
    public:
        Core() = delete;

        static CoreLocator& Locator()
        {
            static CoreLocator instance;
            return instance;
        }

        template<typename T>
        static T& Get()
        {
            return Locator().get<T>();
        }

        template<typename T>
        static void Register(std::shared_ptr<T> module, int priority = 0)
        {
            Locator().registerModule<T>(std::move(module), priority);
        }

        static void StartupAll()
        {
            Locator().startupAll();
        }

        static void ShutdownAll()
        {
            Locator().shutdownAll();
        }
    };
}
