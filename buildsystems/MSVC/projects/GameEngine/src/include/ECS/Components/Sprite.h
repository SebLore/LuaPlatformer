#pragma once

#include "SpriteFlags.h"

#include <raylib.h>

namespace components
{
    struct Sprite
    {
        Texture*  texture = nullptr;
        Rectangle srcRect = {};
        Vector2   origin  = {};
        Color     tint    = MAGENTA;

        SpriteFlags flags;

        int layer = 0;
    };
} // namespace components
