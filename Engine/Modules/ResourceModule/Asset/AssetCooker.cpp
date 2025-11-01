// AssetCooker.cpp
#include "AssetCooker.h"
#include "../Pak/PakBuilder.h"
#include <nlohmann/json.hpp>
#include <fstream>

using namespace ResourceModule;

bool AssetCooker::ensureDirs() const noexcept
{
    std::error_code ec;
    std::filesystem::create_directories(m_contentDir, ec);
    return !ec;
}

bool AssetCooker::writeRuntimeDatabase() const noexcept
{
    // Пишем облегченную базу для Runtime в Content/
    // (только то, что нужно при загрузке)
    nlohmann::json j = nlohmann::json::object();
    m_db.forEach([&](const AssetID& guid, const AssetInfo& info)
    {
        nlohmann::json e;
        e["type"] = static_cast<int>(info.type);
        // importedPath нам в runtime не нужен при наличии .pak, но оставим для fallback
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
    std::error_code ec;
    auto include = m_settings.includeEngineAssets ? AssetOrigin::Project : AssetOrigin::Engine;

    m_db.forEachByOrigin(AssetOrigin::Project,
                         [&](const AssetID&, const AssetInfo& info)
                         {
                             if (info.importedPath.empty())
                                 return;
                             const std::filesystem::path src(info.importedPath);
                             if (!std::filesystem::exists(src))
                                 return;

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
            [&](const AssetID&, const AssetInfo& info)
            {
                if (info.importedPath.empty())
                    return;
                const std::filesystem::path src(info.importedPath);
                if (!std::filesystem::exists(src))
                    return;

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
    LT_LOGI("AssetCooker", "Cook start → Content: " + m_contentDir.string());
    if (!ensureDirs())
    {
        LT_LOGE("AssetCooker", "Failed to create content dir");
        return false;
    }

    if (m_settings.makePak)
    {
        const auto pakPath = m_contentDir / m_settings.pakName;
        if (!PakBuilder::BuildPak(m_db, m_cacheDir, pakPath))
            return false;
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
        if (ofs.is_open()) ofs << m.dump(2);
    }

    LT_LOGI("AssetCooker", "Cook done.");
    return true;
}
