#include <gtest/gtest.h>
#include <Modules/ResourceModule/RWorld.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class RWorldTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("RWorldTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(RWorldTest, ConstructorWithFilePath)
{
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "test.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    EXPECT_FALSE(world.getJsonData().empty());
    EXPECT_NE(world.getJsonData().find("TestWorld"), std::string::npos);
}

TEST_F(RWorldTest, LoadWorldFromJSON)
{
    std::string worldJSON = R"({
        "name": "MyWorld",
        "entities": [
            {"id": 1, "position": [0, 0, 0]},
            {"id": 2, "position": [1, 1, 1]}
        ]
    })";
    
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "world.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_FALSE(jsonData.empty());
    EXPECT_NE(jsonData.find("MyWorld"), std::string::npos);
    EXPECT_NE(jsonData.find("entities"), std::string::npos);
}

TEST_F(RWorldTest, MissingFile)
{
    RWorld world("nonexistent/world.worldbin");
    
    // Resource should be empty but not crash
    EXPECT_TRUE(world.isEmpty());
    EXPECT_FALSE(world.isValid());
    EXPECT_TRUE(world.getJsonData().empty());
}

TEST_F(RWorldTest, InvalidJSON)
{
    std::string invalidJSON = "This is not valid JSON { invalid syntax";
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "invalid.worldbin", invalidJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_FALSE(jsonData.empty());
    EXPECT_EQ(jsonData, invalidJSON);
}

TEST_F(RWorldTest, LoadRealWorld)
{
    std::string worldJSON = R"({
        "name": "RealWorld",
        "entities": [
            {
                "id": "entity1",
                "type": "Mesh",
                "position": [0.0, 0.0, 0.0],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0]
            },
            {
                "id": "entity2",
                "type": "Light",
                "position": [5.0, 5.0, 5.0],
                "color": [1.0, 1.0, 1.0]
            }
        ],
        "settings": {
            "gravity": 9.81,
            "ambientLight": [0.2, 0.2, 0.2]
        }
    })";
    
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "real.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_FALSE(jsonData.empty());
    EXPECT_NE(jsonData.find("RealWorld"), std::string::npos);
    EXPECT_NE(jsonData.find("entity1"), std::string::npos);
    EXPECT_NE(jsonData.find("entity2"), std::string::npos);
}

TEST_F(RWorldTest, WorldStructure)
{
    std::string worldJSON = TestData::createWorldJSON("StructureTest");
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "structure.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_FALSE(jsonData.empty());
    EXPECT_NE(jsonData.find("name"), std::string::npos);
    EXPECT_NE(jsonData.find("entities"), std::string::npos);
}

TEST_F(RWorldTest, EmptyWorld)
{
    std::string worldJSON = "{}";
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "empty.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_EQ(jsonData, worldJSON);
}

TEST_F(RWorldTest, LargeWorld)
{
    std::stringstream ss;
    ss << "{\n\"name\": \"LargeWorld\",\n\"entities\": [\n";
    for (int i = 0; i < 100; ++i)
    {
        ss << "{\"id\": " << i << ", \"position\": [" << i << ", " << i << ", " << i << "]},\n";
    }
    ss << "]\n}";
    
    std::string worldJSON = ss.str();
    std::filesystem::path worldPath = TestData::createWorldBinaryFile(*tempDir, "large.worldbin", worldJSON);
    
    RWorld world(worldPath.string());
    
    const std::string& jsonData = world.getJsonData();
    EXPECT_EQ(jsonData.size(), worldJSON.size());
    EXPECT_FALSE(jsonData.empty());
}

TEST_F(RWorldTest, CorruptedFile)
{
    std::filesystem::path corruptedPath = tempDir->path() / "corrupted.worldbin";
    std::ofstream file(corruptedPath, std::ios::binary);
    
    uint32_t wrongSize = 10000;
    file.write(reinterpret_cast<const char*>(&wrongSize), sizeof(uint32_t));
    file.write("short", 5);
    file.close();
    
    RWorld world(corruptedPath.string());
    
    // Resource should be empty but not crash
    EXPECT_TRUE(world.isEmpty());
    EXPECT_FALSE(world.isValid());
    EXPECT_TRUE(world.getJsonData().empty());
}

