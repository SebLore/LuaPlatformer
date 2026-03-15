#pragma once

#include <cstdint>
#include <raylib.h>

#include "KeyboardInputComponents.h"

namespace components
{
    enum class Action : uint8_t
    {
        Left,
        Right,
        Jump,
        Use,
        Count
    };

    struct KeyInput
    {
        KeyboardKey Key;
        KeyState    State;
        bool        Tracked = false; // for skipping this key in future updates
    };

    // singleton input struct for the entire application
    struct Inputs
    {
        enum INPUT_CODE : uint8_t
        {
            LEFT  = 0,
            RIGHT = 1,
            JUMP  = 2,
            USE = 3
        };


    };
} // namespace components
