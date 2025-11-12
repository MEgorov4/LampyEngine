#include "TestHelpers.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstdint>

namespace ResourceModuleTest
{

TempDirectory::TempDirectory(const std::string& prefix)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << prefix << "_";
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    
    m_path = std::filesystem::temp_directory_path() / ss.str();
    std::filesystem::create_directories(m_path);
    m_cleanup = true;
}

TempDirectory::~TempDirectory()
{
    if (m_cleanup) {
        cleanup();
    }
}

std::filesystem::path TempDirectory::createSubdir(const std::string& name)
{
    auto subdir = m_path / name;
    std::filesystem::create_directories(subdir);
    return subdir;
}

std::filesystem::path TempDirectory::createFile(const std::string& name, const std::string& content)
{
    auto filePath = m_path / name;
    std::filesystem::create_directories(filePath.parent_path());
    
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << content;
        file.close();
    }
    return filePath;
}

std::filesystem::path TempDirectory::createBinaryFile(const std::string& name, const std::vector<uint8_t>& data)
{
    auto filePath = m_path / name;
    std::filesystem::create_directories(filePath.parent_path());
    
    std::ofstream file(filePath, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }
    return filePath;
}

void TempDirectory::cleanup()
{
    if (std::filesystem::exists(m_path)) {
        std::filesystem::remove_all(m_path);
    }
}

namespace TestData
{

std::string createMaterialJSON(
    const std::string& name,
    const std::array<float, 4>& albedoColor,
    float roughness,
    float metallic)
{
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"name\": \"" << name << "\",\n";
    ss << "  \"albedoColor\": [" << albedoColor[0] << ", " << albedoColor[1] << ", " 
       << albedoColor[2] << ", " << albedoColor[3] << "],\n";
    ss << "  \"emissiveColor\": [0.0, 0.0, 0.0],\n";
    ss << "  \"roughness\": " << roughness << ",\n";
    ss << "  \"metallic\": " << metallic << ",\n";
    ss << "  \"normalStrength\": 1.0\n";
    ss << "}";
    return ss.str();
}

std::string createOBJFile(
    const std::vector<std::array<float, 3>>& vertices,
    const std::vector<std::array<int, 3>>& faces)
{
    std::stringstream ss;
    ss << "# Test OBJ file\n";
    
    if (vertices.empty()) {
        ss << "v 0.0 0.0 0.0\n";
        ss << "v 1.0 0.0 0.0\n";
        ss << "v 0.5 1.0 0.0\n";
        ss << "f 1 2 3\n";
    } else {
        for (const auto& v : vertices) {
            ss << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
        }
        for (const auto& f : faces) {
            ss << "f " << f[0] << " " << f[1] << " " << f[2] << "\n";
        }
    }
    
    return ss.str();
}

std::string createVertexShader(const std::string& version)
{
    return version + R"(
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
}

std::string createFragmentShader(const std::string& version)
{
    return version + R"(
out vec4 FragColor;
uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}
)";
}

std::string createLuaScript(const std::string& content)
{
    return content;
}

std::string createWorldJSON(const std::string& worldName)
{
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"name\": \"" << worldName << "\",\n";
    ss << "  \"entities\": []\n";
    ss << "}";
    return ss.str();
}

std::filesystem::path createTexbinFile(TempDirectory& tempDir, const std::string& name, int width, int height, int channels)
{
    std::filesystem::path texbinPath = tempDir.path() / name;
    std::filesystem::create_directories(texbinPath.parent_path());
    
    std::ofstream file(texbinPath, std::ios::binary);
    if (!file.is_open())
        return {};
    
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));
    file.write(reinterpret_cast<const char*>(&channels), sizeof(int));
    
    const size_t pixelCount = static_cast<size_t>(width * height * 4);
    std::vector<uint8_t> pixels(pixelCount, 255);
    file.write(reinterpret_cast<const char*>(pixels.data()), pixelCount);
    
    file.close();
    return texbinPath;
}

std::filesystem::path createMeshbinFile(TempDirectory& tempDir, const std::string& name, 
                                       const std::vector<float>& vertices,
                                       const std::vector<float>& normals,
                                       const std::vector<float>& texcoords,
                                       const std::vector<uint32_t>& indices)
{
    std::filesystem::path meshbinPath = tempDir.path() / name;
    std::filesystem::create_directories(meshbinPath.parent_path());
    
    std::ofstream file(meshbinPath, std::ios::binary);
    if (!file.is_open())
        return {};
    
    uint32_t vertexCount = static_cast<uint32_t>(vertices.size() / 3);
    uint32_t indexCount = static_cast<uint32_t>(indices.size());
    
    file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
    
    file.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(float));
    file.write(reinterpret_cast<const char*>(normals.data()), normals.size() * sizeof(float));
    file.write(reinterpret_cast<const char*>(texcoords.data()), texcoords.size() * sizeof(float));
    file.write(reinterpret_cast<const char*>(indices.data()), indices.size() * sizeof(uint32_t));
    
    file.close();
    return meshbinPath;
}

std::filesystem::path createScriptBinaryFile(TempDirectory& tempDir, const std::string& name, const std::string& content)
{
    std::filesystem::path scriptPath = tempDir.path() / name;
    std::filesystem::create_directories(scriptPath.parent_path());
    
    std::ofstream file(scriptPath, std::ios::binary);
    if (!file.is_open())
        return {};
    
    uint32_t size = static_cast<uint32_t>(content.size());
    file.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    file.write(content.data(), content.size());
    
    file.close();
    return scriptPath;
}

std::filesystem::path createWorldBinaryFile(TempDirectory& tempDir, const std::string& name, const std::string& jsonData)
{
    std::filesystem::path worldPath = tempDir.path() / name;
    std::filesystem::create_directories(worldPath.parent_path());
    
    std::ofstream file(worldPath, std::ios::binary);
    if (!file.is_open())
        return {};
    
    uint32_t size = static_cast<uint32_t>(jsonData.size());
    file.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    file.write(jsonData.data(), jsonData.size());
    
    file.close();
    return worldPath;
}

std::vector<uint8_t> createPNGTexture(int width, int height)
{
    std::vector<uint8_t> png = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, // PNG signature
    };
    
    return png;
}

} // namespace TestData

namespace Assertions
{

bool fileExists(const std::filesystem::path& path)
{
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

bool fileContentEquals(const std::filesystem::path& path, const std::string& expected)
{
    if (!fileExists(path)) {
        return false;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str() == expected;
}

std::string readFileContent(const std::filesystem::path& path)
{
    if (!fileExists(path)) {
        return "";
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace Assertions

} // namespace ResourceModuleTest

