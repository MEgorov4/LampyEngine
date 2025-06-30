#include "ECSModule.h"
#include <format>
#include <fstream>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"
#include "ECSPhysicsSystem.h"
#include "../../ResourceModule/ResourceManager.h"
#include "../../ResourceModule/Mesh.h"
#include "../../ResourceModule/Shader.h"
#include "../../ResourceModule/Texture.h"

#include "../../FilesystemModule/FilesystemModule.h"
#include "../../PhysicsModule/PhysicsModule.h"
#include <btBulletDynamicsCommon.h>

#include "ECSComponents.h"

namespace ECSModule
{
    void ECSModule::startup(const ModuleRegistry& registry)
    {
        m_logger = std::dynamic_pointer_cast<Logger::Logger>(registry.getModule("Logger"));
        m_projectModule = std::dynamic_pointer_cast<ProjectModule::ProjectModule>(registry.getModule("ProjectModule"));
        m_filesystemModule = std::dynamic_pointer_cast<FilesystemModule::FilesystemModule>(registry.getModule("FilesystemModule"));
        m_resourceManager = std::dynamic_pointer_cast<ResourceModule::ResourceManager>(registry.getModule("ResourceManager"));
        m_luaScriptModule = std::dynamic_pointer_cast<ScriptModule::LuaScriptModule>(registry.getModule("ScriptModule"));
        m_physicsModule = std::dynamic_pointer_cast<PhysicsModule>(registry.getModule("PhysicsModule"));

        m_logger->log(Logger::LogVerbosity::Info, "Startup", "ECSModule");
        /*
        LOG_INFO("ECSModule: startup");
        */
        m_world = flecs::world();
        m_world.import<flecs::stats>();

        m_currentWorldFile = m_projectModule->getProjectConfig().getEditorStartWorld();

        registerComponents();
        registerObservers();

        fillDefaultWorld();

        ECSluaScriptsSystem::getInstance().registerSystem(m_world, m_luaScriptModule);
        ECSPhysicsSystem::getInstance().registerSystem(m_world);
    }

    void ECSModule::shutdown()
    {
        m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "ECSModule");
        m_world.reset();
    }

    void ECSModule::fillDefaultWorld()
    {
        clearWorld();


        if (!isWorldSetted())
        {
            auto& viewport_camera = m_world.entity("ViewportCamera")
                                           .set<PositionComponent>({0.f, 2.f, -1.f})
                                           .set<RotationComponent>({60.f, 0.0f, 0.0f})
                                           .set<CameraComponent>({75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true})
                                           .add<InvisibleTag>();

            m_world.entity("Room")
                   .set<PositionComponent>({0.f, 0.f, 0.f})
                   .set<RotationComponent>({0.f, 0.f, 0.f})
                   .set<ScaleComponent>({1.f, 1.f, 1.f})
                   .set<MeshComponent>({
                       "../Resources/Meshes/viking_room.obj", "", "", "../Resources/Textures/viking_room.png"
                   });
        }
        else
        {
            loadWorldFromFile(m_currentWorldFile);
        }
    }

    void ECSModule::saveCurrentWorld()
    {
        /*
        LOG_DEBUG("ECSModule: saveCurrentWorld");
        */
        std::string strJson = m_world.to_json().c_str();

        if (m_filesystemModule->writeTextFile(m_currentWorldFile, strJson) == FilesystemModule::FResult::SUCCESS)
            m_currentWorldData = strJson;
        else
            m_logger->log(Logger::LogVerbosity::Error, "Save world failed", "ECSModule");
    }

    void ECSModule::loadWorldFromFile(const std::string& path)
    {
        /*
        LOG_DEBUG("ECSModule: loadWorldFromFile path: " + path);
        */
        m_currentWorldFile = path;

        std::string strData = m_filesystemModule->readTextFile(path);

        m_world.from_json(strData.c_str());
        m_currentWorldData = m_currentWorldData = m_world.to_json();
    }

    void ECSModule::setCurrentWorldPath(const std::string& path)
    {
        /*
        LOG_DEBUG("ECSModule: setCurrentWorldPath path: " + path);
        */

        m_currentWorldFile = path;
        fillDefaultWorld();
        saveCurrentWorld();
    }


    bool ECSModule::isWorldSetted()
    {
        if (m_currentWorldFile == "default")
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    void ECSModule::clearWorld()
    {
        m_world.each([](flecs::entity& e)
        {
            if (!e.has<InvisibleTag>())
            {
                e.destruct();
            }
        });
    }

    void ECSModule::startSystems()
    {
        /*
        LOG_INFO("ECSModule: Start systems");
        */

        ECSluaScriptsSystem::getInstance().startSystem(m_world);
        m_tickEnabled = true;
        m_physicsModule->setTickEnabled(true);
    }

    void ECSModule::stopSystems()
    {
        /*
        LOG_INFO("ECSModule: Stop systems");
        */
        m_tickEnabled = false;

        m_physicsModule->setTickEnabled(false);
        ECSluaScriptsSystem::getInstance().stopSystem(m_world);

        fillDefaultWorld();
    }



    template <typename T>
    void ECSModule::registerComponent(const std::string& name)
    {
        auto component = m_world.component<T>();
        m_registeredComponents.emplace_back(component.id(), name);
    }

    void ECSModule::registerComponents()
    {
        m_world.component<PositionComponent>()
               .member("Px", &PositionComponent::x)
               .member("Py", &PositionComponent::y)
               .member("Pz", &PositionComponent::z);
        registerComponent<PositionComponent>("PositionComponent");

        m_world.component<RotationComponent>()
               .member("Rx", &RotationComponent::x)
               .member("Ry", &RotationComponent::y)
               .member("Rz", &RotationComponent::z);
        registerComponent<RotationComponent>("RotationComponent");

        m_world.component<ScaleComponent>()
               .member("Sx", &ScaleComponent::x)
               .member("Sy", &ScaleComponent::y)
               .member("Sz", &ScaleComponent::z);
        registerComponent<ScaleComponent>("ScaleComponent");

        m_world.component<ScriptComponent>()
               .member("scriptPath", &ScriptComponent::scriptPath);
        registerComponent<ScriptComponent>("ScriptComponent");

        m_world.component<CameraComponent>()
               .member("fov", &CameraComponent::fov)
               .member("aspect", &CameraComponent::aspect)
               .member("farClip", &CameraComponent::farClip)
               .member("nearClip", &CameraComponent::nearClip);
        registerComponent<CameraComponent>("CameraComponent");

        m_world.component<MeshComponent>()
               .member("meshResourcePath", &MeshComponent::meshResourcePath)
               .member("vertShaderPath", &MeshComponent::vertShaderPath)
               .member("fragShaderPath", &MeshComponent::fragShaderPath)
               .member("texturePath", &MeshComponent::texturePath);
        registerComponent<MeshComponent>("MeshComponent");

        m_world.component<RigidbodyComponent>()
               .member("mass", &RigidbodyComponent::mass);
        registerComponent<RigidbodyComponent>("RigidbodyComponent");

        m_world.component<InvisibleTag>();
    }


    void ECSModule::registerObservers()
    {
        m_world.observer<PositionComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, PositionComponent& mesh)
               {
                   this->OnComponentsChanged();
               });

        m_world.observer<RotationComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, RotationComponent& mesh)
               {
                   this->OnComponentsChanged();
               });

        m_world.observer<ScaleComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, ScaleComponent& mesh)
               {
                   this->OnComponentsChanged();
               });

        m_world.observer<CameraComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, CameraComponent& mesh)
               {
                   this->OnComponentsChanged();
               });

        m_world.observer<DirectionalLightComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, DirectionalLightComponent& dirLight)
               {
                   this->OnComponentsChanged();
               });

        m_world.observer<MeshComponent>()
               .event(flecs::OnSet)
               .each([this](flecs::entity e, MeshComponent& mesh)
               {
                   using namespace ResourceModule;

                   std::shared_ptr<RMesh> resMesh = m_resourceManager->load<RMesh>(mesh.meshResourcePath);
                   if (resMesh && mesh.meshResource != resMesh) mesh.meshResource = resMesh;

                   std::shared_ptr<RShader> resVertShader = m_resourceManager->load<RShader>(mesh.vertShaderPath);
                   std::shared_ptr<RShader> resFragShader = m_resourceManager->load<RShader>(mesh.fragShaderPath);
                   if ((resVertShader && mesh.vertShaderResource != resVertShader) && (resFragShader && mesh.
                       fragShaderResource != resFragShader))
                   {
                       mesh.vertShaderResource = resVertShader;
                       mesh.fragShaderResource = resFragShader;
                   }

                   std::shared_ptr<RTexture> resTexturePath = m_resourceManager->load<RTexture>(mesh.texturePath);
                   if (resTexturePath && mesh.textureResource != resTexturePath)
                       mesh.textureResource = resTexturePath;

                   OnComponentsChanged();
               });

        m_world.observer<MeshComponent>()
               .event(flecs::OnRemove)
               .each([this](flecs::entity e, MeshComponent& mesh)
               {
                   using namespace ResourceModule;
                   if (mesh.meshResource)
                       m_resourceManager->unload<RMesh>(mesh.meshResource.value()->getGUID());

                   if (mesh.vertShaderResource)
                       m_resourceManager->unload<RShader>(mesh.vertShaderResource.value()->getGUID());
                   if (mesh.fragShaderResource)
                       m_resourceManager->unload<RShader>(mesh.fragShaderResource.value()->getGUID());

                   if (mesh.textureResource)
                       m_resourceManager->unload<RShader>(mesh.textureResource.value()->getGUID());

                   OnComponentsChanged();
               });
    }

    void ECSModule::ecsTick(float deltaTime)
    {
        if (m_tickEnabled)
        {
            if (m_world.progress(deltaTime))
            {
            }
        }
    }
}
