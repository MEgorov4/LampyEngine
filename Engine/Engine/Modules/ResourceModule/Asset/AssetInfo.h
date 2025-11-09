#pragma once
#include "AssetID.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include <Foundation/Assert/Assert.h>
#include "nlohmann/json.hpp"

#include <EngineMinimal.h>

namespace ResourceModule
{
enum class AssetType : uint8_t
{
    Unknown = 0,
    Texture,
    Mesh,
    Shader,
    Material,
    Audio,
    Script,
    World,
};

enum class AssetOrigin : uint8_t
{
    Project = 0,
    Engine
};

struct AssetInfo
{
    AssetID guid;
    AssetType type{AssetType::Unknown};
    AssetOrigin origin{AssetOrigin::Project};

    std::string sourcePath;
    std::string importedPath;
    std::vector<std::string, ProfileAllocator<std::string>> dependencies;

    uint64_t sourceTimestamp{0};
    uint64_t importedTimestamp{0};
    uint64_t sourceFileSize{0};
    uint64_t importedFileSize{0};
};

inline void to_json(nlohmann::json& j, const AssetInfo& a)
{
    // При сериализации в JSON GUID и sourcePath должны быть валидными для записей в базе
    LT_ASSERT_MSG(!a.guid.empty(), "AssetInfo GUID is empty");
    LT_ASSERT_MSG(!a.sourcePath.empty(), "AssetInfo source path is empty");
    
    j = nlohmann::json{{"guid", a.guid.str()},
                       {"type", static_cast<int>(a.type)},
                       {"origin", static_cast<int>(a.origin)},
                       {"source", a.sourcePath},
                       {"imported", a.importedPath},
                       {"dependencies", a.dependencies},
                       {"sourceTimestamp", a.sourceTimestamp},
                       {"importedTimestamp", a.importedTimestamp},
                       {"sourceFileSize", a.sourceFileSize},
                       {"importedFileSize", a.importedFileSize}};
}

inline void from_json(const nlohmann::json& j, AssetInfo& a)
{
    LT_ASSERT_MSG(j.contains("guid"), "AssetInfo JSON missing 'guid' field");
    LT_ASSERT_MSG(j.contains("source"), "AssetInfo JSON missing 'source' field");
    
    std::string guidStr;
    j.at("guid").get_to(guidStr);
    
    // Пустая строка GUID допустима при десериализации (для опциональных ресурсов)
    // но для записей в базе GUID должен быть валидным
    a.guid = AssetID(guidStr);

    int typeInt = 0;
    j.at("type").get_to(typeInt);
    a.type = static_cast<AssetType>(typeInt);
    LT_ASSERT_MSG(a.type != AssetType::Unknown, "Asset type is Unknown");

    if (j.contains("origin"))
    {
        int originInt = 0;
        j.at("origin").get_to(originInt);
        a.origin = static_cast<AssetOrigin>(originInt);
    }

    j.at("source").get_to(a.sourcePath);
    LT_ASSERT_MSG(!a.sourcePath.empty(), "Source path from JSON is empty");
    
    j.at("imported").get_to(a.importedPath);

    if (j.contains("dependencies"))
        j.at("dependencies").get_to(a.dependencies);

    if (j.contains("sourceTimestamp"))
        j.at("sourceTimestamp").get_to(a.sourceTimestamp);
    if (j.contains("importedTimestamp"))
        j.at("importedTimestamp").get_to(a.importedTimestamp);
    if (j.contains("sourceFileSize"))
        j.at("sourceFileSize").get_to(a.sourceFileSize);
    if (j.contains("importedFileSize"))
        j.at("importedFileSize").get_to(a.importedFileSize);
}
} // namespace ResourceModule
