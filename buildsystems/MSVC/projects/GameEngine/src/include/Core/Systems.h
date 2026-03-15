#pragma once

#include "Components.h"
#include "ECS.h"

#include <lua.hpp>
#include <raylib.h>

#include <string>

namespace Core
{

    // ------------------------------------------------------------
    // Minimal Lua API binding (raw C API, no sol2)
    // Exposes a global table `ecs` with a few helpers.
    // ------------------------------------------------------------
    namespace LuaAPI
    {

        static ECS* GetECS(lua_State* L)
        {
            // upvalue 1 = ECS*
            return static_cast<ECS*>(lua_touserdata(L, lua_upvalueindex(1)));
        }

        static Entity CheckEntityArg(lua_State* L, int idx)
        {
            // Entities are passed as integer IDs from Lua.
            const lua_Integer id = luaL_checkinteger(L, idx);
            return static_cast<Entity>(static_cast<entt::id_type>(id));
        }

        static int l_create_entity(lua_State* L)
        {
            ECS*   ecs = GetECS(L);
            Entity e   = ecs->CreateEntity();
            lua_pushinteger(L, static_cast<lua_Integer>(ECS::ToId(e)));
            return 1;
        }

        static int l_destroy_entity(lua_State* L)
        {
            ECS*   ecs = GetECS(L);
            Entity e   = CheckEntityArg(L, 1);
            ecs->DestroyEntity(e);
            return 0;
        }

        static int l_get_transform(lua_State* L)
        {
            ECS*   ecs = GetECS(L);
            Entity e   = CheckEntityArg(L, 1);

            if (!ecs->Valid(e) || !ecs->Has<Transform2D>(e))
            {
                lua_pushnil(L);
                return 1;
            }

            const auto& t = ecs->Get<Transform2D>(e);

            lua_newtable(L);
            lua_pushnumber(L, t.position.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, t.position.y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, t.rotationDeg);
            lua_setfield(L, -2, "rot");
            lua_pushnumber(L, t.scale.x);
            lua_setfield(L, -2, "sx");
            lua_pushnumber(L, t.scale.y);
            lua_setfield(L, -2, "sy");
            return 1;
        }

        static int l_set_transform(lua_State* L)
        {
            ECS*   ecs = GetECS(L);
            Entity e   = CheckEntityArg(L, 1);

            if (!ecs->Valid(e))
                return 0;
            if (!ecs->Has<Transform2D>(e))
                ecs->Add<Transform2D>(e);

            auto& t = ecs->Get<Transform2D>(e);

            // Allow either:
            // ecs.set_transform(id, x, y, rot, sx, sy)
            // or ecs.set_transform(id, {x=..., y=..., rot=..., sx=..., sy=...})
            if (lua_istable(L, 2))
            {
                lua_getfield(L, 2, "x");
                if (lua_isnumber(L, -1))
                    t.position.x = (float)lua_tonumber(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, 2, "y");
                if (lua_isnumber(L, -1))
                    t.position.y = (float)lua_tonumber(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, 2, "rot");
                if (lua_isnumber(L, -1))
                    t.rotationDeg = (float)lua_tonumber(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, 2, "sx");
                if (lua_isnumber(L, -1))
                    t.scale.x = (float)lua_tonumber(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, 2, "sy");
                if (lua_isnumber(L, -1))
                    t.scale.y = (float)lua_tonumber(L, -1);
                lua_pop(L, 1);
                return 0;
            }

            t.position.x = (float)luaL_checknumber(L, 2);
            t.position.y = (float)luaL_checknumber(L, 3);
            if (lua_gettop(L) >= 4)
                t.rotationDeg = (float)luaL_checknumber(L, 4);
            if (lua_gettop(L) >= 5)
                t.scale.x = (float)luaL_checknumber(L, 5);
            if (lua_gettop(L) >= 6)
                t.scale.y = (float)luaL_checknumber(L, 6);
            return 0;
        }

        static void Bind(lua_State* L, ECS& ecs)
        {
            // ecs table
            lua_newtable(L);

            auto bindFn = [&](const char* name, lua_CFunction fn)
            {
                lua_pushlightuserdata(L, &ecs); // upvalue 1
                lua_pushcclosure(L, fn, 1);
                lua_setfield(L, -2, name);
            };

            bindFn("create_entity", l_create_entity);
            bindFn("destroy_entity", l_destroy_entity);
            bindFn("get_transform", l_get_transform);
            bindFn("set_transform", l_set_transform);

            lua_setglobal(L, "ecs");
        }

    } // namespace LuaAPI

    // ------------------------------------------------------------
    // MovementSystem: Transform += Velocity
    // ------------------------------------------------------------
    struct MovementSystem final : ISystem
    {
        void Update(ECS& ecs, float dt) override
        {
            auto view = ecs.View<Transform2D, Velocity2D>();
            for (auto e : view)
            {
                auto&       tr  = view.get<Transform2D>(e);
                const auto& v   = view.get<Velocity2D>(e);
                tr.position.x  += v.value.x * dt;
                tr.position.y  += v.value.y * dt;
            }
        }
    };

    // ------------------------------------------------------------
    // ScriptSystem: loads module, creates per-entity instance, calls update()
    // Lua convention expected (simple, flexible):
    //   local M = {}
    //   function M:init() ... end
    //   function M:update(dt) ... end
    //   return M
    //
    // Each entity gets a table `self` with metatable __index = module table.
    // `self.entity` is set to the entity id.
    // ------------------------------------------------------------
    struct ScriptSystem final : ISystem
    {
        void Init(ECS& ecs) override
        {
            lua_State* L = ecs.Lua();
            if (!L)
            {
                TraceLog(
                    LOG_WARNING,
                    "ScriptSystem: No lua_State attached to ECS.");
                return;
            }
            LuaAPI::Bind(L, ecs);
        }

        void Shutdown(ECS& ecs) override
        {
            lua_State* L = ecs.Lua();
            if (!L)
                return;

            // Release all per-entity instance refs
            auto view = ecs.View<ScriptComponent>();
            for (auto e : view)
            {
                auto& sc = view.get<ScriptComponent>(e);
                if (sc.instanceRef != LUA_NOREF)
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, sc.instanceRef);
                    sc.instanceRef = LUA_NOREF;
                }
                sc.needsInit = true;
            }
        }

        void Update(ECS& ecs, float dt) override
        {
            lua_State* L = ecs.Lua();
            if (!L)
                return;

            auto view = ecs.View<ScriptComponent>();
            for (auto e : view)
            {
                auto& sc = view.get<ScriptComponent>(e);

                if (sc.needsInit)
                    Instantiate(ecs, e, sc);

                if (sc.instanceRef == LUA_NOREF)
                    continue;

                // call self:update(dt)
                lua_rawgeti(L, LUA_REGISTRYINDEX, sc.instanceRef); // self

                lua_getfield(L, -1, "update"); // method (respects __index)
                if (lua_isfunction(L, -1))
                {
                    lua_pushvalue(L, -2);  // self
                    lua_pushnumber(L, dt); // dt
                    if (lua_pcall(L, 2, 0, 0) != LUA_OK)
                    {
                        const char* err = lua_tostring(L, -1);
                        TraceLog(
                            LOG_ERROR,
                            "Lua update error (%s): %s",
                            sc.module.c_str(),
                            err ? err : "(unknown)");
                        lua_pop(L, 1);
                    }
                }
                else
                {
                    lua_pop(L, 1); // pop non-function
                }

                lua_pop(L, 1); // pop self
            }
        }

      private:
        static void Instantiate(ECS& ecs, Entity e, ScriptComponent& sc)
        {
            lua_State* L = ecs.Lua();
            if (!L)
                return;

            // Clear any previous instance
            if (sc.instanceRef != LUA_NOREF)
            {
                luaL_unref(L, LUA_REGISTRYINDEX, sc.instanceRef);
                sc.instanceRef = LUA_NOREF;
            }

            // module = require(sc.module)
            lua_getglobal(L, "require");
            lua_pushstring(L, sc.module.c_str());
            if (lua_pcall(L, 1, 1, 0) != LUA_OK)
            {
                const char* err = lua_tostring(L, -1);
                TraceLog(
                    LOG_ERROR,
                    "Lua require error (%s): %s",
                    sc.module.c_str(),
                    err ? err : "(unknown)");
                lua_pop(L, 1);
                sc.needsInit = false;
                return;
            }

            if (!lua_istable(L, -1))
            {
                TraceLog(
                    LOG_ERROR,
                    "Lua module '%s' did not return a table.",
                    sc.module.c_str());
                lua_pop(L, 1);
                sc.needsInit = false;
                return;
            }

            // Stack: [module]
            lua_newtable(L); // self
            // Stack: [module, self]

            // self.entity = entityId
            lua_pushinteger(L, (lua_Integer)ECS::ToId(e));
            lua_setfield(L, -2, "entity");

            // setmetatable(self, { __index = module })
            lua_newtable(L);      // mt
            lua_pushvalue(L, -3); // module
            lua_setfield(L, -2, "__index");
            lua_setmetatable(L, -2); // sets mt on self, pops mt
            // Stack: [module, self]

            // if self.init exists, call self:init()
            lua_getfield(L, -1, "init");
            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2); // self
                if (lua_pcall(L, 1, 0, 0) != LUA_OK)
                {
                    const char* err = lua_tostring(L, -1);
                    TraceLog(
                        LOG_ERROR,
                        "Lua init error (%s): %s",
                        sc.module.c_str(),
                        err ? err : "(unknown)");
                    lua_pop(L, 1);
                }
            }
            else
            {
                lua_pop(L, 1); // pop non-function
            }

            // ref self, pop it
            sc.instanceRef = luaL_ref(L, LUA_REGISTRYINDEX);

            // pop module
            lua_pop(L, 1);

            sc.needsInit = false;
        }
    };

    // ------------------------------------------------------------
    // RenderSystem: draws Sprite + Transform2D
    // (Very basic; you can add sorting / camera / batching later.)
    // ------------------------------------------------------------
    struct RenderSystem final : ISystem
    {
        void Update(ECS& ecs, float /*dt*/) override
        {
            auto view = ecs.View<Transform2D, Sprite>();

            for (auto e : view)
            {
                const auto& tr = view.get<Transform2D>(e);
                const auto& sp = view.get<Sprite>(e);

                if (sp.texture.id == 0)
                    continue;

                Rectangle src = sp.source;
                if (src.width == 0.0f || src.height == 0.0f)
                {
                    src = Rectangle{ 0,
                                     0,
                                     (float)sp.texture.width,
                                     (float)sp.texture.height };
                }

                Rectangle dst;
                dst.x      = tr.position.x;
                dst.y      = tr.position.y;
                dst.width  = src.width * tr.scale.x;
                dst.height = src.height * tr.scale.y;

                DrawTexturePro(
                    sp.texture,
                    src,
                    dst,
                    sp.origin,
                    tr.rotationDeg,
                    sp.tint);
            }
        }
    };

} // namespace Core
