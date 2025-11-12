// AssetCooker.cpp
#include "AssetCooker.h"
#include <Foundation/Assert/Assert.h>
#include "../Pak/PakBuilder.h"
#include <nlohmann/json.hpp>
#include <fstream>

using namespace ResourceModule;

AssetCooker::AssetCooker(const AssetDatabase& db,
                std::filesystem::path cacheDir,
                std::filesystem::path contentDir,
                CookSettings settings) noexcept
    : m_db(db)
    , m_cacheDir(std::move(cacheDir))
    , m_contentDir(std::move(contentDir))
    , m_settings(settings)
{ 
    LT_ASSERT_MSG(!m_cacheDir.empty(), "Cache directory path cannot be empty");
    LT_ASSERT_MSG(!m_contentDir.empty(), "Content directory path cannot be empty");
    LT_ASSERT_MSG(!m_settings.pakName.empty(), "PAK name cannot be empty");
}

bool AssetCooker::ensureDirs() const noexcept
{
    LT_ASSERT_MSG(!m_contentDir.empty(), "Content directory path is empty");
    
    std::error_code ec;
    std::filesystem::create_directories(m_contentDir, ec);
    if (ec)
    {
        LT_LOGE("AssetCooker", "Failed to create content directory: " + ec.message());
        return false;
    }
    
    LT_ASSERT_MSG(std::filesystem::exists(m_contentDir), "Content directory was not created");
    return true;
}

bool AssetCooker::writeRuntimeDatabase() const noexcept
{
    LT_ASSERT_MSG(!m_contentDir.empty(), "Content directory path is empty");
    
    nlohmann::json j = nlohmann::json::object();
    m_db.forEach([&](const AssetID& guid, const AssetInfo& info)
    {
        if (guid.empty() || info.sourcePath.empty())
            return;
        
        nlohmann::json e;
        e["type"] = static_cast<int>(info.type);
        e["imported"] = info.importedPath;
        j[guid.str()] = std::move(e);
    });

    const auto out = m_contentDir / "AssetDatabase.runtime.json";
    std::ofstream ofs(out, std::ios::trunc);
    if (!ofs.is_open())
    {
        LT_LOGE("AssetCooker", "Failed to write runtime DB: " + out.string());
        return false;
    }
    ofs << j.dump(2);
    return true;
}

bool AssetCooker::copyLooseFiles() const noexcept
{
    LT_ASSERT_MSG(!m_contentDir.empty(), "Content directory path is empty");
    
    std::error_code ec;
    auto include = m_settings.includeEngineAssets ? AssetOrigin::Project : AssetOrigin::Engine;

    m_db.forEachByOrigin(AssetOrigin::Project,
                         [&](const AssetID& guid, const AssetInfo& info)
                         {
                             if (guid.empty())
                                 return;
                             
                             if (info.importedPath.empty())
                                 return;
                             
                             const std::filesystem::path src(info.importedPath);
                             if (!std::filesystem::exists(src))
                                 return;
                             
                             LT_ASSERT_MSG(std::filesystem::is_regular_file(src), "Source is not a file: " + src.string());

                             const std::filesystem::path dst = m_contentDir / src.filename();
                             std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing,
                                                        ec);
                             if (ec)
                                 LT_LOGW("AssetCooker", "Copy failed: " + src.string() + " -> " + dst.string());
                         });

    if (m_settings.includeEngineAssets)
    {
        m_db.forEachByOrigin(
            AssetOrigin::Engine,
            [&](const AssetID& guid, const AssetInfo& info)
            {
                if (guid.empty())
                    return;
                
                if (info.importedPath.empty())
                    return;
                    
                const std::filesystem::path src(info.importedPath);
                if (!std::filesystem::exists(src))
                    return;
                
                LT_ASSERT_MSG(std::filesystem::is_regular_file(src), "Source is not a file: " + src.string());

                const std::filesystem::path dst = m_contentDir / src.filename();
                std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                    LT_LOGW("AssetCooker", "Copy failed (engine): " + src.string() + " -> " + dst.string());
            });
    }

    return true;
}

bool AssetCooker::cook()
{
    LT_ASSERT_MSG(!m_cacheDir.empty(), "Cache directory is not set");
    LT_ASSERT_MSG(!m_contentDir.empty(), "Content directory is not set");
    
    LT_LOGI("AssetCooker", "Cook start → Content: " + m_contentDir.string());
    
    if (!ensureDirs())
    {
        LT_LOGE("AssetCooker", "Failed to create content dir");
        return false;
    }

    if (m_settings.makePak)
    {
        const auto pakPath = m_contentDir / m_settings.pakName;
        LT_ASSERT_MSG(!pakPath.empty(), "PAK path is empty");
        
        bool built = PakBuilder::BuildPak(m_db, m_cacheDir, pakPath);
        if (!built)
        {
            LT_LOGE("AssetCooker", "Failed to build PAK");
            return false;
        }
        
        LT_ASSERT_MSG(std::filesystem::exists(pakPath), "PAK file was not created");
    }

    if (m_settings.copyLooseAlongsidePak)
        copyLooseFiles();

    if (!writeRuntimeDatabase())
        return false;

    {
        const auto manifest = m_contentDir / "Content.manifest.json";
        nlohmann::json m = {
            {"usePak", m_settings.makePak},
            {"pakName", m_settings.pakName}
        };
        std::ofstream ofs(manifest, std::ios::trunc);
        if (ofs.is_open()) 
        {
            ofs << m.dump(2);
        }
        else
        {
            LT_LOGW("AssetCooker", "Failed to write manifest file");
        }
    }

    LT_LOGI("AssetCooker", "Cook done.");
    return true;
}
