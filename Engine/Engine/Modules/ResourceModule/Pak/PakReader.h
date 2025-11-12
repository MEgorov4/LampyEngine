#pragma once
#include "../Asset/AssetID.h"
#include "PakEntry.h"
#include "PakHeader.h"
#include "Foundation/Memory/ResourceAllocator.h"

#include <EngineMinimal.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using EngineCore::Foundation::ResourceAllocator;

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

    std::optional<std::vector<uint8_t, ResourceAllocator<uint8_t>>> readAsset(const AssetID& guid);

    bool exists(const AssetID& guid) const noexcept;

    std::vector<AssetID> listAll() const;

  private:
    void loadIndex();

    std::ifstream m_stream;
    PakHeader m_header{};
    std::unordered_map<AssetID, PakEntry, AssetID::Hasher> m_index;
};
} // namespace ResourceModule
