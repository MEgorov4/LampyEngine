#include "OpenGLDebugMesh.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>
#include <cstdint>

namespace RenderModule::OpenGL
{
    OpenGLDebugMesh::OpenGLDebugMesh() : IMesh(nullptr)
    {
        createBuffers();
    }

    OpenGLDebugMesh::~OpenGLDebugMesh()
    {
        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO)
            glDeleteBuffers(1, &m_VBO);
        if (m_IBO)
            glDeleteBuffers(1, &m_IBO);
    }

    void OpenGLDebugMesh::createBuffers()
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void OpenGLDebugMesh::updateBuffers(const float* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
    {
        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

        m_vertexCount = static_cast<int>(vertexCount);
        m_hasIndices = (indices != nullptr && indexCount > 0);

        if (m_hasIndices)
        {
            if (!m_IBO)
                glGenBuffers(1, &m_IBO);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32_t), indices, GL_DYNAMIC_DRAW);
            m_indexCount = static_cast<int>(indexCount);
        }
        else
        {
            m_indexCount = 0;
        }

        glBindVertexArray(0);
    }

    void OpenGLDebugMesh::setLineData(const glm::vec3& from, const glm::vec3& to)
    {
        float vertices[] = {
            from.x, from.y, from.z,
            to.x, to.y, to.z
        };
        updateBuffers(vertices, 2);
    }

    void OpenGLDebugMesh::setBoxData(const glm::vec3& center, const glm::vec3& size)
    {
        glm::vec3 halfSize = size * 0.5f;
        glm::vec3 min = center - halfSize;
        glm::vec3 max = center + halfSize;

        float vertices[] = {
            min.x, min.y, min.z, // 0
            max.x, min.y, min.z, // 1
            max.x, max.y, min.z, // 2
            min.x, max.y, min.z, // 3
            min.x, min.y, max.z, // 4
            max.x, min.y, max.z, // 5
            max.x, max.y, max.z, // 6
            min.x, max.y, max.z  // 7
        };

        uint32_t indices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        updateBuffers(vertices, 8, indices, 24);
    }

    void OpenGLDebugMesh::setSphereData(const glm::vec3& center, float radius, int segments)
    {
        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        for (int i = 0; i <= segments; ++i)
        {
            float theta = static_cast<float>(i) * 3.14159265f / static_cast<float>(segments);
            for (int j = 0; j <= segments; ++j)
            {
                float phi = static_cast<float>(j) * 2.0f * 3.14159265f / static_cast<float>(segments);
                float x = center.x + radius * std::sin(theta) * std::cos(phi);
                float y = center.y + radius * std::cos(theta);
                float z = center.z + radius * std::sin(theta) * std::sin(phi);
                
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        for (int i = 0; i <= segments; ++i)
        {
            for (int j = 0; j < segments; ++j)
            {
                uint32_t curr = static_cast<uint32_t>(i * (segments + 1) + j);
                uint32_t next = static_cast<uint32_t>(curr + 1);
                indices.push_back(curr);
                indices.push_back(next);
            }
        }

        for (int i = 0; i < segments; ++i)
        {
            for (int j = 0; j <= segments; ++j)
            {
                uint32_t curr = static_cast<uint32_t>(i * (segments + 1) + j);
                uint32_t next = static_cast<uint32_t>((i + 1) * (segments + 1) + j);
                indices.push_back(curr);
                indices.push_back(next);
            }
        }

        if (!vertices.empty())
            updateBuffers(vertices.data(), vertices.size() / 3, indices.data(), indices.size());
    }

    void OpenGLDebugMesh::bind() const
    {
        glBindVertexArray(m_VAO);
    }

    void OpenGLDebugMesh::unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLDebugMesh::draw() const
    {
        bind();
        if (m_isTriangleStrip)
        {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);
        }
        else if (m_hasIndices)
        {
            glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        }
        else
        {
            glDrawArrays(GL_LINES, 0, m_vertexCount);
        }
        unbind();
    }

    void OpenGLDebugMesh::drawIndexed(int instanceCount) const
    {
        bind();
        if (m_hasIndices)
        {
            if (instanceCount > 0)
                glDrawElementsInstanced(GL_LINES, m_indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
            else
                glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        }
        else
        {
            if (instanceCount > 0)
                glDrawArraysInstanced(GL_LINES, 0, m_vertexCount, instanceCount);
            else
                glDrawArrays(GL_LINES, 0, m_vertexCount);
        }
        unbind();
    }

    void OpenGLDebugMesh::drawLines() const
    {
        draw();
    }

    void OpenGLDebugMesh::drawWireframe() const
    {
        draw();
    }

    void OpenGLDebugMesh::setVerticesData(const float* vertices, size_t vertexCount)
    {
        m_isTriangleStrip = false;
        updateBuffers(vertices, vertexCount, nullptr, 0);
    }
    
    void OpenGLDebugMesh::setQuadData()
    {
        float vertices[] = {
            -1.0f, 0.0f, -1.0f,  // 0
            -1.0f, 0.0f,  1.0f,  // 1
             1.0f, 0.0f, -1.0f,  // 2
             1.0f, 0.0f,  1.0f   // 3
        };
        m_isTriangleStrip = true;
        updateBuffers(vertices, 4, nullptr, 0);
    }
    
    void OpenGLDebugMesh::drawTriangleStrip() const
    {
        bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);
        unbind();
    }
}

