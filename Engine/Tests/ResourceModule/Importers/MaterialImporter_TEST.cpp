#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/MaterialImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class MaterialImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("MaterialImporterTest");
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

TEST_F(MaterialImporterTest, SupportsExtension)
{
    MaterialImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".lmat"));
    EXPECT_FALSE(importer.supportsExtension(".png"));
    EXPECT_FALSE(importer.supportsExtension(".obj"));
    EXPECT_FALSE(importer.supportsExtension(".lua"));
}

TEST_F(MaterialImporterTest, GetAssetType)
{
    MaterialImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::Material);
}

TEST_F(MaterialImporterTest, Import)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial", {1.0f, 0.0f, 0.0f, 1.0f}, 0.5f, 0.0f);
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Material);
    std::string materialPathStr = materialPath.string();
    EXPECT_EQ(info.sourcePath, materialPathStr);
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(MaterialImporterTest, CreateAssetInfo)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Material);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
}

TEST_F(MaterialImporterTest, GenerateGUID)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info1 = importer.import(materialPath, cacheDir);
    AssetInfo info2 = importer.import(materialPath, cacheDir);
    
    EXPECT_EQ(info1.guid, info2.guid);
}

TEST_F(MaterialImporterTest, MissingFile)
{
    MaterialImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.lmat";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(MaterialImporterTest, InvalidJSON)
{
    std::string invalidJSON = "This is not valid JSON { invalid syntax";
    std::filesystem::path materialPath = resourcesDir / "invalid.lmat";
    std::ofstream file(materialPath);
    file << invalidJSON;
    file.close();
    
    MaterialImporter importer;
    
    EXPECT_THROW({
        importer.import(materialPath, cacheDir);
    }, std::exception);
}

TEST_F(MaterialImporterTest, TextureDependencies)
{
    std::string materialJSON = R"({
        "name": "TexturedMaterial",
        "albedoTexture": "textures/albedo.png",
        "normalTexture": "textures/normal.png",
        "roughnessMetallicTexture": "textures/roughness_metallic.png",
        "emissiveTexture": "textures/emissive.png"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "textured.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_EQ(info.dependencies.size(), 4);
    EXPECT_FALSE(info.dependencies.empty());
}

TEST_F(MaterialImporterTest, ImportRealMaterial)
{
    std::string materialJSON = R"({
        "name": "RealMaterial",
        "albedoColor": [0.8, 0.2, 0.2, 1.0],
        "emissiveColor": [0.1, 0.0, 0.0],
        "roughness": 0.6,
        "metallic": 0.2,
        "normalStrength": 1.0,
        "albedoTexture": "textures/diffuse.png"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "real.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Material);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
}

TEST_F(MaterialImporterTest, CacheFile)
{
    std::string materialJSON = TestData::createMaterialJSON("CacheTestMaterial");
    std::filesystem::path materialPath = resourcesDir / "cache_test.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
    
    std::ifstream cacheFile(info.importedPath);
    std::string cacheContent((std::istreambuf_iterator<char>(cacheFile)),
                            std::istreambuf_iterator<char>());
    cacheFile.close();
    
    EXPECT_FALSE(cacheContent.empty());
    EXPECT_NE(cacheContent.find("CacheTestMaterial"), std::string::npos);
}

TEST_F(MaterialImporterTest, Metadata)
{
    std::string materialJSON = TestData::createMaterialJSON("MetadataTestMaterial");
    std::filesystem::path materialPath = resourcesDir / "metadata_test.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
    
    EXPECT_EQ(info.sourceFileSize, std::filesystem::file_size(materialPath));
    
    EXPECT_EQ(info.importedFileSize, std::filesystem::file_size(info.importedPath));
}

TEST_F(MaterialImporterTest, MinimalMaterial)
{
    std::string materialJSON = R"({
        "name": "MinimalMaterial"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "minimal.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Material);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(MaterialImporterTest, MaterialWithoutTextures)
{
    std::string materialJSON = R"({
        "name": "NoTexturesMaterial",
        "albedoColor": [1.0, 1.0, 1.0, 1.0],
        "roughness": 0.5,
        "metallic": 0.0
    })";
    
    std::filesystem::path materialPath = resourcesDir / "no_textures.lmat";
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    
    EXPECT_TRUE(info.dependencies.empty());
}

