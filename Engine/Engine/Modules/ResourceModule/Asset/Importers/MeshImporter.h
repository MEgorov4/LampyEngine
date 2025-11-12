#pragma once
#include "../IAssetImporter.h"
#include "Foundation/Memory/ResourceAllocator.h"
#include <Foundation/Assert/Assert.h>

#include <fstream>
#include <tiny_obj_loader.h>

using EngineCore::Foundation::ResourceAllocator;

namespace ResourceModule
{
class MeshImporter final : public IAssetImporter
{
  public:
    bool supportsExtension(const std::string& ext) const noexcept override
    {
        return ext == ".obj";
    }

    AssetType getAssetType() const noexcept override
    {
        return AssetType::Mesh;
    }

    AssetInfo import(const std::filesystem::path& sourcePath, const std::filesystem::path& cacheRoot) override
    {
        LT_ASSERT_MSG(!sourcePath.empty(), "Source path cannot be empty");
        LT_ASSERT_MSG(!cacheRoot.empty(), "Cache root cannot be empty");
        LT_ASSERT_MSG(std::filesystem::exists(sourcePath), "Source file does not exist: " + sourcePath.string());
        LT_ASSERT_MSG(std::filesystem::is_regular_file(sourcePath), "Source path is not a file: " + sourcePath.string());
        
        AssetInfo info{};
        info.type       = AssetType::Mesh;
        info.sourcePath = sourcePath.string();
        info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());
        LT_ASSERT_MSG(!info.guid.empty(), "Generated GUID is empty");

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, sourcePath.string().c_str()))
            throw std::runtime_error(warn + err);

        struct VertexKey
        {
            int v, n, t;
            bool operator==(const VertexKey& other) const noexcept
            {
                return v == other.v && n == other.n && t == other.t;
            }
        };

        struct HashKey
        {
            size_t operator()(const VertexKey& key) const noexcept
            {
                size_t h1 = std::hash<int>()(key.v);
                size_t h2 = std::hash<int>()(key.n);
                size_t h3 = std::hash<int>()(key.t);
                return (h1 ^ (h2 << 1)) ^ (h3 << 2);
            }
        };

        std::unordered_map<VertexKey, uint32_t, HashKey> vertexMap;
        std::vector<float, ResourceAllocator<float>> vertices;
        std::vector<float, ResourceAllocator<float>> normals;
        std::vector<float, ResourceAllocator<float>> texcoords;
        std::vector<uint32_t, ResourceAllocator<uint32_t>> indices;

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                VertexKey key{index.vertex_index, index.normal_index, index.texcoord_index};
                auto it = vertexMap.find(key);
                if (it != vertexMap.end())
                {
                    // ��� ���� ����� ������� � ���������� ������
                    indices.push_back(it->second);
                }
                else
                {
                    // ����� �������
                    uint32_t newIndex = static_cast<uint32_t>(vertices.size() / 3);
                    vertexMap[key]    = newIndex;
                    indices.push_back(newIndex);

                    if (index.vertex_index >= 0)
                    {
                        int vi = 3 * index.vertex_index;
                        vertices.push_back(attrib.vertices[vi + 0]);
                        vertices.push_back(attrib.vertices[vi + 1]);
                        vertices.push_back(attrib.vertices[vi + 2]);
                    }

                    if (index.normal_index >= 0)
                    {
                        int ni = 3 * index.normal_index;
                        normals.push_back(attrib.normals[ni + 0]);
                        normals.push_back(attrib.normals[ni + 1]);
                        normals.push_back(attrib.normals[ni + 2]);
                    }

                    if (index.texcoord_index >= 0)
                    {
                        int ti = 2 * index.texcoord_index;
                        texcoords.push_back(attrib.texcoords[ti + 0]);
                        texcoords.push_back(attrib.texcoords[ti + 1]);
                    }
                }
            }
        }

        LT_ASSERT_MSG(!vertices.empty(), "No vertices loaded from mesh");
        LT_ASSERT_MSG(!indices.empty(), "No indices loaded from mesh");
        LT_ASSERT_MSG(vertices.size() % 3 == 0, "Vertex count is not multiple of 3");
        LT_ASSERT_MSG(normals.size() % 3 == 0, "Normal count is not multiple of 3");
        LT_ASSERT_MSG(texcoords.size() % 2 == 0, "Texture coordinate count is not multiple of 2");

        std::filesystem::path outDir = cacheRoot / "Meshes";
        std::filesystem::create_directories(outDir);
        LT_ASSERT_MSG(std::filesystem::exists(outDir), "Failed to create output directory");
        
        std::filesystem::path outFile = outDir / (sourcePath.stem().string() + ".meshbin");
        LT_ASSERT_MSG(!outFile.empty(), "Output file path is empty");

        std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
        LT_ASSERT_MSG(ofs.is_open(), "Failed to open output file for writing: " + outFile.string());

        uint32_t vertexCount = static_cast<uint32_t>(vertices.size() / 3);
        uint32_t indexCount  = static_cast<uint32_t>(indices.size());
        
        LT_ASSERT_MSG(vertexCount > 0, "Vertex count is zero");
        LT_ASSERT_MSG(indexCount > 0, "Index count is zero");
        LT_ASSERT_MSG(vertexCount < 1000000, "Vertex count is unreasonably large");
        LT_ASSERT_MSG(indexCount < 10000000, "Index count is unreasonably large");

        ofs.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        ofs.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));

        ofs.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(normals.data()), normals.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(texcoords.data()), texcoords.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));

        ofs.close();
        LT_ASSERT_MSG(std::filesystem::exists(outFile), "Output file was not created");
        LT_ASSERT_MSG(std::filesystem::file_size(outFile) > 0, "Output file is empty");

        info.importedPath      = outFile.string();
        info.sourceFileSize    = std::filesystem::file_size(sourcePath);
        info.importedFileSize  = std::filesystem::file_size(outFile);
        info.sourceTimestamp   = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
        info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();
        
        LT_ASSERT_MSG(!info.importedPath.empty(), "Imported path is empty");
        LT_ASSERT_MSG(info.importedFileSize > 0, "Imported file size is zero");

        LT_LOGI("MeshImporter", std::format("Imported mesh: {} ({} verts, {} tris)", sourcePath.filename().string(),
                                            vertexCount, indexCount / 3));
        return info;
    }
};
} // namespace ResourceModule
