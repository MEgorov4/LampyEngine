#include "RWorld.h"
#include "../../EngineContext/Foundation/Assert/Assert.h"
#include <fstream>

using namespace ResourceModule;

RWorld::RWorld(const std::string& path)
    : BaseResource(path)
{
    LT_ASSERT_MSG(!path.empty(), "World path cannot be empty");
    
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open world file: " + path);

    uint32_t size = 0;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    LT_ASSERT_MSG(size > 0, "World file size is zero");
    LT_ASSERT_MSG(size < 100 * 1024 * 1024, "World file size is unreasonably large"); // 100MB limit
    
    m_jsonData.resize(size);
    ifs.read(m_jsonData.data(), size);
    ifs.close();
    
    LT_ASSERT_MSG(m_jsonData.size() == size, "World data size mismatch");
    LT_ASSERT_MSG(!m_jsonData.empty(), "World JSON data is empty");

    LT_LOGI("RWorld", "Loaded world data: " + path + " (" + std::to_string(size) + " bytes)");
}

