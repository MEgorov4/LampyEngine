#pragma once

#include <EngineMinimal.h>
#include "Core/IModule.h"

namespace PhysicsModule
{
    class PhysicsModule final : public IModule
    {
    public:
        PhysicsModule();
        ~PhysicsModule() override;

        void startup() override;
        void shutdown() override;
        void tick(float dt) noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
