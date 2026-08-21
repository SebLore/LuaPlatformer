#include "LuaInput.hpp"

#include <cstring>
#include <raylib.h>

namespace
{
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

        int raylibKey = KEY_NULL;

        if (std::strcmp(key, "1") == 0)
            raylibKey = KEY_ONE;
        else if (std::strcmp(key, "2") == 0)
            raylibKey = KEY_TWO;
        else
            return luaL_error(L, "Unknown input key '%s'", key);

        lua_pushboolean(L, IsKeyPressed(raylibKey));

        return 1;
    }
} // namespace

void LuaInput::Open(lua_State* L)
{
    lua_newtable(L);

    static const luaL_Reg methods[] = { { "GetMousePosition", lua_GetMousePosition },
                                        { "MousePressed", lua_MousePressed },
                                        { "KeyPressed", lua_KeyPressed },

                                        { nullptr, nullptr } };

    luaL_setfuncs(L, methods, 0);

    lua_setglobal(L, "input");
}
