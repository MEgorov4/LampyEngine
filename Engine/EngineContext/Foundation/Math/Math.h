#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

namespace Math
{
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;

    using Mat3 = glm::mat3;
    using Mat4 = glm::mat4;

    using Quat = glm::quat;

    constexpr float PI      = glm::pi<float>();
    constexpr float HALF_PI = glm::half_pi<float>();
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;

    inline Vec3 Forward(const Quat& q) { return q * Vec3(0, 0, -1); }
    inline Vec3 Right  (const Quat& q) { return q * Vec3(1, 0, 0); }
    inline Vec3 Up     (const Quat& q) { return q * Vec3(0, 1, 0); }

    inline Mat4 TRS(const Vec3& pos, const Quat& rot, const Vec3& scale)
    {
        return glm::translate(Mat4(1.0f), pos)
             * glm::toMat4(rot)
             * glm::scale(Mat4(1.0f), scale);
    }

    inline Quat EulerToQuat(const Vec3& eulerDegrees)
    {
        return glm::quat(glm::radians(eulerDegrees));
    }

    inline Vec3 QuatToEuler(const Quat& q)
    {
        return glm::degrees(glm::eulerAngles(q));
    }

    inline float Clamp01(float v)
    {
        return glm::clamp(v, 0.0f, 1.0f);
    }

    inline float Lerp(float a, float b, float t)
    {
        return glm::mix(a, b, Clamp01(t));
    }

    inline Vec3 Lerp(const Vec3& a, const Vec3& b, float t)
    {
        return glm::mix(a, b, Clamp01(t));
    }

    inline Quat Slerp(const Quat& a, const Quat& b, float t)
    {
        return glm::slerp(a, b, Clamp01(t));
    }
}