#pragma once
#include "RenderGraphTypes.h"

#include <unordered_map>
#include <vector>
#include "../Abstract/ITexture.h"

namespace RenderModule
{
class RenderGraph
{
    std::vector<RenderGraphPass> m_passes;
    std::unordered_map<std::string, RenderGraphResource> m_resources;

  public:
    void addResource(const std::string& name, int w, int h)
    {
        m_resources[name] = RenderGraphResource{name, {}, w, h};
    }

    void addPass(RenderGraphPass pass)
    {
        m_passes.push_back(std::move(pass));
    }

    void resizeAll(int w, int h)
    {
        for (auto& [_, r] : m_resources)
        {
            r.width  = w;
            r.height = h;
        }
    }

    TextureHandle execute()
    {
        for (auto& pass : m_passes)
        {
            std::vector<RenderGraphResource> inputs;
            for (auto& name : pass.reads)
                inputs.push_back(m_resources.at(name));

            std::vector<RenderGraphResource> outputs;
            for (auto& name : pass.writes)
                outputs.push_back(m_resources.at(name));

            pass.execute(inputs, outputs);

            for (auto& res : outputs)
                m_resources[res.name] = res;
        }
        return m_resources["final"].handle;
    }
};
} // namespace RenderModule