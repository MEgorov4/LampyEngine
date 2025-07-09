#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "../../../ResourceModule/Shader.h"
#include "../../Abstract/IShader.h"

namespace ResourceModule
{
    class RShader;
}

namespace RenderModule::OpenGL
{
    class OpenGLShader : public IShader
    {
        unsigned int m_programID;

        unsigned int m_cameraUBO;
        unsigned int m_modelUBO;
        unsigned int m_directionalLightUBO;
        unsigned int m_pointLightUBO;

        std::unordered_map<std::string, unsigned int> m_uniformBlocks;
        std::unordered_map<std::string, unsigned int> m_ubos;

        std::unordered_map<int, TextureBinding> m_textureBindings;

    public:
        OpenGLShader(const std::shared_ptr<ResourceModule::RShader>& vertShader,
                     const std::shared_ptr<ResourceModule::RShader>& fragShader);
        ~OpenGLShader();

        unsigned int getProgramID() const { return m_programID; }
        void use() override;

        void setUniformBlock(const ShaderUniformBlock& data) override;

        void setUniformData(const std::string& blockName, const void* data, size_t dataSize) override;
        bool hasUniformBlock(const std::string& blockName) override;

        void bindTextures(const std::unordered_map<std::string, TextureHandle>& textures) override;

        unsigned int getOrCreateUBO(const std::string& blockName, size_t dataSize);

        void debugPrintUBO(const std::string& blockName, size_t dataSize);

        void scanTextureBindings(const std::unordered_map<std::string, int>& bindingMap) override;
    private:
        unsigned int createShaderFromSPIRV(const std::vector<uint8_t> spirvCode, unsigned int shaderType);
        unsigned int createShaderFromGLSL(const std::string& source, unsigned int shaderType);

        void unbind() override;

        void setUniformCameraData(const ShaderUniformBlock_CameraData& data) override
        {
        };

        void setUniformModelsData(const std::vector<ShaderUniformBlock_ModelData>& data) override
        {
        };

        void setUniformDirectionalLightData(const ShaderUniformBlock_DirectionalLightData& data) override
        {
        };

        void setUniformPointLightsData(const std::vector<ShaderUniformBlock_PointLight>& data) override
        {
        };

        void scanUniformBlocks();
    };
}
