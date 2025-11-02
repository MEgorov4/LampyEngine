// OpenGLRenderFactory.h
#pragma once
#include "../Abstract/IRenderFactory.h"
#include "../OpenGL/OpenGLObjects/OpenGLFramebuffer.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh.h"
#include "../OpenGL/OpenGLObjects/OpenGLMesh2D.h"
#include "../OpenGL/OpenGLObjects/OpenGLShader.h"
#include "../OpenGL/OpenGLObjects/OpenGLTexture.h"
#include "../RenderConfig.h"

#include <mutex>
#include <unordered_map>

namespace RenderModule::OpenGL
{
class OpenGLRenderFactory final : public IRenderFactory
{
    std::unordered_map<std::string, std::shared_ptr<ITexture>> m_textureCache;
    std::unordered_map<std::string, std::shared_ptr<IMesh>> m_meshCache;
    std::unordered_map<std::string, std::shared_ptr<IShader>> m_shaderCache;
    std::unordered_map<std::string, std::shared_ptr<IFramebuffer>> m_framebufferCache;

    std::shared_ptr<IMesh> m_mesh2DCache;

    std::mutex m_mutex;

  public:
    std::shared_ptr<ITexture> createTexture(const ResourceModule::AssetID &id) override
    {
        std::scoped_lock lock(m_mutex);
        const std::string key = id.str();

        if (auto it = m_textureCache.find(key); it != m_textureCache.end())
            return it->second;

        auto *resourceMgr = GCM(ResourceModule::ResourceManager);
        LT_ASSERT_MSG(resourceMgr, "ResourceManager is null");

        auto rtex = resourceMgr->load<ResourceModule::RTexture>(id);
        LT_ASSERT_MSG(rtex, "Failed to load texture resource");

        auto tex = std::make_shared<OpenGLTexture>(rtex);
        m_textureCache[key] = tex;
        return tex;
    }

    std::shared_ptr<IMesh> createMesh(const ResourceModule::AssetID &id) override
    {
        std::scoped_lock lock(m_mutex);
        const std::string key = id.str();

        if (auto it = m_meshCache.find(key); it != m_meshCache.end())
            return it->second;

        auto *resourceMgr = GCM(ResourceModule::ResourceManager);
        LT_ASSERT_MSG(resourceMgr, "ResourceManager is null");

        auto rmesh = resourceMgr->load<ResourceModule::RMesh>(id);
        LT_ASSERT_MSG(rmesh, "Failed to load mesh resource");

        auto mesh = std::make_shared<OpenGLMesh>(rmesh);
        m_meshCache[key] = mesh;
        return mesh;
    }

    std::shared_ptr<IMesh> createMesh2D() override
    {
        std::scoped_lock lock(m_mutex);
        if (m_mesh2DCache)
            return m_mesh2DCache;

        m_mesh2DCache = std::make_shared<OpenGLMesh2D>();
        return m_mesh2DCache;
    }

    std::shared_ptr<IShader> createShader(const ResourceModule::AssetID &vs, const ResourceModule::AssetID &fs) override
    {
        std::scoped_lock lock(m_mutex);
        const std::string key = vs.str() + fs.str();

        if (auto it = m_shaderCache.find(key); it != m_shaderCache.end())
            return it->second;

        auto *resourceMgr = GCM(ResourceModule::ResourceManager);
        LT_ASSERT_MSG(resourceMgr, "ResourceManager is null");

        auto v = resourceMgr->load<ResourceModule::RShader>(vs);
        LT_ASSERT_MSG(v, "Failed to load vertex shader resource");

        auto f = resourceMgr->load<ResourceModule::RShader>(fs);
        LT_ASSERT_MSG(f, "Failed to load fragment shader resource");

        auto shader = std::make_shared<OpenGLShader>(v, f);
        m_shaderCache[key] = shader;
        return shader;
    }

    std::shared_ptr<IFramebuffer> createFramebuffer(const FramebufferData &data) override
    {
        std::scoped_lock lock(m_mutex);
        LT_ASSERT_MSG(!data.name.empty(), "Framebuffer name cannot be empty");

        const std::string key = data.name;

        if (auto it = m_framebufferCache.find(key); it != m_framebufferCache.end())
            return it->second;

        auto fb = std::make_shared<OpenGLFramebuffer>(data);
        LT_ASSERT_MSG(fb, "Failed to create framebuffer");
        m_framebufferCache[key] = fb;
        return fb;
    }

    void clearCaches() override
    {
        std::scoped_lock lock(m_mutex);
        m_textureCache.clear();
        m_meshCache.clear();
        m_shaderCache.clear();
        m_framebufferCache.clear();
        m_mesh2DCache.reset();
    }
};
} // namespace RenderModule::OpenGL
