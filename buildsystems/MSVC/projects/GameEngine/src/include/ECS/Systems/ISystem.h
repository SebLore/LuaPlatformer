#pragma once

#include <entt/entt.hpp>

class ISystem
{
  public:
    virtual ~ISystem()                               = default;
    virtual bool OnUpdate(entt::registry&, float dt) = 0;
};
