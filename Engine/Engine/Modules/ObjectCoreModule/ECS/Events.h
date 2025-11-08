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

/// Данные трансформации объекта для рендеринга
struct ObjectTransformData
{
    uint64_t entityId;
    float posX, posY, posZ;
    float rotX, rotY, rotZ, rotQX, rotQY, rotQZ, rotQW;
    float scaleX, scaleY, scaleZ;
};

/// Данные камеры для рендеринга
struct CameraRenderData
{
    float posX, posY, posZ;
    float rotX, rotY, rotZ, rotQX, rotQY, rotQZ, rotQW;
    float fov;
    float aspect;
    float nearClip;
    float farClip;
};

/// Событие обновления данных для рендеринга кадра
/// Эмитится каждый кадр ECSModule с данными для рендеринга
struct RenderFrameData
{
    std::vector<ObjectTransformData> objectsTransforms;
    CameraRenderData camera;
};
} // namespace Events::ECS
