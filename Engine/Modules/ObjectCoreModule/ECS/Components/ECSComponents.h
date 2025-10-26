#pragma once 

#include <optional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../../ResourceModule/Mesh.h"
#include "../../../ResourceModule/Shader.h"
#include "../../../ResourceModule/Texture.h"

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
            // если quat ещё пустой → собираем из старых Эйлеров
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
        glm::quat q(rad); // glm::quat от XYZ
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
	std::string meshResourcePath;
	std::string vertShaderPath;
	std::string fragShaderPath;
	std::string texturePath;

	std::optional<std::shared_ptr<ResourceModule::RMesh>> meshResource;

	std::optional<std::shared_ptr<ResourceModule::RShader>> vertShaderResource;
	std::optional<std::shared_ptr<ResourceModule::RShader>> fragShaderResource;

	std::optional <std::shared_ptr<ResourceModule::RTexture>> textureResource;
};

struct PointLightComponent
{
	float radius;
	float intencity;
};

struct DirectionalLightComponent
{
	float intencity;
};

// Tags
struct EditorOnlyTag {};
struct InvisibleTag{};
