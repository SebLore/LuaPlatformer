// LevelLoader.h
#pragma once

#include "LevelData.h"
#include "LuaWrapper.h"

#include <string>

namespace game
{
    inline bool LoadLevelFromLua(Lua::LuaWrapper& lua, const std::string& moduleName, LevelData& outLevel)
    {
        lua_State* L = lua.State();

        // require(moduleName)
        lua_getglobal(L, "require");
        lua_pushlstring(L, moduleName.data(), moduleName.size());

        if (!lua.PCall(1, 1))
            return false;

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }

        const int levelIndex = lua_gettop(L);

        // tileSize
        lua_getfield(L, levelIndex, "tileSize");
        const int tileSize = static_cast<int>(luaL_optinteger(L, -1, 16));
        lua_pop(L, 1);

        // rows
        lua_getfield(L, levelIndex, "rows");
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 2); // rows + level
            return false;
        }

        const int rowsIndex = lua_gettop(L);
        const int height    = static_cast<int>(lua_rawlen(L, rowsIndex));

        int                  width = 0;
        std::vector<uint8_t> tiles;
        tiles.reserve(static_cast<size_t>(height) * 32);

        for (int row = 1; row <= height; ++row)
        {
            lua_geti(L, rowsIndex, row);

            size_t      len       = 0;
            const char* rowString = luaL_checklstring(L, -1, &len);

            if (width == 0)
                width = static_cast<int>(len);

            if (static_cast<int>(len) != width)
            {
                lua_pop(L, 3); // row string + rows + level
                return false;
            }

            for (int x = 0; x < width; ++x)
            {
                const char c = rowString[x];
                tiles.push_back(c == '1' ? 1 : 0);
            }

            lua_pop(L, 1); // row string
        }

        lua_pop(L, 1); // rows

        // build map
        outLevel.map = TileMap{ width, height, tileSize };

        // spawn
        lua_getfield(L, levelIndex, "spawn");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "x");
            outLevel.playerSpawn.x = static_cast<float>(luaL_optnumber(L, -1, 0.0));
            lua_pop(L, 1);

            lua_getfield(L, -1, "y");
            outLevel.playerSpawn.y = static_cast<float>(luaL_optnumber(L, -1, 0.0));
            lua_pop(L, 1);
        }
        lua_pop(L, 1); // spawn

        outLevel.playerSpawn.width  = 32.0f;
        outLevel.playerSpawn.height = 64.0f;

        lua_pop(L, 1); // level table
        return true;
    }
} // namespace game
