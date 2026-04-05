#pragma once

#include "ISystem.h"

class UpdateSystem : public ISystem
{
  public:
    bool OnUpdate(entt::registry&, float dt) override;
};
