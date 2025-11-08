#pragma once
#include <EngineMinimal.h>
#include "BaseResource.h"

namespace ResourceModule
{
    struct ShaderInfo
    {
        std::string vertexText;
        std::string fragmentText;
        size_t totalSize = 0;
    };

    class RShader : public BaseResource
    {
    public:
        explicit RShader(const std::string& path);
        ~RShader() noexcept = default;

        const ShaderInfo& getShaderInfo() const noexcept { return m_info; }

    private:
        ShaderInfo m_info;
    };
}
