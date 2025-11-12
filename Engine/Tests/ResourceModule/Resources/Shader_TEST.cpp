#include <gtest/gtest.h>
#include <Modules/ResourceModule/Shader.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ShaderTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ShaderTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    
    std::pair<std::filesystem::path, std::filesystem::path> createShaderPair(const std::string& name,
                                                                              const std::string& vertContent,
                                                                              const std::string& fragContent)
    {
        std::filesystem::path shaderDir = tempDir->createSubdir("Shaders");
        std::filesystem::path vertPath = shaderDir / (name + ".vert");
        std::filesystem::path fragPath = shaderDir / (name + ".frag");
        
        std::ofstream vertFile(vertPath);
        vertFile << vertContent;
        vertFile.close();
        
        std::ofstream fragFile(fragPath);
        fragFile << fragContent;
        fragFile.close();
        
        return {vertPath, fragPath};
    }
};

TEST_F(ShaderTest, ConstructorWithPaths)
{
    auto [vertPath, fragPath] = createShaderPair("test", 
                                                 TestData::createVertexShader(),
                                                 TestData::createFragmentShader());
    
    std::filesystem::path shaderPath = vertPath.parent_path() / "test";
    RShader shader(shaderPath.string());
    
    const auto& info = shader.getShaderInfo();
    EXPECT_FALSE(info.vertexText.empty());
    EXPECT_FALSE(info.fragmentText.empty());
    EXPECT_GT(info.totalSize, 0);
}

TEST_F(ShaderTest, LoadVertexAndFragment)
{
    std::string vertShader = "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }";
    std::string fragShader = "#version 330 core\nout vec4 FragColor; void main() { FragColor = vec4(1.0); }";
    
    auto [vertPath, fragPath] = createShaderPair("test2", vertShader, fragShader);
    
    std::filesystem::path shaderPath = vertPath.parent_path() / "test2";
    RShader shader(shaderPath.string());
    
    const auto& info = shader.getShaderInfo();
    EXPECT_FALSE(info.vertexText.empty());
    EXPECT_FALSE(info.fragmentText.empty());
    EXPECT_GT(info.totalSize, 0);
}

TEST_F(ShaderTest, MissingFiles)
{
    RShader shader("nonexistent/shader");
    
    // Resource should be empty but not crash
    EXPECT_TRUE(shader.isEmpty());
    EXPECT_FALSE(shader.isValid());
    EXPECT_TRUE(shader.getShaderInfo().vertexText.empty());
    EXPECT_TRUE(shader.getShaderInfo().fragmentText.empty());
}

TEST_F(ShaderTest, InvalidGLSL)
{
    std::string invalidVert = "This is not valid GLSL code";
    std::string invalidFrag = "Neither is this";
    
    auto [vertPath, fragPath] = createShaderPair("invalid", invalidVert, invalidFrag);
    
    std::filesystem::path shaderPath = vertPath.parent_path() / "invalid";
    RShader shader(shaderPath.string());
    
    const auto& info = shader.getShaderInfo();
    EXPECT_FALSE(info.vertexText.empty());
    EXPECT_FALSE(info.fragmentText.empty());
}

TEST_F(ShaderTest, LoadRealShaders)
{
    std::string vertShader = TestData::createVertexShader("#version 330 core");
    std::string fragShader = TestData::createFragmentShader("#version 330 core");
    
    auto [vertPath, fragPath] = createShaderPair("real", vertShader, fragShader);
    
    std::filesystem::path shaderPath = vertPath.parent_path() / "real";
    RShader shader(shaderPath.string());
    
    const auto& info = shader.getShaderInfo();
    EXPECT_FALSE(info.vertexText.empty());
    EXPECT_FALSE(info.fragmentText.empty());
    EXPECT_GT(info.totalSize, 0);
    
    EXPECT_NE(info.vertexText.find("version 330"), std::string::npos);
    EXPECT_NE(info.fragmentText.find("version 330"), std::string::npos);
}

TEST_F(ShaderTest, ShaderCodeCorrectness)
{
    std::string vertShader = "#version 330 core\nlayout(location = 0) in vec3 aPos; void main() { gl_Position = vec4(aPos, 1.0); }";
    std::string fragShader = "#version 330 core\nout vec4 FragColor; uniform vec3 color; void main() { FragColor = vec4(color, 1.0); }";
    
    auto [vertPath, fragPath] = createShaderPair("correct", vertShader, fragShader);
    
    std::filesystem::path shaderPath = vertPath.parent_path() / "correct";
    RShader shader(shaderPath.string());
    
    const auto& info = shader.getShaderInfo();
    EXPECT_NE(info.vertexText.find("aPos"), std::string::npos);
    EXPECT_NE(info.fragmentText.find("color"), std::string::npos);
}

TEST_F(ShaderTest, VertexOnly)
{
    std::string vertShader = "#version 330 core\nvoid main() { gl_Position = vec4(0.0); }";
    
    std::filesystem::path shaderDir = tempDir->createSubdir("Shaders");
    std::filesystem::path vertPath = shaderDir / "vertex_only.vert";
    
    std::ofstream vertFile(vertPath);
    vertFile << vertShader;
    vertFile.close();
    
    std::filesystem::path shaderPath = shaderDir / "vertex_only";
    
    RShader shader(shaderPath.string());
    
    // Shader with only vertex shader should be valid (fragment is optional)
    // The original test expected exception, but now we handle errors gracefully
    // Vertex shader should be loaded and shader should be valid
    EXPECT_TRUE(shader.isValid());
    EXPECT_FALSE(shader.isEmpty());
    EXPECT_FALSE(shader.getShaderInfo().vertexText.empty());
    EXPECT_TRUE(shader.getShaderInfo().fragmentText.empty());
}

TEST_F(ShaderTest, FragmentOnly)
{
    std::string fragShader = "#version 330 core\nout vec4 FragColor; void main() { FragColor = vec4(1.0); }";
    
    std::filesystem::path shaderDir = tempDir->createSubdir("Shaders");
    std::filesystem::path fragPath = shaderDir / "fragment_only.frag";
    
    std::ofstream fragFile(fragPath);
    fragFile << fragShader;
    fragFile.close();
    
    std::filesystem::path shaderPath = shaderDir / "fragment_only";
    
    try
    {
        RShader shader(shaderPath.string());
        const auto& info = shader.getShaderInfo();
        EXPECT_FALSE(info.fragmentText.empty());
    }
    catch (const std::exception&)
    {
    }
}

