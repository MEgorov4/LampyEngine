#include <gtest/gtest.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>
#include "../TestHelpers.h"
#include <thread>
#include <vector>
#include <atomic>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class AssetDatabaseTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("AssetDatabaseTest");
        dbPath = tempDir->path() / "test_db.json";
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    AssetInfo createTestAssetInfo(const std::string& path, AssetType type = AssetType::Material)
    {
        AssetInfo info;
        info.guid = AssetID(path);
        info.type = type;
        info.origin = AssetOrigin::Project;
        info.sourcePath = path;
        info.importedPath = "cache/" + path;
        info.sourceTimestamp = 1234567890;
        info.importedTimestamp = 1234567891;
        info.sourceFileSize = 1024;
        info.importedFileSize = 2048;
        return info;
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    std::filesystem::path dbPath;
};

TEST_F(AssetDatabaseTest, Upsert)
{
    AssetDatabase db;
    
    AssetInfo info1 = createTestAssetInfo("Materials/test1.lmat");
    AssetInfo info2 = createTestAssetInfo("Materials/test2.lmat");
    
    EXPECT_EQ(db.size(), 0);
    
    db.upsert(info1);
    EXPECT_EQ(db.size(), 1);
    
    db.upsert(info2);
    EXPECT_EQ(db.size(), 2);
    
    info1.sourceFileSize = 2048;
    db.upsert(info1);
    EXPECT_EQ(db.size(), 2);
    
    auto retrieved = db.get(info1.guid);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->sourceFileSize, 2048);
}

TEST_F(AssetDatabaseTest, Get)
{
    AssetDatabase db;
    
    AssetInfo info = createTestAssetInfo("Materials/test.lmat");
    db.upsert(info);
    
    auto retrieved = db.get(info.guid);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->guid, info.guid);
    EXPECT_EQ(retrieved->sourcePath, info.sourcePath);
    
    AssetID nonExistent = AssetID("non/existent.lmat");
    auto notFound = db.get(nonExistent);
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(AssetDatabaseTest, FindBySource)
{
    AssetDatabase db;
    
    AssetInfo info = createTestAssetInfo("Materials/test.lmat");
    db.upsert(info);
    
    auto found = db.findBySource("Materials/test.lmat");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->guid, info.guid);
    EXPECT_EQ(found->sourcePath, info.sourcePath);
    
    auto notFound = db.findBySource("Materials/nonexistent.lmat");
    EXPECT_FALSE(notFound.has_value());
}

TEST_F(AssetDatabaseTest, Remove)
{
    AssetDatabase db;
    
    AssetInfo info = createTestAssetInfo("Materials/test.lmat");
    db.upsert(info);
    EXPECT_EQ(db.size(), 1);
    
    bool removed = db.remove(info.guid);
    EXPECT_TRUE(removed);
    EXPECT_EQ(db.size(), 0);
    
    auto retrieved = db.get(info.guid);
    EXPECT_FALSE(retrieved.has_value());
    
    bool notRemoved = db.remove(info.guid);
    EXPECT_FALSE(notRemoved);
}

TEST_F(AssetDatabaseTest, Clear)
{
    AssetDatabase db;
    
    db.upsert(createTestAssetInfo("Materials/test1.lmat"));
    db.upsert(createTestAssetInfo("Materials/test2.lmat"));
    db.upsert(createTestAssetInfo("Materials/test3.lmat"));
    
    EXPECT_EQ(db.size(), 3);
    
    db.clear();
    EXPECT_EQ(db.size(), 0);
    
    AssetID id = AssetID("Materials/test1.lmat");
    auto retrieved = db.get(id);
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(AssetDatabaseTest, Size)
{
    AssetDatabase db;
    
    EXPECT_EQ(db.size(), 0);
    
    db.upsert(createTestAssetInfo("Materials/test1.lmat"));
    EXPECT_EQ(db.size(), 1);
    
    db.upsert(createTestAssetInfo("Materials/test2.lmat"));
    EXPECT_EQ(db.size(), 2);
    
    db.remove(AssetID("Materials/test1.lmat"));
    EXPECT_EQ(db.size(), 1);
    
    db.clear();
    EXPECT_EQ(db.size(), 0);
}

TEST_F(AssetDatabaseTest, ForEach)
{
    AssetDatabase db;
    
    std::vector<AssetID> insertedIds;
    insertedIds.push_back(AssetID("Materials/test1.lmat"));
    insertedIds.push_back(AssetID("Materials/test2.lmat"));
    insertedIds.push_back(AssetID("Materials/test3.lmat"));
    
    for (const auto& id : insertedIds)
    {
        AssetInfo info = createTestAssetInfo(id.str());
        db.upsert(info);
    }
    
    std::vector<AssetID> foundIds;
    db.forEach([&foundIds](const AssetID& guid, const AssetInfo& info) {
        foundIds.push_back(guid);
    });
    
    EXPECT_EQ(foundIds.size(), 3);
    for (const auto& id : insertedIds)
    {
        EXPECT_NE(std::find(foundIds.begin(), foundIds.end(), id), foundIds.end());
    }
}

TEST_F(AssetDatabaseTest, ForEachByOrigin)
{
    AssetDatabase db;
    
    AssetInfo projectInfo = createTestAssetInfo("Project/test.lmat");
    projectInfo.origin = AssetOrigin::Project;
    db.upsert(projectInfo);
    
    AssetInfo engineInfo = createTestAssetInfo("Engine/test.lmat");
    engineInfo.origin = AssetOrigin::Engine;
    engineInfo.guid = AssetID("Engine/test.lmat");
    db.upsert(engineInfo);
    
    std::vector<AssetID> projectIds;
    db.forEachByOrigin(AssetOrigin::Project, [&projectIds](const AssetID& guid, const AssetInfo& info) {
        projectIds.push_back(guid);
        EXPECT_EQ(info.origin, AssetOrigin::Project);
    });
    
    EXPECT_EQ(projectIds.size(), 1);
    EXPECT_EQ(projectIds[0], projectInfo.guid);
    
    std::vector<AssetID> engineIds;
    db.forEachByOrigin(AssetOrigin::Engine, [&engineIds](const AssetID& guid, const AssetInfo& info) {
        engineIds.push_back(guid);
        EXPECT_EQ(info.origin, AssetOrigin::Engine);
    });
    
    EXPECT_EQ(engineIds.size(), 1);
    EXPECT_EQ(engineIds[0], engineInfo.guid);
}

TEST_F(AssetDatabaseTest, GetByOrigin)
{
    AssetDatabase db;
    
    AssetInfo projectInfo1 = createTestAssetInfo("Project/test1.lmat");
    projectInfo1.origin = AssetOrigin::Project;
    db.upsert(projectInfo1);
    
    AssetInfo projectInfo2 = createTestAssetInfo("Project/test2.lmat");
    projectInfo2.origin = AssetOrigin::Project;
    db.upsert(projectInfo2);
    
    AssetInfo engineInfo = createTestAssetInfo("Engine/test.lmat");
    engineInfo.origin = AssetOrigin::Engine;
    engineInfo.guid = AssetID("Engine/test.lmat");
    db.upsert(engineInfo);
    
    auto projectAssets = db.getByOrigin(AssetOrigin::Project);
    EXPECT_EQ(projectAssets.size(), 2);
    
    auto engineAssets = db.getByOrigin(AssetOrigin::Engine);
    EXPECT_EQ(engineAssets.size(), 1);
}

TEST_F(AssetDatabaseTest, ThreadSafety)
{
    AssetDatabase db;
    const int numThreads = 4;
    const int assetsPerThread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&db, t, assetsPerThread, &successCount]() {
            for (int i = 0; i < assetsPerThread; ++i)
            {
                std::string path = "Materials/thread" + std::to_string(t) + "_asset" + std::to_string(i) + ".lmat";
                AssetInfo info;
                info.guid = AssetID(path);
                info.type = AssetType::Material;
                info.origin = AssetOrigin::Project;
                info.sourcePath = path;
                info.importedPath = "cache/" + path;
                
                db.upsert(info);
                
                auto retrieved = db.get(info.guid);
                if (retrieved.has_value() && retrieved->guid == info.guid)
                {
                    successCount++;
                }
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(db.size(), numThreads * assetsPerThread);
    EXPECT_EQ(successCount.load(), numThreads * assetsPerThread);
}

TEST_F(AssetDatabaseTest, Load)
{
    AssetDatabase db;
    
    nlohmann::json j;
    AssetInfo info1 = createTestAssetInfo("Materials/test1.lmat");
    AssetInfo info2 = createTestAssetInfo("Materials/test2.lmat", AssetType::Texture);
    j[info1.guid.str()] = info1;
    j[info2.guid.str()] = info2;
    
    std::ofstream file(dbPath);
    file << j.dump(2);
    file.close();
    
    bool loaded = db.load(dbPath.string());
    EXPECT_TRUE(loaded);
    EXPECT_EQ(db.size(), 2);
    
    auto retrieved1 = db.get(info1.guid);
    ASSERT_TRUE(retrieved1.has_value());
    EXPECT_EQ(retrieved1->sourcePath, info1.sourcePath);
    EXPECT_EQ(retrieved1->type, AssetType::Material);
    
    auto retrieved2 = db.get(info2.guid);
    ASSERT_TRUE(retrieved2.has_value());
    EXPECT_EQ(retrieved2->sourcePath, info2.sourcePath);
    EXPECT_EQ(retrieved2->type, AssetType::Texture);
}

TEST_F(AssetDatabaseTest, Save)
{
    AssetDatabase db;
    
    AssetInfo info1 = createTestAssetInfo("Materials/test1.lmat");
    AssetInfo info2 = createTestAssetInfo("Materials/test2.lmat");
    
    db.upsert(info1);
    db.upsert(info2);
    
    bool saved = db.save(dbPath.string());
    EXPECT_TRUE(saved);
    EXPECT_TRUE(std::filesystem::exists(dbPath));
    
    AssetDatabase db2;
    bool loaded = db2.load(dbPath.string());
    EXPECT_TRUE(loaded);
    EXPECT_EQ(db2.size(), 2);
    
    auto retrieved1 = db2.get(info1.guid);
    ASSERT_TRUE(retrieved1.has_value());
    EXPECT_EQ(retrieved1->sourcePath, info1.sourcePath);
    
    auto retrieved2 = db2.get(info2.guid);
    ASSERT_TRUE(retrieved2.has_value());
    EXPECT_EQ(retrieved2->sourcePath, info2.sourcePath);
}

TEST_F(AssetDatabaseTest, LoadSaveRoundTrip)
{
    AssetDatabase db1;
    
    AssetInfo info1 = createTestAssetInfo("Materials/test1.lmat");
    AssetInfo info2 = createTestAssetInfo("Materials/test2.lmat");
    info1.dependencies.push_back("texture.png");
    
    db1.upsert(info1);
    db1.upsert(info2);
    
    std::filesystem::path tempFile = tempDir->path() / "roundtrip.json";
    EXPECT_TRUE(db1.save(tempFile.string()));
    
    AssetDatabase db2;
    EXPECT_TRUE(db2.load(tempFile.string()));
    EXPECT_EQ(db2.size(), 2);
    
    auto retrieved1 = db2.get(info1.guid);
    ASSERT_TRUE(retrieved1.has_value());
    EXPECT_EQ(retrieved1->guid, info1.guid);
    EXPECT_EQ(retrieved1->type, info1.type);
    EXPECT_EQ(retrieved1->origin, info1.origin);
    EXPECT_EQ(retrieved1->sourcePath, info1.sourcePath);
    EXPECT_EQ(retrieved1->importedPath, info1.importedPath);
    EXPECT_EQ(retrieved1->dependencies.size(), 1);
    EXPECT_EQ(retrieved1->dependencies[0], "texture.png");
    EXPECT_EQ(retrieved1->sourceTimestamp, info1.sourceTimestamp);
    EXPECT_EQ(retrieved1->importedTimestamp, info1.importedTimestamp);
    EXPECT_EQ(retrieved1->sourceFileSize, info1.sourceFileSize);
    EXPECT_EQ(retrieved1->importedFileSize, info1.importedFileSize);
}

TEST_F(AssetDatabaseTest, InvalidJSON)
{
    AssetDatabase db;
    
    std::ofstream file(dbPath);
    file << "This is not valid JSON {";
    file.close();
    
    bool loaded = db.load(dbPath.string());
    EXPECT_FALSE(loaded);
    EXPECT_EQ(db.size(), 0);
}

TEST_F(AssetDatabaseTest, LoadNonexistentFile)
{
    AssetDatabase db;
    
    bool loaded = db.load("nonexistent_file.json");
    EXPECT_FALSE(loaded);
    EXPECT_EQ(db.size(), 0);
}

TEST_F(AssetDatabaseTest, EmptyAssetID)
{
    AssetDatabase db;
    
    AssetID emptyId;
    auto retrieved = db.get(emptyId);
    EXPECT_FALSE(retrieved.has_value());
    
    bool removed = db.remove(emptyId);
    EXPECT_FALSE(removed);
}

TEST_F(AssetDatabaseTest, PathNormalization)
{
    AssetDatabase db;
    
    AssetInfo info = createTestAssetInfo("Materials/test.lmat");
    db.upsert(info);
    
    auto found1 = db.findBySource("Materials/test.lmat");
    auto found2 = db.findBySource("Materials\\test.lmat");
    
    ASSERT_TRUE(found1.has_value());
    ASSERT_TRUE(found2.has_value());
    EXPECT_EQ(found1->guid, found2->guid);
}

TEST_F(AssetDatabaseTest, MultipleDependencies)
{
    AssetDatabase db;
    
    AssetInfo info = createTestAssetInfo("Materials/test.lmat");
    info.dependencies.push_back("texture1.png");
    info.dependencies.push_back("texture2.png");
    info.dependencies.push_back("texture3.png");
    info.dependencies.push_back("normal.png");
    
    db.upsert(info);
    
    EXPECT_TRUE(db.save(dbPath.string()));
    
    AssetDatabase db2;
    EXPECT_TRUE(db2.load(dbPath.string()));
    
    auto retrieved = db2.get(info.guid);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->dependencies.size(), 4);
    EXPECT_EQ(retrieved->dependencies[0], "texture1.png");
    EXPECT_EQ(retrieved->dependencies[1], "texture2.png");
    EXPECT_EQ(retrieved->dependencies[2], "texture3.png");
    EXPECT_EQ(retrieved->dependencies[3], "normal.png");
}

