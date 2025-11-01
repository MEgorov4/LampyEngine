#pragma once

namespace EngineCore::Foundation
{
    class Profiler 
    {
    public:
        static void BeginFrame() noexcept;
        static void EndFrame() noexcept;
        static void MarkText(const char* text) noexcept;
        static void BeginZone(const char* name) noexcept;
    };
}