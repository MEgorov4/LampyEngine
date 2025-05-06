#pragma once

#include <btBulletDynamicsCommon.h>

class BulletDebugDrawer : public btIDebugDraw
{
public:
    int m_debugMode;

    BulletDebugDrawer() : m_debugMode(btIDebugDraw::DBG_DrawWireframe) {}

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

    void reportErrorWarning(const char* warningString) override;

    void draw3dText(const btVector3& location, const char* textString) override {}
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {}

    void setDebugMode(int debugMode) override { m_debugMode = debugMode; }
    int getDebugMode() const override { return m_debugMode; }
};