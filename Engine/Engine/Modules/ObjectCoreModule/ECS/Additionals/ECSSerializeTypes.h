#pragma once

#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <flecs.h>
#include <string>
namespace ECSModule
{
namespace Utils
{

inline int string_serialize(const flecs::serializer* s, const std::string* data)
{
    if (!s || !data)
        return -1;
    ecs_string_t tmp = const_cast<char*>(data->c_str());
    return s->value(flecs::String, &tmp);
}

inline void string_assign_string(std::string* dst, const char* value)
{
    if (!dst)
        return;
    if (value)
        *dst = value;
    else
        dst->clear();
}

inline void string_assign_null(std::string* dst)
{
    if (dst)
        dst->clear();
}
using namespace ResourceModule;
inline int AssetID_serialize(const flecs::serializer* s, const AssetID* id)
{
    if (!s || !id)
        return -1;

    std::string text;

    if (auto* db = AssetRegistryAccessor::Get())
    {
        if (const auto& info = db->get(*id))
            text = info->sourcePath;
        else
            text = id->str();
    }
    else
    {
        text = id->str();
    }

    ecs_string_t tmp = const_cast<char*>(text.c_str());
    return s->value(flecs::String, &tmp);
}

inline void AssetID_assign_string(AssetID* dst, const char* value)
{
    if (!dst)
        return;

    if (!value || !*value)
    {
        *dst = AssetID{};
        return;
    }

    if (auto* db = AssetRegistryAccessor::Get())
    {
        if (const auto& info = db->findBySource(value))
        {
            *dst = info->guid;
            return;
        }
    }

    *dst = MakeDeterministicIDFromPath(value);
}

inline void AssetID_assign_null(AssetID* dst)
{
    if (dst)
        *dst = AssetID{};
}

} // namespace Utils
} // namespace ECSModule
