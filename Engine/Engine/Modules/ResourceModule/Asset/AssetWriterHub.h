#pragma once

#include "IAssetWriter.h"
#include <Foundation/Assert/Assert.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ResourceModule
{
class AssetWriterHub
{
  public:
    using WriterPtr = std::unique_ptr<IAssetWriter, std::function<void(IAssetWriter *)>>;

    template <typename Deleter = std::default_delete<IAssetWriter>>
    void registerWriter(std::unique_ptr<IAssetWriter, Deleter> writer)
    {
        LT_ASSERT_MSG(writer, "Cannot register null writer");
        LT_ASSERT_MSG(writer->getAssetType() != AssetType::Unknown, "Cannot register writer with Unknown type");

        Deleter originalDeleter = writer.get_deleter();
        IAssetWriter *rawPtr    = writer.release();

        std::function<void(IAssetWriter *)> deleterFunc = [originalDeleter](IAssetWriter *p) mutable {
            if (p)
            {
                originalDeleter(p);
            }
        };

        m_writers.emplace_back(WriterPtr(rawPtr, deleterFunc));
    }

    IAssetWriter *findWriter(const std::string &ext) const noexcept
    {
        LT_ASSERT_MSG(!ext.empty(), "Extension cannot be empty");
        auto it = std::find_if(m_writers.begin(), m_writers.end(),
                               [&](const auto &ptr) { return ptr->supportsExtension(ext); });
        return it != m_writers.end() ? it->get() : nullptr;
    }

    IAssetWriter *findWriter(AssetType type) const noexcept
    {
        LT_ASSERT_MSG(type != AssetType::Unknown, "Cannot find writer for Unknown type");
        auto it = std::find_if(m_writers.begin(), m_writers.end(),
                               [&](const auto &ptr) { return ptr->getAssetType() == type; });
        return it != m_writers.end() ? it->get() : nullptr;
    }

  private:
    std::vector<WriterPtr> m_writers;
};
} // namespace ResourceModule


