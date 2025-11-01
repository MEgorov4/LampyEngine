#pragma once
#include "../Asset/AssetDatabase.h"
#include "PakEntry.h"
#include "PakHeader.h"

#include <EngineMinimal.h>

namespace ResourceModule
{
class PakBuilder
{
  public:
    static bool BuildPak(const AssetDatabase& db, const std::filesystem::path& cacheDir,
                         const std::filesystem::path& outPakPath);
};
} // namespace ResourceModule
