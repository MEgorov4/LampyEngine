#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Foundation/Memory/MemorySystem.h>

namespace ResourceModuleTest
{

// Base class for all ResourceModule tests that automatically clears static caches between tests
// This prevents memory leaks from accumulating across test runs
class ResourceModuleTestBase : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize MemorySystem before tests run
        // This is required because ResourceModule now uses ResourceAllocator,
        // which requires MemorySystem to be initialized
        using namespace EngineCore::Foundation;
        MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
    }
    
    void TearDown() override
    {
        // Clear all static ResourceManager caches between tests to prevent memory accumulation
        // Static caches persist across test instances, so we need to explicitly clear them
        // This is critical because resources can hold large amounts of memory (meshes, textures, etc.)
        // 
        // IMPORTANT: This only clears weak_ptr entries from static caches.
        // Actual memory is freed when the last shared_ptr to a resource is destroyed.
        // Make sure all local shared_ptr variables in tests are destroyed before TearDown() is called.
        ResourceModule::ResourceManager tempRM;
        
        // First cleanup expired entries (weak_ptr that are no longer valid)
        // This removes entries where the resource has already been destroyed
        tempRM.cleanupExpiredCacheEntries();
        
        // Clear registry first to release shared_ptr references
        // Then clear all caches completely
        tempRM.clearAll();
        
        // Final cleanup pass to remove any remaining expired entries
        tempRM.cleanupExpiredCacheEntries();
        
        // Shutdown MemorySystem after clearing ResourceManager caches
        using namespace EngineCore::Foundation;
        MemorySystem::shutdown();
        
        // Call parent TearDown if needed
        ::testing::Test::TearDown();
    }
};

class TempDirectory
{
public:
    TempDirectory(const std::string& prefix = "ResourceModuleTest");
    ~TempDirectory();
    
    std::filesystem::path path() const { return m_path; }
    std::string pathString() const { return m_path.string(); }
    
    std::filesystem::path createSubdir(const std::string& name);
    
    std::filesystem::path createFile(const std::string& name, const std::string& content);
    
    std::filesystem::path createBinaryFile(const std::string& name, const std::vector<uint8_t>& data);
    
    void cleanup();
    
private:
    std::filesystem::path m_path;
    bool m_cleanup;
};

namespace TestData
{
    std::string createMaterialJSON(
        const std::string& name = "TestMaterial",
        const std::array<float, 4>& albedoColor = {1.0f, 0.0f, 0.0f, 1.0f},
        float roughness = 0.5f,
        float metallic = 0.0f
    );
    
    std::string createOBJFile(
        const std::vector<std::array<float, 3>>& vertices = {},
        const std::vector<std::array<int, 3>>& faces = {}
    );
    
    std::string createVertexShader(const std::string& version = "#version 330 core");
    std::string createFragmentShader(const std::string& version = "#version 330 core");
    
    std::string createLuaScript(const std::string& content = "return {}");
    
    std::string createWorldJSON(const std::string& worldName = "TestWorld");
    
    std::vector<uint8_t> createPNGTexture(int width = 1, int height = 1);
    
    std::filesystem::path createTexbinFile(TempDirectory& tempDir, const std::string& name, 
                                          int width, int height, int channels);
    
    std::filesystem::path createMeshbinFile(TempDirectory& tempDir, const std::string& name,
                                           const std::vector<float>& vertices,
                                           const std::vector<float>& normals,
                                           const std::vector<float>& texcoords,
                                           const std::vector<uint32_t>& indices);
    
    std::filesystem::path createScriptBinaryFile(TempDirectory& tempDir, const std::string& name, const std::string& content);
    
    std::filesystem::path createWorldBinaryFile(TempDirectory& tempDir, const std::string& name, const std::string& jsonData);
}

namespace Assertions
{
    bool fileExists(const std::filesystem::path& path);
    
    bool fileContentEquals(const std::filesystem::path& path, const std::string& expected);
    
    std::string readFileContent(const std::filesystem::path& path);
}

} // namespace ResourceModuleTest

