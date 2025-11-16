#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Writers/MaterialWriter.h>
#include <Modules/ResourceModule/Material.h>
#include "../TestHelpers.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class MaterialWriterTest : public ResourceModuleTestBase
{
  protected:
    void SetUp() override
    {
        ResourceModuleTestBase::SetUp();
        tempDir = std::make_unique<TempDirectory>("MaterialWriterTest");
    }

    void TearDown() override
    {
        tempDir.reset();
        ResourceModuleTestBase::TearDown();
    }

    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(MaterialWriterTest, WritesMaterialJsonFile)
{
    RMaterial material;
    material.name            = "WritableMaterial";
    material.albedoColor     = {0.3f, 0.4f, 0.5f, 0.6f};
    material.emissiveColor   = {0.9f, 0.8f, 0.7f};
    material.roughness       = 0.2f;
    material.metallic        = 0.4f;
    material.normalStrength  = 1.3f;
    material.albedoTexture   = AssetID("Textures/albedo.texbin");
    material.normalTexture   = AssetID("Textures/normal.texbin");
    material.roughnessMetallicTexture = AssetID("Textures/rm.texbin");
    material.emissiveTexture = AssetID("Textures/emissive.texbin");
    material.materialID      = AssetID("material-guid");

    MaterialWriter writer;
    WriterContext ctx{};
    auto targetPath = tempDir->path() / "output.lmat";
    ASSERT_TRUE(writer.write(material, targetPath, ctx));
    
    ASSERT_TRUE(std::filesystem::exists(targetPath));
    std::ifstream ifs(targetPath);
    ASSERT_TRUE(ifs.is_open());
    nlohmann::json payload;
    ifs >> payload;
    ifs.close();

    EXPECT_EQ(payload["name"].get<std::string>(), "WritableMaterial");
    EXPECT_EQ(payload["guid"].get<std::string>(), material.materialID.str());
    auto albedo = payload["albedoColor"];
    ASSERT_EQ(albedo.size(), 4);
    EXPECT_FLOAT_EQ(albedo[0].get<float>(), 0.3f);
    EXPECT_FLOAT_EQ(albedo[3].get<float>(), 0.6f);
    EXPECT_EQ(payload["albedoTexture"].get<std::string>(), material.albedoTexture.str());
    EXPECT_EQ(payload["roughnessMetallicTexture"].get<std::string>(), material.roughnessMetallicTexture.str());
}

TEST_F(MaterialWriterTest, RejectsNonMaterialResource)
{
    struct DummyResource final : BaseResource
    {
        DummyResource() : BaseResource("dummy") {}
    } dummy;

    MaterialWriter writer;
    WriterContext ctx{};
    auto targetPath = tempDir->path() / "invalid.lmat";
    EXPECT_FALSE(writer.write(dummy, targetPath, ctx));
    EXPECT_FALSE(std::filesystem::exists(targetPath));
}


