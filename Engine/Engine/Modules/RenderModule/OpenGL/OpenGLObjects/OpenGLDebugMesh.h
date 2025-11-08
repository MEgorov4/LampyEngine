#pragma once
#include "../../Abstract/IMesh.h"
#include <glm/glm.hpp>

namespace RenderModule::OpenGL
{
    /// Простой меш для отладочных примитивов (линии, боксы, сферы)
    class OpenGLDebugMesh : public IMesh
    {
    public:
        OpenGLDebugMesh();
        virtual ~OpenGLDebugMesh();

        // Для линий
        void setLineData(const glm::vec3& from, const glm::vec3& to);
        
        // Для бокса (wireframe)
        void setBoxData(const glm::vec3& center, const glm::vec3& size);
        
        // Для сферы (wireframe, упрощенная)
        void setSphereData(const glm::vec3& center, float radius, int segments = 16);
        
        // Для инициализации mesh из массива вершин (для сетки)
        void setVerticesData(const float* vertices, size_t vertexCount);

        void bind() const override;
        void draw() const override;
        void drawIndexed(int instanceCount) const override;
        void unbind() const override;

        void drawLines() const;
        void drawWireframe() const;

    private:
        unsigned int m_VAO = 0;
        unsigned int m_VBO = 0;
        unsigned int m_IBO = 0;
        int m_vertexCount = 0;
        int m_indexCount = 0;
        bool m_hasIndices = false;

        void createBuffers();
        void updateBuffers(const float* vertices, size_t vertexCount, const uint32_t* indices = nullptr, size_t indexCount = 0);
    };
}
