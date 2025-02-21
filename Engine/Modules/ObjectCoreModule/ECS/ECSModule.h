#pragma once
#include <flecs.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../../EventModule/Event.h"

struct Position {
	float x = 0.0f, y = 0.0f, z = 0.0f;

	glm::vec3 toGLMVec() const { return glm::vec3(x, y, z); }

	void fromGLMVec(const glm::vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
};

struct Rotation {
	float w = 1.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	glm::quat toQuat() const {
		return glm::quat(w, x, y, z);
	}

	glm::vec3 toEuler() const {
		return glm::eulerAngles(toQuat());
	}

	void fromQuat(const glm::quat& q) {
		w = q.w;
		x = q.x;
		y = q.y;
		z = q.z;
	}

	void fromEuler(const glm::vec3& euler) {
		auto q = glm::quat(euler);
		fromQuat(q);
	}
};

struct Scale {
	float x = 1.0f, y = 1.0f, z = 1.0f;

	glm::vec3 toGLMVec() const {return glm::vec3(x, y , z);}

	void fromGMLVec(const glm::vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
};

struct Camera
{
	float fov;
	float aspect;
	float nearClip;
	float farClip;
	bool isViewportCamera;
};

struct MeshComponent
{
	char meshResourcePath[256];
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

	Event<> OnLoadInitialWorldState;

private:
	void registerComponents();
};