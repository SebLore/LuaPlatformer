#pragma once

#include "entt/entity/registry.hpp"

namespace systems
{
    struct ISystem
    {
        virtual ~ISystem() = default;
        virtual void Update(entt::registry&, float /*dt*/) = 0;
    };
}