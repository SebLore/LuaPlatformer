#pragma once

#include <raylib.h>

#include "Assets/AssetTags.h"

namespace components
{
    struct Sprite2D
    {
        TextureId texture;
        Rectangle src;
        Rectangle dst;
        Vector2   origin   = { 0, 0 };
        float     rotation = 0.0f;
        Color     tint     = RAYWHITE;
    };

    struct PlayerTest
    {
        Sprite2D sprite;
    };

    // config for dividing the playable level into a grid of cells, for things like collision and tile placement
    struct LevelGrid
    {
        int   cellSize    = 32;              // size of each cell in pixels
        float invCellSize = 1.0f / cellSize; // precomputed inverse for faster calculations
        float pixelRatio  = 1.0f;            // ratio of world units to pixels (for scaling)
        int   width       = 800;             // total width of the level in pixels
        int   height      = 600;             // total height of the level in pixels
    };

    struct Renderable
    {
    };

} // namespace components
