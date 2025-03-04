#pragma once
#include <GL/glew.h>
#include "../../Abstract/IMesh.h"

class OpenGLMesh2D {
public:
    OpenGLMesh2D();
    ~OpenGLMesh2D();

    void bind() const;
    void draw() const;
    void unbind() const;

private:
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
};
