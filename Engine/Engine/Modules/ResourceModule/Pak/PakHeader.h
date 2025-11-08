#pragma once
#include <cstdint>

namespace ResourceModule
{
    struct PakHeader
    {
        char magic[4] = { 'L', 'P', 'A', 'K' }; // Lampy Pak
        uint32_t version = 1;
        uint64_t indexOffset = 0;
        uint64_t indexSize = 0;
    };
}