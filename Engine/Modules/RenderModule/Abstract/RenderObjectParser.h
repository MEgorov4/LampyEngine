#pragma once

#include <flecs.h>
#include <sol/object.hpp>

#include "../../ObjectCoreModule/ECS/ECSComponents.h"

#include  "RenderObject.h"

namespace RenderModule
{
    class IParseStrategy
    {
    public:
        virtual ~IParseStrategy() = default;
        virtual std::vector<RenderObject> parse(flecs::world& world) = 0;
    };

    class MeshesOnly final : public IParseStrategy
    {
    public:
        ~MeshesOnly() override = default;

        std::vector<RenderObject> parse(flecs::world& world) override
        {
            std::vector<RenderObject> objects;

            auto query = world.query<PositionComponent, RotationComponent, ScaleComponent, MeshComponent>();

            query.each([&objects](flecs::entity e,
                                  PositionComponent& pos,
                                  RotationComponent& rot,
                                  ScaleComponent& scale,
                                  MeshComponent& mesh)
            {
                RenderObject renderObject;

                glm::mat4 model{1.f};

                model = glm::translate(model, pos.toGLMVec());

                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                glm::vec3 radiansRotation = glm::radians(rot.toEuler());

                model = glm::rotate(model, radiansRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, radiansRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, radiansRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, scale.toGLMVec());

                renderObject.modelMatrix.model = model;

                if (mesh.meshResource && mesh.meshResource.value())
                {
                    renderObject.mesh = MeshFactory::createOrGetMesh(mesh.meshResource.value());
                    objects.push_back(renderObject);
                }
            });

            return objects;
        }
    };

    class AllWithoutDebug final : public IParseStrategy
    {
    public:
        ~AllWithoutDebug() override = default;

        std::vector<RenderObject> parse(flecs::world& world) override
        {
            std::vector<RenderObject> objects;
            auto query = world.query<PositionComponent, RotationComponent, ScaleComponent, MeshComponent>();

            query.each([&objects](PositionComponent& pos, RotationComponent& rot,
                                  ScaleComponent& scale,
                                  MeshComponent& mesh)
            {
                RenderObject renderObject;

                glm::mat4 model{1.f};

                model = glm::translate(model, pos.toGLMVec());

                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                glm::vec3 radiansRotation = glm::radians(rot.toEuler());

                model = glm::rotate(model, radiansRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, radiansRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, radiansRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, scale.toGLMVec());

                renderObject.modelMatrix = {model};

                if (mesh.meshResource && mesh.meshResource.value())
                {
                    renderObject.mesh = MeshFactory::createOrGetMesh(mesh.meshResource.value());

                    if ((mesh.fragShaderResource && mesh.vertShaderResource) && (mesh.fragShaderResource.value() &&
                        mesh.vertShaderResource.value()))
                    {
                        renderObject.shader = ShaderFactory::createOrGetShader(
                            mesh.vertShaderResource.value(), mesh.fragShaderResource.value());
                    }

                    if (mesh.textureResource && mesh.textureResource.value())
                    {
                        renderObject.texture = TextureFactory::createOrGetTexture(mesh.textureResource.value());
                    }

                    objects.push_back(renderObject);
                }
            });
            return objects;
        }
    };


    class DebugOnly final : public IParseStrategy
    {
    public:
        std::vector<RenderObject> parse(flecs::world& world) override;
    };

    class RenderObjectParser
    {
    public:
        template <typename T>
        static std::vector<RenderObject> parse(flecs::world& world)
        {
            static_assert(std::is_base_of<IParseStrategy, T>(), "Object must be derived from IParseStrategy");
            return T().parse(world);
        }
    };
}
