#pragma once
#include <bitset>
#include <raylib.h>
#include <vector>

#include "raylib.h"

namespace components
{
    // Singleton struct for tracking keyboard input

    struct KeyState
    {
        bool down, pressed, up;
    };

    struct KeyboardInput
    {
        static constexpr size_t KeyCount = 256;

        std::bitset<KeyCount> down{};
        std::bitset<KeyCount> pressed{};
        std::bitset<KeyCount> released{};

        [[nodiscard]] KeyState State(KeyboardKey k) const
        {
            const size_t i = static_cast<size_t>(k);
            return KeyState{ .down    = down.test(i),
                             .pressed = pressed.test(i),
                             .up      = released.test(i) };
        }
    };

    struct TrackedKeys
    {
        std::vector<KeyboardKey> keys; // data only: "keys we want to poll"
    };

} // namespace components
