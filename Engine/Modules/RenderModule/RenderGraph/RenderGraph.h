#pragma once
#include "../Abstract/ITexture.h"
#include "Foundation/Profiler/ProfileAllocator.h"
#include "RenderGraphTypes.h"

#include <unordered_map>
#include <vector>

namespace RenderModule
{
class RenderGraph
{
    std::vector<RenderGraphPass, ProfileAllocator<RenderGraphPass>> m_passes;
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
        ZoneScopedN("RenderGraph::execute");
        
        for (auto& pass : m_passes)
        {
            ZoneScoped;
            ZoneText(pass.name.c_str(), pass.name.size());
            
            std::vector<RenderGraphResource, ProfileAllocator<RenderGraphResource>> inputs;
            for (auto& name : pass.reads)
                inputs.push_back(m_resources.at(name));

            std::vector<RenderGraphResource, ProfileAllocator<RenderGraphResource>> outputs;
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
