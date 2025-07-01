#include "BulletDebugDrawer.h"

#include "../RenderModule/RenderModule.h"
#include <glm/glm.hpp>
#include "../LoggerModule/Logger.h"
namespace PhysicsModule
{
    void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        /*RenderModule::getInstance().getRenderer()->drawLine(glm::vec3(from.x(), from.y(), from.z()),
            glm::vec3(to.x(), to.y(), to.z()),
            glm::vec3(color.x(), color.y(), color.z()));*/
    }

    void BulletDebugDrawer::reportErrorWarning(const char* warningString)
    {
        /*
        LOG_INFO(std::format("BULLET DEBUG DRAWER ERROR: {}", warningString));
    */
    }
}