#pragma once

#include "BaseResource.h"
#include "Asset/AssetID.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace ResourceModule
{
class RMaterial : public BaseResource
{
  public:
    glm::vec4 albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec3 emissiveColor{0.0f, 0.0f, 0.0f};

    float roughness = 0.5f;
    float metallic = 0.0f;
    float normalStrength = 1.0f;

    AssetID albedoTexture{};
    AssetID normalTexture{};
    AssetID roughnessMetallicTexture{};
    AssetID emissiveTexture{};

    std::string name{"DefaultMaterial"};
    AssetID materialID{};

    RMaterial() : BaseResource("") {}
    
    explicit RMaterial(const std::string& filePath);
    
    ~RMaterial() noexcept = default;
};
} // namespace ResourceModule
