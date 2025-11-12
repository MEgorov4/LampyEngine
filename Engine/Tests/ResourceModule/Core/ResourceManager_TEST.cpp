#include <gtest/gtest.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/Asset/Importers/MaterialImporter.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ResourceManagerTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ResourceManagerTest");
        resourcesDir = tempDir->createSubdir("Resources");
        cacheDir = tempDir->createSubdir("Cache");
        
        db = std::make_unique<AssetDatabase>();
        manager = std::make_unique<ResourceManager>();
        manager->setDatabase(db.get());
        manager->setEngineResourcesRoot(resourcesDir);
        manager->setProjectResourcesRoot(resourcesDir);
    }
    
    void TearDown() override
    {
        manager.reset();
        db.reset();
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    AssetInfo createMaterialAssetInfo(const std::string& sourcePath, const std::string& importedPath)
    {
        AssetInfo info;
        info.guid = AssetID(sourcePath);
        info.type = AssetType::Material;
        info.origin = AssetOrigin::Project;
        info.sourcePath = sourcePath;
        info.importedPath = importedPath;
        info.sourceTimestamp = 1234567890;
        info.importedTimestamp = 1234567891;
        info.sourceFileSize = 1024;
        info.importedFileSize = 2048;
        return info;
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    std::filesystem::path resourcesDir;
    std::filesystem::path cacheDir;
    std::unique_ptr<AssetDatabase> db;
    std::unique_ptr<ResourceManager> manager;
};

TEST_F(ResourceManagerTest, SetDatabase)
{
    ResourceManager rm;
    AssetDatabase database;
    
    rm.setDatabase(&database);
    EXPECT_EQ(rm.getDatabase(), &database);
}

TEST_F(ResourceManagerTest, SetEngineResourcesRoot)
{
    ResourceManager rm;
    std::filesystem::path engineRoot = tempDir->path() / "EngineResources";
    
    rm.setEngineResourcesRoot(engineRoot);
}

TEST_F(ResourceManagerTest, SetProjectResourcesRoot)
{
    ResourceManager rm;
    std::filesystem::path projectRoot = tempDir->path() / "ProjectResources";
    
    rm.setProjectResourcesRoot(projectRoot);
}

TEST_F(ResourceManagerTest, LoadByAssetID)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial", {1.0f, 0.0f, 0.0f, 1.0f}, 0.5f, 0.0f);
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    std::filesystem::path cachePath = cacheDir / "Materials";
    std::filesystem::create_directories(cachePath);
    
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
    EXPECT_FLOAT_EQ(material->albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.g, 0.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.b, 0.0f);
}

TEST_F(ResourceManagerTest, LoadBySource)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->loadBySource<RMaterial>(info.sourcePath);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
}

TEST_F(ResourceManagerTest, Caching)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material1, nullptr);
    
    auto material2 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material2, nullptr);
    
    EXPECT_EQ(material1, material2);
}

TEST_F(ResourceManagerTest, MissingResource)
{
    AssetID nonExistent = AssetID("nonexistent/path.lmat");
    
    auto material = manager->load<RMaterial>(nonExistent);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, EmptyAssetID)
{
    AssetID emptyId;
    
    auto material = manager->load<RMaterial>(emptyId);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, ResourceCacheIntegration)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material1, nullptr);
    
    material1.reset();
    
    auto material2 = manager->load<RMaterial>(info.guid);
}

TEST_F(ResourceManagerTest, ResourceRegistryIntegration)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material, nullptr);
    
    manager->unload(info.guid);
    
}

TEST_F(ResourceManagerTest, LoadMaterialFromFile)
{
    std::string materialJSON = R"({
        "name": "TestMaterial",
        "albedoColor": [1.0, 0.5, 0.0, 1.0],
        "emissiveColor": [0.2, 0.0, 0.0],
        "roughness": 0.7,
        "metallic": 0.3,
        "normalStrength": 1.5,
        "albedoTexture": "textures/albedo.png"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
    EXPECT_FLOAT_EQ(material->albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material->albedoColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material->emissiveColor.r, 0.2f);
    EXPECT_FLOAT_EQ(material->roughness, 0.7f);
    EXPECT_FLOAT_EQ(material->metallic, 0.3f);
    EXPECT_FLOAT_EQ(material->normalStrength, 1.5f);
    EXPECT_FALSE(material->albedoTexture.empty());
}

TEST_F(ResourceManagerTest, FallbackOnMissingResource)
{
    AssetID nonExistent = AssetID("Materials/nonexistent.lmat");
    
    auto material = manager->load<RMaterial>(nonExistent);
    EXPECT_EQ(material, nullptr);
    
}

TEST_F(ResourceManagerTest, CorruptedFile)
{
    std::filesystem::path materialPath = resourcesDir / "Materials" / "corrupted.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << "This is not valid JSON { invalid syntax";
    file.close();
    
    MaterialImporter importer;
    
    try
    {
        AssetInfo info = importer.import(materialPath, cacheDir);
        db->upsert(info);
        auto material = manager->load<RMaterial>(info.guid);
    }
    catch (const std::exception&)
    {
    }
}

TEST_F(ResourceManagerTest, LoadWithoutDatabase)
{
    ResourceManager rm;
    AssetID id("test/path.lmat");
    
    auto material = rm.load<RMaterial>(id);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, LoadWithDatabaseButMissingAsset)
{
    ResourceManager rm;
    AssetDatabase db;
    rm.setDatabase(&db);
    
    AssetID id("test/path.lmat");
    
    auto material = rm.load<RMaterial>(id);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, Unload)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material, nullptr);
    
    manager->unload(info.guid);
    
}

TEST_F(ResourceManagerTest, ClearAll)
{
    for (int i = 0; i < 3; ++i)
    {
        std::string materialJSON = TestData::createMaterialJSON("TestMaterial" + std::to_string(i));
        std::filesystem::path materialPath = resourcesDir / "Materials" / ("test" + std::to_string(i) + ".lmat");
        std::filesystem::create_directories(materialPath.parent_path());
        std::ofstream file(materialPath);
        file << materialJSON;
        file.close();
        
        MaterialImporter importer;
        AssetInfo info = importer.import(materialPath, cacheDir);
        db->upsert(info);
        
        auto material = manager->load<RMaterial>(info.guid);
        ASSERT_NE(material, nullptr);
    }
    
    manager->clearAll();
    
}

TEST_F(ResourceManagerTest, StartupShutdown)
{
    ResourceManager rm;
    
    rm.startup();
    
    rm.shutdown();
}

TEST_F(ResourceManagerTest, MultipleLoads)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    auto material2 = manager->load<RMaterial>(info.guid);
    auto material3 = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material1, nullptr);
    ASSERT_NE(material2, nullptr);
    ASSERT_NE(material3, nullptr);
    
    EXPECT_EQ(material1, material2);
    EXPECT_EQ(material2, material3);
}

