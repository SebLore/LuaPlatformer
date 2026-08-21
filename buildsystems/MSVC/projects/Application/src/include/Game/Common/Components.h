#pragma once

#include <raylib.h>
#include <lua.hpp>

#include <cstdint>
#include <cstring>

namespace components
{
    struct PositionI
    {
        int x = 0, y = 0;
    };

    struct Transform
    {
        Vector2 position{ .x = 0, .y = 0 };
        float   rotation = 0;
        Vector2 scale{ .x = 1, .y = 1 };
    };

    struct Velocity : Vector2
    {
    };

    struct Position : Vector2
    {
    };

    struct Solid
    {
    };

    struct Hazard
    {
    };

    struct Goal
    {
    };

    struct Collider
    {
        Vector2 size{ 0.0f, 0.0f };
    };

    struct Renderable
    {
        Color color = WHITE;
    };

    struct Player
    {
        bool grounded    = false;
        bool dead        = false;
        bool reachedGoal = false;
    };

    /// Special component to be registered through Lua when creating new behaviours
    struct Behaviour
    {
        char scriptPath[64];
        int  luaRef = LUA_NOREF; // table reference

        int  threadRef = LUA_NOREF; // thread reference holding coroutine alive
        bool started   = false;
        bool finished  = false;

        Behaviour(const char* path, int luaRef) : luaRef(luaRef)
        {
            memset(scriptPath, '\0', 64);

            strcpy_s(this->scriptPath, sizeof(this->scriptPath), path);
        }
    };

    struct UIText
    {
        char text[256];

        int   fontSize;
        Color color;

        UIText(const char* text, int fontSize = 24, Color color = BLACK) : fontSize(fontSize), color(color)
        {
            memset(this->text, '\0', sizeof(this->text));
            strcpy_s(this->text, sizeof(this->text), text);
        }
    };

} // namespace components
