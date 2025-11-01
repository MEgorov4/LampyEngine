#include "Texture.h"

#include "fstream"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

namespace ResourceModule
{
RTexture::RTexture(const std::string& path) : BaseResource(path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open texture file: " + path);

    file.read(reinterpret_cast<char*>(&m_info.width), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_info.height), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_info.channels), sizeof(int));

    const size_t pixelCount = static_cast<size_t>(m_info.width * m_info.height * 4);
    m_info.pixels.resize(pixelCount);
    file.read(reinterpret_cast<char*>(m_info.pixels.data()), pixelCount);

    if (file.fail())
        throw std::runtime_error("Corrupted .texbin file: " + path);

    LT_LOGI("RTexture",
            "Loaded texbin " + path + " (" + std::to_string(m_info.width) + "x" + std::to_string(m_info.height) + ")");
}
}
