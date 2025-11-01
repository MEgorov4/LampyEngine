#pragma once
#include "../IAssetImporter.h"

#include <fstream>
#include <tiny_obj_loader.h>

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
        AssetInfo info{};
        info.type       = AssetType::Mesh;
        info.sourcePath = sourcePath.string();
        info.guid       = MakeDeterministicIDFromPath(std::filesystem::path(sourcePath).generic_string());

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
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texcoords;
        std::vector<uint32_t> indices;

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                VertexKey key{index.vertex_index, index.normal_index, index.texcoord_index};
                auto it = vertexMap.find(key);
                if (it != vertexMap.end())
                {
                    // Уже есть такая вершина — используем индекс
                    indices.push_back(it->second);
                }
                else
                {
                    // Новая вершина
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

        // ---- Запись в meshbin ----
        std::filesystem::path outDir = cacheRoot / "Meshes";
        std::filesystem::create_directories(outDir);
        std::filesystem::path outFile = outDir / (sourcePath.stem().string() + ".meshbin");

        std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);

        uint32_t vertexCount = static_cast<uint32_t>(vertices.size() / 3);
        uint32_t indexCount  = static_cast<uint32_t>(indices.size());

        ofs.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        ofs.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));

        ofs.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(normals.data()), normals.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(texcoords.data()), texcoords.size() * sizeof(float));
        ofs.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));

        ofs.close();

        info.importedPath      = outFile.string();
        info.sourceFileSize    = std::filesystem::file_size(sourcePath);
        info.importedFileSize  = std::filesystem::file_size(outFile);
        info.sourceTimestamp   = std::filesystem::last_write_time(sourcePath).time_since_epoch().count();
        info.importedTimestamp = std::filesystem::last_write_time(outFile).time_since_epoch().count();

        LT_LOGI("MeshImporter", std::format("Imported mesh: {} ({} verts, {} tris)", sourcePath.filename().string(),
                                            vertexCount, indexCount / 3));
        return info;
    }
};
} // namespace ResourceModule
