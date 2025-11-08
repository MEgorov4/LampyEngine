#pragma once

#include <EngineMinimal.h>

#include <memory>
#include <string_view>

#include "../Asset/AssetID.h"

namespace ResourceModule
{
    class ResourceModule; // forward

    /**
     * @brief Type-safe resource handle. Non-throwing API returning nullptr on failures.
     */
    template <typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;
        explicit ResourceHandle(const AssetID& id) noexcept : m_id(id) {}
        explicit ResourceHandle(std::string_view sourcePath) noexcept; // defined in implementation TU

        [[nodiscard]] bool valid() const noexcept { return !m_id.empty(); }
        [[nodiscard]] const AssetID& id() const noexcept { return m_id; }

        [[nodiscard]] bool exists(ResourceModule::ExistsPolicy policy = ResourceModule::ExistsPolicy::DatabaseOnly) const noexcept;
        [[nodiscard]] std::shared_ptr<T> get() const noexcept;

        explicit operator bool() const noexcept { return valid(); }
        void reset() noexcept { m_id.clear(); }

        bool operator==(const ResourceHandle& rhs) const noexcept { return m_id == rhs.m_id; }
        bool operator!=(const ResourceHandle& rhs) const noexcept { return !(*this == rhs); }

    private:
        AssetID m_id;
    };
}


