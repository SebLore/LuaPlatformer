// LevelData.h
#pragma once

#include <raylib.h>
#include "Common/TileMap.h"

namespace game
{
    struct LevelData
    {
        TileMap   map;
        Rectangle playerSpawn{ 0, 0, 32, 64 };

        bool IsSolidForCollision(int tx, int ty) const
        {
            if (tx < 0 || tx >= map.Width())
                return true;
            if (ty < 0)
                return false;
            if (ty >= map.Height())
                return true;

            return map.Get(tx, ty) == TileMap::Solid;
        }
    };

    struct Player
    {
        Vector2 pos{ 0, 0 };
        Vector2 vel{ 0, 0 };
        Vector2 size{ 32, 64 };
        bool    onGround = false;
    };
} // namespace game
