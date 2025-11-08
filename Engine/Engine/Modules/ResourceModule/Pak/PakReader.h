#pragma once
#include "../Asset/AssetID.h"
#include "PakEntry.h"
#include "PakHeader.h"

#include <EngineMinimal.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ResourceModule
{
class PakReader
{
  public:
    explicit PakReader(const std::string& pakPath);
    bool isOpen() const noexcept
    {
        return m_stream.is_open();
    }

    /// Читает бинарные данные ассета по GUID
    std::optional<std::vector<uint8_t, ProfileAllocator<uint8_t>>> readAsset(const AssetID& guid);

    /// Проверяет наличие ассета
    bool exists(const AssetID& guid) const noexcept;

    /// Возвращает список всех ассетов
    std::vector<AssetID> listAll() const;

  private:
    void loadIndex();

    std::ifstream m_stream;
    PakHeader m_header{};
    std::unordered_map<AssetID, PakEntry, AssetID::Hasher> m_index;
};
} // namespace ResourceModule
