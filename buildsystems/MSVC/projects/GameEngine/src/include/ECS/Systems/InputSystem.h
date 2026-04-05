/**
 * @file InputSystem.h
 * @brief Declaration of the InputSystem class for handling user input in the ECS framework.
 * @date 2025-12-14
 */
#pragma once

#include "ECS/Components/BaseComponents.h"
#include "ECS/Components/Input/InputComponents.h"
#include "ISystem.h"

#include <array>
#include <iostream>

class InputSystem : public ISystem
{
  public:
    bool OnUpdate(entt::registry& r, float dt) override;

  private:
    static std::string ToKeyString(KeyboardKey k)
    {
        int a = k;

        // clamp between a and Z for letters
        if ((a >= 0x41 && a <= 0x5A) || (a >= 0x61 && a <= 0x7A))
            return std::string{ static_cast<char>(a) };

        if (k == KEY_SPACE)
            return "SPACE";

        return "N/A";
    }

    static void DebugPrint(const KeyboardKey keys[], size_t count = 0)
    {

        bool any = false;
        for (size_t i = 0; i < count; i++)
        {
            const auto& k = keys[i];

            if (IsKeyDown(k))
            {
                if (!any)
                {
                    std::cout << "Key(s) pressed: ";
                    any = true;
                }
                std::cout << ToKeyString(k) << " ";
            }
        }
        if (any)
            std::cout << "\n";
    }
};

/// @brief Registers user input into a resource component, including keyboard, mouse and gamepad input.
/// @param r entt registry
inline bool InputSystem::OnUpdate(entt::registry& r, float dt)
{
    using namespace components;

    auto viewInput  = r.view<TrackedKeys, KeyboardInput>();
    auto viewPlayer = r.view<Player, Position, PlayerController>();

    for (auto e : viewInput)
    {
        TrackedKeys&   tracked = r.get<TrackedKeys>(e);
        KeyboardInput& input   = r.get<KeyboardInput>(e);

        for (KeyboardKey key : tracked.keys)
        {
            // for now just update player position in x and y direction based on WASD
        }
    }

    return false;
}
