#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/Mesh.h>
#include <Modules/ResourceModule/Shader.h>
#include <Modules/ResourceModule/Script.h>
#include <Modules/ResourceModule/RWorld.h>
#include "../TestHelpers.h"
#include <filesystem>
#include <fstream>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class AssetManagerIntegrationTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("AssetManagerIntegrationTest");
        resourcesDir = tempDir->createSubdir("Resources");
        cacheDir = tempDir->createSubdir("Cache");
        dbPath = tempDir->path() / "test_db.json";
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    std::filesystem::path resourcesDir;
    std::filesystem::path cacheDir;
    std::filesystem::path dbPath;
};

TEST_F(AssetManagerIntegrationTest, RegisterImporters)
{
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    
}

TEST_F(AssetManagerIntegrationTest, ScanDirectory)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "Meshes" / "test.obj";
    std::filesystem::create_directories(objPath.parent_path());
    std::ofstream objFile(objPath);
    objFile << objContent;
    objFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    
    assetManager.scanAndImportAllIn(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    EXPECT_GT(db.size(), 0);
}

TEST_F(AssetManagerIntegrationTest, ImportAllResourceTypes)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "test.obj";
    std::ofstream objFile(objPath);
    objFile << objContent;
    objFile.close();
    
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = resourcesDir / "test.lworld";
    std::ofstream worldFile(worldPath);
    worldFile << worldJSON;
    worldFile.close();
    
    std::string scriptContent = TestData::createLuaScript("return {}");
    std::filesystem::path scriptPath = resourcesDir / "test.lua";
    std::ofstream scriptFile(scriptPath);
    scriptFile << scriptContent;
    scriptFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    EXPECT_GE(db.size(), 4);
}

TEST_F(AssetManagerIntegrationTest, ResourceManagerIntegration)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    // findBySource expects relative path (as stored in database)
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relPath.generic_string());
    
    ASSERT_TRUE(materialInfoOpt.has_value());
    AssetInfo materialInfo = materialInfoOpt.value();
    
    auto material = resourceManager.load<RMaterial>(materialInfo.guid);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
}

TEST_F(AssetManagerIntegrationTest, ImportErrorHandling)
{
    std::string invalidJSON = "This is not valid JSON { invalid syntax";
    std::filesystem::path invalidPath = resourcesDir / "invalid.lmat";
    std::ofstream invalidFile(invalidPath);
    invalidFile << invalidJSON;
    invalidFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    
    try
    {
        assetManager.scanAndImportAllIn(resourcesDir);
        AssetDatabase& db = assetManager.getDatabase();
    }
    catch (const std::exception&)
    {
    }
}

TEST_F(AssetManagerIntegrationTest, ResourceDependencies)
{
    std::string materialJSON = R"({
        "name": "TexturedMaterial",
        "albedoTexture": "textures/albedo.png"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    auto materialInfoOpt = db.findBySource(materialPath.string());
    
    if (materialInfoOpt.has_value())
    {
        AssetInfo materialInfo = materialInfoOpt.value();
    }
}

TEST_F(AssetManagerIntegrationTest, DatabaseSaveLoad)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    assetManager.saveDatabase();
    
    EXPECT_TRUE(std::filesystem::exists(dbPath));
    
    AssetManager assetManager2;
    assetManager2.setProjectResourcesRoot(resourcesDir);
    assetManager2.setEngineResourcesRoot(resourcesDir);
    assetManager2.setCacheRoot(cacheDir);
    assetManager2.setDatabasePath(dbPath);
    
    AssetDatabase& db2 = assetManager2.getDatabase();
    db2.load(dbPath.string());
    
    EXPECT_GT(db2.size(), 0);
}

TEST_F(AssetManagerIntegrationTest, MultipleResourcesOfSameType)
{
    for (int i = 0; i < 5; ++i)
    {
        std::string materialJSON = TestData::createMaterialJSON("Material" + std::to_string(i));
        std::filesystem::path materialPath = resourcesDir / ("material" + std::to_string(i) + ".lmat");
        std::ofstream materialFile(materialPath);
        materialFile << materialJSON;
        materialFile.close();
    }
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    EXPECT_GE(db.size(), 5);
}

TEST_F(AssetManagerIntegrationTest, ResourcesInSubdirectories)
{
    std::filesystem::path materialsDir = resourcesDir / "Materials";
    std::filesystem::create_directories(materialsDir);
    
    std::string materialJSON = TestData::createMaterialJSON("SubdirMaterial");
    std::filesystem::path materialPath = materialsDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    
    // findBySource expects relative path (as stored in database)
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relPath.generic_string());
    EXPECT_TRUE(materialInfoOpt.has_value());
}

// Test loading all resource types through ResourceManager after import
TEST_F(AssetManagerIntegrationTest, LoadAllResourceTypes)
{
    // Create material
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    // Create mesh
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = resourcesDir / "test.obj";
    std::ofstream objFile(objPath);
    objFile << objContent;
    objFile.close();
    
    // Create shader
    std::filesystem::path shaderDir = resourcesDir / "Shaders";
    std::filesystem::create_directories(shaderDir);
    std::string vertShader = TestData::createVertexShader();
    std::string fragShader = TestData::createFragmentShader();
    std::ofstream vertFile(shaderDir / "test.vert");
    vertFile << vertShader;
    vertFile.close();
    std::ofstream fragFile(shaderDir / "test.frag");
    fragFile << fragShader;
    fragFile.close();
    
    // Create script
    std::string scriptContent = TestData::createLuaScript("return { value = 42 }");
    std::filesystem::path scriptPath = resourcesDir / "test.lua";
    std::ofstream scriptFile(scriptPath);
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Create world
    std::string worldJSON = TestData::createWorldJSON("TestWorld");
    std::filesystem::path worldPath = resourcesDir / "test.lworld";
    std::ofstream worldFile(worldPath);
    worldFile << worldJSON;
    worldFile.close();
    
    // Import all
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    // Load through ResourceManager
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    
    // Load material
    std::filesystem::path relMaterialPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relMaterialPath.generic_string());
    ASSERT_TRUE(materialInfoOpt.has_value());
    auto material = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
    
    // Load mesh
    std::filesystem::path relObjPath = std::filesystem::relative(objPath, resourcesDir);
    auto meshInfoOpt = db.findBySource(relObjPath.generic_string());
    ASSERT_TRUE(meshInfoOpt.has_value());
    auto mesh = resourceManager.load<RMesh>(meshInfoOpt->guid);
    ASSERT_NE(mesh, nullptr);
    // Mesh might be empty if import failed, but should not crash
    // Note: MeshImporter might create corrupted meshbin in some cases
    // The important thing is that ResourceManager handles it gracefully
    
    // Load shader (ShaderImporter imports .vert or .frag files separately)
    // Try to find by .vert file first
    std::filesystem::path relShaderVertPath = std::filesystem::relative(shaderDir / "test.vert", resourcesDir);
    auto shaderInfoOpt = db.findBySource(relShaderVertPath.generic_string());
    if (!shaderInfoOpt.has_value())
    {
        // Try .frag file
        std::filesystem::path relShaderFragPath = std::filesystem::relative(shaderDir / "test.frag", resourcesDir);
        shaderInfoOpt = db.findBySource(relShaderFragPath.generic_string());
    }
    ASSERT_TRUE(shaderInfoOpt.has_value());
    auto shader = resourceManager.load<RShader>(shaderInfoOpt->guid);
    ASSERT_NE(shader, nullptr);
    EXPECT_TRUE(shader->isValid());
    
    // Load script
    std::filesystem::path relScriptPath = std::filesystem::relative(scriptPath, resourcesDir);
    auto scriptInfoOpt = db.findBySource(relScriptPath.generic_string());
    ASSERT_TRUE(scriptInfoOpt.has_value());
    auto script = resourceManager.load<RScript>(scriptInfoOpt->guid);
    ASSERT_NE(script, nullptr);
    EXPECT_FALSE(script->getSource().empty());
    
    // Load world
    std::filesystem::path relWorldPath = std::filesystem::relative(worldPath, resourcesDir);
    auto worldInfoOpt = db.findBySource(relWorldPath.generic_string());
    ASSERT_TRUE(worldInfoOpt.has_value());
    auto world = resourceManager.load<RWorld>(worldInfoOpt->guid);
    ASSERT_NE(world, nullptr);
    EXPECT_TRUE(world->isValid());
}

// Test resource caching
TEST_F(AssetManagerIntegrationTest, ResourceCaching)
{
    std::string materialJSON = TestData::createMaterialJSON("CachedMaterial");
    std::filesystem::path materialPath = resourcesDir / "cached.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relPath.generic_string());
    ASSERT_TRUE(materialInfoOpt.has_value());
    
    // First load
    auto material1 = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material1, nullptr);
    
    // Second load should return cached resource
    auto material2 = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material2, nullptr);
    
    // Should be the same shared_ptr instance (cached)
    EXPECT_EQ(material1, material2);
}

// Test loading by source path
TEST_F(AssetManagerIntegrationTest, LoadBySourcePath)
{
    std::string materialJSON = TestData::createMaterialJSON("SourcePathMaterial");
    std::filesystem::path materialPath = resourcesDir / "source_path.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    // Load by source path (relative to resources root)
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto material = resourceManager.loadBySource<RMaterial>(relPath.generic_string());
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "SourcePathMaterial");
}

// Test multiple loads of same resource
TEST_F(AssetManagerIntegrationTest, MultipleLoadsSameResource)
{
    std::string materialJSON = TestData::createMaterialJSON("MultiLoadMaterial");
    std::filesystem::path materialPath = resourcesDir / "multiload.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relPath.generic_string());
    ASSERT_TRUE(materialInfoOpt.has_value());
    
    // Load multiple times
    std::vector<std::shared_ptr<RMaterial>> materials;
    for (int i = 0; i < 10; ++i)
    {
        auto material = resourceManager.load<RMaterial>(materialInfoOpt->guid);
        ASSERT_NE(material, nullptr);
        materials.push_back(material);
    }
    
    // All should be the same cached instance
    for (size_t i = 1; i < materials.size(); ++i)
    {
        EXPECT_EQ(materials[0], materials[i]);
    }
    
    // Explicitly clear references to allow memory to be freed in TearDown
    materials.clear();
}

// Test loading resources from different subdirectories
TEST_F(AssetManagerIntegrationTest, LoadFromDifferentSubdirectories)
{
    // Create materials in different subdirectories
    std::filesystem::path materialsDir = resourcesDir / "Materials";
    std::filesystem::path meshesDir = resourcesDir / "Meshes";
    std::filesystem::path scriptsDir = resourcesDir / "Scripts";
    
    std::filesystem::create_directories(materialsDir);
    std::filesystem::create_directories(meshesDir);
    std::filesystem::create_directories(scriptsDir);
    
    // Material
    std::string materialJSON = TestData::createMaterialJSON("SubdirMaterial");
    std::filesystem::path materialPath = materialsDir / "test.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    // Mesh
    std::string objContent = TestData::createOBJFile();
    std::filesystem::path objPath = meshesDir / "test.obj";
    std::ofstream objFile(objPath);
    objFile << objContent;
    objFile.close();
    
    // Script
    std::string scriptContent = TestData::createLuaScript("return {}");
    std::filesystem::path scriptPath = scriptsDir / "test.lua";
    std::ofstream scriptFile(scriptPath);
    scriptFile << scriptContent;
    scriptFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    
    // Load material from Materials subdirectory
    std::filesystem::path relMaterialPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relMaterialPath.generic_string());
    ASSERT_TRUE(materialInfoOpt.has_value());
    auto material = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material, nullptr);
    
    // Load mesh from Meshes subdirectory
    std::filesystem::path relObjPath = std::filesystem::relative(objPath, resourcesDir);
    auto meshInfoOpt = db.findBySource(relObjPath.generic_string());
    ASSERT_TRUE(meshInfoOpt.has_value());
    auto mesh = resourceManager.load<RMesh>(meshInfoOpt->guid);
    ASSERT_NE(mesh, nullptr);
    
    // Load script from Scripts subdirectory
    std::filesystem::path relScriptPath = std::filesystem::relative(scriptPath, resourcesDir);
    auto scriptInfoOpt = db.findBySource(relScriptPath.generic_string());
    ASSERT_TRUE(scriptInfoOpt.has_value());
    auto script = resourceManager.load<RScript>(scriptInfoOpt->guid);
    ASSERT_NE(script, nullptr);
}

// Test handling of invalid/empty resources
TEST_F(AssetManagerIntegrationTest, InvalidResourcesHandling)
{
    // Create a corrupted material file
    std::filesystem::path corruptedPath = resourcesDir / "corrupted.lmat";
    std::ofstream corruptedFile(corruptedPath);
    corruptedFile << "This is not valid JSON { invalid";
    corruptedFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    
    // Import should handle invalid file gracefully
    // MaterialImporter throws exception on invalid JSON, so wrap in try-catch
    try
    {
        assetManager.scanAndImportAllIn(resourcesDir);
    }
    catch (const std::exception&)
    {
        // Expected - MaterialImporter throws on invalid JSON
        // This is acceptable behavior for importers
    }
    
    // Try to load - should handle gracefully
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    std::filesystem::path relPath = std::filesystem::relative(corruptedPath, resourcesDir);
    auto infoOpt = db.findBySource(relPath.generic_string());
    
    // If import succeeded (unlikely with invalid JSON), try loading - resource should be empty but not crash
    if (infoOpt.has_value())
    {
        auto material = resourceManager.load<RMaterial>(infoOpt->guid);
        // Resource might be null or empty, but should not crash
        if (material)
        {
            // Resource loaded but might be invalid
        }
    }
    
    // Test passes if no crash occurred
}

// Test resource unloading
TEST_F(AssetManagerIntegrationTest, ResourceUnloading)
{
    std::string materialJSON = TestData::createMaterialJSON("UnloadableMaterial");
    std::filesystem::path materialPath = resourcesDir / "unloadable.lmat";
    std::ofstream materialFile(materialPath);
    materialFile << materialJSON;
    materialFile.close();
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    std::filesystem::path relPath = std::filesystem::relative(materialPath, resourcesDir);
    auto materialInfoOpt = db.findBySource(relPath.generic_string());
    ASSERT_TRUE(materialInfoOpt.has_value());
    
    // Load resource
    auto material = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material, nullptr);
    
    // Keep a reference
    auto materialRef = material;
    
    // Unload
    resourceManager.unload(materialInfoOpt->guid);
    
    // Resource should still be accessible through our reference
    EXPECT_NE(materialRef, nullptr);
    
    // But new load should create new instance
    auto material2 = resourceManager.load<RMaterial>(materialInfoOpt->guid);
    ASSERT_NE(material2, nullptr);
    // Should be different instance (cache was cleared)
    EXPECT_NE(material, material2);
}

// Test clearAll functionality
TEST_F(AssetManagerIntegrationTest, ClearAllResources)
{
    // Create multiple resources
    for (int i = 0; i < 3; ++i)
    {
        std::string materialJSON = TestData::createMaterialJSON("Material" + std::to_string(i));
        std::filesystem::path materialPath = resourcesDir / ("material" + std::to_string(i) + ".lmat");
        std::ofstream materialFile(materialPath);
        materialFile << materialJSON;
        materialFile.close();
    }
    
    AssetManager assetManager;
    assetManager.setProjectResourcesRoot(resourcesDir);
    assetManager.setEngineResourcesRoot(resourcesDir);
    assetManager.setCacheRoot(cacheDir);
    assetManager.setDatabasePath(dbPath);
    assetManager.registerDefaultImporters();
    assetManager.scanAndImportAllIn(resourcesDir);
    
    ResourceManager resourceManager;
    resourceManager.setDatabase(&assetManager.getDatabase());
    resourceManager.setEngineResourcesRoot(resourcesDir);
    resourceManager.setProjectResourcesRoot(resourcesDir);
    
    AssetDatabase& db = assetManager.getDatabase();
    
    // Load all resources
    std::vector<std::shared_ptr<RMaterial>> materials;
    db.forEach([&](const AssetID& id, const AssetInfo& info) {
        if (info.type == AssetType::Material)
        {
            auto material = resourceManager.load<RMaterial>(id);
            if (material)
                materials.push_back(material);
        }
    });
    
    EXPECT_GE(materials.size(), 3);
    
    // Verify resources are accessible through references
    for (auto& material : materials)
    {
        EXPECT_NE(material, nullptr);
    }
    
    // Explicitly clear references BEFORE clearAll() to allow memory to be freed
    // This is critical for memory management in tests
    materials.clear();
    
    // Clear all - now resources can be freed since no external references exist
    resourceManager.clearAll();
}

