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
    
    // Вычисляем матрицу модели
    obj.modelMatrix.model = computeModelMatrix(state.position, state.rotation, state.scale);
    
    // Загружаем mesh
    obj.mesh = RenderFactory::get().createMesh(state.mesh.meshID);
    
    // Загружаем shader если есть
    if (!state.mesh.vertShaderID.empty() && !state.mesh.fragShaderID.empty())
    {
        obj.shader = RenderFactory::get().createShader(state.mesh.vertShaderID, state.mesh.fragShaderID);
    }
    
    // Загружаем texture если есть (fallback если нет материала)
    if (!state.mesh.textureID.empty())
    {
        obj.texture = RenderFactory::get().createTexture(state.mesh.textureID);
    }
    
    // Загружаем material если есть
    if (!state.material.materialID.empty())
    {
        auto rm = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        if (rm)
        {
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            // Если есть материал, используем его текстуры
            if (obj.material)
            {
                if (!obj.material->albedoTexture.empty())
                {
                    obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
                }
            }
        }
    }
    
    // Сохраняем renderObject в состоянии
    state.renderObject = std::make_unique<RenderObject>(obj);
    
    return obj;
}

void RenderObjectFactory::updateTransform(RenderObject& obj, const EntityRenderState& state)
{
    obj.modelMatrix.model = computeModelMatrix(state.position, state.rotation, state.scale);
}

void RenderObjectFactory::updateResources(RenderObject& obj, const EntityRenderState& state)
{
    // Обновляем mesh
    obj.mesh = RenderFactory::get().createMesh(state.mesh.meshID);
    
    // Обновляем shader если есть
    if (!state.mesh.vertShaderID.empty() && !state.mesh.fragShaderID.empty())
    {
        obj.shader = RenderFactory::get().createShader(state.mesh.vertShaderID, state.mesh.fragShaderID);
    }
    else
    {
        obj.shader.reset();
    }
    
    // Обновляем material если есть
    if (!state.material.materialID.empty())
    {
        auto rm = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        if (rm)
        {
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            // Используем текстуры из материала
            if (obj.material && !obj.material->albedoTexture.empty())
            {
                obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
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
    }
    else
    {
        obj.material.reset();
        // Обновляем texture если есть
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

