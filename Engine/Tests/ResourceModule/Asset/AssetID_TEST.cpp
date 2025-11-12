#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <unordered_map>
#include <filesystem>
#include "../TestHelpers.h"

using namespace ResourceModule;
using namespace ResourceModuleTest;

class AssetIDTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
    }
    // TearDown from ResourceModuleTestBase will clear static caches and shutdown MemorySystem
};

TEST_F(AssetIDTest, DefaultConstructor)
{
    AssetID id;
    EXPECT_TRUE(id.empty());
    EXPECT_EQ(id.str(), "00000000-0000-0000-0000-000000000000");
}

TEST_F(AssetIDTest, FromUUIDString)
{
    const std::string uuidStr = "123e4567-e89b-12d3-a456-426655440000";
    AssetID id(uuidStr);
    
    EXPECT_FALSE(id.empty());
    EXPECT_EQ(id.str(), uuidStr);
    
    AssetID id2(uuidStr);
    EXPECT_EQ(id, id2);
}

TEST_F(AssetIDTest, FromAbsolutePath)
{
    std::filesystem::path absPath = std::filesystem::absolute("test/path/file.txt");
    AssetID id1(absPath.string());
    AssetID id2(absPath.string());
    
    EXPECT_EQ(id1, id2);
    EXPECT_FALSE(id1.empty());
}

TEST_F(AssetIDTest, FromRelativePath)
{
    const std::string relPath = "Materials/test.lmat";
    AssetID id1(relPath);
    AssetID id2(relPath);
    
    EXPECT_EQ(id1, id2);
    EXPECT_FALSE(id1.empty());
}

TEST_F(AssetIDTest, DeterministicGeneration)
{
    const std::string path1 = "Materials/default_material.lmat";
    const std::string path2 = "Materials/default_material.lmat";
    
    AssetID id1(path1);
    AssetID id2(path2);
    
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(id1.str(), id2.str());
}

TEST_F(AssetIDTest, StringConversion)
{
    const std::string uuidStr = "123e4567-e89b-12d3-a456-426655440000";
    AssetID id(uuidStr);
    
    std::string result = id.str();
    EXPECT_EQ(result, uuidStr);
    EXPECT_EQ(result.size(), 36);
}

TEST_F(AssetIDTest, EmptyCheck)
{
    AssetID emptyId;
    EXPECT_TRUE(emptyId.empty());
    
    AssetID nonEmptyId("test/path");
    EXPECT_FALSE(nonEmptyId.empty());
    
    AssetID uuidId("123e4567-e89b-12d3-a456-426655440000");
    EXPECT_FALSE(uuidId.empty());
}

TEST_F(AssetIDTest, EqualityOperators)
{
    AssetID id1("test/path");
    AssetID id2("test/path");
    AssetID id3("other/path");
    
    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
    EXPECT_TRUE(id1 == id2);
    EXPECT_FALSE(id1 != id2);
    EXPECT_TRUE(id1 != id3);
    EXPECT_FALSE(id1 == id3);
}

TEST_F(AssetIDTest, Hasher)
{
    std::unordered_map<AssetID, int, AssetID::Hasher> map;
    
    AssetID id1("test/path1");
    AssetID id2("test/path2");
    AssetID id3("test/path1");
    
    map[id1] = 1;
    map[id2] = 2;
    map[id3] = 3;
    
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map[id1], 3);
    EXPECT_EQ(map[id2], 2);
}

TEST_F(AssetIDTest, PathNormalization)
{
    std::string winPath = "Materials\\test.lmat";
    std::string unixPath = "Materials/test.lmat";
    
#ifdef _WIN32
    AssetID id1(winPath);
    AssetID id2(unixPath);
    EXPECT_EQ(id1, id2);
#else
    AssetID id1(winPath);
    AssetID id2(unixPath);
    EXPECT_EQ(id1, id2);
#endif
}

TEST_F(AssetIDTest, EmptyString)
{
    AssetID id1("");
    AssetID id2;
    
    EXPECT_TRUE(id1.empty());
    EXPECT_TRUE(id2.empty());
    EXPECT_EQ(id1, id2);
}

TEST_F(AssetIDTest, InvalidUUID)
{
    const std::string invalidUUID = "123e4567-e89b-12d3-a456-42665544xxxx";
    
    AssetID id(invalidUUID);
    EXPECT_FALSE(id.empty());
    
    AssetID id2(invalidUUID);
    EXPECT_EQ(id, id2);
}

TEST_F(AssetIDTest, DifferentPathsDifferentGUIDs)
{
    AssetID id1("Materials/material1.lmat");
    AssetID id2("Materials/material2.lmat");
    AssetID id3("Textures/texture.png");
    
    EXPECT_NE(id1, id2);
    EXPECT_NE(id1, id3);
    EXPECT_NE(id2, id3);
}

TEST_F(AssetIDTest, CaseSensitivity)
{
    std::string path1 = "Materials/test.lmat";
    std::string path2 = "MATERIALS/TEST.LMAT";
    
#ifdef _WIN32
    AssetID id1(path1);
    AssetID id2(path2);
    EXPECT_EQ(id1, id2);
#else
    AssetID id1(path1);
    AssetID id2(path2);
#endif
}

// MakeDeterministicIDFromPath
TEST_F(AssetIDTest, MakeDeterministicIDFromPath)
{
    const std::string path = "test/path/file.txt";
    AssetID id1 = MakeDeterministicIDFromPath(path);
    AssetID id2 = MakeDeterministicIDFromPath(path);
    
    EXPECT_EQ(id1, id2);
    EXPECT_FALSE(id1.empty());
    
    AssetID emptyId = MakeDeterministicIDFromPath("");
    EXPECT_TRUE(emptyId.empty());
}

TEST_F(AssetIDTest, ContainerUsage)
{
    std::vector<AssetID> ids;
    ids.push_back(AssetID("path1"));
    ids.push_back(AssetID("path2"));
    ids.push_back(AssetID("path3"));
    
    EXPECT_EQ(ids.size(), 3);
    EXPECT_NE(ids[0], ids[1]);
    EXPECT_NE(ids[1], ids[2]);
    
    auto it = std::find(ids.begin(), ids.end(), AssetID("path2"));
    EXPECT_NE(it, ids.end());
    EXPECT_EQ(*it, ids[1]);
}

