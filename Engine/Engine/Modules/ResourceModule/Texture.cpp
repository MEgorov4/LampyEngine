#include "Texture.h"
#include "Foundation/Assert/Assert.h"

#include "fstream"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

namespace ResourceModule
{
RTexture::RTexture(const std::string& path) : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "Texture path cannot be empty");
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open texture file: " + path);

    file.read(reinterpret_cast<char*>(&m_info.width), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_info.height), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_info.channels), sizeof(int));

    LT_ASSERT_MSG(m_info.width > 0, "Texture width must be positive");
    LT_ASSERT_MSG(m_info.height > 0, "Texture height must be positive");
    LT_ASSERT_MSG(m_info.width <= 16384, "Texture width is unreasonably large");
    LT_ASSERT_MSG(m_info.height <= 16384, "Texture height is unreasonably large");
    LT_ASSERT_MSG(m_info.channels > 0 && m_info.channels <= 4, "Invalid texture channel count");

    const size_t pixelCount = static_cast<size_t>(m_info.width * m_info.height * 4);
    LT_ASSERT_MSG(pixelCount > 0, "Texture pixel count is zero");
    
    m_info.pixels.resize(pixelCount);
    file.read(reinterpret_cast<char*>(m_info.pixels.data()), pixelCount);

    if (file.fail())
        throw std::runtime_error("Corrupted .texbin file: " + path);
    
    LT_ASSERT_MSG(m_info.pixels.size() == pixelCount, "Texture pixel data size mismatch");

    LT_LOGI("RTexture",
            "Loaded texbin " + path + " (" + std::to_string(m_info.width) + "x" + std::to_string(m_info.height) + ")");
}
}
