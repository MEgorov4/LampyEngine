#include <gtest/gtest.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include "../TestHelpers.h"
#include <filesystem>
#include <fstream>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class MaterialTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("MaterialTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(MaterialTest, DefaultConstructor)
{
    RMaterial material;
    
    EXPECT_EQ(material.name, "DefaultMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.r, 0.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.g, 0.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material.roughness, 0.5f);
    EXPECT_FLOAT_EQ(material.metallic, 0.0f);
    EXPECT_FLOAT_EQ(material.normalStrength, 1.0f);
    EXPECT_TRUE(material.albedoTexture.empty());
    EXPECT_TRUE(material.normalTexture.empty());
    EXPECT_TRUE(material.roughnessMetallicTexture.empty());
    EXPECT_TRUE(material.emissiveTexture.empty());
    EXPECT_TRUE(material.materialID.empty());
}

TEST_F(MaterialTest, ConstructorWithFilePath)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial", {1.0f, 0.5f, 0.0f, 1.0f}, 0.7f, 0.3f);
    std::filesystem::path materialPath = tempDir->createFile("test.lmat", materialJSON);
    
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "TestMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material.roughness, 0.7f);
    EXPECT_FLOAT_EQ(material.metallic, 0.3f);
}

TEST_F(MaterialTest, LoadFromJSON)
{
    std::string materialJSON = R"({
        "name": "LoadedMaterial",
        "albedoColor": [0.2, 0.4, 0.8, 1.0],
        "emissiveColor": [0.1, 0.2, 0.3],
        "roughness": 0.8,
        "metallic": 0.5,
        "normalStrength": 2.0
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("loaded.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "LoadedMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 0.2f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.4f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.8f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.r, 0.1f);
    EXPECT_FLOAT_EQ(material.emissiveColor.g, 0.2f);
    EXPECT_FLOAT_EQ(material.emissiveColor.b, 0.3f);
    EXPECT_FLOAT_EQ(material.roughness, 0.8f);
    EXPECT_FLOAT_EQ(material.metallic, 0.5f);
    EXPECT_FLOAT_EQ(material.normalStrength, 2.0f);
}

TEST_F(MaterialTest, MaterialParameters)
{
    std::string materialJSON = R"({
        "albedoColor": [1.0, 0.0, 0.0, 0.5],
        "emissiveColor": [0.5, 0.0, 0.0],
        "roughness": 0.25,
        "metallic": 0.75,
        "normalStrength": 1.5
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("params.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 0.5f);
    EXPECT_FLOAT_EQ(material.emissiveColor.r, 0.5f);
    EXPECT_FLOAT_EQ(material.emissiveColor.g, 0.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material.roughness, 0.25f);
    EXPECT_FLOAT_EQ(material.metallic, 0.75f);
    EXPECT_FLOAT_EQ(material.normalStrength, 1.5f);
}

TEST_F(MaterialTest, MaterialTextures)
{
    std::string materialJSON = R"({
        "name": "TexturedMaterial",
        "albedoTexture": "textures/albedo.png",
        "normalTexture": "textures/normal.png",
        "roughnessMetallicTexture": "textures/roughness_metallic.png",
        "emissiveTexture": "textures/emissive.png"
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("textured.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_FALSE(material.albedoTexture.empty());
    EXPECT_FALSE(material.normalTexture.empty());
    EXPECT_FALSE(material.roughnessMetallicTexture.empty());
    EXPECT_FALSE(material.emissiveTexture.empty());
    
    AssetID albedoId("textures/albedo.png");
    AssetID normalId("textures/normal.png");
    
    EXPECT_EQ(material.albedoTexture, albedoId);
    EXPECT_EQ(material.normalTexture, normalId);
}

TEST_F(MaterialTest, MissingFile)
{
    RMaterial material("nonexistent/file.lmat");
    
    EXPECT_EQ(material.name, "DefaultMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 1.0f);
    EXPECT_FLOAT_EQ(material.roughness, 0.5f);
    EXPECT_FLOAT_EQ(material.metallic, 0.0f);
}

TEST_F(MaterialTest, InvalidJSON)
{
    std::string invalidJSON = "This is not valid JSON { invalid syntax";
    std::filesystem::path materialPath = tempDir->createFile("invalid.lmat", invalidJSON);
    
    try
    {
        RMaterial material(materialPath.string());
        EXPECT_EQ(material.name, "DefaultMaterial");
    }
    catch (const std::exception&)
    {
    }
}

// MaterialID
TEST_F(MaterialTest, MaterialID)
{
    std::string materialJSON = R"({
        "name": "MaterialWithID",
        "guid": "123e4567-e89b-12d3-a456-426655440000"
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("with_id.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_FALSE(material.materialID.empty());
    EXPECT_EQ(material.materialID.str(), "123e4567-e89b-12d3-a456-426655440000");
}

TEST_F(MaterialTest, LoadRealMaterial)
{
    std::string materialJSON = R"({
        "name": "RealMaterial",
        "albedoColor": [0.8, 0.2, 0.2, 1.0],
        "emissiveColor": [0.1, 0.0, 0.0],
        "roughness": 0.6,
        "metallic": 0.2,
        "normalStrength": 1.0,
        "albedoTexture": "textures/diffuse.png",
        "normalTexture": "textures/normal.png"
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("real.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "RealMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 0.8f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.2f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.2f);
    EXPECT_FLOAT_EQ(material.roughness, 0.6f);
    EXPECT_FLOAT_EQ(material.metallic, 0.2f);
    EXPECT_FALSE(material.albedoTexture.empty());
    EXPECT_FALSE(material.normalTexture.empty());
}

TEST_F(MaterialTest, CreateTemporaryMaterial)
{
    std::string materialJSON = TestData::createMaterialJSON("TempMaterial", {0.5f, 0.5f, 0.5f, 1.0f}, 0.5f, 0.0f);
    std::filesystem::path materialPath = tempDir->createFile("temp.lmat", materialJSON);
    
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "TempMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 0.5f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.5f);
}

TEST_F(MaterialTest, AllParameters)
{
    std::string materialJSON = R"({
        "name": "FullMaterial",
        "albedoColor": [1.0, 1.0, 1.0, 1.0],
        "emissiveColor": [0.5, 0.5, 0.5],
        "roughness": 0.0,
        "metallic": 1.0,
        "normalStrength": 0.5,
        "albedoTexture": "albedo.png",
        "normalTexture": "normal.png",
        "roughnessMetallicTexture": "roughness_metallic.png",
        "emissiveTexture": "emissive.png",
        "guid": "test-guid-123"
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("full.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "FullMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material.emissiveColor.r, 0.5f);
    EXPECT_FLOAT_EQ(material.emissiveColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material.emissiveColor.b, 0.5f);
    EXPECT_FLOAT_EQ(material.roughness, 0.0f);
    EXPECT_FLOAT_EQ(material.metallic, 1.0f);
    EXPECT_FLOAT_EQ(material.normalStrength, 0.5f);
    EXPECT_FALSE(material.albedoTexture.empty());
    EXPECT_FALSE(material.normalTexture.empty());
    EXPECT_FALSE(material.roughnessMetallicTexture.empty());
    EXPECT_FALSE(material.emissiveTexture.empty());
}

TEST_F(MaterialTest, MinimalMaterial)
{
    std::string materialJSON = R"({
        "name": "MinimalMaterial"
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("minimal.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_EQ(material.name, "MinimalMaterial");
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.roughness, 0.5f);
    EXPECT_FLOAT_EQ(material.metallic, 0.0f);
}

TEST_F(MaterialTest, AlbedoColorWithoutAlpha)
{
    std::string materialJSON = R"({
        "albedoColor": [1.0, 0.5, 0.0]
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("no_alpha.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_FLOAT_EQ(material.albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material.albedoColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material.albedoColor.a, 1.0f);
}

TEST_F(MaterialTest, EmptyTextures)
{
    std::string materialJSON = R"({
        "name": "NoTextures",
        "albedoTexture": "",
        "normalTexture": ""
    })";
    
    std::filesystem::path materialPath = tempDir->createFile("no_textures.lmat", materialJSON);
    RMaterial material(materialPath.string());
    
    EXPECT_TRUE(material.albedoTexture.empty());
    EXPECT_TRUE(material.normalTexture.empty());
}

