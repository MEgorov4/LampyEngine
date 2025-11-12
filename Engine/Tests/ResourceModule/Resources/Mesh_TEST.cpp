#include <gtest/gtest.h>
#include <Modules/ResourceModule/Mesh.h>
#include "../TestHelpers.h"
#include <filesystem>

using namespace ResourceModule;
using namespace ResourceModuleTest;

class MeshTest : public ResourceModuleTestBase
{
protected:
    void SetUp() override
    {
        // Call parent SetUp to initialize MemorySystem
        ResourceModuleTestBase::SetUp();
        
        tempDir = std::make_unique<TempDirectory>("MeshTest");
    }
    
    void TearDown() override
    {
        tempDir.reset();
        
        // Call parent TearDown to clear static caches and shutdown MemorySystem
        ResourceModuleTestBase::TearDown();
    }
    
    std::unique_ptr<TempDirectory> tempDir;
    
    std::filesystem::path createSimpleTriangleMesh()
    {
        std::vector<float> vertices = {
            0.0f, 0.0f, 0.0f,  // v0
            1.0f, 0.0f, 0.0f,  // v1
            0.5f, 1.0f, 0.0f   // v2
        };
        
        std::vector<float> normals = {
            0.0f, 0.0f, 1.0f,  // n0
            0.0f, 0.0f, 1.0f,  // n1
            0.0f, 0.0f, 1.0f   // n2
        };
        
        std::vector<float> texcoords = {
            0.0f, 0.0f,  // t0
            1.0f, 0.0f,  // t1
            0.5f, 1.0f   // t2
        };
        
        std::vector<uint32_t> indices = {0, 1, 2};
        
        return TestData::createMeshbinFile(*tempDir, "triangle.meshbin", vertices, normals, texcoords, indices);
    }
};

TEST_F(MeshTest, ConstructorWithFilePath)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    EXPECT_EQ(meshData.vertices.size(), 3);
    EXPECT_EQ(meshData.indices.size(), 3);
}

TEST_F(MeshTest, LoadMeshData)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    EXPECT_FALSE(meshData.vertices.empty());
    EXPECT_FALSE(meshData.indices.empty());
}

TEST_F(MeshTest, VerticesAndIndices)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    EXPECT_EQ(meshData.vertices.size(), 3);
    EXPECT_EQ(meshData.indices.size(), 3);
    
    EXPECT_FLOAT_EQ(meshData.vertices[0].pos.x, 0.0f);
    EXPECT_FLOAT_EQ(meshData.vertices[0].pos.y, 0.0f);
    EXPECT_FLOAT_EQ(meshData.vertices[0].pos.z, 0.0f);
    
    EXPECT_EQ(meshData.indices[0], 0);
    EXPECT_EQ(meshData.indices[1], 1);
    EXPECT_EQ(meshData.indices[2], 2);
}

TEST_F(MeshTest, MissingFile)
{
    RMesh mesh("nonexistent/mesh.meshbin");
    
    // Resource should be empty but not crash
    EXPECT_TRUE(mesh.isEmpty());
    EXPECT_FALSE(mesh.isValid());
    EXPECT_TRUE(mesh.getMeshData().vertices.empty());
    EXPECT_TRUE(mesh.getMeshData().indices.empty());
}

TEST_F(MeshTest, InvalidMeshbin)
{
    std::filesystem::path corruptedPath = tempDir->createFile("corrupted.meshbin", "This is not a valid meshbin file");
    
    RMesh mesh(corruptedPath.string());
    
    // Resource should be empty but not crash
    // This prevents 32GB memory allocation from invalid vertexCount/indexCount
    EXPECT_TRUE(mesh.isEmpty());
    EXPECT_FALSE(mesh.isValid());
    EXPECT_TRUE(mesh.getMeshData().vertices.empty());
    EXPECT_TRUE(mesh.getMeshData().indices.empty());
}

TEST_F(MeshTest, LoadRealMesh)
{
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,  // v0
         0.5f, -0.5f, 0.0f,  // v1
         0.5f,  0.5f, 0.0f,  // v2
        -0.5f,  0.5f, 0.0f   // v3
    };
    
    std::vector<float> normals = {
        0.0f, 0.0f, 1.0f,  // n0
        0.0f, 0.0f, 1.0f,  // n1
        0.0f, 0.0f, 1.0f,  // n2
        0.0f, 0.0f, 1.0f   // n3
    };
    
    std::vector<float> texcoords = {
        0.0f, 0.0f,  // t0
        1.0f, 0.0f,  // t1
        1.0f, 1.0f,  // t2
        0.0f, 1.0f   // t3
    };
    
    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3
    };
    
    std::filesystem::path meshbinPath = TestData::createMeshbinFile(*tempDir, "square.meshbin", 
                                                                    vertices, normals, texcoords, indices);
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    EXPECT_EQ(meshData.vertices.size(), 4);
    EXPECT_EQ(meshData.indices.size(), 6);
}

TEST_F(MeshTest, GeometryCorrectness)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    
    EXPECT_LE(meshData.aabbMin.x, meshData.aabbMax.x);
    EXPECT_LE(meshData.aabbMin.y, meshData.aabbMax.y);
    EXPECT_LE(meshData.aabbMin.z, meshData.aabbMax.z);
    
    for (const auto& vertex : meshData.vertices)
    {
        EXPECT_GE(vertex.pos.x, meshData.aabbMin.x);
        EXPECT_LE(vertex.pos.x, meshData.aabbMax.x);
        EXPECT_GE(vertex.pos.y, meshData.aabbMin.y);
        EXPECT_LE(vertex.pos.y, meshData.aabbMax.y);
        EXPECT_GE(vertex.pos.z, meshData.aabbMin.z);
        EXPECT_LE(vertex.pos.z, meshData.aabbMax.z);
    }
}

TEST_F(MeshTest, VertexNormals)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    
    for (const auto& vertex : meshData.vertices)
    {
        float length = glm::length(vertex.normal);
        EXPECT_GT(length, 0.0f);
    }
}

TEST_F(MeshTest, UVCoordinates)
{
    std::filesystem::path meshbinPath = createSimpleTriangleMesh();
    
    RMesh mesh(meshbinPath.string());
    
    const auto& meshData = mesh.getMeshData();
    
    for (const auto& vertex : meshData.vertices)
    {
        EXPECT_GE(vertex.uv.x, 0.0f);
        EXPECT_LE(vertex.uv.x, 1.0f);
        EXPECT_GE(vertex.uv.y, 0.0f);
        EXPECT_LE(vertex.uv.y, 1.0f);
    }
}

