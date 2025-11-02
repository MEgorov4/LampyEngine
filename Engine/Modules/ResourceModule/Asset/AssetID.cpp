#include "AssetID.h"
#include <EngineContext/Foundation/Assert/Assert.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <ctype.h>
#include <stdint.h>
#include <boost/uuid/detail/basic_name_generator.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/string_generator.hpp>
#include <filesystem>
#include <string>
#include <type_traits>
#include <xutility>

using namespace ResourceModule;

AssetID::AssetID() noexcept = default;

AssetID::AssetID(const std::string& str)
{
    LT_ASSERT_MSG(!str.empty(), "AssetID string cannot be empty");
    
    // Если это UUID (36 символов с дефисами) в стандартном формате
    if (str.size() == 36 && str[8] == '-' && str[13] == '-' && str[18] == '-' && str[23] == '-') {
        try {
            boost::uuids::string_generator gen;
            const auto uuid = gen(str);
            std::copy(uuid.begin(), uuid.end(), m_bytes.begin());
            return;
        } catch (...) {
            // Fall through to path-based generation
        }
    }

    // Если это путь, то делаем детерминированный GUID
    try {
        std::filesystem::path p = std::filesystem::weakly_canonical(str);
        *this = MakeDeterministicIDFromPath(p.generic_string());
    } catch (...) {
        *this = MakeDeterministicIDFromPath(str);
    }
    
    LT_ASSERT_MSG(!empty(), "Generated AssetID is empty");
}

std::string AssetID::str() const
{
    boost::uuids::uuid uuid;
    std::copy(m_bytes.begin(), m_bytes.end(), uuid.begin());
    return boost::uuids::to_string(uuid);
}

bool AssetID::empty() const noexcept
{
    for (auto b : m_bytes)
        if (b != 0)
            return false;
    return true;
}

bool AssetID::operator==(const AssetID& rhs) const noexcept
{
    return m_bytes == rhs.m_bytes;
}

size_t AssetID::Hasher::operator()(const AssetID& id) const noexcept
{
    uint64_t a = 0, b = 0;
    for (int i = 0; i < 8; ++i)
        a |= (uint64_t(id.m_bytes[i]) << (i * 8));
    for (int i = 0; i < 8; ++i)
        b |= (uint64_t(id.m_bytes[8 + i]) << (i * 8));
    return std::hash<uint64_t>{}(a ^ b);
}

// --------------------------------------------------------
// Генерация: детерминированный GUID от пути
// --------------------------------------------------------
AssetID ResourceModule::MakeDeterministicIDFromPath(const std::string& absPath)
{
    LT_ASSERT_MSG(!absPath.empty(), "Cannot generate ID from empty path");
    
    static const auto LampyNamespaceUUID =
        boost::uuids::string_generator()("123e4567-e89b-12d3-a456-426655440000");

    // --- Нормализация пути ---
    std::string norm = absPath;
    for (auto& c : norm)
    {
#ifdef _WIN32
        if (c == '\\') c = '/';
        c = (char)tolower(c);
#else
        if (c == '\\') c = '/';
#endif
    }

    boost::uuids::name_generator_sha1 gen(LampyNamespaceUUID);
    const auto uuid = gen(norm);

    AssetID id;
    std::copy(uuid.begin(), uuid.end(), id.m_bytes.begin());
    
    LT_ASSERT_MSG(!id.empty(), "Generated deterministic ID is empty");
    return id;
}
