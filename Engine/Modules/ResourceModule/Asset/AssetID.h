#pragma once
#include <array>
#include <cstddef>
#include <string>
#include <xstring>

namespace ResourceModule
{
class AssetID
{
  public:
    AssetID() noexcept;
    AssetID(const std::string& str);

    [[nodiscard]] std::string str() const;
    [[nodiscard]] bool empty() const noexcept;

    bool operator==(const AssetID& rhs) const noexcept;
    bool operator!=(const AssetID& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    struct Hasher
    {
        size_t operator()(const AssetID& id) const noexcept;
    };

    friend ::ResourceModule::AssetID MakeRandomAssetID();
    friend ::ResourceModule::AssetID MakeDeterministicIDFromPath(const std::string& absPath);

  private:
    std::array<unsigned char, 16> m_bytes{};
};
AssetID MakeRandomAssetID();
AssetID MakeDeterministicIDFromPath(const std::string& absPath);
} // namespace ResourceModule
