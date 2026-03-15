#pragma once

#include <cstdint>
#include <string>

// raylib types
#include <raylib.h>

// for Lua registry refs in ScriptComponent
#include <lua.hpp>

namespace Core
{

    // ------------------------
    // Common gameplay bits
    // ------------------------
    struct Tag
    {
        std::string value;
    };

    struct Transform2D
    {
        Vector2 position{ 0.0f, 0.0f };
        float   rotationDeg = 0.0f;
        Vector2 scale{ 1.0f, 1.0f };
    };

    struct Velocity2D
    {
        Vector2 value{ 0.0f, 0.0f };
    };

    // ------------------------
    // Rendering
    // ------------------------
    struct Sprite
    {
        // Keep this simple; you can replace with handles/asset manager later.
        Texture2D texture{};
        Rectangle source{ 0,
                          0,
                          0,
                          0 }; // if width/height are 0, treat as full texture
        Vector2   origin{ 0.0f, 0.0f };
        Color     tint{ WHITE };
        float     z = 0.0f; // simple layering if you want it later
    };

    // Optional camera “as a component”
    struct Camera2DComponent
    {
        Camera2D cam{};
    };

    // ------------------------
    // Lua scripting
    // ------------------------
    struct ScriptComponent
    {
        // Lua module name that `require()` can load (e.g. "scripts.player")
        std::string module;

        // Per-entity script instance (Lua registry reference)
        int instanceRef = LUA_NOREF;

        // If true, ScriptSystem will attempt to (re)instantiate on next Update
        bool needsInit = true;
    };

} // namespace Core
