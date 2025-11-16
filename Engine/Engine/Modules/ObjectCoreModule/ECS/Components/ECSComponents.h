#pragma once 

#include <optional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <flecs.h>

#include "../../../ResourceModule/Mesh.h"
#include "../../../ResourceModule/Shader.h"
#include "../../../ResourceModule/Texture.h"
#include <Modules/ResourceModule/Asset/AssetID.h>

struct PositionComponent {
	float x = 0.0f, y = 0.0f, z = 0.0f;

	glm::vec3 toGLMVec() const { return glm::vec3(x, y, z); }

	void fromGLMVec(const glm::vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
};

struct RotationComponent {
    float x = 0.0f; 
    float y = 0.0f; 
    float z = 0.0f; 

    float qx = 0.f;
    float qy = 0.f;
    float qz = 0.f;
    float qw = 1.f;

    glm::quat toQuat() const noexcept {
        if (qx==0 && qy==0 && qz==0 && qw==1) {
		glm::quat qy_ = glm::angleAxis(glm::radians(y), glm::vec3(0.f, 1.f, 0.f));
		glm::quat qx_ = glm::angleAxis(glm::radians(x), glm::vec3(1.f, 0.f, 0.f));
		glm::quat qz_ = glm::angleAxis(glm::radians(z), glm::vec3(0.f, 0.f, 1.f));
            return glm::normalize(qy_ * qx_ * qz_);
        }
        return glm::normalize(glm::quat(qw, qx, qy, qz));
    }

    void fromQuat(const glm::quat& q) noexcept {
        glm::quat norm = glm::normalize(q);
        qx = norm.x; qy = norm.y; qz = norm.z; qw = norm.w;

		auto euler = glm::eulerAngles(norm);
		x = glm::degrees(euler.x);
		y = glm::degrees(euler.y);
		z = glm::degrees(euler.z);
    }

    glm::vec3 toEulerDegrees() const noexcept {
        glm::vec3 euler = glm::eulerAngles(toQuat());
        return glm::degrees(euler);
    }

    void fromEulerDegrees(const glm::vec3& eulerDeg) noexcept {
        glm::vec3 rad = glm::radians(eulerDeg);
        glm::quat q(rad);
        fromQuat(q);
    }
};
struct ScaleComponent {
	float x = 1.0f, y = 1.0f, z = 1.0f;

	glm::vec3 toGLMVec() const {return glm::vec3(x, y , z);}

	void fromGMLVec(const glm::vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
};

struct TransformComponent {
    PositionComponent position{};
    RotationComponent rotation{};
    ScaleComponent scale{};

    TransformComponent() = default;
    TransformComponent(const PositionComponent& pos,
                       const RotationComponent& rot = {},
                       const ScaleComponent& scl = {})
        : position(pos), rotation(rot), scale(scl) {}

    glm::mat4 toMatrix() const noexcept {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position.toGLMVec());
        glm::mat4 rotationMat = glm::mat4_cast(rotation.toQuat());
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale.toGLMVec());
        return translation * rotationMat * scaleMat;
    }

    glm::mat4 toMatrixNoScale() const noexcept {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position.toGLMVec());
        glm::mat4 rotationMat = glm::mat4_cast(rotation.toQuat());
        return translation * rotationMat;
    }

    static TransformComponent FromTRS(const glm::vec3& pos,
                                      const glm::quat& rot,
                                      const glm::vec3& scl = glm::vec3(1.0f)) noexcept {
        TransformComponent t{};
        t.position.fromGLMVec(pos);
        t.rotation.fromQuat(rot);
        t.scale.fromGMLVec(scl);
        return t;
    }
};

inline TransformComponent& EnsureTransformComponent(flecs::entity& entity) {
    if (!entity.has<TransformComponent>()) {
        entity.set<TransformComponent>(TransformComponent{});
    }
    return *entity.get_mut<TransformComponent>();
}

inline void SetEntityPosition(flecs::entity& entity, const PositionComponent& pos) {
    auto& transform = EnsureTransformComponent(entity);
    transform.position = pos;
    entity.modified<TransformComponent>();
}

inline const PositionComponent* GetEntityPosition(const flecs::entity& entity) {
    if (const auto* transform = entity.get<TransformComponent>()) {
        return &transform->position;
    }
    return nullptr;
}

inline void SetEntityRotation(flecs::entity& entity, const RotationComponent& rot) {
    auto& transform = EnsureTransformComponent(entity);
    transform.rotation = rot;
    entity.modified<TransformComponent>();
}

inline const RotationComponent* GetEntityRotation(const flecs::entity& entity) {
    if (const auto* transform = entity.get<TransformComponent>()) {
        return &transform->rotation;
    }
    return nullptr;
}

inline void SetEntityScale(flecs::entity& entity, const ScaleComponent& scale) {
    auto& transform = EnsureTransformComponent(entity);
    transform.scale = scale;
    entity.modified<TransformComponent>();
}

inline const ScaleComponent* GetEntityScale(const flecs::entity& entity) {
    if (const auto* transform = entity.get<TransformComponent>()) {
        return &transform->scale;
    }
    return nullptr;
}

struct CameraComponent
{
	float fov = 75.f;
	float aspect = 1.65f;
	float nearClip = 0.01f;
	float farClip = 10000.f;
	bool isViewportCamera = true;
};

struct MeshComponent
{
    ResourceModule::AssetID meshID;
    ResourceModule::AssetID textureID;
    ResourceModule::AssetID vertShaderID;
    ResourceModule::AssetID fragShaderID;
};

struct PointLightComponent
{
	float innerRadius = 0.0f;  
	float outerRadius = 10.0f; 
	float intencity = 1.0f;    
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); 
};

struct DirectionalLightComponent
{
	float intencity;
};

struct MaterialComponent
{
	ResourceModule::AssetID materialID;
};

// Tags
struct EditorOnlyTag {};
struct InvisibleTag{};
