#pragma once

#include <EngineMinimal.h>
#include <flecs.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

namespace ECSModule
{
    class IComponentFactory
    {
    public:
        virtual ~IComponentFactory() = default;
        virtual void addToEntity(flecs::entity& entity) = 0;
        virtual void removeFromEntity(flecs::entity& entity) = 0;
        virtual bool hasComponent(flecs::entity& entity) const = 0;
        virtual std::string getTypeName() const = 0;
        virtual std::string getDisplayName() const = 0;
    };

    class ComponentRegistry
    {
    public:
        static ComponentRegistry& getInstance()
        {
            static ComponentRegistry instance;
            return instance;
        }

        void registerComponent(std::unique_ptr<IComponentFactory> factory)
        {
            std::string typeName = factory->getTypeName();
            m_factories[typeName] = std::move(factory);
        }

        std::vector<std::pair<std::string, std::string>> getAvailableComponents() const
        {
            std::vector<std::pair<std::string, std::string>> result;
            result.reserve(m_factories.size());
            for (const auto& [typeName, factory] : m_factories)
            {
                result.emplace_back(typeName, factory->getDisplayName());
            }
            return result;
        }

        bool addComponent(flecs::entity& entity, const std::string& componentTypeName) const
        {
            auto it = m_factories.find(componentTypeName);
            if (it != m_factories.end())
            {
                if (!it->second->hasComponent(entity))
                {
                    it->second->addToEntity(entity);
                    return true;
                }
            }
            return false;
        }

        bool hasComponent(flecs::entity& entity, const std::string& componentTypeName) const
        {
            auto it = m_factories.find(componentTypeName);
            if (it != m_factories.end())
            {
                return it->second->hasComponent(entity);
            }
            return false;
        }

        bool isRegistered(const std::string& componentTypeName) const
        {
            return m_factories.find(componentTypeName) != m_factories.end();
        }

        bool removeComponent(flecs::entity& entity, const std::string& componentTypeName) const
        {
            auto it = m_factories.find(componentTypeName);
            if (it != m_factories.end() && it->second->hasComponent(entity))
            {
                it->second->removeFromEntity(entity);
                return true;
            }
            return false;
        }

        bool resetComponent(flecs::entity& entity, const std::string& componentTypeName) const
        {
            auto it = m_factories.find(componentTypeName);
            if (it != m_factories.end() && it->second->hasComponent(entity))
            {
                // Remove and re-add to reset to defaults
                it->second->addToEntity(entity);
                return true;
            }
            return false;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<IComponentFactory>> m_factories;
    };

    class ComponentFactory : public IComponentFactory
    {
    public:
        ComponentFactory(std::string typeName, std::string displayName, 
                         std::function<void(flecs::entity&)> adder,
                         std::function<void(flecs::entity&)> remover,
                         std::function<bool(flecs::entity&)> checker)
            : m_typeName(std::move(typeName))
            , m_displayName(std::move(displayName))
            , m_adder(std::move(adder))
            , m_remover(std::move(remover))
            , m_checker(std::move(checker))
        {}

        void addToEntity(flecs::entity& entity) override
        {
            m_adder(entity);
        }

        void removeFromEntity(flecs::entity& entity) override
        {
            m_remover(entity);
        }

        bool hasComponent(flecs::entity& entity) const override
        {
            return m_checker(entity);
        }

        std::string getTypeName() const override { return m_typeName; }
        std::string getDisplayName() const override { return m_displayName; }

    private:
        std::string m_typeName;
        std::string m_displayName;
        std::function<void(flecs::entity&)> m_adder;
        std::function<void(flecs::entity&)> m_remover;
        std::function<bool(flecs::entity&)> m_checker;
    };
}

