#include <gtest/gtest.h>
#include <Modules/ResourceModule/ResourceRegistry.h>
#include <Modules/ResourceModule/BaseResource.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <memory>
#include <thread>
#include <vector>
#include "../TestHelpers.h"

using namespace ResourceModule;
using namespace ResourceModuleTest;

class TestResource : public BaseResource
{
public:
    TestResource(const std::string& path) : BaseResource(path), value(0) {}
    int value;
};

class ResourceRegistryTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
    }
    // TearDown from ResourceModuleTestBase will clear static caches and shutdown MemorySystem
};

TEST_F(ResourceRegistryTest, RegisterResource)
{
    ResourceRegistry registry;
    
    AssetID id("test/resource.txt");
    auto resource = std::make_shared<TestResource>("test/resource.txt");
    resource->value = 42;
    
    EXPECT_EQ(registry.size(), 0);
    
    registry.registerResource(id, resource);
    
    EXPECT_EQ(registry.size(), 1);
    
    auto retrieved = registry.get(id);
    ASSERT_NE(retrieved, nullptr);
    auto testResource = std::static_pointer_cast<TestResource>(retrieved);
    EXPECT_EQ(testResource->value, 42);
}

TEST_F(ResourceRegistryTest, Get)
{
    ResourceRegistry registry;
    
    AssetID id1("test/resource1.txt");
    AssetID id2("test/resource2.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource1.txt");
    resource1->value = 10;
    
    registry.registerResource(id1, resource1);
    
    auto retrieved1 = registry.get(id1);
    ASSERT_NE(retrieved1, nullptr);
    auto testResource1 = std::static_pointer_cast<TestResource>(retrieved1);
    EXPECT_EQ(testResource1->value, 10);
    
    auto retrieved2 = registry.get(id2);
    EXPECT_EQ(retrieved2, nullptr);
}

TEST_F(ResourceRegistryTest, Unregister)
{
    ResourceRegistry registry;
    
    AssetID id("test/resource.txt");
    auto resource = std::make_shared<TestResource>("test/resource.txt");
    
    registry.registerResource(id, resource);
    EXPECT_EQ(registry.size(), 1);
    
    registry.unregister(id);
    EXPECT_EQ(registry.size(), 0);
    
    auto retrieved = registry.get(id);
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(ResourceRegistryTest, Clear)
{
    ResourceRegistry registry;
    
    AssetID id1("test/resource1.txt");
    AssetID id2("test/resource2.txt");
    AssetID id3("test/resource3.txt");
    
    registry.registerResource(id1, std::make_shared<TestResource>("test/resource1.txt"));
    registry.registerResource(id2, std::make_shared<TestResource>("test/resource2.txt"));
    registry.registerResource(id3, std::make_shared<TestResource>("test/resource3.txt"));
    
    EXPECT_EQ(registry.size(), 3);
    
    registry.clear();
    EXPECT_EQ(registry.size(), 0);
    
    EXPECT_EQ(registry.get(id1), nullptr);
    EXPECT_EQ(registry.get(id2), nullptr);
    EXPECT_EQ(registry.get(id3), nullptr);
}

TEST_F(ResourceRegistryTest, Size)
{
    ResourceRegistry registry;
    
    EXPECT_EQ(registry.size(), 0);
    
    registry.registerResource(AssetID("test/resource1.txt"), 
                             std::make_shared<TestResource>("test/resource1.txt"));
    EXPECT_EQ(registry.size(), 1);
    
    registry.registerResource(AssetID("test/resource2.txt"), 
                             std::make_shared<TestResource>("test/resource2.txt"));
    EXPECT_EQ(registry.size(), 2);
    
    registry.unregister(AssetID("test/resource1.txt"));
    EXPECT_EQ(registry.size(), 1);
    
    registry.clear();
    EXPECT_EQ(registry.size(), 0);
}

TEST_F(ResourceRegistryTest, ThreadSafety)
{
    ResourceRegistry registry;
    const int numThreads = 4;
    const int resourcesPerThread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([&registry, t, resourcesPerThread, &successCount]() {
            for (int i = 0; i < resourcesPerThread; ++i)
            {
                std::string path = "test/thread" + std::to_string(t) + "_resource" + std::to_string(i) + ".txt";
                AssetID id(path);
                auto resource = std::make_shared<TestResource>(path);
                resource->value = t * 1000 + i;
                
                registry.registerResource(id, resource);
                
                auto retrieved = registry.get(id);
                if (retrieved != nullptr)
                {
                    auto testResource = std::static_pointer_cast<TestResource>(retrieved);
                    if (testResource->value == t * 1000 + i)
                    {
                        successCount++;
                    }
                }
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(registry.size(), numThreads * resourcesPerThread);
    EXPECT_EQ(successCount.load(), numThreads * resourcesPerThread);
}

TEST_F(ResourceRegistryTest, EmptyAssetID)
{
    ResourceRegistry registry;
    
    AssetID emptyId;
    
    auto retrieved = registry.get(emptyId);
    EXPECT_EQ(retrieved, nullptr);
    
    registry.unregister(emptyId);
    EXPECT_EQ(registry.size(), 0);
}

TEST_F(ResourceRegistryTest, OverwriteResource)
{
    ResourceRegistry registry;
    
    AssetID id("test/resource.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource.txt");
    resource1->value = 1;
    
    auto resource2 = std::make_shared<TestResource>("test/resource.txt");
    resource2->value = 2;
    
    registry.registerResource(id, resource1);
    EXPECT_EQ(registry.size(), 1);
    
    auto retrieved1 = registry.get(id);
    ASSERT_NE(retrieved1, nullptr);
    auto testResource1 = std::static_pointer_cast<TestResource>(retrieved1);
    EXPECT_EQ(testResource1->value, 1);
    
    registry.registerResource(id, resource2);
    EXPECT_EQ(registry.size(), 1);
    
    auto retrieved2 = registry.get(id);
    ASSERT_NE(retrieved2, nullptr);
    auto testResource2 = std::static_pointer_cast<TestResource>(retrieved2);
    EXPECT_EQ(testResource2->value, 2);
}

TEST_F(ResourceRegistryTest, MultipleResources)
{
    ResourceRegistry registry;
    
    std::vector<AssetID> ids;
    std::vector<std::shared_ptr<TestResource>> resources;
    
    for (int i = 0; i < 10; ++i)
    {
        std::string path = "test/resource" + std::to_string(i) + ".txt";
        AssetID id(path);
        auto resource = std::make_shared<TestResource>(path);
        resource->value = i;
        
        ids.push_back(id);
        resources.push_back(resource);
        
        registry.registerResource(id, resource);
    }
    
    EXPECT_EQ(registry.size(), 10);
    
    for (size_t i = 0; i < ids.size(); ++i)
    {
        auto retrieved = registry.get(ids[i]);
        ASSERT_NE(retrieved, nullptr);
        auto testResource = std::static_pointer_cast<TestResource>(retrieved);
        EXPECT_EQ(testResource->value, i);
    }
}

TEST_F(ResourceRegistryTest, UnregisterNonexistent)
{
    ResourceRegistry registry;
    
    AssetID id("test/nonexistent.txt");
    
    registry.unregister(id);
    EXPECT_EQ(registry.size(), 0);
    
    registry.registerResource(id, std::make_shared<TestResource>("test/nonexistent.txt"));
    EXPECT_EQ(registry.size(), 1);
    
    registry.unregister(id);
    EXPECT_EQ(registry.size(), 0);
    
    registry.unregister(id);
    EXPECT_EQ(registry.size(), 0);
}

TEST_F(ResourceRegistryTest, ConcurrentReadWrite)
{
    ResourceRegistry registry;
    
    const int numWriters = 2;
    const int numReaders = 4;
    const int resourcesPerWriter = 50;
    
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};
    std::atomic<int> readSuccess{0};
    
    for (int w = 0; w < numWriters; ++w)
    {
        threads.emplace_back([&registry, w, resourcesPerWriter, &start]() {
            while (!start.load()) { std::this_thread::yield(); }
            
            for (int i = 0; i < resourcesPerWriter; ++i)
            {
                std::string path = "test/writer" + std::to_string(w) + "_resource" + std::to_string(i) + ".txt";
                AssetID id(path);
                auto resource = std::make_shared<TestResource>(path);
                registry.registerResource(id, resource);
            }
        });
    }
    
    for (int r = 0; r < numReaders; ++r)
    {
        threads.emplace_back([&registry, numWriters, resourcesPerWriter, &start, &readSuccess]() {
            while (!start.load()) { std::this_thread::yield(); }
            
            for (int w = 0; w < numWriters; ++w)
            {
                for (int i = 0; i < resourcesPerWriter; ++i)
                {
                    std::string path = "test/writer" + std::to_string(w) + "_resource" + std::to_string(i) + ".txt";
                    AssetID id(path);
                    auto retrieved = registry.get(id);
                    if (retrieved != nullptr)
                    {
                        readSuccess++;
                    }
                }
            }
        });
    }
    
    start.store(true);
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(registry.size(), numWriters * resourcesPerWriter);
}

