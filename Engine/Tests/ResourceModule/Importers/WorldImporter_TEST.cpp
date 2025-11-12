#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/WorldImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class WorldImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("WorldImporterTest");
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

TEST_F(WorldImporterTest, SupportsExtension)
{
    WorldImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".lworld"));
    EXPECT_FALSE(importer.supportsExtension(".png"));
    EXPECT_FALSE(importer.supportsExtension(".lmat"));
    EXPECT_FALSE(importer.supportsExtension(".lua"));
}

TEST_F(WorldImporterTest, GetAssetType)
{
    WorldImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::World);
}

TEST_F(WorldImporterTest, Import)
{
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = resourcesDir / "test.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    std::string worldPathStr = worldPath.string();
    EXPECT_EQ(info.sourcePath, worldPathStr);
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(WorldImporterTest, CreateAssetInfo)
{
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = resourcesDir / "test.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
}

TEST_F(WorldImporterTest, GenerateGUID)
{
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = resourcesDir / "test.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info1 = importer.import(worldPath, cacheDir);
    AssetInfo info2 = importer.import(worldPath, cacheDir);
    
    EXPECT_EQ(info1.guid, info2.guid);
}

TEST_F(WorldImporterTest, MissingFile)
{
    WorldImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.lworld";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(WorldImporterTest, InvalidJSON)
{
    std::string invalidJSON = "This is not valid JSON { invalid syntax";
    std::filesystem::path worldPath = resourcesDir / "invalid.lworld";
    std::ofstream file(worldPath);
    file << invalidJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(WorldImporterTest, ImportRealWorld)
{
    std::string worldJSON = R"({
        "name": "RealWorld",
        "entities": [
            {
                "id": "entity1",
                "type": "Mesh",
                "position": [0.0, 0.0, 0.0]
            },
            {
                "id": "entity2",
                "type": "Light",
                "position": [5.0, 5.0, 5.0]
            }
        ]
    })";
    
    std::filesystem::path worldPath = resourcesDir / "real.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(WorldImporterTest, CacheFile)
{
    std::string worldJSON = TestData::createWorldJSON("CacheTestWorld");
    std::filesystem::path worldPath = resourcesDir / "cache_test.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
    
    std::string importedPathStr = info.importedPath;
    EXPECT_NE(importedPathStr.find(".worldbin"), std::string::npos);
    
    std::ifstream cacheFile(info.importedPath, std::ios::binary);
    uint32_t size = 0;
    cacheFile.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    EXPECT_EQ(size, worldJSON.size());
    cacheFile.close();
}

TEST_F(WorldImporterTest, EmptyWorld)
{
    std::string worldJSON = "{}";
    std::filesystem::path worldPath = resourcesDir / "empty.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(WorldImporterTest, LargeWorld)
{
    std::stringstream ss;
    ss << "{\n\"name\": \"LargeWorld\",\n\"entities\": [\n";
    for (int i = 0; i < 100; ++i)
    {
        ss << "{\"id\": " << i << ", \"position\": [" << i << ", " << i << ", " << i << "]},\n";
    }
    ss << "]\n}";
    
    std::string worldJSON = ss.str();
    std::filesystem::path worldPath = resourcesDir / "large.lworld";
    std::ofstream file(worldPath);
    file << worldJSON;
    file.close();
    
    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::World);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

