#pragma once

#include <entt/entt.hpp>

class ISystem
{
  public:
    virtual ~ISystem()                               = default;
    virtual void OnUpdate(entt::registry&, float dt) = 0;
    virtual void OnRender(entt::registry&) {}
};
