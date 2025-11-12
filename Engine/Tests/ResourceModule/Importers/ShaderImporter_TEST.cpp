#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/ShaderImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ShaderImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ShaderImporterTest");
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

TEST_F(ShaderImporterTest, SupportsExtension)
{
    ShaderImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".vert"));
    EXPECT_TRUE(importer.supportsExtension(".frag"));
    EXPECT_FALSE(importer.supportsExtension(".png"));
    EXPECT_FALSE(importer.supportsExtension(".lmat"));
}

TEST_F(ShaderImporterTest, GetAssetType)
{
    ShaderImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::Shader);
}

TEST_F(ShaderImporterTest, Import)
{
    std::string shaderContent = TestData::createVertexShader();
    std::filesystem::path shaderPath = resourcesDir / "test.vert";
    std::ofstream file(shaderPath);
    file << shaderContent;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info = importer.import(shaderPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Shader);
    std::string shaderPathStr = shaderPath.string();
    EXPECT_EQ(info.sourcePath, shaderPathStr);
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(ShaderImporterTest, CreateAssetInfo)
{
    std::string shaderContent = TestData::createFragmentShader();
    std::filesystem::path shaderPath = resourcesDir / "test.frag";
    std::ofstream file(shaderPath);
    file << shaderContent;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info = importer.import(shaderPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Shader);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
    EXPECT_GT(info.sourceTimestamp, 0);
    EXPECT_GT(info.importedTimestamp, 0);
    EXPECT_GT(info.sourceFileSize, 0);
    EXPECT_GT(info.importedFileSize, 0);
}

TEST_F(ShaderImporterTest, GenerateGUID)
{
    std::string shaderContent = TestData::createVertexShader();
    std::filesystem::path shaderPath = resourcesDir / "test.vert";
    std::ofstream file(shaderPath);
    file << shaderContent;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info1 = importer.import(shaderPath, cacheDir);
    AssetInfo info2 = importer.import(shaderPath, cacheDir);
    
    EXPECT_EQ(info1.guid, info2.guid);
}

TEST_F(ShaderImporterTest, MissingFile)
{
    ShaderImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.vert";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(ShaderImporterTest, ImportRealShaders)
{
    std::string vertShader = TestData::createVertexShader("#version 330 core");
    std::string fragShader = TestData::createFragmentShader("#version 330 core");
    
    std::filesystem::path vertPath = resourcesDir / "real.vert";
    std::filesystem::path fragPath = resourcesDir / "real.frag";
    
    std::ofstream vertFile(vertPath);
    vertFile << vertShader;
    vertFile.close();
    
    std::ofstream fragFile(fragPath);
    fragFile << fragShader;
    fragFile.close();
    
    ShaderImporter importer;
    
    AssetInfo vertInfo = importer.import(vertPath, cacheDir);
    AssetInfo fragInfo = importer.import(fragPath, cacheDir);
    
    EXPECT_FALSE(vertInfo.guid.empty());
    EXPECT_FALSE(fragInfo.guid.empty());
    EXPECT_EQ(vertInfo.type, AssetType::Shader);
    EXPECT_EQ(fragInfo.type, AssetType::Shader);
    EXPECT_TRUE(std::filesystem::exists(vertInfo.importedPath));
    EXPECT_TRUE(std::filesystem::exists(fragInfo.importedPath));
}

TEST_F(ShaderImporterTest, CacheFile)
{
    std::string shaderContent = TestData::createVertexShader();
    std::filesystem::path shaderPath = resourcesDir / "cache_test.vert";
    std::ofstream file(shaderPath);
    file << shaderContent;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info = importer.import(shaderPath, cacheDir);
    
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
    EXPECT_GT(std::filesystem::file_size(info.importedPath), 0);
    
    std::ifstream cacheFile(info.importedPath);
    std::string cacheContent((std::istreambuf_iterator<char>(cacheFile)),
                            std::istreambuf_iterator<char>());
    cacheFile.close();
    
    EXPECT_EQ(cacheContent, shaderContent);
}

TEST_F(ShaderImporterTest, ImportVertexShader)
{
    std::string vertShader = "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }";
    std::filesystem::path vertPath = resourcesDir / "vertex.vert";
    std::ofstream file(vertPath);
    file << vertShader;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info = importer.import(vertPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Shader);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

TEST_F(ShaderImporterTest, ImportFragmentShader)
{
    std::string fragShader = "#version 330 core\nout vec4 FragColor; void main() { FragColor = vec4(1.0); }";
    std::filesystem::path fragPath = resourcesDir / "fragment.frag";
    std::ofstream file(fragPath);
    file << fragShader;
    file.close();
    
    ShaderImporter importer;
    AssetInfo info = importer.import(fragPath, cacheDir);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Shader);
    EXPECT_TRUE(std::filesystem::exists(info.importedPath));
}

