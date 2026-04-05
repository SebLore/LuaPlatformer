#pragma once

#include "LuaWrapper.h"
#include "raylib.h"
#include <entt/entt.hpp>

#include "ECS/Components/BaseComponents.h"
//#include "Components.h"

using namespace Lua;

namespace binding
{

    namespace
    {
        entt::entity lNumberToEntity(lua_State* L, int index)
        {
            return static_cast<entt::entity>(static_cast<entt::id_type>(luaL_checkinteger(L, index)));
        }
    } // namespace

    using namespace components;
    // DrawCircle(x, y, r)  -- default color RED for now
    static int l_DrawCircle(lua_State* L)
    {
        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);
        float r = (float)luaL_checknumber(L, 3);

        DrawCircle((int)x, (int)y, r, RED);
        return 0;
    }

    // ClearBackground(r, g, b, a)  -- all 0..255
    static int l_ClearBackgroundRGBA(lua_State* L)
    {
        int r = (int)luaL_checkinteger(L, 1);
        int g = (int)luaL_checkinteger(L, 2);
        int b = (int)luaL_checkinteger(L, 3);
        int a = (int)luaL_optinteger(L, 4, 255);

        ClearBackground(Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a });
        return 0;
    }

    // GetScreenWidth() / GetScreenHeight()
    static int l_GetScreenWidth(lua_State* L)
    {
        lua_pushinteger(L, GetScreenWidth());
        return 1;
    }
    static int l_GetScreenHeight(lua_State* L)
    {
        lua_pushinteger(L, GetScreenHeight());
        return 1;
    }

    // Input: IsKeyDown(keycode)
    static int l_IsKeyDown(lua_State* L)
    {
        int key = (int)luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsKeyDown(key));
        return 1;
    }

    static entt::registry* get_registry(lua_State* L)
    {
        return static_cast<entt::registry*>(lua_touserdata(L, lua_upvalueindex(1)));
    }

    static int l_ecs_create(lua_State* L)
    {
        auto*        reg = get_registry(L);
        entt::entity e   = reg->create();
        lua_pushinteger(L, (lua_Integer)entt::to_integral(e));
        return 1;
    }

    // entt / ECS

    static int l_ecs_add(lua_State* L)
    {
        auto* reg = get_registry(L);

        auto eid = lNumberToEntity(L, 1);

        const char* cname = luaL_checkstring(L, 2);
        luaL_checktype(L, 3, LUA_TTABLE);
        int t = lua_absindex(L, 3);

        if (std::string(cname) == "Position")
        {
            auto& c = reg->emplace_or_replace<Position>(eid);

            lua_getfield(L, t, "x");
            c.x = (float)luaL_optnumber(L, -1, c.x);
            lua_pop(L, 1);
            lua_getfield(L, t, "y");
            c.y = (float)luaL_optnumber(L, -1, c.y);
            lua_pop(L, 1);
            return 0;
        }

        if (std::string(cname) == "Circle")
        {
            auto& c = reg->emplace_or_replace<Circle>(eid);

            lua_getfield(L, t, "r");
            c.r = (float)luaL_optnumber(L, -1, c.r);
            lua_pop(L, 1);
            return 0;
        }

        return luaL_error(L, "Unknown component type: %s", cname);
    }

    static int l_ecs_get(lua_State* L)
    {
        auto* reg = get_registry(L);

        auto        eid   = lNumberToEntity(L, 1);
        const char* cname = luaL_checkstring(L, 2);

        lua_newtable(L);

        if (std::string(cname) == "Position")
        {
            auto* c = reg->try_get<Position>(eid);
            if (!c)
                return 1;

            lua_pushnumber(L, c->x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, c->y);
            lua_setfield(L, -2, "y");
            return 1;
        }

        if (std::string(cname) == "Circle")
        {
            auto* c = reg->try_get<Circle>(eid);
            if (!c)
                return 1;

            lua_pushnumber(L, c->r);
            lua_setfield(L, -2, "r");
            return 1;
        }

        return luaL_error(L, "Unknown component type: %s", cname);
    }

    static int l_ecs_set(lua_State* L)
    {
        auto* reg = get_registry(L);

        auto        eid   = lNumberToEntity(L, 1);
        const char* cname = luaL_checkstring(L, 2);
        luaL_checktype(L, 3, LUA_TTABLE);
        int t = lua_absindex(L, 3);

        if (std::string(cname) == "Position")
        {
            auto& c = reg->get_or_emplace<Position>(eid);
            lua_getfield(L, t, "x");
            c.x = (float)luaL_optnumber(L, -1, c.x);
            lua_pop(L, 1);
            lua_getfield(L, t, "y");
            c.y = (float)luaL_optnumber(L, -1, c.y);
            lua_pop(L, 1);
            return 0;
        }

        if (std::string(cname) == "Circle")
        {
            auto& c = reg->get_or_emplace<Circle>(eid);
            lua_getfield(L, t, "r");
            c.r = (float)luaL_optnumber(L, -1, c.r);
            lua_pop(L, 1);
            return 0;
        }

        return luaL_error(L, "Unknown component type: %s", cname);
    }

    static void lua_bind_ecs(LuaWrapper& lua, entt::registry& registry)
    {
        lua_State* L = lua.State();
        lua_newtable(L); // ecs

        auto bind = [&](const char* name, lua_CFunction fn)
        {
            lua_pushlightuserdata(L, &registry);
            lua_pushcclosure(L, fn, 1);
            lua_setfield(L, -2, name);
        };

        bind("create", l_ecs_create);
        bind("add", l_ecs_add);
        bind("get", l_ecs_get);
        bind("set", l_ecs_set);

        lua_setglobal(L, "ecs");
    }

    inline void lua_register_functions(LuaWrapper& lua)
    {
        lua.RegisterFunction("DrawCircle", l_DrawCircle);
        lua.RegisterFunction("ClearBackgroundRGBA", l_ClearBackgroundRGBA);
        lua.RegisterFunction("GetScreenWidth", l_GetScreenWidth);
        lua.RegisterFunction("GetScreenHeight", l_GetScreenHeight);
        lua.RegisterFunction("IsKeyDown", l_IsKeyDown);
    }

} // namespace binding
