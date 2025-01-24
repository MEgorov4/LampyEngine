#pragma once 
#include <cstdint>

class EngineConfig
{
    uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
    static EngineConfig& getInstance()
    {
        static EngineConfig config;
        return config;
    }

    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
private:
    EngineConfig() = default;
    ~EngineConfig() = default;

    EngineConfig(const EngineConfig&) = delete;
    EngineConfig& operator=(const EngineConfig&) = delete;
};
