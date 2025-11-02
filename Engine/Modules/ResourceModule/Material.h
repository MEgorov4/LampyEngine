#pragma once

#include "BaseResource.h"
#include "Asset/AssetID.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace ResourceModule
{
/// Material resource - содержит параметры материала для PBR рендеринга
class RMaterial : public BaseResource
{
  public:
    // Цвета
    glm::vec4 albedoColor{1.0f, 1.0f, 1.0f, 1.0f};      // Базовый цвет альбедо
    glm::vec3 emissiveColor{0.0f, 0.0f, 0.0f};            // Эмиссивный цвет (свечение)

    // PBR параметры
    float roughness = 0.5f;   // Шероховатость (0 = зеркало, 1 = матовая поверхность)
    float metallic = 0.0f;     // Металличность (0 = диэлектрик, 1 = металл)
    float normalStrength = 1.0f; // Сила нормальной карты

    // Текстуры
    AssetID albedoTexture{};           // Альбедо текстура
    AssetID normalTexture{};           // Нормальная карта
    AssetID roughnessMetallicTexture{}; // Roughness/Metallic карта
    AssetID emissiveTexture{};         // Эмиссивная текстура

    // Метаданные
    std::string name{"DefaultMaterial"};
    AssetID materialID{}; // ID этого материала

    // Конструктор по умолчанию (пустой материал)
    RMaterial() : BaseResource("") {}
    
    // Конструктор для загрузки из файла (для ResourceManager)
    explicit RMaterial(const std::string& filePath);
    
    ~RMaterial() noexcept = default;
};
} // namespace ResourceModule
