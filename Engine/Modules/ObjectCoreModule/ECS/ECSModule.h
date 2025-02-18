#pragma once
#include <memory>
#include <flecs.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Position {
	float x = 0.0f, y = 0.0f, z = 0.0f;

	glm::vec3 toGLMVec() const{ return glm::vec3(x, y, z); }

	void fromGLMVec(const glm::vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
};

struct Rotation {
	glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); 

	Rotation() = default;

	Rotation(const glm::vec3& eulerAngles) : value(glm::quat(eulerAngles)) {}

	glm::vec3 toEulerAngles() const { return glm::eulerAngles(value); }
};

struct Scale {
	float x = 1.0f, y = 1.0f, z = 1.0f;

	Scale() = default;

	Scale(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}

	operator glm::vec3() const { return glm::vec3(x, y, z); }
};

struct Camera
{
	float fov;
	float aspect;
	float nearClip;
	float farClip;
	bool isViewportCamera;
};

class ECSModule
{
	bool m_tickEnabled = false;
	std::string m_currentWorldFile;
	flecs::world m_world;
public:
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startup();
	
	void loadInitialWorldState();

	void fillDefaultWorld();
	void loadWorldFromFile(const std::string& path);
	void setCurrentWorldPath(const std::string& path);
	void saveCurrentWorld();
	bool isWorldSetted();
	void clearWorld();

	void startSystems();
	void stopSystems();
	
	bool getTickEnabled() { return m_tickEnabled; }

	flecs::world& getCurrentWorld();

	void ecsTick(float deltaTime);

	void shutDown();
};