#include <gtest/gtest.h>
#include <Modules/ResourceModule/ResourceCache.h>
#include <Modules/ResourceModule/BaseResource.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <memory>
#include "../TestHelpers.h"

using namespace ResourceModule;
using namespace ResourceModuleTest;

class TestResource : public BaseResource
{
public:
    TestResource(const std::string& path) : BaseResource(path), value(0) {}
    int value;
};

class ResourceCacheTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
    }
    // TearDown from ResourceModuleTestBase will clear static caches and shutdown MemorySystem
};

TEST_F(ResourceCacheTest, Find)
{
    ResourceCache<TestResource> cache;
    
    AssetID id1("test/resource1.txt");
    AssetID id2("test/resource2.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource1.txt");
    resource1->value = 42;
    
    auto found = cache.find(id1);
    EXPECT_EQ(found, nullptr);
    
    cache.put(id1, resource1);
    
    found = cache.find(id1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->value, 42);
    EXPECT_EQ(found, resource1);
    
    found = cache.find(id2);
    EXPECT_EQ(found, nullptr);
}

TEST_F(ResourceCacheTest, Put)
{
    ResourceCache<TestResource> cache;
    
    AssetID id("test/resource.txt");
    auto resource = std::make_shared<TestResource>("test/resource.txt");
    resource->value = 100;
    
    cache.put(id, resource);
    
    auto found = cache.find(id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->value, 100);
}

TEST_F(ResourceCacheTest, RemoveUnused)
{
    ResourceCache<TestResource> cache;
    
    AssetID id1("test/resource1.txt");
    AssetID id2("test/resource2.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource1.txt");
    auto resource2 = std::make_shared<TestResource>("test/resource2.txt");
    
    cache.put(id1, resource1);
    cache.put(id2, resource2);
    
    EXPECT_NE(cache.find(id1), nullptr);
    EXPECT_NE(cache.find(id2), nullptr);
    
    resource1.reset();
    
    cache.removeUnused();
    
    EXPECT_EQ(cache.find(id1), nullptr);
    
    EXPECT_NE(cache.find(id2), nullptr);
}

TEST_F(ResourceCacheTest, Clear)
{
    ResourceCache<TestResource> cache;
    
    AssetID id1("test/resource1.txt");
    AssetID id2("test/resource2.txt");
    AssetID id3("test/resource3.txt");
    
    // Keep shared_ptrs to prevent resources from being deleted
    auto res1 = std::make_shared<TestResource>("test/resource1.txt");
    auto res2 = std::make_shared<TestResource>("test/resource2.txt");
    auto res3 = std::make_shared<TestResource>("test/resource3.txt");
    
    cache.put(id1, res1);
    cache.put(id2, res2);
    cache.put(id3, res3);
    
    EXPECT_NE(cache.find(id1), nullptr);
    EXPECT_NE(cache.find(id2), nullptr);
    EXPECT_NE(cache.find(id3), nullptr);
    
    cache.clear();
    
    // After clear, cache should be empty even if resources still exist
    EXPECT_EQ(cache.find(id1), nullptr);
    EXPECT_EQ(cache.find(id2), nullptr);
    EXPECT_EQ(cache.find(id3), nullptr);
}

TEST_F(ResourceCacheTest, WeakPointerSemantics)
{
    ResourceCache<TestResource> cache;
    
    AssetID id("test/resource.txt");
    
    {
        auto resource = std::make_shared<TestResource>("test/resource.txt");
        resource->value = 50;
        
        cache.put(id, resource);
        
        auto found = cache.find(id);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->value, 50);
    }
    
    cache.removeUnused();
    
    auto found = cache.find(id);
    EXPECT_EQ(found, nullptr);
}

TEST_F(ResourceCacheTest, EmptyAssetID)
{
    ResourceCache<TestResource> cache;
    
    AssetID emptyId;
    auto found = cache.find(emptyId);
    EXPECT_EQ(found, nullptr);
}

TEST_F(ResourceCacheTest, TypedCaches)
{
    ResourceCache<TestResource> cache1;
    ResourceCache<TestResource> cache2;
    
    AssetID id("test/resource.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource.txt");
    resource1->value = 10;
    
    auto resource2 = std::make_shared<TestResource>("test/resource.txt");
    resource2->value = 20;
    
    cache1.put(id, resource1);
    cache2.put(id, resource2);
    
    auto found1 = cache1.find(id);
    auto found2 = cache2.find(id);
    
    ASSERT_NE(found1, nullptr);
    ASSERT_NE(found2, nullptr);
    EXPECT_EQ(found1->value, 10);
    EXPECT_EQ(found2->value, 20);
    EXPECT_NE(found1, found2);
}

TEST_F(ResourceCacheTest, MultipleReferences)
{
    ResourceCache<TestResource> cache;
    
    AssetID id("test/resource.txt");
    auto resource = std::make_shared<TestResource>("test/resource.txt");
    resource->value = 99;
    
    cache.put(id, resource);
    
    auto ref1 = cache.find(id);
    auto ref2 = cache.find(id);
    auto ref3 = cache.find(id);
    
    EXPECT_EQ(ref1, ref2);
    EXPECT_EQ(ref2, ref3);
    EXPECT_EQ(ref1, resource);
    EXPECT_EQ(ref1->value, 99);
    
    resource.reset();
    
    EXPECT_NE(ref1, nullptr);
    EXPECT_EQ(ref1->value, 99);
    
    ref1.reset();
    ref2.reset();
    ref3.reset();
    
    cache.removeUnused();
    
    EXPECT_EQ(cache.find(id), nullptr);
}

TEST_F(ResourceCacheTest, OverwriteResource)
{
    ResourceCache<TestResource> cache;
    
    AssetID id("test/resource.txt");
    
    auto resource1 = std::make_shared<TestResource>("test/resource.txt");
    resource1->value = 1;
    
    auto resource2 = std::make_shared<TestResource>("test/resource.txt");
    resource2->value = 2;
    
    cache.put(id, resource1);
    EXPECT_EQ(cache.find(id)->value, 1);
    
    cache.put(id, resource2);
    EXPECT_EQ(cache.find(id)->value, 2);
    EXPECT_EQ(cache.find(id), resource2);
}

TEST_F(ResourceCacheTest, DifferentResourceTypes)
{
    ResourceCache<TestResource> testCache;
    
    AssetID id("test/resource.txt");
    auto resource = std::make_shared<TestResource>("test/resource.txt");
    testCache.put(id, resource);
    
    EXPECT_NE(testCache.find(id), nullptr);
}

