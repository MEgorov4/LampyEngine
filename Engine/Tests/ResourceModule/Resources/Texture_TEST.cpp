#include <gtest/gtest.h>
#include <Modules/ResourceModule/Texture.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class TextureTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("TextureTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(TextureTest, ConstructorWithFilePath)
{
    std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, "test.texbin", 64, 64, 4);
    
    RTexture texture(texbinPath.string());
    
    const auto& info = texture.getInfo();
    EXPECT_EQ(info.width, 64);
    EXPECT_EQ(info.height, 64);
    EXPECT_EQ(info.channels, 4);
    EXPECT_EQ(info.pixels.size(), 64 * 64 * 4);
}

TEST_F(TextureTest, LoadTextureInfo)
{
    std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, "texture.texbin", 128, 128, 4);
    
    RTexture texture(texbinPath.string());
    
    const auto& info = texture.getInfo();
    EXPECT_EQ(info.width, 128);
    EXPECT_EQ(info.height, 128);
    EXPECT_EQ(info.channels, 4);
    EXPECT_FALSE(info.pixels.empty());
}

TEST_F(TextureTest, TextureDimensions)
{
    std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, "dimensions.texbin", 256, 512, 4);
    
    RTexture texture(texbinPath.string());
    
    const auto& info = texture.getInfo();
    EXPECT_EQ(info.width, 256);
    EXPECT_EQ(info.height, 512);
    EXPECT_EQ(info.channels, 4);
}

TEST_F(TextureTest, MissingFile)
{
    EXPECT_THROW({
        RTexture texture("nonexistent/texture.texbin");
    }, std::runtime_error);
}

TEST_F(TextureTest, CorruptedFile)
{
    std::filesystem::path corruptedPath = tempDir->createFile("corrupted.texbin", "This is not a valid texbin file");
    
    EXPECT_THROW({
        RTexture texture(corruptedPath.string());
    }, std::runtime_error);
}

TEST_F(TextureTest, LoadRealTexture)
{
    std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, "real.texbin", 1024, 1024, 4);
    
    RTexture texture(texbinPath.string());
    
    const auto& info = texture.getInfo();
    EXPECT_EQ(info.width, 1024);
    EXPECT_EQ(info.height, 1024);
    EXPECT_EQ(info.channels, 4);
    EXPECT_EQ(info.pixels.size(), 1024 * 1024 * 4);
}

TEST_F(TextureTest, PixelData)
{
    std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, "pixels.texbin", 2, 2, 4);
    
    RTexture texture(texbinPath.string());
    
    const auto& info = texture.getInfo();
    EXPECT_EQ(info.pixels.size(), 2 * 2 * 4);
    
    for (size_t i = 0; i < info.pixels.size(); ++i)
    {
        EXPECT_EQ(info.pixels[i], 255);
    }
}

TEST_F(TextureTest, VariousSizes)
{
    struct SizeTest
    {
        int width, height;
    };
    
    SizeTest sizes[] = {
        {1, 1},
        {32, 32},
        {256, 256},
        {512, 256},
        {128, 512}
    };
    
    for (const auto& size : sizes)
    {
        std::string name = "texture_" + std::to_string(size.width) + "x" + std::to_string(size.height) + ".texbin";
        std::filesystem::path texbinPath = TestData::createTexbinFile(*tempDir, name, size.width, size.height, 4);
        
        RTexture texture(texbinPath.string());
        
        const auto& info = texture.getInfo();
        EXPECT_EQ(info.width, size.width);
        EXPECT_EQ(info.height, size.height);
        EXPECT_EQ(info.pixels.size(), size.width * size.height * 4);
    }
}

