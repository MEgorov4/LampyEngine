#include <gtest/gtest.h>
#include <Modules/ResourceModule/Script.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ScriptTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ScriptTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
};

TEST_F(ScriptTest, ConstructorWithFilePath)
{
    std::string scriptContent = "return { value = 42 }";
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "test.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    EXPECT_FALSE(script.getSource().empty());
    EXPECT_EQ(script.getSource(), scriptContent);
}

TEST_F(ScriptTest, LoadLuaScript)
{
    std::string scriptContent = TestData::createLuaScript("local x = 10\nreturn x * 2");
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "test.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_FALSE(source.empty());
    EXPECT_NE(source.find("x = 10"), std::string::npos);
}

TEST_F(ScriptTest, GetSource)
{
    std::string scriptContent = "function test()\n    return true\nend";
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "test.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_EQ(source, scriptContent);
    EXPECT_NE(source.find("function test"), std::string::npos);
}

TEST_F(ScriptTest, MissingFile)
{
    EXPECT_THROW({
        RScript script("nonexistent/script.luabin");
    }, std::runtime_error);
}

TEST_F(ScriptTest, LoadRealScript)
{
    std::string scriptContent = R"(local function calculate(a, b)
    return a + b
end

return {
    calculate = calculate,
    version = "1.0.0"
})";
    
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "real.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_FALSE(source.empty());
    EXPECT_NE(source.find("calculate"), std::string::npos);
    EXPECT_NE(source.find("version"), std::string::npos);
}

TEST_F(ScriptTest, CodeCorrectness)
{
    std::string scriptContent = "local x = 5\nlocal y = 10\nreturn x + y";
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "correct.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_EQ(source, scriptContent);
    EXPECT_NE(source.find("x = 5"), std::string::npos);
    EXPECT_NE(source.find("y = 10"), std::string::npos);
}

TEST_F(ScriptTest, EmptyScript)
{
    std::string scriptContent = "";
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "empty.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_TRUE(source.empty());
}

TEST_F(ScriptTest, LargeScript)
{
    std::string scriptContent;
    for (int i = 0; i < 1000; ++i)
    {
        scriptContent += "local var" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    }
    
    std::filesystem::path scriptPath = TestData::createScriptBinaryFile(*tempDir, "large.luabin", scriptContent);
    
    RScript script(scriptPath.string());
    
    const std::string& source = script.getSource();
    EXPECT_EQ(source.size(), scriptContent.size());
    EXPECT_FALSE(source.empty());
}

TEST_F(ScriptTest, CorruptedFile)
{
    std::filesystem::path corruptedPath = tempDir->path() / "corrupted.luabin";
    std::ofstream file(corruptedPath, std::ios::binary);
    
    uint32_t wrongSize = 1000;
    file.write(reinterpret_cast<const char*>(&wrongSize), sizeof(uint32_t));
    file.write("short", 5);
    file.close();
    
    EXPECT_THROW({
        RScript script(corruptedPath.string());
    }, std::runtime_error);
}

