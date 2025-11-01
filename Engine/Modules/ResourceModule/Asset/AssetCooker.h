// AssetCooker.h
#pragma once
#include <filesystem>
#include "AssetDatabase.h"

namespace ResourceModule
{
    struct CookSettings
    {
        bool makePak = true;              // true: Content.pak, false: loose files
        bool copyLooseAlongsidePak = false; // опционально: положить копию loose в Content/
        bool includeEngineAssets = true;
        std::string pakName = "Content.pak";
    };

    class AssetCooker
    {
    public:
        AssetCooker(const AssetDatabase& db,
                    std::filesystem::path cacheDir,
                    std::filesystem::path contentDir,
                    CookSettings settings = {}) noexcept
            : m_db(db)
            , m_cacheDir(std::move(cacheDir))
            , m_contentDir(std::move(contentDir))
            , m_settings(settings)
        { }

        bool cook(); // главный вход

    private:
        bool ensureDirs() const noexcept;
        bool writeRuntimeDatabase() const noexcept;
        bool copyLooseFiles() const noexcept;

        const AssetDatabase& m_db;
        std::filesystem::path m_cacheDir;
        std::filesystem::path m_contentDir;
        CookSettings m_settings;
    };
}
