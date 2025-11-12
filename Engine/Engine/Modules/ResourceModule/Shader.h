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

        bool isValid() const noexcept
        {
            return m_info.totalSize > 0 && (!m_info.vertexText.empty() || !m_info.fragmentText.empty());
        }

        bool isEmpty() const noexcept
        {
            return m_info.vertexText.empty() && m_info.fragmentText.empty();
        }

    private:
        ShaderInfo m_info;
    };
}
