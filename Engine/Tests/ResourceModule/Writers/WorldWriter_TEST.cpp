#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/Writers/WorldWriter.h>
#include <Modules/ResourceModule/RWorld.h>
#include "../TestHelpers.h"

using namespace ResourceModule;
using namespace ResourceModuleTest;

class WorldWriterTest : public ResourceModuleTestBase
{
  protected:
    void SetUp() override
    {
        ResourceModuleTestBase::SetUp();
        tempDir = std::make_unique<TempDirectory>("WorldWriterTest");
    }

    void TearDown() override
    {
        tempDir.reset();
        ResourceModuleTestBase::TearDown();
    }

    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(WorldWriterTest, WritesBinaryWorldFile)
{
    const std::string initialJson = TestData::createWorldJSON("InitialWorld");
    auto inputPath = TestData::createWorldBinaryFile(*tempDir, "input.worldbin", initialJson);
    ASSERT_TRUE(std::filesystem::exists(inputPath));

    RWorld world(inputPath.string());
    ASSERT_TRUE(world.isValid());

    const std::string updatedJson = TestData::createWorldJSON("UpdatedWorld");
    world.setJsonData(updatedJson);

    WorldWriter writer;
    WriterContext ctx{};
    auto targetPath = tempDir->path() / "output.worldbin";
    ASSERT_TRUE(writer.write(world, targetPath, ctx));
    ASSERT_TRUE(std::filesystem::exists(targetPath));

    std::ifstream ifs(targetPath, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    uint32_t size = 0;
    ifs.read(reinterpret_cast<char *>(&size), sizeof(size));
    ASSERT_EQ(size, updatedJson.size());
    std::string payload(size, '\0');
    ifs.read(payload.data(), size);
    EXPECT_EQ(payload, updatedJson);
}

TEST_F(WorldWriterTest, FailsOnEmptyData)
{
    const std::string initialJson = TestData::createWorldJSON("InitialWorld");
    auto inputPath = TestData::createWorldBinaryFile(*tempDir, "input.worldbin", initialJson);
    RWorld world(inputPath.string());
    ASSERT_TRUE(world.isValid());

    world.setJsonData("");
    WorldWriter writer;
    WriterContext ctx{};
    auto targetPath = tempDir->path() / "output.worldbin";
    EXPECT_FALSE(writer.write(world, targetPath, ctx));
}



