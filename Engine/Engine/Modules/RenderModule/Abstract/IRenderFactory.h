// IRenderFactory.h
#pragma once
#include "IFramebuffer.h"
#include "IMesh.h"
#include "IShader.h"
#include "ITexture.h"

#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Mesh.h>
#include <Modules/ResourceModule/Shader.h>
#include <Modules/ResourceModule/Texture.h>

namespace RenderModule
{
class IRenderFactory
{
  public:
    virtual ~IRenderFactory() = default;

    virtual std::shared_ptr<ITexture> createTexture(const ResourceModule::AssetID &id) = 0;
    virtual std::shared_ptr<IMesh> createMesh(const ResourceModule::AssetID &id) = 0;
    virtual std::shared_ptr<IMesh> createMesh2D() = 0;
    virtual std::shared_ptr<IShader> createShader(const ResourceModule::AssetID &vs,
                                                  const ResourceModule::AssetID &fs) = 0;
    virtual std::shared_ptr<IFramebuffer> createFramebuffer(const FramebufferData &data) = 0;

    virtual void clearCaches() = 0;
};
} // namespace RenderModule
