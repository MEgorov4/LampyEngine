#pragma once 

class VulkanApplicationConfig
{
    uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
    static VulkanApplicationConfig& getInstance()
    {
        static VulkanApplicationConfig config;
        return config;
    }

    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
private:
    VulkanApplicationConfig() = default;
    ~VulkanApplicationConfig() = default;

    VulkanApplicationConfig(const VulkanApplicationConfig&) = delete;
    VulkanApplicationConfig& operator=(const VulkanApplicationConfig&) = delete;
};