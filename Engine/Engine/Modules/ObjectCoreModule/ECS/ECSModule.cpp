#include "ECSModule.h"

#include "ComponentRegistry.h"
#include "Components/ECSComponents.h"
#include "Events.h"
#include "Systems/ECSLuaScriptsSystem.h"
#include "Systems/ECSPhysicsSystem.h"
#include "Editor/Modules/EditorGuiModule/Events.h"
#include <Modules/LuaScriptModule/LuaScriptModule.h>
#include <Modules/PhysicsModule/PhysicsModule.h>
#include <Modules/PhysicsModule/PhysicsLocator.h>
#include <Modules/PhysicsModule/PhysicsContext/PhysicsContext.h>
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Components/CharacterControllerComponent.h>
#include <Modules/PhysicsModule/Components/PhysicsMaterialComponent.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/RWorld.h>
#include <glm/fwd.hpp>

namespace ECSModule
{
void ECSModule::startup()
{
    ZoneScopedN("ECSModule::startup");
    LT_LOGI("ECSModule", "Startup");

    auto *res = GCM(ResourceModule::ResourceManager);
    auto *scripts = GCM(ScriptModule::LuaScriptModule);
    auto *physics = GCM(PhysicsModule::PhysicsModule);

    m_worldManager = std::make_unique<WorldManager>(res, scripts, physics);

    createWorld("Default");
    openBasicWorld();

    // Register all available components
    registerComponents();

    // Subscribe to EditorUI events
    using namespace Events::EditorUI;
    m_binder.bind<SimulationStart>([this](const SimulationStart &) { simulate(true); });

    m_binder.bind<SimulationStop>([this](const SimulationStop &) { simulate(false); });

    m_binder.bind<EntityCreateRequest>([this](const EntityCreateRequest &evt) {
        if (!getCurrentWorld())
            return;

        auto &world = getCurrentWorld()->get();
        auto entity = world.entity(evt.entityName.c_str());

        if (evt.withDefaultComponents)
        {
            entity.set<PositionComponent>({0.0f, 0.0f, 0.0f});
            entity.set<RotationComponent>({0.0f, 0.0f, 0.0f});
            entity.set<ScaleComponent>({1.0f, 1.0f, 1.0f});
        }

        GCEB().emit(Events::ECS::EntityCreated{entity.id(), evt.entityName});
    });

    m_binder.bind<EntityDeleteRequest>([this](const EntityDeleteRequest &evt) {
        if (!getCurrentWorld())
            return;

        auto &world = getCurrentWorld()->get();
        auto entity = world.entity(evt.entityId);
        if (entity.is_valid())
        {
            entity.destruct();
            GCEB().emit(Events::ECS::EntityDestroyed{evt.entityId});
        }
    });

    m_binder.bind<ComponentAddRequest>([this](const ComponentAddRequest &evt) {
        if (!getCurrentWorld())
            return;

        auto &world = getCurrentWorld()->get();
        auto entity = world.entity(evt.entityId);
        if (!entity.is_valid())
            return;

        // Use ComponentRegistry to add component
        auto &registry = ComponentRegistry::getInstance();
        if (registry.addComponent(entity, evt.componentTypeName))
        {
            GCEB().emit(Events::ECS::ComponentChanged{evt.entityId, evt.componentTypeName});
        }
    });

    m_binder.bind<ComponentRemoveRequest>([this](const ComponentRemoveRequest &evt) {
        if (!getCurrentWorld())
            return;

        auto &world = getCurrentWorld()->get();
        auto entity = world.entity(evt.entityId);
        if (!entity.is_valid())
            return;

        // Use ComponentRegistry to remove component
        auto &registry = ComponentRegistry::getInstance();
        if (registry.removeComponent(entity, evt.componentTypeName))
        {
            GCEB().emit(Events::EditorUI::ComponentRemoved{evt.entityId, evt.componentTypeName});
            GCEB().emit(Events::ECS::ComponentChanged{evt.entityId, evt.componentTypeName});
        }
    });

    m_binder.bind<ComponentResetRequest>([this](const ComponentResetRequest &evt) {
        if (!getCurrentWorld())
            return;

        auto &world = getCurrentWorld()->get();
        auto entity = world.entity(evt.entityId);
        if (!entity.is_valid())
            return;

        // Use ComponentRegistry to reset component
        auto &registry = ComponentRegistry::getInstance();
        if (registry.resetComponent(entity, evt.componentTypeName))
        {
            GCEB().emit(Events::ECS::ComponentChanged{evt.entityId, evt.componentTypeName});
        }
    });
}

void ECSModule::shutdown()
{
    ZoneScopedN("ECSModule::shutdown");
    LT_LOGI("ECSModule", "Shutdown");
    m_worldManager->clear();
    m_worldManager.reset();
}

EntityWorld *ECSModule::getCurrentWorld()
{
    ZoneScopedN("ECSModule::getCurrentWorld");
    if (!m_worldManager)
        return nullptr;
    return m_worldManager->getActiveWorld();
}

EntityWorld &ECSModule::createWorld(const std::string &name)
{
    ZoneScopedN("ECSModule::createWorld");
    auto &world = m_worldManager->createWorld(name);
    GCEB().emit(Events::ECS::WorldOpened{name});
    return world;
}

void ECSModule::closeWorld(const std::string &name)
{
    ZoneScopedN("ECSModule::closeWorld");
    m_worldManager->destroyWorld(name);
    GCEB().emit(Events::ECS::WorldClosed{name});
}

void ECSModule::openBasicWorld()
{
    ZoneScopedN("ECSModule::openBasicWorld");
    auto &world = createWorld("EditorWorld");
    // world.deserialize(Fs::readTextFile("../Resources/World/Templates/basic_world.lworld"));
    m_worldManager->setActiveWorld("EditorWorld");
}

void ECSModule::openWorldByResource(const std::string &guid)
{
    ZoneScopedN("ECSModule::openWorldByResource");
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
    ZoneScopedN("ECSModule::ecsTick");
    if (m_inSimulate)
        m_worldManager->tickActive(dt);

    emitRenderFrameData();
}

void ECSModule::simulate(bool state)
{
    ZoneScopedN("ECSModule::simulate");
    auto* world = getCurrentWorld();
    if (!world)
        return;

    if (state && !m_inSimulate)
    {
        // Save current world state before starting simulation
        m_simulationSnapshot = world->serialize();
        m_inSimulate = true;
        GCEB().emit(Events::ECS::WorldOpened{"Simulation"});
    }
    else if (!state && m_inSimulate)
    {
        m_inSimulate = false;
        
        // Clear all physics bodies before restoring state
        auto* physicsCtx = PhysicsModule::PhysicsLocator::TryGet();
        if (physicsCtx)
        {
            auto& flecsWorld = world->get();
            // Collect all entities with RigidBodyComponent to avoid iterator invalidation
            std::vector<flecs::entity> entitiesToClear;
            flecsWorld.each<PhysicsModule::RigidBodyComponent>([&entitiesToClear](flecs::entity e, PhysicsModule::RigidBodyComponent&) {
                entitiesToClear.push_back(e);
            });
            
            // Destroy physics bodies for all entities
            for (auto& entity : entitiesToClear)
            {
                if (entity.is_valid())
                {
                    physicsCtx->destroyBodyForEntity(entity);
                }
            }
        }
        
        // Restore world state from snapshot
        if (!m_simulationSnapshot.empty())
        {
            world->deserialize(m_simulationSnapshot);
            
            // Reset RigidBodyComponent flags to ensure physics bodies are recreated
            auto& flecsWorld = world->get();
            using namespace PhysicsModule;
            flecsWorld.each<RigidBodyComponent>([&](flecs::entity e, RigidBodyComponent& rb) {
                rb.bodyHandle = InvalidBodyHandle;
                rb.needsCreation = true;
            });
            
            // Also reset ColliderComponent flags
            flecsWorld.each<ColliderComponent>([&](flecs::entity e, ColliderComponent& collider) {
                collider.needsCreation = true;
            });
            
            m_simulationSnapshot.clear();
        }
        
        // Emit WorldClosed event to notify renderer to clear render list
        // This happens BEFORE the next frame, so renderer will rebuild on next frame
        GCEB().emit(Events::ECS::WorldClosed{"Simulation"});
        
        // Emit WorldOpened event to notify renderer to rebuild render list
        // This ensures renderer knows the world is ready after deserialization
        GCEB().emit(Events::ECS::WorldOpened{"EditorWorld"});
    }
}

void ECSModule::emitRenderFrameData()
{
    ZoneScopedN("ECSModule::emitRenderFrameData");

    auto *worldPtr = getCurrentWorld();
    if (!worldPtr)
        return;

    auto &world = worldPtr->get();
    Events::ECS::RenderFrameData frameData;

    auto qCam = world.query<PositionComponent, RotationComponent, CameraComponent>();
    qCam.each(
        [&](flecs::entity, const PositionComponent &pos, const RotationComponent &rot, const CameraComponent &cam) {
            frameData.camera.posX = pos.x;
            frameData.camera.posY = pos.y;
            frameData.camera.posZ = pos.z;
            frameData.camera.rotX = rot.x;
            frameData.camera.rotY = rot.y;
            frameData.camera.rotZ = rot.z;
            const glm::quat quat = rot.toQuat();
            frameData.camera.rotQX = quat.x;
            frameData.camera.rotQY = quat.y;
            frameData.camera.rotQZ = quat.z;
            frameData.camera.rotQW = quat.w;
            frameData.camera.fov = cam.fov;
            frameData.camera.aspect = cam.aspect;
            frameData.camera.nearClip = cam.nearClip;
            frameData.camera.farClip = cam.farClip;
        });

    auto qMesh = world.query<PositionComponent, RotationComponent, ScaleComponent, MeshComponent>();
    qMesh.each([&](flecs::entity e, const PositionComponent &pos, const RotationComponent &rot,
                   const ScaleComponent &scale, const MeshComponent &) {
        if (!e.is_valid())
        {
            return;
        }

        Events::ECS::ObjectTransformData transform;
        transform.entityId = e.id();
        transform.posX = pos.x;
        transform.posY = pos.y;
        transform.posZ = pos.z;
        transform.rotX = rot.x;
        transform.rotY = rot.y;
        transform.rotZ = rot.z;

        const glm::quat quat = rot.toQuat();
        transform.rotQX = quat.x;
        transform.rotQY = quat.y;
        transform.rotQZ = quat.z;
        transform.rotQW = quat.w;
        transform.scaleX = scale.x;
        transform.scaleY = scale.y;
        transform.scaleZ = scale.z;
        frameData.objectsTransforms.push_back(transform);
    });

    GCEB().emit(frameData);
}

void ECSModule::registerComponents()
{
    ZoneScopedN("ECSModule::registerComponents");
    auto &registry = ComponentRegistry::getInstance();
    using Factory = ComponentFactory;

    // Register PositionComponent
    registry.registerComponent(std::make_unique<Factory>(
        "PositionComponent", "Position Component",
        [](flecs::entity &e) { e.set<PositionComponent>({0.0f, 0.0f, 0.0f}); },
        [](flecs::entity &e) { e.remove<PositionComponent>(); },
        [](flecs::entity &e) { return e.has<PositionComponent>(); }));

    // Register RotationComponent
    registry.registerComponent(std::make_unique<Factory>(
        "RotationComponent", "Rotation Component",
        [](flecs::entity &e) { e.set<RotationComponent>({0.0f, 0.0f, 0.0f}); },
        [](flecs::entity &e) { e.remove<RotationComponent>(); },
        [](flecs::entity &e) { return e.has<RotationComponent>(); }));

    // Register ScaleComponent
    registry.registerComponent(std::make_unique<Factory>(
        "ScaleComponent", "Scale Component", [](flecs::entity &e) { e.set<ScaleComponent>({1.0f, 1.0f, 1.0f}); },
        [](flecs::entity &e) { e.remove<ScaleComponent>(); },
        [](flecs::entity &e) { return e.has<ScaleComponent>(); }));

    // Register MeshComponent
    registry.registerComponent(std::make_unique<Factory>(
        "MeshComponent", "Mesh Component",
        [](flecs::entity &e) {
            e.set<MeshComponent>({ResourceModule::AssetID(), ResourceModule::AssetID(), ResourceModule::AssetID(),
                                  ResourceModule::AssetID()});
        },
        [](flecs::entity &e) { e.remove<MeshComponent>(); }, [](flecs::entity &e) { return e.has<MeshComponent>(); }));

    // Register MaterialComponent
    registry.registerComponent(std::make_unique<Factory>(
        "MaterialComponent", "Material Component",
        [](flecs::entity &e) { e.set<MaterialComponent>({ResourceModule::AssetID()}); },
        [](flecs::entity &e) { e.remove<MaterialComponent>(); },
        [](flecs::entity &e) { return e.has<MaterialComponent>(); }));

    // Register CameraComponent
    registry.registerComponent(std::make_unique<Factory>(
        "CameraComponent", "Camera Component",
        [](flecs::entity &e) { e.set<CameraComponent>({75.0f, 16.0f / 9.0f, 0.1f, 10000.0f, false}); },
        [](flecs::entity &e) { e.remove<CameraComponent>(); },
        [](flecs::entity &e) { return e.has<CameraComponent>(); }));

    // Register ScriptComponent
    registry.registerComponent(std::make_unique<Factory>(
        "ScriptComponent", "Script Component", [](flecs::entity &e) { e.set<ScriptComponent>({""}); },
        [](flecs::entity &e) { e.remove<ScriptComponent>(); },
        [](flecs::entity &e) { return e.has<ScriptComponent>(); }));

    // Register DirectionalLightComponent
    registry.registerComponent(std::make_unique<Factory>(
        "DirectionalLightComponent", "Directional Light Component",
        [](flecs::entity &e) { e.set<DirectionalLightComponent>({1.0f}); },
        [](flecs::entity &e) { e.remove<DirectionalLightComponent>(); },
        [](flecs::entity &e) { return e.has<DirectionalLightComponent>(); }));

    // Register PointLightComponent
    registry.registerComponent(std::make_unique<Factory>(
        "PointLightComponent", "Point Light Component",
        [](flecs::entity &e) {
            PointLightComponent light{};
            light.innerRadius = 0.0f;
            light.outerRadius = 10.0f;
            light.intencity = 1.0f;
            light.color = glm::vec3(1.0f, 1.0f, 1.0f);
            e.set<PointLightComponent>(light);
        },
        [](flecs::entity &e) { e.remove<PointLightComponent>(); },
        [](flecs::entity &e) { return e.has<PointLightComponent>(); }));

    // Register RigidbodyComponent (legacy)
    registry.registerComponent(std::make_unique<Factory>(
        "RigidbodyComponent", "Rigidbody Component", [](flecs::entity &e) { e.add<RigidbodyComponent>(); },
        [](flecs::entity &e) { e.remove<RigidbodyComponent>(); },
        [](flecs::entity &e) { return e.has<RigidbodyComponent>(); }));

    // Register PhysicsModule components
    using namespace PhysicsModule;
    
    // Register RigidBodyComponent
    registry.registerComponent(std::make_unique<Factory>(
        "RigidBodyComponent", "Rigid Body Component",
        [](flecs::entity &e) { e.set<RigidBodyComponent>({}); },
        [](flecs::entity &e) { e.remove<RigidBodyComponent>(); },
        [](flecs::entity &e) { return e.has<RigidBodyComponent>(); }));

    // Register ColliderComponent
    registry.registerComponent(std::make_unique<Factory>(
        "ColliderComponent", "Collider Component",
        [](flecs::entity &e) { e.set<ColliderComponent>({}); },
        [](flecs::entity &e) { e.remove<ColliderComponent>(); },
        [](flecs::entity &e) { return e.has<ColliderComponent>(); }));

    // Register CharacterControllerComponent
    registry.registerComponent(std::make_unique<Factory>(
        "CharacterControllerComponent", "Character Controller Component",
        [](flecs::entity &e) { e.set<CharacterControllerComponent>({}); },
        [](flecs::entity &e) { e.remove<CharacterControllerComponent>(); },
        [](flecs::entity &e) { return e.has<CharacterControllerComponent>(); }));

    // Register PhysicsMaterialComponent
    registry.registerComponent(std::make_unique<Factory>(
        "PhysicsMaterialComponent", "Physics Material Component",
        [](flecs::entity &e) { e.set<PhysicsMaterialComponent>({}); },
        [](flecs::entity &e) { e.remove<PhysicsMaterialComponent>(); },
        [](flecs::entity &e) { return e.has<PhysicsMaterialComponent>(); }));
}
} // namespace ECSModule
