#pragma once 
#include <cstdint>

/// <summary>
/// Singleton class for managing engine-wide configuration settings.
/// </summary>
class EngineConfig
{
    uint32_t MAX_FRAMES_IN_FLIGHT = 2; ///< Maximum number of frames in flight.

public:
    /// <summary>
    /// Retrieves the singleton instance of EngineConfig.
    /// </summary>
    /// <returns>Reference to the singleton EngineConfig instance.</returns>
    static EngineConfig& getInstance()
    {
        static EngineConfig config;
        return config;
    }

    /// <summary>
    /// Gets the maximum number of frames that can be processed concurrently.
    /// </summary>
    /// <returns>The maximum number of frames in flight.</returns>
    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

private:
    /// <summary>
    /// Private constructor to enforce singleton pattern.
    /// </summary>
    EngineConfig() = default;

    /// <summary>
    /// Private destructor.
    /// </summary>
    ~EngineConfig() = default;

    /// <summary>
    /// Deleted copy constructor to prevent copying.
    /// </summary>
    EngineConfig(const EngineConfig&) = delete;

    /// <summary>
    /// Deleted assignment operator to prevent copying.
    /// </summary>
    EngineConfig& operator=(const EngineConfig&) = delete;
};
