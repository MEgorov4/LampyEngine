#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/ScriptImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ScriptImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ScriptImporterTest");
        resourcesDir = tempDir->createSubdir("Resources");
        cacheDir = tempDir->createSubdir("Cache");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    std::filesystem::path resourcesDir;
    std::filesystem::path cacheDir;
};

TEST_F(ScriptImporterTest, SupportsExtension)
{
    ScriptImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".lua"));
    EXPECT_FALSE(importer.supportsExtension(".png"));
    EXPECT_FALSE(importer.supportsExtension(".lmat"));
    EXPECT_FALSE(importer.supportsExtension(".obj"));
}

TEST_F(ScriptImporterTest, GetAssetType)
{
    ScriptImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::Script);
}

TEST_F(ScriptImporterTest, Import)
{
    std::string scriptContent = TestData::createLuaScript("return { value = 42 }");
    std::filesystem::path scriptPath = resourcesDir / "test.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info = importer.import(scriptPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Script);
    std::string scriptPathStr = scriptPath.string();
    EXPECT_EQ(info.sourcePath, scriptPathStr);
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(ScriptImporterTest, CreateAssetInfo)
{
    std::string scriptContent = TestData::createLuaScript();
    std::filesystem::path scriptPath = resourcesDir / "test.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info = importer.import(scriptPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Script);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
}

TEST_F(ScriptImporterTest, GenerateGUID)
{
    std::string scriptContent = TestData::createLuaScript();
    std::filesystem::path scriptPath = resourcesDir / "test.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info1 = importer.import(scriptPath, cacheDir);
    AssetInfo info2 = importer.import(scriptPath, cacheDir);
    
    EXPECT_EQ(info1.guid, info2.guid);
}

TEST_F(ScriptImporterTest, MissingFile)
{
    ScriptImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.lua";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(ScriptImporterTest, ImportRealScript)
{
    std::string scriptContent = R"(local function calculate(a, b)
    return a + b
end

return {
    calculate = calculate,
    version = "1.0.0"
})";
    
    std::filesystem::path scriptPath = resourcesDir / "real.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info = importer.import(scriptPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Script);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(ScriptImporterTest, CacheFile)
{
    std::string scriptContent = TestData::createLuaScript("return {}");
    std::filesystem::path scriptPath = resourcesDir / "cache_test.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info = importer.import(scriptPath, cacheDir);
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
    
    std::string importedPathStr = info.importedPath;
    EXPECT_NE(importedPathStr.find(".Scriptbin"), std::string::npos);
    
    std::ifstream cacheFile(info.importedPath, std::ios::binary);
    uint32_t size = 0;
    cacheFile.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    EXPECT_EQ(size, scriptContent.size());
    cacheFile.close();
}

TEST_F(ScriptImporterTest, EmptyScript)
{
    std::string scriptContent = "";
    std::filesystem::path scriptPath = resourcesDir / "empty.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    
    try
    {
        AssetInfo info = importer.import(scriptPath, cacheDir);
        EXPECT_FALSE(info.guid.empty());
    }
    catch (const std::exception&)
    {
    }
}

TEST_F(ScriptImporterTest, LargeScript)
{
    std::string scriptContent;
    for (int i = 0; i < 1000; ++i)
    {
        scriptContent += "local var" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    }
    
    std::filesystem::path scriptPath = resourcesDir / "large.lua";
    std::ofstream file(scriptPath);
    file << scriptContent;
    file.close();
    
    ScriptImporter importer;
    AssetInfo info = importer.import(scriptPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Script);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

