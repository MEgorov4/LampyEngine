#pragma once
#include "../OpenGL/OpenGLObjects/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"
#include "../OpenGL/OpenGLObjects/OpenGLTexture.h"
#include "../OpengL/OpenGLObjects/OpenGLFramebuffer.h"
#include "../RenderConfig.h"
#include "IFramebuffer.h"
#include "IMesh.h"
#include "IShader.h"
#include "ITexture.h"

#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/ResourceManager.h>

namespace RenderModule
{
class TextureFactory
{
    static std::unordered_map<std::string, std::weak_ptr<ITexture>> textureCache;

  public:
    static std::shared_ptr<ITexture> createOrGetTexture(const ResourceModule::AssetID& textureID)
    {
        if (!textureCache[textureID.str()].expired())
            return textureCache[textureID.str()].lock();

        if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
        {
            auto glTexture = std::make_shared<OpenGL::OpenGLTexture>(
                GCM(ResourceModule::ResourceManager)->load<ResourceModule::RTexture>(textureID));
            textureCache[textureID.str()] = glTexture;
            return glTexture;
        }

        return nullptr;
    }
};

class MeshFactory
{
    static std::unordered_map<std::string, std::weak_ptr<IMesh>> meshCache;

  public:
    static std::shared_ptr<IMesh> createOrGetMesh(const ResourceModule::AssetID& id)
    {
        if (!meshCache[id.str()].expired())
            return meshCache[id.str()].lock();

        if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
        {
            auto glMesh = std::make_shared<OpenGL::OpenGLMesh>(
                GCM(ResourceModule::ResourceManager)->load<ResourceModule::RMesh>(id));
            meshCache[id.str()] = glMesh;
            return glMesh;
        }

        return nullptr;
    }
};

class ShaderFactory
{
    static std::unordered_map<std::string, std::weak_ptr<IShader>> shaderCache;

  public:
    static std::shared_ptr<IShader> createOrGetShader(const ResourceModule::AssetID& vShaderID,
                                                      const ResourceModule::AssetID& fShaderID)
    {
        std::string hash = vShaderID.str() + fShaderID.str();
        if (!shaderCache[hash].expired())
            return shaderCache[hash].lock();

        if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
        {
            auto vertShader = GCM(ResourceModule::ResourceManager)->load<ResourceModule::RShader>(vShaderID);
            auto fragShader = GCM(ResourceModule::ResourceManager)->load<ResourceModule::RShader>(fShaderID);

            auto glShader     = std::make_shared<OpenGL::OpenGLShader>(vertShader, fragShader);
            shaderCache[hash] = glShader;
            return glShader;
        }
        return nullptr;
    }
};

class FramebufferFactory
{
    static std::unordered_map<std::string, std::weak_ptr<IFramebuffer>> framebufferCache;

  public:
    static std::shared_ptr<IFramebuffer> createOrGetFramebuffer(const FramebufferData& data)
    {
        std::string hash = data.name;
        if (!framebufferCache[hash].expired())
            return framebufferCache[hash].lock();

        if (RC.getGraphicsAPI() == GraphicsAPI::OpenGL)
        {
            auto glFramebuffer     = std::make_shared<OpenGL::OpenGLFramebuffer>(data);
            framebufferCache[hash] = glFramebuffer;
            return glFramebuffer;
        }
        return nullptr;
    }
};

} // namespace RenderModule
