#pragma once
#include "AssetInfo.h"

#include <EngineMinimal.h>

namespace ResourceModule
{
    class IAssetImporter
    {
    public:
        virtual ~IAssetImporter() = default;

        /// Проверяет, поддерживает ли импортёр данный тип файла
        virtual bool supportsExtension(const std::string& ext) const noexcept = 0;

        /// Возвращает тип ассета (Texture, Mesh, Shader и т.д.)
        virtual AssetType getAssetType() const noexcept = 0;

        /// Выполняет импорт файла: создает runtime-бинарь в Cache/
        /// Возвращает заполненную структуру AssetInfo (в том числе GUID)
        virtual AssetInfo import(const std::filesystem::path& sourcePath,
                                 const std::filesystem::path& cacheRoot) = 0;
    };
}
