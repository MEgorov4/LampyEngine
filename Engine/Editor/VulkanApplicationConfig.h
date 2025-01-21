#pragma once 
#include <cstdint>

class EngineApplicationConfig
{
    uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
    static EngineApplicationConfig& getInstance()
    {
        static EngineApplicationConfig config;
        return config;
    }

    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
private:
    EngineApplicationConfig() = default;
    ~EngineApplicationConfig() = default;

    EngineApplicationConfig(const EngineApplicationConfig&) = delete;
    EngineApplicationConfig& operator=(const EngineApplicationConfig&) = delete;
};
