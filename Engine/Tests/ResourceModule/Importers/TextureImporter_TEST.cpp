#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Importers/TextureImporter.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class TextureImporterTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("TextureImporterTest");
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
    
    std::filesystem::path createSimplePNG(const std::string& name)
    {
        std::filesystem::path pngPath = resourcesDir / name;
        std::ofstream file(pngPath, std::ios::binary);
        
        std::vector<uint8_t> pngHeader = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
        };
        file.write(reinterpret_cast<const char*>(pngHeader.data()), pngHeader.size());
        file.close();
        
        return pngPath;
    }
};

TEST_F(TextureImporterTest, SupportsExtension)
{
    TextureImporter importer;
    
    EXPECT_TRUE(importer.supportsExtension(".png"));
    EXPECT_TRUE(importer.supportsExtension(".jpg"));
    EXPECT_TRUE(importer.supportsExtension(".jpeg"));
    EXPECT_FALSE(importer.supportsExtension(".lmat"));
    EXPECT_FALSE(importer.supportsExtension(".obj"));
}

TEST_F(TextureImporterTest, GetAssetType)
{
    TextureImporter importer;
    EXPECT_EQ(importer.getAssetType(), AssetType::Texture);
}

TEST_F(TextureImporterTest, DISABLED_Import)
{
}

TEST_F(TextureImporterTest, DISABLED_CreateAssetInfo)
{
}

TEST_F(TextureImporterTest, DISABLED_GenerateGUID)
{
}

TEST_F(TextureImporterTest, MissingFile)
{
    TextureImporter importer;
    std::filesystem::path missingPath = resourcesDir / "nonexistent.png";
    
    EXPECT_THROW({
        importer.import(missingPath, cacheDir);
    }, std::runtime_error);
}

TEST_F(TextureImporterTest, DISABLED_CorruptedImage)
{
}

TEST_F(TextureImporterTest, DISABLED_ImportRealTexture)
{
}

TEST_F(TextureImporterTest, DISABLED_DifferentFormats)
{
}

TEST_F(TextureImporterTest, DISABLED_CacheFile)
{
}

