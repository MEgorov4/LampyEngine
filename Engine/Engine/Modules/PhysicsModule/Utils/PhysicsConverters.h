#pragma once

#include "PhysicsTypes.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <flecs.h>
#include <btBulletDynamicsCommon.h>

namespace PhysicsModule
{
    inline btVector3 ToBullet(const glm::vec3& v)
    {
        return btVector3(v.x, v.y, v.z);
    }

    inline glm::vec3 FromBullet(const btVector3& v)
    {
        return glm::vec3(v.x(), v.y(), v.z());
    }

    inline btQuaternion ToBullet(const glm::quat& q)
    {
        return btQuaternion(q.x, q.y, q.z, q.w);
    }

    inline glm::quat FromBullet(const btQuaternion& q)
    {
        return glm::quat(q.w(), q.x(), q.y(), q.z());
    }

    inline btTransform ToBullet(const glm::vec3& pos, const glm::quat& rot)
    {
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(ToBullet(pos));
        transform.setRotation(ToBullet(rot));
        return transform;
    }

    inline void FromBullet(const btTransform& transform, glm::vec3& pos, glm::quat& rot)
    {
        pos = FromBullet(transform.getOrigin());
        rot = FromBullet(transform.getRotation());
    }
}

