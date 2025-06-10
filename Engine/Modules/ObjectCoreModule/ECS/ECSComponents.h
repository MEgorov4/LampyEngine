#pragma once 

#include <optional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../ResourceModule/Mesh.h"
#include "../../ResourceModule/Shader.h"
#include "../../ResourceModule/Texture.h"

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

	glm::quat toQuat() const {
		return glm::quat(glm::vec3(glm::radians(x), glm::radians(y), glm::radians(z)));
	}

	glm::vec3 toEuler() const {
		return glm::vec3(x, y, z);
	}

	void fromQuat(const glm::quat& q) {
		glm::vec3 rot = glm::eulerAngles(q);
		x = rot.x;
		y = rot.y;
		z = rot.z;
	}

	void fromEuler(const glm::vec3& euler) {
		auto q = glm::quat(euler);
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
	char meshResourcePath[256] = "";
	char vertShaderPath[256] = "";
	char fragShaderPath[256] = "";
	char texturePath[256] = "";

	std::optional<std::shared_ptr<RMesh>> meshResource;

	std::optional<std::shared_ptr<RShader>> vertShaderResource;
	std::optional<std::shared_ptr<RShader>> fragShaderResource;

	std::optional <std::shared_ptr<RTexture>> textureResource;
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
