#pragma once

#include "ISystem.h"

class UpdateSystem : public ISystem
{
  public:
    void OnUpdate(entt::registry&, float dt) override;
};
