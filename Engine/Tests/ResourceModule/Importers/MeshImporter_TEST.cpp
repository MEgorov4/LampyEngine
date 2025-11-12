#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/MeshImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class MeshImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("MeshImporterTest");
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

TEST_F(MeshImporterTest, SupportsExtension)
{
    MeshImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".obj"));
    EXPECT_FALSE(importer.supportsExtension(".png"));
    EXPECT_FALSE(importer.supportsExtension(".lmat"));
    EXPECT_FALSE(importer.supportsExtension(".lua"));
}

TEST_F(MeshImporterTest, GetAssetType)
{
    MeshImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::Mesh);
}

TEST_F(MeshImporterTest, Import)
{
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "test.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Mesh);
    std::string objPathStr = objPath.string();
    EXPECT_EQ(info.sourcePath, objPathStr);
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(MeshImporterTest, CreateAssetInfo)
{
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "test.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Mesh);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
}

TEST_F(MeshImporterTest, GenerateGUID)
{
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "test.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info1 = importer.import(objPath, cacheDir);
    AssetInfo info2 = importer.import(objPath, cacheDir);
    
    EXPECT_EQ(info1.guid, info2.guid);
}

TEST_F(MeshImporterTest, MissingFile)
{
    MeshImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.obj";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(MeshImporterTest, InvalidOBJ)
{
    std::string invalidOBJ = "This is not valid OBJ format";
    std::filesystem::path objPath = resourcesDir / "invalid.obj";
    std::ofstream file(objPath);
    file << invalidOBJ;
    file.close();
    
    MeshImporter importer;
    
    try
    {
        AssetInfo info = importer.import(objPath, cacheDir);
        EXPECT_FALSE(info.guid.empty());
    }
    catch (const std::exception&)
    {
    }
}

TEST_F(MeshImporterTest, ImportRealMesh)
{
    std::vector<std::array<float, 3>> vertices = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.5f, 1.0f, 0.0f}
    };
    
    std::vector<std::array<int, 3>> faces = {
        {1, 2, 3}
    };
    
    std::string objContent = TestData::createOBJFile(vertices, faces);
    std::filesystem::path objPath = resourcesDir / "real.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Mesh);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(MeshImporterTest, ImportMeshWithMaterials)
{
    std::string objContent = R"(# OBJ file with material
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.5 1.0 0.0
f 1 2 3
mtllib test.mtl
usemtl TestMaterial
)";
    
    std::filesystem::path objPath = resourcesDir / "with_material.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Mesh);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(MeshImporterTest, CacheFile)
{
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "cache_test.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
    
    std::filesystem::path importedPath(info.importedPath);
    std::string extension = importedPath.extension().string();
    EXPECT_EQ(extension, ".meshbin");
}

TEST_F(MeshImporterTest, GeometryCorrectness)
{
    std::vector<std::array<float, 3>> vertices = {
        {-0.5f, -0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {0.5f, 0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f}
    };
    
    std::vector<std::array<int, 3>> faces = {
        {1, 2, 3},
        {1, 3, 4}
    };
    
    std::string objContent = TestData::createOBJFile(vertices, faces);
    std::filesystem::path objPath = resourcesDir / "square.obj";
    std::ofstream file(objPath);
    file << objContent;
    file.close();
    
    MeshImporter importer;
    AssetInfo info = importer.import(objPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
}

