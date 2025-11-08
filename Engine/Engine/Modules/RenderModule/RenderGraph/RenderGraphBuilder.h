#pragma once
#include "RenderGraph.h"

namespace RenderModule
{
    class RenderGraphBuilder;

    class RenderGraphPassBuilder
    {
        RenderGraphBuilder& m_parent;
        RenderGraphPass m_pass;

    public:
        RenderGraphPassBuilder(RenderGraphBuilder& parent, const std::string& name)
            : m_parent(parent)
        {
            m_pass.name = name;
        }

        RenderGraphPassBuilder& read(const std::string& res)
        {
            m_pass.reads.push_back(res);
            return *this;
        }

        RenderGraphPassBuilder& write(const std::string& res)
        {
            m_pass.writes.push_back(res);
            return *this;
        }

        template<typename Func>
        RenderGraphPassBuilder& exec(Func&& fn)
        {
            m_pass.execute = std::forward<Func>(fn);
            return *this;
        }

        RenderGraphBuilder& end();
    };

    class RenderGraphBuilder
    {
        RenderGraph& m_graph;
    public:
        explicit RenderGraphBuilder(RenderGraph& g) : m_graph(g) {}

        RenderGraphBuilder& addResource(const std::string& name, int w, int h)
        {
            m_graph.addResource(name, w, h);
            return *this;
        }

        RenderGraphPassBuilder addPass(const std::string& name)
        {
            return RenderGraphPassBuilder(*this, name);
        }

        void commit(RenderGraphPass&& pass)
        {
            m_graph.addPass(std::move(pass));
        }

        RenderGraph& build() { return m_graph; }
    };

    inline RenderGraphBuilder& RenderGraphPassBuilder::end()
    {
        m_parent.commit(std::move(m_pass));
        return m_parent;
    }
}