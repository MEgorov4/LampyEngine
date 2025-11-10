#pragma once
#include "../IAssetImporter.h"
#include "../../Material.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include <Foundation/Assert/Assert.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace ResourceModule
{
    class MaterialImporter final : public IAssetImporter
    {
    public:
        bool supportsExtension(const std::string& ext) const noexcept override
        {
            return ext == ".lmat";
        }

        AssetType getAssetType() const noexcept override { return AssetType::Material; }

        AssetInfo import(const std::filesystem::path& sourcePath, const std::filesystem::path& cacheRoot) override
        {
            LT_ASSERT_MSG(!sourcePath.empty(), "Source path cannot be empty");
            LT_ASSERT_MSG(!cacheRoot.empty(), "Cache root cannot be empty");
            LT_ASSERT_MSG(std::filesystem::exists(sourcePath), "Source file does not exist: " + sourcePath.string());
            LT_ASSERT_MSG(std::filesystem::is_regular_file(sourcePath), "Source path is not a file");

            AssetInfo info{};
            info.type       = AssetType::Material;
            info.sourcePath = sourcePath.string();
            info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());
            LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

            // Читаем JSON файл материала
            std::ifstream file(sourcePath);
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open material file: " + sourcePath.string());
            }

            nlohmann::json j;
            file >> j;
            file.close();

            // Создаем Material из JSON
            RMaterial mat{};
            if (j.contains("albedoColor"))
            {
                auto& albedo = j["albedoColor"];
                if (albedo.is_array() && albedo.size() >= 3)
                {
                    mat.albedoColor = glm::vec4(albedo[0].get<float>(), albedo[1].get<float>(), 
                                               albedo[2].get<float>(), 
                                               albedo.size() >= 4 ? albedo[3].get<float>() : 1.0f);
                }
            }
            
            if (j.contains("emissiveColor"))
            {
                auto& emissive = j["emissiveColor"];
                if (emissive.is_array() && emissive.size() >= 3)
                {
                    mat.emissiveColor = glm::vec3(emissive[0].get<float>(), emissive[1].get<float>(), emissive[2].get<float>());
                }
            }

            if (j.contains("roughness"))
                mat.roughness = j["roughness"].get<float>();
            if (j.contains("metallic"))
                mat.metallic = j["metallic"].get<float>();
            if (j.contains("normalStrength"))
                mat.normalStrength = j["normalStrength"].get<float>();

            if (j.contains("name"))
                mat.name = j["name"].get<std::string>();

            // Загружаем AssetID для текстур
            if (j.contains("albedoTexture"))
            {
                std::string texID = j["albedoTexture"].get<std::string>();
                if (!texID.empty())
                    mat.albedoTexture = AssetID(texID);
            }
            if (j.contains("normalTexture"))
            {
                std::string texID = j["normalTexture"].get<std::string>();
                if (!texID.empty())
                    mat.normalTexture = AssetID(texID);
            }
            if (j.contains("roughnessMetallicTexture"))
            {
                std::string texID = j["roughnessMetallicTexture"].get<std::string>();
                if (!texID.empty())
                    mat.roughnessMetallicTexture = AssetID(texID);
            }
            if (j.contains("emissiveTexture"))
            {
                std::string texID = j["emissiveTexture"].get<std::string>();
                if (!texID.empty())
                    mat.emissiveTexture = AssetID(texID);
            }

            mat.materialID = info.guid;

            // Сохраняем Material в cache как JSON (для простоты пока используем JSON, можно позже оптимизировать)
            std::filesystem::path cachePath = cacheRoot / "Materials" / (info.guid.str() + ".lmat");
            std::filesystem::create_directories(cachePath.parent_path());

            nlohmann::json cacheJson;
            cacheJson["guid"] = info.guid.str();
            cacheJson["name"] = mat.name;
            cacheJson["albedoColor"] = {mat.albedoColor.r, mat.albedoColor.g, mat.albedoColor.b, mat.albedoColor.a};
            cacheJson["emissiveColor"] = {mat.emissiveColor.r, mat.emissiveColor.g, mat.emissiveColor.b};
            cacheJson["roughness"] = mat.roughness;
            cacheJson["metallic"] = mat.metallic;
            cacheJson["normalStrength"] = mat.normalStrength;
            cacheJson["albedoTexture"] = mat.albedoTexture.str();
            cacheJson["normalTexture"] = mat.normalTexture.str();
            cacheJson["roughnessMetallicTexture"] = mat.roughnessMetallicTexture.str();
            cacheJson["emissiveTexture"] = mat.emissiveTexture.str();

            std::ofstream cacheFile(cachePath);
            if (!cacheFile.is_open())
            {
                throw std::runtime_error("Failed to create cache file: " + cachePath.string());
            }
            cacheFile << cacheJson.dump(4);
            cacheFile.close();

            info.importedPath = cachePath.string();
            info.sourceTimestamp = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
            info.importedTimestamp = std::filesystem::last_write_time(cachePath).time_since_epoch().count();

            // Добавляем зависимости от текстур
            if (!mat.albedoTexture.empty())
                info.dependencies.push_back(mat.albedoTexture.str());
            if (!mat.normalTexture.empty())
                info.dependencies.push_back(mat.normalTexture.str());
            if (!mat.roughnessMetallicTexture.empty())
                info.dependencies.push_back(mat.roughnessMetallicTexture.str());
            if (!mat.emissiveTexture.empty())
                info.dependencies.push_back(mat.emissiveTexture.str());

            return info;
        }
    };
} // namespace ResourceModule


