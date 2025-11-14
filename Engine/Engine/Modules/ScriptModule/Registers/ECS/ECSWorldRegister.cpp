#include "ECSWorldRegister.h"

#include "ECSBindingUtils.h"

#include <sol/state.hpp>

#include <optional>

namespace ScriptModule
{
namespace
{
flecs::world* ResolveWorld(IECSWorldScriptService* service)
{
    if (!service)
        return nullptr;

    EntityWorld* wrapper = service->currentWorld();
    if (!wrapper)
        return nullptr;

    return &wrapper->get();
}
} // namespace

void ECSWorldRegister::registerTypes(sol::state& state, sol::environment& env)
{
    env.set_function("GetCurrentWorld",
                     [service = m_service]() -> flecs::world*
                     { return ResolveWorld(service); });

    env.new_usertype<ResourceModule::AssetID>(
        "AssetIDType",
        "str", &ResourceModule::AssetID::str,
        "empty", &ResourceModule::AssetID::empty,
        "__tostring", [](const ResourceModule::AssetID& id) { return id.str(); });

    sol::table assetIDTable = state.create_table();
    assetIDTable["new"]     = sol::overload([]() -> ResourceModule::AssetID { return ResourceModule::AssetID(); },
                                            [](const std::string& str) -> ResourceModule::AssetID { return ResourceModule::AssetID(str); });
    env["AssetID"] = assetIDTable;

    env.new_usertype<flecs::world>(
        "World",
        "entity",
        [](flecs::world& w, const std::string& name) -> flecs::entity
        {
            if (name.empty())
                return w.entity();
            return w.entity(name.c_str());
        },
        "create",
        [](flecs::world& w, const std::optional<std::string>& name) -> flecs::entity
        {
            if (name && !name->empty())
                return w.entity(name->c_str());
            return w.entity();
        },
        "find",
        [](flecs::world& w, const std::string& name) -> flecs::entity
        {
            return w.lookup(name.c_str());
        },
        "destroy",
        [](flecs::world&, flecs::entity entity)
        {
            if (entity.is_alive())
                entity.destruct();
        });
}
} // namespace ScriptModule

