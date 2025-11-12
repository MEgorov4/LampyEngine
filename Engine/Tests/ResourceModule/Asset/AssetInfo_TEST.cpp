#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <nlohmann/json.hpp>
#include "../TestHelpers.h"

using namespace ResourceModule;
using namespace ResourceModuleTest;

class AssetInfoTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
    }
    // TearDown from ResourceModuleTestBase will clear static caches and shutdown MemorySystem
    
    AssetInfo createValidAssetInfo()
    {
        AssetInfo info;
        info.guid = AssetID("test/path/file.txt");
        info.type = AssetType::Material;
        info.origin = AssetOrigin::Project;
        info.sourcePath = "test/path/file.txt";
        info.importedPath = "cache/file.bin";
        info.sourceTimestamp = 1234567890;
        info.importedTimestamp = 1234567891;
        info.sourceFileSize = 1024;
        info.importedFileSize = 2048;
        return info;
    }
};

TEST_F(AssetInfoTest, JSONSerialization)
{
    AssetInfo info = createValidAssetInfo();
    info.dependencies.push_back("dep1");
    info.dependencies.push_back("dep2");
    
    nlohmann::json j;
    to_json(j, info);
    
    EXPECT_EQ(j["guid"], info.guid.str());
    EXPECT_EQ(j["type"], static_cast<int>(AssetType::Material));
    EXPECT_EQ(j["origin"], static_cast<int>(AssetOrigin::Project));
    EXPECT_EQ(j["source"], info.sourcePath);
    EXPECT_EQ(j["imported"], info.importedPath);
    EXPECT_EQ(j["dependencies"].size(), 2);
    EXPECT_EQ(j["sourceTimestamp"], info.sourceTimestamp);
    EXPECT_EQ(j["importedTimestamp"], info.importedTimestamp);
    EXPECT_EQ(j["sourceFileSize"], info.sourceFileSize);
    EXPECT_EQ(j["importedFileSize"], info.importedFileSize);
}

TEST_F(AssetInfoTest, JSONDeserialization)
{
    nlohmann::json j = {
        {"guid", "123e4567-e89b-12d3-a456-426655440000"},
        {"type", static_cast<int>(AssetType::Texture)},
        {"origin", static_cast<int>(AssetOrigin::Engine)},
        {"source", "textures/test.png"},
        {"imported", "cache/test.texbin"},
        {"dependencies", {"tex1", "tex2"}},
        {"sourceTimestamp", 1234567890},
        {"importedTimestamp", 1234567891},
        {"sourceFileSize", 1024},
        {"importedFileSize", 2048}
    };
    
    AssetInfo info;
    from_json(j, info);
    
    EXPECT_EQ(info.guid.str(), "123e4567-e89b-12d3-a456-426655440000");
    EXPECT_EQ(info.type, AssetType::Texture);
    EXPECT_EQ(info.origin, AssetOrigin::Engine);
    EXPECT_EQ(info.sourcePath, "textures/test.png");
    EXPECT_EQ(info.importedPath, "cache/test.texbin");
    EXPECT_EQ(info.dependencies.size(), 2);
    EXPECT_EQ(info.dependencies[0], "tex1");
    EXPECT_EQ(info.dependencies[1], "tex2");
    EXPECT_EQ(info.sourceTimestamp, 1234567890);
    EXPECT_EQ(info.importedTimestamp, 1234567891);
    EXPECT_EQ(info.sourceFileSize, 1024);
    EXPECT_EQ(info.importedFileSize, 2048);
}

TEST_F(AssetInfoTest, RoundTrip)
{
    AssetInfo original = createValidAssetInfo();
    original.dependencies.push_back("dep1");
    
    nlohmann::json j;
    to_json(j, original);
    
    AssetInfo restored;
    from_json(j, restored);
    
    EXPECT_EQ(original.guid, restored.guid);
    EXPECT_EQ(original.type, restored.type);
    EXPECT_EQ(original.origin, restored.origin);
    EXPECT_EQ(original.sourcePath, restored.sourcePath);
    EXPECT_EQ(original.importedPath, restored.importedPath);
    EXPECT_EQ(original.dependencies.size(), restored.dependencies.size());
    EXPECT_EQ(original.sourceTimestamp, restored.sourceTimestamp);
    EXPECT_EQ(original.importedTimestamp, restored.importedTimestamp);
    EXPECT_EQ(original.sourceFileSize, restored.sourceFileSize);
    EXPECT_EQ(original.importedFileSize, restored.importedFileSize);
}

TEST_F(AssetInfoTest, DifferentAssetTypes)
{
    AssetType types[] = {
        AssetType::Texture,
        AssetType::Mesh,
        AssetType::Shader,
        AssetType::Material,
        AssetType::Script,
        AssetType::World
    };
    
    for (auto type : types)
    {
        AssetInfo info = createValidAssetInfo();
        info.type = type;
        
        nlohmann::json j;
        to_json(j, info);
        
        AssetInfo restored;
        from_json(j, restored);
        
        EXPECT_EQ(info.type, restored.type);
        EXPECT_EQ(restored.type, type);
    }
}

TEST_F(AssetInfoTest, DifferentOrigins)
{
    AssetInfo projectInfo = createValidAssetInfo();
    projectInfo.origin = AssetOrigin::Project;
    
    AssetInfo engineInfo = createValidAssetInfo();
    engineInfo.origin = AssetOrigin::Engine;
    engineInfo.guid = AssetID("engine/path/file.txt");
    engineInfo.sourcePath = "engine/path/file.txt";
    
    nlohmann::json j1, j2;
    to_json(j1, projectInfo);
    to_json(j2, engineInfo);
    
    AssetInfo restored1, restored2;
    from_json(j1, restored1);
    from_json(j2, restored2);
    
    EXPECT_EQ(restored1.origin, AssetOrigin::Project);
    EXPECT_EQ(restored2.origin, AssetOrigin::Engine);
}

TEST_F(AssetInfoTest, Dependencies)
{
    AssetInfo info = createValidAssetInfo();
    info.dependencies.push_back("texture1.png");
    info.dependencies.push_back("texture2.png");
    info.dependencies.push_back("material.lmat");
    
    nlohmann::json j;
    to_json(j, info);
    
    EXPECT_EQ(j["dependencies"].size(), 3);
    EXPECT_EQ(j["dependencies"][0], "texture1.png");
    EXPECT_EQ(j["dependencies"][1], "texture2.png");
    EXPECT_EQ(j["dependencies"][2], "material.lmat");
    
    AssetInfo restored;
    from_json(j, restored);
    
    EXPECT_EQ(restored.dependencies.size(), 3);
    EXPECT_EQ(restored.dependencies[0], "texture1.png");
    EXPECT_EQ(restored.dependencies[1], "texture2.png");
    EXPECT_EQ(restored.dependencies[2], "material.lmat");
}

TEST_F(AssetInfoTest, EmptyDependencies)
{
    AssetInfo info = createValidAssetInfo();
    info.dependencies.clear();
    
    nlohmann::json j;
    to_json(j, info);
    
    EXPECT_TRUE(j["dependencies"].is_array());
    EXPECT_EQ(j["dependencies"].size(), 0);
    
    AssetInfo restored;
    from_json(j, restored);
    
    EXPECT_TRUE(restored.dependencies.empty());
}

TEST_F(AssetInfoTest, TimestampsAndSizes)
{
    AssetInfo info = createValidAssetInfo();
    info.sourceTimestamp = 1000000000;
    info.importedTimestamp = 1000000001;
    info.sourceFileSize = 512;
    info.importedFileSize = 1024;
    
    nlohmann::json j;
    to_json(j, info);
    
    EXPECT_EQ(j["sourceTimestamp"], 1000000000);
    EXPECT_EQ(j["importedTimestamp"], 1000000001);
    EXPECT_EQ(j["sourceFileSize"], 512);
    EXPECT_EQ(j["importedFileSize"], 1024);
    
    AssetInfo restored;
    from_json(j, restored);
    
    EXPECT_EQ(restored.sourceTimestamp, 1000000000);
    EXPECT_EQ(restored.importedTimestamp, 1000000001);
    EXPECT_EQ(restored.sourceFileSize, 512);
    EXPECT_EQ(restored.importedFileSize, 1024);
}

TEST_F(AssetInfoTest, OptionalFields)
{
    nlohmann::json j = {
        {"guid", "123e4567-e89b-12d3-a456-426655440000"},
        {"type", static_cast<int>(AssetType::Material)},
        {"source", "test.lmat"},
        {"imported", "cache/test.bin"}
    };
    
    AssetInfo info;
    from_json(j, info);
    
    EXPECT_EQ(info.sourceTimestamp, 0);
    EXPECT_EQ(info.importedTimestamp, 0);
    EXPECT_EQ(info.sourceFileSize, 0);
    EXPECT_EQ(info.importedFileSize, 0);
}

TEST_F(AssetInfoTest, MinimalValidJSON)
{
    nlohmann::json j = {
        {"guid", "123e4567-e89b-12d3-a456-426655440000"},
        {"type", static_cast<int>(AssetType::Material)},
        {"source", "test.lmat"},
        {"imported", "cache/test.bin"}
    };
    
    AssetInfo info;
    from_json(j, info);
    
    EXPECT_FALSE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Material);
    EXPECT_FALSE(info.sourcePath.empty());
    EXPECT_FALSE(info.importedPath.empty());
}

TEST_F(AssetInfoTest, EmptyImportedPath)
{
    nlohmann::json j = {
        {"guid", "123e4567-e89b-12d3-a456-426655440000"},
        {"type", static_cast<int>(AssetType::Material)},
        {"source", "test.lmat"},
        {"imported", ""}
    };
    
    AssetInfo info;
    from_json(j, info);
    
    EXPECT_TRUE(info.importedPath.empty());
}

TEST_F(AssetInfoTest, DefaultValues)
{
    AssetInfo info;
    
    EXPECT_TRUE(info.guid.empty());
    EXPECT_EQ(info.type, AssetType::Unknown);
    EXPECT_EQ(info.origin, AssetOrigin::Project);
    EXPECT_TRUE(info.sourcePath.empty());
    EXPECT_TRUE(info.importedPath.empty());
    EXPECT_TRUE(info.dependencies.empty());
    EXPECT_EQ(info.sourceTimestamp, 0);
    EXPECT_EQ(info.importedTimestamp, 0);
    EXPECT_EQ(info.sourceFileSize, 0);
    EXPECT_EQ(info.importedFileSize, 0);
}

