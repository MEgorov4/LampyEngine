#include "DebugDrawer.h"
#include "../../../Modules/RenderModule/RenderContext.h"
#include "../Utils/PhysicsConverters.h"

namespace PhysicsModule
{
    DebugDrawer::DebugDrawer(RenderModule::RenderContext* renderContext)
        : m_renderContext(renderContext)
    {
    }

    void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        if (!m_renderContext)
            return;

        RenderModule::DebugLine line;
        line.from = FromBullet(from);
        line.to = FromBullet(to);
        line.color = FromBullet(color);
        m_renderContext->addDebugLine(line);
    }

    void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
                                       btScalar distance, int lifeTime, const btVector3& color)
    {
        if (!m_renderContext)
            return;

        // Draw contact point as a small sphere
        RenderModule::DebugSphere sphere;
        sphere.center = FromBullet(PointOnB);
        sphere.radius = 0.05f;
        sphere.color = FromBullet(color);
        m_renderContext->addDebugSphere(sphere);
    }

    void DebugDrawer::reportErrorWarning(const char* warningString)
    {
        LT_LOGW("PhysicsModule", "Bullet Warning: " + std::string(warningString));
    }
}

