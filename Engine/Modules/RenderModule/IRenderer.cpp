#include "IRenderer.h"

#include "Foundation/Assert/Assert.h"
#include "RenderContext.h"
#include "RenderGraph/RenderGraphBuilder.h"
#include "RenderGraph/RenderNodes.h"
#include "RenderLocator.h"

#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ResourceModule/ResourceManager.h>

namespace RenderModule
{
IRenderer::IRenderer() : m_ecsModule(GCM(ECSModule::ECSModule))
{
}

IRenderer::~IRenderer()
{
    // ничего: граф сам управляет ресурсами-описателями, а GPU-объекты живут в фабриках/нодах
}

void IRenderer::postInit()
{
    RenderGraphBuilder builder(m_renderGraph);

    builder.addResource("shadow_pass_depth", 1920, 1080)
        .addResource("light_pass_color", 1920, 1080)
        .addResource("texture_pass_color", 1920, 1080)
        .addResource("final", 1920, 1080)

        .addPass("Shadow")
        .write("shadow_pass_depth")
        .exec(RenderNodes::ShadowPass)
        .end()

        .addPass("Light")
        .read("shadow_pass_depth")
        .write("light_pass_color")
        .exec(RenderNodes::LightPass)
        .end()

        .addPass("Texture")
        .read("light_pass_color")
        .write("texture_pass_color")
        .exec(RenderNodes::TexturePass)
        .end()

        .addPass("Final")
        .read("texture_pass_color")
        .write("final")
        .exec(RenderNodes::FinalCompose)
        .end()

        .build();
}

void IRenderer::updateRenderList()
{
    auto *worldPtr = m_ecsModule->getCurrentWorld();
    if (!worldPtr)
    {
        LT_ASSERT_MSG(false, "Not active world");
        return;
    }

    auto &world = worldPtr->get();
    auto &ctx = RenderLocator::Ref();
    auto &scene = ctx.scene();

    scene.clear();

    // ============================================================
    // 1️⃣ Камера
    // ============================================================
    {
        auto qCam = world.query<PositionComponent, RotationComponent, CameraComponent>();
        qCam.each(
            [&](flecs::entity, const PositionComponent &pos, const RotationComponent &rot, const CameraComponent &cam) {
                const glm::vec3 camPos = pos.toGLMVec();
                const glm::quat q = glm::normalize(rot.toQuat());
                const glm::vec3 forward = q * glm::vec3(0, 0, -1);
                const glm::vec3 up = q * glm::vec3(0, 1, 0);

                const auto [w, h] = ctx.getViewport();
                const float aspect = (h != 0) ? float(w) / float(h) : 16.f / 9.f;

                scene.camera.position = glm::vec4(camPos, 1.f);
                scene.camera.view = glm::lookAt(camPos, camPos + forward, up);
                scene.camera.projection = glm::perspective(glm::radians(cam.fov), aspect, cam.nearClip, cam.farClip);
            });
    }

    // ============================================================
    // 2️⃣ Рендер-объекты (модели)
    // ============================================================
    {
        auto qMesh = world.query<PositionComponent, RotationComponent, ScaleComponent, MeshComponent>();
        qMesh.each([&](flecs::entity, const PositionComponent &pos, const RotationComponent &rot,
                       const ScaleComponent &scale, const MeshComponent &mesh) {
            RenderObject obj;
            glm::mat4 model(1.f);
            model = glm::translate(model, pos.toGLMVec());
            model *= glm::mat4_cast(rot.toQuat());
            model = glm::scale(model, scale.toGLMVec());
            obj.modelMatrix.model = model;

            obj.mesh = MeshFactory::createOrGetMesh(mesh.meshID);
            // obj.shader  = ShaderFactory::createOrGetShader(mesh.fragShaderID, mesh.vertShaderID);
            // obj.texture = TextureFactory::createOrGetTexture(mesh.textureID);

            scene.objects.push_back(std::move(obj));
        });
    }

    // ============================================================
    // 3️⃣ DirectionalLight (если есть)
    // ============================================================
    //{
    //    if (world.component<ECSModule::DirectionalLightComponent>().is_valid())
    //    {
    //        auto qDirLight = world.query<ECSModule::DirectionalLightComponent>();
    //        qDirLight.each(
    //            [&](flecs::entity, const ECSModule::DirectionalLightComponent& light)
    //            {
    //                scene.sun.direction = glm::vec4(light.direction, 0.0f);
    //                scene.sun.color     = glm::vec4(light.color, 1.0f);
    //                scene.sun.intensity = light.intensity;
    //            });
    //    }
    //    else
    //    {
    //        scene.sun.direction = glm::vec4(0.f, -1.f, 0.f, 0.f);
    //        scene.sun.color     = glm::vec4(1.f);
    //        scene.sun.intensity = 1.f;
    //    }
    //}

    //// ============================================================
    //// 4️⃣ PointLight (если есть)
    //// ============================================================
    //{
    //    if (world.component<ECSModule::PointLightComponent>().is_valid())
    //    {
    //        auto qPoint = world.query<ECSModule::PointLightComponent>();
    //        qPoint.each(
    //            [&](flecs::entity, const ECSModule::PointLightComponent& light)
    //            {
    //                scene.pointLights.push_back({
    //                    .position  = glm::vec4(light.position, 1.f),
    //                    .color     = glm::vec4(light.color, 1.f),
    //                    .intensity = light.intensity
    //                });
    //            });
    //    }

    // LT_LOGI("Renderer", std::format(
    //     "RenderContext updated: objects = {}, light spot = {}",
    //     scene.objects.size(), scene.pointLights.size()));
}

void IRenderer::render()
{
    LT_PROFILE_SCOPE("IRenderer::render");
    updateRenderList();
    m_activeTextureHandle = m_renderGraph.execute();
}

TextureHandle IRenderer::getOutputRenderHandle(int w, int h)
{
    m_renderGraph.resizeAll(w, h);
    return m_activeTextureHandle;
}

} // namespace RenderModule
