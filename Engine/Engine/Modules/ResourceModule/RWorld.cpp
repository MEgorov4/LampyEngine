#include "RWorld.h"
#include "Foundation/Assert/Assert.h"
#include <fstream>

using namespace ResourceModule;

RWorld::RWorld(const std::string& path)
    : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "World path cannot be empty");
    
    // Initialize as empty
    m_jsonData.clear();
    
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
    {
        LT_LOGE("RWorld", "Failed to open world file: " + path);
        return; // Leave resource empty
    }

    uint32_t size = 0;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    if (ifs.fail())
    {
        LT_LOGE("RWorld", "Failed to read world file header: " + path);
        return; // Leave resource empty
    }
    
    // Validate size BEFORE allocating memory
    if (size == 0)
    {
        LT_LOGE("RWorld", "World file size is zero: " + path);
        return; // Leave resource empty
    }
    
    if (size >= 100 * 1024 * 1024) // 100MB limit
    {
        LT_LOGE("RWorld", std::string("World file size is unreasonably large (") + std::to_string(size) + "): " + path);
        return; // Leave resource empty - prevent memory leak
    }
    
    m_jsonData.resize(size);
    ifs.read(m_jsonData.data(), size);
    
    // Check if read failed or read less than expected (file truncated)
    if (ifs.fail() || ifs.gcount() != static_cast<std::streamsize>(size))
    {
        LT_LOGE("RWorld", "Failed to read world data or file truncated: " + path);
        m_jsonData.clear();
        ifs.close();
        return; // Leave resource empty
    }
    ifs.close();
    
    if (m_jsonData.size() != size)
    {
        LT_LOGE("RWorld", "World data size mismatch: " + path);
        m_jsonData.clear();
        return; // Leave resource empty
    }
    
    if (m_jsonData.empty())
    {
        LT_LOGE("RWorld", "World JSON data is empty: " + path);
        return; // Leave resource empty
    }

    LT_LOGI("RWorld", "Loaded world data: " + path + " (" + std::to_string(size) + " bytes)");
}

