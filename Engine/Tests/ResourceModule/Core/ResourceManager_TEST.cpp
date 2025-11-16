#include <gtest/gtest.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include <Modules/ResourceModule/Asset/AssetWriterHub.h>
#include <Modules/ResourceModule/Material.h>
#include <Modules/ResourceModule/Asset/Importers/MaterialImporter.h>
#include <Modules/ResourceModule/Asset/Importers/WorldImporter.h>
#include <Modules/ResourceModule/Asset/Writers/WorldWriter.h>
#include <Modules/ResourceModule/Asset/Writers/MaterialWriter.h>
#include <Modules/ResourceModule/RWorld.h>
#include <Modules/ObjectCoreModule/ECS/EntityWorld.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/Systems/ECSLuaScriptsSystem.h>
#include "../TestHelpers.h"
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class ResourceManagerTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("ResourceManagerTest");
        resourcesDir = tempDir->createSubdir("Resources");
        cacheDir = tempDir->createSubdir("Cache");
        
        db = std::make_unique<AssetDatabase>();
        manager = std::make_unique<ResourceManager>();
        manager->setDatabase(db.get());
        AssetRegistryAccessor::Set(db.get());
        manager->setEngineResourcesRoot(resourcesDir);
        manager->setProjectResourcesRoot(resourcesDir);
        manager->setCacheRoot(cacheDir);

        writerHub = std::make_unique<AssetWriterHub>();
        auto worldWriter = std::unique_ptr<IAssetWriter>(new WorldWriter());
        writerHub->registerWriter(std::move(worldWriter));

        auto materialWriter = std::unique_ptr<IAssetWriter>(new MaterialWriter());
        writerHub->registerWriter(std::move(materialWriter));
        manager->setWriterHub(writerHub.get());
    }
    
    void TearDown() override
    {
        manager.reset();
        writerHub.reset();
        db.reset();
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    AssetInfo createMaterialAssetInfo(const std::string& sourcePath, const std::string& importedPath)
    {
        AssetInfo info;
        info.guid = AssetID(sourcePath);
        info.type = AssetType::Material;
        info.origin = AssetOrigin::Project;
        info.sourcePath = sourcePath;
        info.importedPath = importedPath;
        info.sourceTimestamp = 1234567890;
        info.importedTimestamp = 1234567891;
        info.sourceFileSize = 1024;
        info.importedFileSize = 2048;
        return info;
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    std::filesystem::path resourcesDir;
    std::filesystem::path cacheDir;
    std::unique_ptr<AssetDatabase> db;
    std::unique_ptr<ResourceManager> manager;
    std::unique_ptr<AssetWriterHub> writerHub;
};

TEST_F(ResourceManagerTest, SetDatabase)
{
    ResourceManager rm;
    AssetDatabase database;
    
    rm.setDatabase(&database);
    EXPECT_EQ(rm.getDatabase(), &database);
}

TEST_F(ResourceManagerTest, SetEngineResourcesRoot)
{
    ResourceManager rm;
    std::filesystem::path engineRoot = tempDir->path() / "EngineResources";
    
    rm.setEngineResourcesRoot(engineRoot);
}

TEST_F(ResourceManagerTest, SetProjectResourcesRoot)
{
    ResourceManager rm;
    std::filesystem::path projectRoot = tempDir->path() / "ProjectResources";
    
    rm.setProjectResourcesRoot(projectRoot);
}

TEST_F(ResourceManagerTest, LoadByAssetID)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial", {1.0f, 0.0f, 0.0f, 1.0f}, 0.5f, 0.0f);
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    std::filesystem::path cachePath = cacheDir / "Materials";
    std::filesystem::create_directories(cachePath);
    
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
    EXPECT_FLOAT_EQ(material->albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.g, 0.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.b, 0.0f);
}

TEST_F(ResourceManagerTest, LoadBySource)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->loadBySource<RMaterial>(info.sourcePath);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
}

TEST_F(ResourceManagerTest, Caching)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material1, nullptr);
    
    auto material2 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material2, nullptr);
    
    EXPECT_EQ(material1, material2);
}

TEST_F(ResourceManagerTest, MissingResource)
{
    AssetID nonExistent = AssetID("nonexistent/path.lmat");
    
    auto material = manager->load<RMaterial>(nonExistent);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, EmptyAssetID)
{
    AssetID emptyId;
    
    auto material = manager->load<RMaterial>(emptyId);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, ResourceCacheIntegration)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material1, nullptr);
    
    material1.reset();
    
    auto material2 = manager->load<RMaterial>(info.guid);
}

TEST_F(ResourceManagerTest, ResourceRegistryIntegration)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material, nullptr);
    
    manager->unload(info.guid);
    
}

TEST_F(ResourceManagerTest, SaveWorldUpdatesImportedFile)
{
    WorldImporter importer;
    std::string worldJSON = TestData::createWorldJSON("OriginalWorld");
    std::filesystem::path worldPath = resourcesDir / "Worlds" / "test.lworld";
    std::filesystem::create_directories(worldPath.parent_path());
    {
        std::ofstream file(worldPath);
        file << worldJSON;
    }

    AssetInfo info = importer.import(worldPath, cacheDir);
    info.sourcePath = std::filesystem::relative(worldPath, resourcesDir).generic_string();
    db->upsert(info);

    auto world = manager->load<RWorld>(info.guid);
    ASSERT_NE(world, nullptr);

    const std::string updatedJSON = TestData::createWorldJSON("EditedWorld");
    world->setJsonData(updatedJSON);

    EXPECT_TRUE(manager->save(info.guid));

    std::ifstream ifs(info.importedPath, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    uint32_t size = 0;
    ifs.read(reinterpret_cast<char *>(&size), sizeof(size));
    ASSERT_EQ(size, updatedJSON.size());
    std::string payload(size, '\0');
    ifs.read(payload.data(), size);
    EXPECT_EQ(payload, updatedJSON);
}

TEST_F(ResourceManagerTest, WorldLoadSaveRoundTrip)
{
    const std::string worldJSON = TestData::createWorldJSON("OriginalWorld");
    std::filesystem::path worldPath = resourcesDir / "Worlds" / "roundtrip.lworld";
    std::filesystem::create_directories(worldPath.parent_path());
    {
        std::ofstream file(worldPath);
        file << worldJSON;
    }

    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    info.sourcePath = std::filesystem::relative(worldPath, resourcesDir).generic_string();
    db->upsert(info);

    auto world = manager->load<RWorld>(info.guid);
    ASSERT_NE(world, nullptr);
    EXPECT_EQ(world->getJsonData(), TestData::createWorldJSON("OriginalWorld"));

    const std::string updatedJSON = TestData::createWorldJSON("RoundTripWorld");
    world->setJsonData(updatedJSON);

    EXPECT_TRUE(manager->save(info.guid));

    manager->clearAll();

    auto reloaded = manager->load<RWorld>(info.guid);
    ASSERT_NE(reloaded, nullptr);
    EXPECT_EQ(reloaded->getJsonData(), updatedJSON);
}

TEST_F(ResourceManagerTest, SaveAsCreatesNewAsset)
{
    WorldImporter importer;
    std::string worldJSON = TestData::createWorldJSON("OriginalWorld");
    std::filesystem::path worldPath = resourcesDir / "Worlds" / "test.lworld";
    std::filesystem::create_directories(worldPath.parent_path());
    {
        std::ofstream file(worldPath);
        file << worldJSON;
    }

    AssetInfo info = importer.import(worldPath, cacheDir);
    info.sourcePath = std::filesystem::relative(worldPath, resourcesDir).generic_string();
    db->upsert(info);

    auto world = manager->load<RWorld>(info.guid);
    ASSERT_NE(world, nullptr);

    const std::string copyRelative = "Worlds/test_copy.worldbin";
    std::filesystem::path targetPath = cacheDir / "Worlds" / "test_copy.worldbin";

    ResourceManager::ResourceSaveParams params;
    params.sourcePathOverride = copyRelative;
    params.originOverride     = AssetOrigin::Project;

    auto newGuidOpt = manager->saveAs(info.guid, targetPath, params);
    ASSERT_TRUE(newGuidOpt.has_value());

    auto savedInfo = db->get(*newGuidOpt);
    ASSERT_TRUE(savedInfo.has_value());
    EXPECT_EQ(savedInfo->sourcePath, copyRelative);
    EXPECT_TRUE(std::filesystem::exists(targetPath));

    manager->clearAll();
    auto duplicate = manager->load<RWorld>(*newGuidOpt);
    ASSERT_NE(duplicate, nullptr);
    EXPECT_EQ(duplicate->getJsonData(), TestData::createWorldJSON("OriginalWorld"));
}

TEST_F(ResourceManagerTest, ECSWorldLoadSavePipeline)
{
    EntityWorld initialWorld(manager.get(), nullptr, nullptr);
    initialWorld.init();
    auto &builderFlecs = initialWorld.get();
    auto builderEntity = builderFlecs.entity("PipelineEntity");
    SetEntityPosition(builderEntity, PositionComponent{1.f, 2.f, 3.f});
    const std::string initialJson = initialWorld.serialize();

    std::filesystem::path worldPath = resourcesDir / "Worlds" / "pipeline.lworld";
    std::filesystem::create_directories(worldPath.parent_path());
    {
        std::ofstream file(worldPath);
        file << initialJson;
    }

    WorldImporter importer;
    AssetInfo info = importer.import(worldPath, cacheDir);
    info.sourcePath = std::filesystem::relative(worldPath, resourcesDir).generic_string();
    db->upsert(info);

    auto worldResource = manager->load<RWorld>(info.guid);
    ASSERT_TRUE(worldResource);

    EntityWorld runtimeWorld(manager.get(), nullptr, nullptr);
    runtimeWorld.init();
    runtimeWorld.deserialize(worldResource->getJsonData());

    auto &runtimeFlecs = runtimeWorld.get();

    auto registerAsset = [&](const AssetID &id, AssetType type, const std::string &source) {
        AssetInfo asset{};
        asset.guid        = id;
        asset.type        = type;
        asset.origin      = AssetOrigin::Project;
        asset.sourcePath  = source;
        asset.importedPath = (cacheDir / source).generic_string();
        db->upsert(asset);
    };

    const AssetID runtimeMeshId       = MakeDeterministicIDFromPath("runtime_mesh.meshbin");
    const AssetID runtimeTextureId    = MakeDeterministicIDFromPath("runtime_texture.texbin");
    const AssetID runtimeVertShaderId = MakeDeterministicIDFromPath("runtime_vs.shader");
    const AssetID runtimeFragShaderId = MakeDeterministicIDFromPath("runtime_fs.shader");
    const AssetID runtimeMaterialId   = MakeDeterministicIDFromPath("runtime_material.lmat");
    const AssetID runtimeScriptId     = AssetID("Scripts/runtime_pipeline.lua");
    const AssetID createdScriptId     = AssetID("Scripts/runtime_created.lua");

    registerAsset(runtimeMeshId, AssetType::Mesh, "runtime_mesh.meshbin");
    registerAsset(runtimeTextureId, AssetType::Texture, "runtime_texture.texbin");
    registerAsset(runtimeVertShaderId, AssetType::Shader, "runtime_vs.shader");
    registerAsset(runtimeFragShaderId, AssetType::Shader, "runtime_fs.shader");
    registerAsset(runtimeMaterialId, AssetType::Material, "runtime_material.lmat");
    registerAsset(runtimeScriptId, AssetType::Script, "Scripts/runtime_pipeline.lua");
    registerAsset(createdScriptId, AssetType::Script, "Scripts/runtime_created.lua");

    auto pipelineEntity = runtimeFlecs.lookup("PipelineEntity");
    ASSERT_TRUE(pipelineEntity.is_valid());
    SetEntityPosition(pipelineEntity, PositionComponent{10.f, 20.f, 30.f});
    SetEntityRotation(pipelineEntity, RotationComponent{45.f, 90.f, 0.f});
    SetEntityScale(pipelineEntity, ScaleComponent{2.f, 3.f, 4.f});

    MeshComponent mesh{};
    mesh.meshID       = runtimeMeshId;
    mesh.textureID    = runtimeTextureId;
    mesh.vertShaderID = runtimeVertShaderId;
    mesh.fragShaderID = runtimeFragShaderId;
    pipelineEntity.set<MeshComponent>(mesh);

    MaterialComponent material{};
    material.materialID = runtimeMaterialId;
    pipelineEntity.set<MaterialComponent>(material);

    PointLightComponent pointLight{};
    pointLight.innerRadius = 1.5f;
    pointLight.outerRadius = 15.f;
    pointLight.intencity   = 3.0f;
    pointLight.color       = {0.2f, 0.4f, 0.8f};
    pipelineEntity.set<PointLightComponent>(pointLight);

    DirectionalLightComponent dirLight{};
    dirLight.intencity = 2.5f;
    pipelineEntity.set<DirectionalLightComponent>(dirLight);

    ScriptComponent script{};
    script.scriptID = runtimeScriptId;
    pipelineEntity.set<ScriptComponent>(script);

    auto dynamicEntity = runtimeFlecs.entity("RuntimeCreatedEntity");
    SetEntityPosition(dynamicEntity, PositionComponent{5.f, 6.f, 7.f});
    SetEntityScale(dynamicEntity, ScaleComponent{0.5f, 0.5f, 0.5f});
    ScriptComponent runtimeCreatedScript{};
    runtimeCreatedScript.scriptID = createdScriptId;
    dynamicEntity.set<ScriptComponent>(runtimeCreatedScript);

    worldResource->setJsonData(runtimeWorld.serialize());
    EXPECT_TRUE(manager->save(info.guid));

    manager->clearAll();

    auto reopenedResource = manager->load<RWorld>(info.guid);
    ASSERT_TRUE(reopenedResource);

    EntityWorld reopenedWorld(manager.get(), nullptr, nullptr);
    reopenedWorld.init();
    reopenedWorld.deserialize(reopenedResource->getJsonData());

    auto &reopenedFlecs = reopenedWorld.get();
    auto updatedEntity = reopenedFlecs.lookup("PipelineEntity");
    ASSERT_TRUE(updatedEntity.is_valid());
    const auto *updatedPos = GetEntityPosition(updatedEntity);
    ASSERT_NE(updatedPos, nullptr);
    EXPECT_FLOAT_EQ(updatedPos->x, 10.f);
    EXPECT_FLOAT_EQ(updatedPos->y, 20.f);
    EXPECT_FLOAT_EQ(updatedPos->z, 30.f);

    const auto *updatedRot = GetEntityRotation(updatedEntity);
    ASSERT_NE(updatedRot, nullptr);
    EXPECT_FLOAT_EQ(updatedRot->x, 45.f);
    EXPECT_FLOAT_EQ(updatedRot->y, 90.f);
    EXPECT_FLOAT_EQ(updatedRot->z, 0.f);

    const auto *updatedScale = GetEntityScale(updatedEntity);
    ASSERT_NE(updatedScale, nullptr);
    EXPECT_FLOAT_EQ(updatedScale->x, 2.f);
    EXPECT_FLOAT_EQ(updatedScale->y, 3.f);
    EXPECT_FLOAT_EQ(updatedScale->z, 4.f);

    const auto *updatedMesh = updatedEntity.get<MeshComponent>();
    ASSERT_NE(updatedMesh, nullptr);
    EXPECT_EQ(updatedMesh->meshID.str(), runtimeMeshId.str());
    EXPECT_EQ(updatedMesh->textureID.str(), runtimeTextureId.str());
    EXPECT_EQ(updatedMesh->vertShaderID.str(), runtimeVertShaderId.str());
    EXPECT_EQ(updatedMesh->fragShaderID.str(), runtimeFragShaderId.str());

    const auto *updatedMaterial = updatedEntity.get<MaterialComponent>();
    ASSERT_NE(updatedMaterial, nullptr);
    EXPECT_EQ(updatedMaterial->materialID.str(), runtimeMaterialId.str());

    const auto *updatedPointLight = updatedEntity.get<PointLightComponent>();
    ASSERT_NE(updatedPointLight, nullptr);
    EXPECT_FLOAT_EQ(updatedPointLight->innerRadius, 1.5f);
    EXPECT_FLOAT_EQ(updatedPointLight->outerRadius, 15.f);
    EXPECT_FLOAT_EQ(updatedPointLight->intencity, 3.0f);
    EXPECT_FLOAT_EQ(updatedPointLight->color.x, 0.2f);
    EXPECT_FLOAT_EQ(updatedPointLight->color.y, 0.4f);
    EXPECT_FLOAT_EQ(updatedPointLight->color.z, 0.8f);

    const auto *updatedDirLight = updatedEntity.get<DirectionalLightComponent>();
    ASSERT_NE(updatedDirLight, nullptr);
    EXPECT_FLOAT_EQ(updatedDirLight->intencity, 2.5f);

    const auto *updatedScript = updatedEntity.get<ScriptComponent>();
    ASSERT_NE(updatedScript, nullptr);
    EXPECT_EQ(updatedScript->scriptID.str(), runtimeScriptId.str());

    auto recreatedEntity = reopenedFlecs.lookup("RuntimeCreatedEntity");
    ASSERT_TRUE(recreatedEntity.is_valid());
    const auto *createdPos = GetEntityPosition(recreatedEntity);
    ASSERT_NE(createdPos, nullptr);
    EXPECT_FLOAT_EQ(createdPos->x, 5.f);
    EXPECT_FLOAT_EQ(createdPos->y, 6.f);
    EXPECT_FLOAT_EQ(createdPos->z, 7.f);

    const auto *createdScale = GetEntityScale(recreatedEntity);
    ASSERT_NE(createdScale, nullptr);
    EXPECT_FLOAT_EQ(createdScale->x, 0.5f);
    EXPECT_FLOAT_EQ(createdScale->y, 0.5f);
    EXPECT_FLOAT_EQ(createdScale->z, 0.5f);

    const auto *createdScript = recreatedEntity.get<ScriptComponent>();
    ASSERT_NE(createdScript, nullptr);
    EXPECT_EQ(createdScript->scriptID.str(), createdScriptId.str());
}

TEST_F(ResourceManagerTest, SaveMaterialUpdatesImportedFile)
{
    std::string materialJSON = TestData::createMaterialJSON("OriginalMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "editable.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    {
        std::ofstream file(materialPath);
        file << materialJSON;
    }

    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    info.sourcePath = std::filesystem::relative(materialPath, resourcesDir).generic_string();
    db->upsert(info);

    auto material = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material, nullptr);

    material->name            = "UpdatedMaterial";
    material->albedoColor     = {0.25f, 0.5f, 0.75f, 0.9f};
    material->emissiveColor   = {0.1f, 0.2f, 0.3f};
    material->roughness       = 0.15f;
    material->metallic        = 0.85f;
    material->normalStrength  = 1.8f;
    const AssetID updatedAlbedo("Textures/updated_albedo.texbin");
    const AssetID updatedNormal("Textures/updated_normal.texbin");
    const AssetID updatedRM("Textures/updated_rm.texbin");
    const AssetID updatedEmissive("Textures/updated_emissive.texbin");
    material->albedoTexture   = updatedAlbedo;
    material->normalTexture   = updatedNormal;
    material->roughnessMetallicTexture = updatedRM;
    material->emissiveTexture = updatedEmissive;
    material->materialID      = info.guid;

    EXPECT_TRUE(manager->save(info.guid));

    std::ifstream cacheFile(info.importedPath);
    ASSERT_TRUE(cacheFile.is_open());
    nlohmann::json cachedJson;
    cacheFile >> cachedJson;
    cacheFile.close();

    EXPECT_EQ(cachedJson["name"].get<std::string>(), "UpdatedMaterial");
    auto albedo = cachedJson["albedoColor"];
    ASSERT_EQ(albedo.size(), 4);
    EXPECT_FLOAT_EQ(albedo[0].get<float>(), 0.25f);
    EXPECT_FLOAT_EQ(albedo[1].get<float>(), 0.5f);
    EXPECT_FLOAT_EQ(albedo[2].get<float>(), 0.75f);
    EXPECT_FLOAT_EQ(albedo[3].get<float>(), 0.9f);
    auto emissive = cachedJson["emissiveColor"];
    EXPECT_FLOAT_EQ(emissive[0].get<float>(), 0.1f);
    EXPECT_FLOAT_EQ(emissive[1].get<float>(), 0.2f);
    EXPECT_FLOAT_EQ(emissive[2].get<float>(), 0.3f);
    EXPECT_FLOAT_EQ(cachedJson["roughness"].get<float>(), 0.15f);
    EXPECT_FLOAT_EQ(cachedJson["metallic"].get<float>(), 0.85f);
    EXPECT_FLOAT_EQ(cachedJson["normalStrength"].get<float>(), 1.8f);
    EXPECT_EQ(cachedJson["albedoTexture"].get<std::string>(), updatedAlbedo.str());
    EXPECT_EQ(cachedJson["normalTexture"].get<std::string>(), updatedNormal.str());
    EXPECT_EQ(cachedJson["roughnessMetallicTexture"].get<std::string>(), updatedRM.str());
    EXPECT_EQ(cachedJson["emissiveTexture"].get<std::string>(), updatedEmissive.str());

    manager->clearAll();
    auto reloaded = manager->load<RMaterial>(info.guid);
    ASSERT_NE(reloaded, nullptr);
    EXPECT_EQ(reloaded->name, "UpdatedMaterial");
    EXPECT_FLOAT_EQ(reloaded->albedoColor.r, 0.25f);
    EXPECT_FLOAT_EQ(reloaded->emissiveColor.b, 0.3f);
    EXPECT_FLOAT_EQ(reloaded->roughness, 0.15f);
    EXPECT_EQ(reloaded->albedoTexture.str(), updatedAlbedo.str());
    EXPECT_EQ(reloaded->normalTexture.str(), updatedNormal.str());
    EXPECT_EQ(reloaded->roughnessMetallicTexture.str(), updatedRM.str());
    EXPECT_EQ(reloaded->emissiveTexture.str(), updatedEmissive.str());
}

TEST_F(ResourceManagerTest, LoadMaterialFromFile)
{
    std::string materialJSON = R"({
        "name": "TestMaterial",
        "albedoColor": [1.0, 0.5, 0.0, 1.0],
        "emissiveColor": [0.2, 0.0, 0.0],
        "roughness": 0.7,
        "metallic": 0.3,
        "normalStrength": 1.5,
        "albedoTexture": "textures/albedo.png"
    })";
    
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->name, "TestMaterial");
    EXPECT_FLOAT_EQ(material->albedoColor.r, 1.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.g, 0.5f);
    EXPECT_FLOAT_EQ(material->albedoColor.b, 0.0f);
    EXPECT_FLOAT_EQ(material->albedoColor.a, 1.0f);
    EXPECT_FLOAT_EQ(material->emissiveColor.r, 0.2f);
    EXPECT_FLOAT_EQ(material->roughness, 0.7f);
    EXPECT_FLOAT_EQ(material->metallic, 0.3f);
    EXPECT_FLOAT_EQ(material->normalStrength, 1.5f);
    EXPECT_FALSE(material->albedoTexture.empty());
}

TEST_F(ResourceManagerTest, FallbackOnMissingResource)
{
    AssetID nonExistent = AssetID("Materials/nonexistent.lmat");
    
    auto material = manager->load<RMaterial>(nonExistent);
    EXPECT_EQ(material, nullptr);
    
}

TEST_F(ResourceManagerTest, CorruptedFile)
{
    std::filesystem::path materialPath = resourcesDir / "Materials" / "corrupted.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << "This is not valid JSON { invalid syntax";
    file.close();
    
    MaterialImporter importer;
    
    try
    {
        AssetInfo info = importer.import(materialPath, cacheDir);
        db->upsert(info);
        auto material = manager->load<RMaterial>(info.guid);
    }
    catch (const std::exception&)
    {
    }
}

TEST_F(ResourceManagerTest, LoadWithoutDatabase)
{
    ResourceManager rm;
    AssetID id("test/path.lmat");
    
    auto material = rm.load<RMaterial>(id);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, LoadWithDatabaseButMissingAsset)
{
    ResourceManager rm;
    AssetDatabase db;
    rm.setDatabase(&db);
    
    AssetID id("test/path.lmat");
    
    auto material = rm.load<RMaterial>(id);
    EXPECT_EQ(material, nullptr);
}

TEST_F(ResourceManagerTest, Unload)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material = manager->load<RMaterial>(info.guid);
    ASSERT_NE(material, nullptr);
    
    manager->unload(info.guid);
    
}

TEST_F(ResourceManagerTest, ClearAll)
{
    for (int i = 0; i < 3; ++i)
    {
        std::string materialJSON = TestData::createMaterialJSON("TestMaterial" + std::to_string(i));
        std::filesystem::path materialPath = resourcesDir / "Materials" / ("test" + std::to_string(i) + ".lmat");
        std::filesystem::create_directories(materialPath.parent_path());
        std::ofstream file(materialPath);
        file << materialJSON;
        file.close();
        
        MaterialImporter importer;
        AssetInfo info = importer.import(materialPath, cacheDir);
        db->upsert(info);
        
        auto material = manager->load<RMaterial>(info.guid);
        ASSERT_NE(material, nullptr);
    }
    
    manager->clearAll();
    
}

TEST_F(ResourceManagerTest, StartupShutdown)
{
    ResourceManager rm;
    
    rm.startup();
    
    rm.shutdown();
}

TEST_F(ResourceManagerTest, MultipleLoads)
{
    std::string materialJSON = TestData::createMaterialJSON("TestMaterial");
    std::filesystem::path materialPath = resourcesDir / "Materials" / "test.lmat";
    std::filesystem::create_directories(materialPath.parent_path());
    std::ofstream file(materialPath);
    file << materialJSON;
    file.close();
    
    MaterialImporter importer;
    AssetInfo info = importer.import(materialPath, cacheDir);
    db->upsert(info);
    
    auto material1 = manager->load<RMaterial>(info.guid);
    auto material2 = manager->load<RMaterial>(info.guid);
    auto material3 = manager->load<RMaterial>(info.guid);
    
    ASSERT_NE(material1, nullptr);
    ASSERT_NE(material2, nullptr);
    ASSERT_NE(material3, nullptr);
    
    EXPECT_EQ(material1, material2);
    EXPECT_EQ(material2, material3);
}

