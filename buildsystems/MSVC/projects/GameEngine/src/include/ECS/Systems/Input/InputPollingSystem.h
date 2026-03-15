#pragma once

#include "ECS/Components/Input/InputComponents.h"
#include "ECS/Systems/ISystem.h"

#include <functional>
#include <raylib.h>

namespace components
{
    struct PlayerController;
}

class InputPollingSystem : public ISystem
{
  public:
    void OnUpdate(entt::registry& reg, float dt) override;
};

inline void InputPollingSystem::OnUpdate(entt::registry& reg, float dt)
{
    using namespace components;

    auto view = reg.view<KeyboardInput, TrackedKeys>();

    for (auto e : view)
    {
        KeyboardInput input = reg.get<KeyboardInput>(e);

        input.pressed.reset();
        input.released.reset();

        TrackedKeys tracked = reg.get<TrackedKeys>(e);
        for (const KeyboardKey k : tracked.keys)
        {
            const size_t idx = static_cast<size_t>(k);
            input.down.set(idx, IsKeyDown(k));
            input.pressed.set(idx, IsKeyPressed(k));
            input.released.set(idx, IsKeyReleased(k));
        }
    }
}
