#include "RenderObjectFactory.h"
#include "RenderFactory.h"
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include "Core/Core.h"

namespace RenderModule
{

RenderObject RenderObjectFactory::createFromState(EntityRenderState& state)
{
    RenderObject obj;
    
    obj.modelMatrix.model = computeModelMatrix(state.position, state.rotation, state.scale);
    
    obj.mesh = RenderFactory::get().createMesh(state.mesh.meshID);
    
    if (!state.mesh.vertShaderID.empty() && !state.mesh.fragShaderID.empty())
    {
        try
        {
            obj.shader = RenderFactory::get().createShader(state.mesh.vertShaderID, state.mesh.fragShaderID);
        }
        catch (const std::exception& e)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create shader: " + std::string(e.what()));
            obj.shader.reset();
        }
        catch (...)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create shader: unknown error");
            obj.shader.reset();
        }
    }
    
    if (!state.mesh.textureID.empty())
    {
        try
        {
            obj.texture = RenderFactory::get().createTexture(state.mesh.textureID);
        }
        catch (const std::exception& e)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create texture: " + std::string(e.what()));
            obj.texture.reset();
        }
        catch (...)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create texture: unknown error");
            obj.texture.reset();
        }
    }
    
    if (!state.material.materialID.empty())
    {
        auto rm = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        if (rm)
        {
            LT_LOGI("RenderObjectFactory", "Loading material: " + state.material.materialID.str());
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            if (obj.material)
            {
                LT_LOGI("RenderObjectFactory", "Material loaded successfully: " + obj.material->name);
                if (!obj.material->albedoTexture.empty())
                {
                    try
                    {
                        obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
                    }
                    catch (const std::exception& e)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture from material: " + std::string(e.what()));
                        obj.texture.reset();
                    }
                    catch (...)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture from material: unknown error");
                        obj.texture.reset();
                    }
                }
            }
            else
            {
                LT_LOGW("RenderObjectFactory", "Failed to load material: " + state.material.materialID.str());
            }
        }
        else
        {
            LT_LOGW("RenderObjectFactory", "ResourceManager not available");
        }
    }
    
    state.renderObject = std::make_unique<RenderObject>(obj);
    
    return obj;
}

void RenderObjectFactory::updateTransform(RenderObject& obj, const EntityRenderState& state)
{
    obj.modelMatrix.model = computeModelMatrix(state.position, state.rotation, state.scale);
}

void RenderObjectFactory::updateResources(RenderObject& obj, const EntityRenderState& state)
{
    obj.mesh = RenderFactory::get().createMesh(state.mesh.meshID);
    
    if (!state.mesh.vertShaderID.empty() && !state.mesh.fragShaderID.empty())
    {
        try
        {
            obj.shader = RenderFactory::get().createShader(state.mesh.vertShaderID, state.mesh.fragShaderID);
        }
        catch (const std::exception& e)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create shader: " + std::string(e.what()));
            obj.shader.reset();
        }
        catch (...)
        {
            LT_LOGW("RenderObjectFactory", "Failed to create shader: unknown error");
            obj.shader.reset();
        }
    }
    else
    {
        obj.shader.reset();
    }
    
    if (!state.material.materialID.empty())
    {
        auto rm = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        if (rm)
        {
            bool materialChanged = !obj.material || 
                                   (obj.material && obj.material->materialID != state.material.materialID);
            
            if (materialChanged)
            {
                LT_LOGI("RenderObjectFactory", "Material ID changed, reloading: " + state.material.materialID.str());
            }
            
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            
            if (obj.material)
            {
                LT_LOGI("RenderObjectFactory", "Material loaded: " + obj.material->name + " (ID: " + obj.material->materialID.str() + ")");
                if (!obj.material->albedoTexture.empty())
                {
                    try
                    {
                        obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
                    }
                    catch (const std::exception& e)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture from material: " + std::string(e.what()));
                        obj.texture.reset();
                    }
                    catch (...)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture from material: unknown error");
                        obj.texture.reset();
                    }
                }
                else if (!state.mesh.textureID.empty())
                {
                    obj.texture = RenderFactory::get().createTexture(state.mesh.textureID);
                }
                else
                {
                    obj.texture.reset();
                }
            }
            else
            {
                LT_LOGW("RenderObjectFactory", "Failed to load material: " + state.material.materialID.str());
                if (!state.mesh.textureID.empty())
                {
                    try
                    {
                        obj.texture = RenderFactory::get().createTexture(state.mesh.textureID);
                    }
                    catch (const std::exception& e)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture: " + std::string(e.what()));
                        obj.texture.reset();
                    }
                    catch (...)
                    {
                        LT_LOGW("RenderObjectFactory", "Failed to create texture: unknown error");
                        obj.texture.reset();
                    }
                }
                else
                {
                    obj.texture.reset();
                }
            }
        }
        else
        {
            LT_LOGW("RenderObjectFactory", "ResourceManager not available for material update");
        }
    }
    else
    {
        obj.material.reset();
        if (!state.mesh.textureID.empty())
        {
            obj.texture = RenderFactory::get().createTexture(state.mesh.textureID);
        }
        else
        {
            obj.texture.reset();
        }
    }
}

glm::mat4 RenderObjectFactory::computeModelMatrix(const PositionComponent& pos, 
                                                   const RotationComponent& rot, 
                                                   const ScaleComponent& scale)
{
    glm::mat4 model(1.f);
    model = glm::translate(model, pos.toGLMVec());
    model *= glm::mat4_cast(rot.toQuat());
    model = glm::scale(model, scale.toGLMVec());
    return model;
}

} // namespace RenderModule

