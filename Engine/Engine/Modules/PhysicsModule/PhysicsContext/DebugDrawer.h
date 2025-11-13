#pragma once

#include <btBulletDynamicsCommon.h>
#include <memory>

namespace RenderModule
{
    class RenderContext;
}

namespace PhysicsModule
{
    class DebugDrawer : public btIDebugDraw
    {
    public:
        explicit DebugDrawer(RenderModule::RenderContext* renderContext);
        ~DebugDrawer() override = default;

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, 
                             btScalar distance, int lifeTime, const btVector3& color) override;
        void reportErrorWarning(const char* warningString) override;
        void draw3dText(const btVector3& location, const char* textString) override {}

        void setDebugMode(int debugMode) override { m_debugMode = debugMode; }
        int getDebugMode() const override { return m_debugMode; }

    private:
        RenderModule::RenderContext* m_renderContext;
        int m_debugMode = DBG_DrawWireframe;
    };
}

