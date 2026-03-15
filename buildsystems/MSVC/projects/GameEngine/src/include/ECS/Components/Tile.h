#pragma once

#include "BaseComponents.h"
#include <vector>

namespace components
{

    enum TileProperties
    {
        SOLID,
        ONEWAY,
        PASSTHROUGH
    };

    struct Tile
    {
        int            x          = 0;
        int            y          = 0;
        TileProperties prop       = SOLID;
        int            layer      = 0;
        int            resolution = 16;
    };

    struct TileCollection
    {
        Vector2          start;
        Vector2          stop;
        std::vector<int> override;
    };

} // namespace components
