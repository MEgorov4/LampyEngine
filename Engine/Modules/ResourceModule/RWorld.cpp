#include "RWorld.h"
#include <fstream>

using namespace ResourceModule;

RWorld::RWorld(const std::string& path) // TODO: использовать Fs::
    : BaseResource(path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open world file: " + path);

    uint32_t size = 0;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (size == 0)
        throw std::runtime_error("Corrupted worldbin file: " + path);

    m_jsonData.resize(size);
    ifs.read(m_jsonData.data(), size);
    ifs.close();

    LT_LOGI("RWorld", "Loaded world data: " + path + " (" + std::to_string(size) + " bytes)");
}

