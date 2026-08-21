#include "LuaBindInput.h"

#include <cstring>
#include <raylib.h>

namespace
{
    // helper
    int GetKeyCode(const char* key)
    {
        if (strcmp(key, "1") == 0)
            return KEY_ONE;
        if (strcmp(key, "2") == 0)
            return KEY_TWO;
        if (strcmp(key, "S") == 0)
            return KEY_S;
        if (strcmp(key, "L") == 0)
            return KEY_L;
        if (strcmp(key, "A") == 0)
            return KEY_A;
        if (strcmp(key, "D") == 0)
            return KEY_D;
        if (strcmp(key, "W") == 0)
            return KEY_W;
        if (strcmp(key, "SPACE") == 0)
            return KEY_SPACE;
        if (strcmp(key, "LEFT") == 0)
            return KEY_LEFT;
        if (strcmp(key, "RIGHT") == 0)
            return KEY_RIGHT;
        if (strcmp(key, "UP") == 0)
            return KEY_UP;

        return KEY_NULL;
    }

    /// @brief Enable lua environment to get mouse position from raylib's function
    /// @param L Lua State
    /// @return Number of params passed to lua, 2
    int lua_GetMousePosition(lua_State* L)
    {
        const Vector2 mouse = GetMousePosition();

        lua_pushnumber(L, mouse.x);
        lua_pushnumber(L, mouse.y);

        return 2;
    }

    int lua_MousePressed(lua_State* L)
    {
        lua_pushboolean(L, IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

        return 1;
    }

    int lua_KeyPressed(lua_State* L)
    {
        const char* key = luaL_checkstring(L, 1);

        int raylibKey = GetKeyCode(key);

        if (raylibKey == KEY_NULL)
            return luaL_error(L, "Unknown input key '%s'", key);

        lua_pushboolean(L, IsKeyPressed(raylibKey));

        return 1;
    }

    int lua_KeyDown(lua_State* L)
    {
        const char* key = luaL_checkstring(L, 1);

        int raylibKey = GetKeyCode(key);

        if (raylibKey == KEY_NULL)
            return luaL_error(L, "Unknown input key '%s'", key);

        lua_pushboolean(L, IsKeyDown(raylibKey));

        return 1;
    }
} // namespace

// create new table in lua for inputs
void LuaInput::Open(lua_State* L)
{
    lua_newtable(L);

    // because we used an anonymous namespace these are 'static' by default
    static constexpr luaL_Reg methods[] = { { "GetMousePosition", lua_GetMousePosition },
                                            { "MousePressed", lua_MousePressed },
                                            { "KeyPressed", lua_KeyPressed },
                                            { "KeyDown", lua_KeyDown },

                                            { nullptr, nullptr } };

    luaL_setfuncs(L, methods, 0);

    lua_setglobal(L, "input");
}
