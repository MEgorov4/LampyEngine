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
            LT_LOGI("RenderObjectFactory", "Loading material: " + state.material.materialID.str());
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            // Если есть материал, используем его текстуры
            if (obj.material)
            {
                LT_LOGI("RenderObjectFactory", "Material loaded successfully: " + obj.material->name);
                if (!obj.material->albedoTexture.empty())
                {
                    obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
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
    // ВАЖНО: Всегда перезагружаем материал, даже если materialID не изменился,
    // так как материал может быть изменен в файле или в базе данных
    if (!state.material.materialID.empty())
    {
        auto rm = Core::Locator().tryGet<ResourceModule::ResourceManager>();
        if (rm)
        {
            // Проверяем, изменился ли materialID
            bool materialChanged = !obj.material || 
                                   (obj.material && obj.material->materialID != state.material.materialID);
            
            if (materialChanged)
            {
                LT_LOGI("RenderObjectFactory", "Material ID changed, reloading: " + state.material.materialID.str());
            }
            
            // Всегда перезагружаем материал (ResourceManager кэширует, но мы получаем актуальную версию)
            obj.material = rm->load<ResourceModule::RMaterial>(state.material.materialID);
            
            if (obj.material)
            {
                LT_LOGI("RenderObjectFactory", "Material loaded: " + obj.material->name + " (ID: " + obj.material->materialID.str() + ")");
                // Используем текстуры из материала
                if (!obj.material->albedoTexture.empty())
                {
                    obj.texture = RenderFactory::get().createTexture(obj.material->albedoTexture);
                }
                else if (!state.mesh.textureID.empty())
                {
                    // Fallback на текстуру из MeshComponent
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
                // Если материал не загрузился, используем текстуру из MeshComponent
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
        else
        {
            LT_LOGW("RenderObjectFactory", "ResourceManager not available for material update");
        }
    }
    else
    {
        // MaterialComponent пуст или отсутствует
        obj.material.reset();
        // Используем текстуру из MeshComponent
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

