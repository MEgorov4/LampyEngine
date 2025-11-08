#pragma once
#include <string>
#include <cstdint>
#include <Modules/ResourceModule/Asset/AssetID.h>

namespace ResourceModule
{
    struct PakEntry
    {
        AssetID guid;
        uint64_t offset = 0;
        uint64_t size = 0;
        std::string type;
        std::string path; // относительный путь (опционально)
    };
}