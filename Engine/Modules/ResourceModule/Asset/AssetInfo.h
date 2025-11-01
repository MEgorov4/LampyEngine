#pragma once
#include <EngineMinimal.h>
#include "AssetID.h"
#include "nlohmann/json.hpp"

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
        Scene
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
        std::vector<std::string> dependencies;

        uint64_t sourceTimestamp{0};
        uint64_t importedTimestamp{0};
        uint64_t sourceFileSize{0};
        uint64_t importedFileSize{0};
    };

    inline void to_json(nlohmann::json& j, const AssetInfo& a)
    {
        j = nlohmann::json{
            {"guid", a.guid.str()},
            {"type", static_cast<int>(a.type)},
            {"origin", static_cast<int>(a.origin)}, 
            {"source", a.sourcePath},
            {"imported", a.importedPath},
            {"dependencies", a.dependencies},
            {"sourceTimestamp", a.sourceTimestamp},
            {"importedTimestamp", a.importedTimestamp},
            {"sourceFileSize", a.sourceFileSize},
            {"importedFileSize", a.importedFileSize}
        };
    }

    inline void from_json(const nlohmann::json& j, AssetInfo& a)
    {
        std::string guidStr;
        j.at("guid").get_to(guidStr);
        a.guid = AssetID(guidStr);

        int typeInt = 0;
        j.at("type").get_to(typeInt);
        a.type = static_cast<AssetType>(typeInt);

        if (j.contains("origin"))
        {
            int originInt = 0;
            j.at("origin").get_to(originInt);
            a.origin = static_cast<AssetOrigin>(originInt);
        }

        j.at("source").get_to(a.sourcePath);
        j.at("imported").get_to(a.importedPath);

        if (j.contains("dependencies"))
            j.at("dependencies").get_to(a.dependencies);

        if (j.contains("sourceTimestamp"))  j.at("sourceTimestamp").get_to(a.sourceTimestamp);
        if (j.contains("importedTimestamp"))j.at("importedTimestamp").get_to(a.importedTimestamp);
        if (j.contains("sourceFileSize"))   j.at("sourceFileSize").get_to(a.sourceFileSize);
        if (j.contains("importedFileSize")) j.at("importedFileSize").get_to(a.importedFileSize);
    }
}
