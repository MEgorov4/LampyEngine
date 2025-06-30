#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace ResourceModule
{
    template <typename T>
    class ResourceCache
    {
    public:
        ResourceCache() = default;

        template <typename... Args>
        std::shared_ptr<T> load(const std::string& path, Args&&... args)
        {
            auto it = cache.find(path);
            if (it != cache.end() && it->second)
            {
                return it->second;
            }

            auto resource = std::make_shared<T>(path, std::forward<Args>(args)...);
            cache[path] = resource;

            return resource;
        }

        template <typename T>
        void unload(const std::string& path)
        {
            auto it = cache.find(path);
            if (it != cache.end())
            {
                cache.erase(it);
            }
        }


        template <typename T>
        void clear()
        {
            cache.clear();
        }


        template <typename T>
        void removeUnused()
        {
            for (auto it = cache.begin(); it != cache.end();)
            {
                if (it->second.use_count() == 1)
                {
                    it = cache.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<T>> cache;
    };
}
