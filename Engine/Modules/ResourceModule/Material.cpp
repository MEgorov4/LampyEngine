#include "Material.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace ResourceModule
{
RMaterial::RMaterial(const std::string& filePath) : BaseResource(filePath)
{
    // Читаем JSON файл материала
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        // Если файл не открылся, используем значения по умолчанию
        return;
    }

    nlohmann::json j;
    file >> j;
    file.close();

    // Загружаем параметры из JSON
    if (j.contains("albedoColor"))
    {
        auto& albedo = j["albedoColor"];
        if (albedo.is_array() && albedo.size() >= 3)
        {
            albedoColor = glm::vec4(albedo[0].get<float>(), albedo[1].get<float>(), 
                                   albedo[2].get<float>(), 
                                   albedo.size() >= 4 ? albedo[3].get<float>() : 1.0f);
        }
    }
    
    if (j.contains("emissiveColor"))
    {
        auto& emissive = j["emissiveColor"];
        if (emissive.is_array() && emissive.size() >= 3)
        {
            emissiveColor = glm::vec3(emissive[0].get<float>(), emissive[1].get<float>(), emissive[2].get<float>());
        }
    }

    if (j.contains("roughness"))
        roughness = j["roughness"].get<float>();
    if (j.contains("metallic"))
        metallic = j["metallic"].get<float>();
    if (j.contains("normalStrength"))
        normalStrength = j["normalStrength"].get<float>();

    if (j.contains("name"))
        name = j["name"].get<std::string>();

    // Загружаем AssetID для текстур
    if (j.contains("albedoTexture"))
    {
        std::string texID = j["albedoTexture"].get<std::string>();
        if (!texID.empty())
            albedoTexture = AssetID(texID);
    }
    if (j.contains("normalTexture"))
    {
        std::string texID = j["normalTexture"].get<std::string>();
        if (!texID.empty())
            normalTexture = AssetID(texID);
    }
    if (j.contains("roughnessMetallicTexture"))
    {
        std::string texID = j["roughnessMetallicTexture"].get<std::string>();
        if (!texID.empty())
            roughnessMetallicTexture = AssetID(texID);
    }
    if (j.contains("emissiveTexture"))
    {
        std::string texID = j["emissiveTexture"].get<std::string>();
        if (!texID.empty())
            emissiveTexture = AssetID(texID);
    }

    if (j.contains("guid"))
    {
        std::string guidStr = j["guid"].get<std::string>();
        if (!guidStr.empty())
            materialID = AssetID(guidStr);
    }
}

} // namespace ResourceModule
