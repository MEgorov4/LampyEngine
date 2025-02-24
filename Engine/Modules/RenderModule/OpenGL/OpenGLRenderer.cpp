#include "OpenGLRenderer.h"

#include <GL/glew.h>
#include <imgui.h>

#include "../../LoggerModule/Logger.h"
#include "../../ShaderCompilerModule/ShaderCompiler.h"
#include "../../ImGuiModule/OpenGLBackends/imgui_impl_opengl3.h"
#include "../../ImGuiModule/GLFWBackends/imgui_impl_glfw.h"

#include "OpenGLObjects/OpenGLFramebuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "OpenGLObjects/OpenGLVertexBuffer.h"

OpenGLRenderer::OpenGLRenderer(Window* window) : m_window(window)
{
    init();
}

void OpenGLRenderer::init()
{
    if (!glewInit())
    {
        LOG_INFO("OpenGLRenderer: Failed to initialize GLEW");
    }

    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (strstr(extensions, "GL_ARB_gl_spirv") == NULL || strstr(extensions, "GL_ARB_spirv_extensions") == NULL) {
        std::cerr << "SPIR-V не поддерживается!" << std::endl;
        exit(1);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glfwSwapInterval(0);
    glViewport(0, 0, m_window->getExtent().width, m_window->getExtent().height);
    
    glClearColor(0, 0, 0, 1);

    m_offscreenFramebuffer = std::make_unique<OpenGLFramebuffer>(1920, 1080);
    
    std::string fragVShader = SH.compileShader("C:/Users/mikhail/Desktop/LampyEngine(Vulkan)/build/Engine/Resources/Shaders/GLSL/shader.frag");
    std::string vertVShader = SH.compileShader("C:/Users/mikhail/Desktop/LampyEngine(Vulkan)/build/Engine/Resources/Shaders/GLSL/shader.vert");
    
    //m_shader = std::make_unique<OpenGLShader>(vertVShader, fragVShader);

    initImGui();

    LOG_INFO("OpenGLRenderer: OpenGL initialized successfully");
}

void OpenGLRenderer::initImGui()
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
}



void OpenGLRenderer::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 1.0f);
    glViewport(0, 0, m_window->getExtent().width, m_window->getExtent().height);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window->getGLFWWindow());
    
    m_offscreenFramebuffer->bind();
    renderWorld();
    m_offscreenFramebuffer->unbind();
}

void OpenGLRenderer::renderWorld()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //m_shader->use();

    //glm::mat4 model = glm::mat4(1.0f);
    //model = glm::rotate(model, static_cast<float>(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));

    //glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    //glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //float aspectRatio = static_cast<float>(m_window->getExtent().width) / static_cast<float>(m_window->getExtent().height);
    //glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    //glm::mat4 viewProjection = projection * view;

    //glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    //m_shader->setUniformMat4f("model", model);
    //m_shader->setUniformMat4f("viewProjection", viewProjection);
    //m_shader->setUniformMat3f("normalMatrix", normalMatrix);

    // Рисуем все буферы
    for (auto& buffer : m_vertexBuffers)
    {
        buffer->bind();
        buffer->draw();
        buffer->unbind();
    }

    glDisable(GL_DEPTH_TEST);
}

void OpenGLRenderer::registerShader(const std::string& vertPath, const std::string& fragPath)
{

}

void OpenGLRenderer::removeShader(const std::string& vertPath, const std::string& fragPath)
{

}

void OpenGLRenderer::registerVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{
    m_vertexBuffers.push_back(std::make_unique<OpenGLVertexBuffer>(vertexData));
}

void OpenGLRenderer::removeVertexData(const std::vector<Vertex>& vertexData, const std::string& pathToFile)
{

}

void OpenGLRenderer::registerIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{

}

void OpenGLRenderer::removeIndexData(const std::vector<uint32_t>& indexData, const std::string& pathToFile)
{
}

void* OpenGLRenderer::getOffscreenImageDescriptor()
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(m_offscreenFramebuffer->getTexture()));
}

void OpenGLRenderer::waitIdle()
{
    glFinish();
}

