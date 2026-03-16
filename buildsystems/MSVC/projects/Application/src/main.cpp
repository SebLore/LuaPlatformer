

#include <entt/entt.hpp>
#include <raylib.h>

#include "Assets/AssetManager.h"
#include "LuaBindings.h"
#include "LuaWrapper/LuaWrapper.h"
#include "utility/fileutils.h"

#ifdef USE_META // meta support for reflection
#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp> // meta_factory
#include <entt/meta/meta.hpp>
#endif

#include <filesystem>
#include <iostream>

// -- tiny helpers to call Lua functions safely --
namespace
{
    void register_meta()
    {
#ifdef USE_META
        using namespace entt::literals;

        entt::meta_factory<Position>{}
            .type("Position"_hs)
            .data<&Position::x>("x"_hs)
            .data<&Position::y>("y"_hs);

        entt::meta_factory<Circle>{}.type("Circle"_hs).data<&Circle::r>("r"_hs);
#endif
    }
} // namespace

struct PlayerTest
{
    TextureId texture;
    Rectangle srcRect  = { 0, 0, 32, 32 };
    Rectangle dstRect  = { 64, 64, 64, 64 };
    Vector2   origin   = { 0, 0 };
    float     rotation = 0;
    Color     tint     = RAYWHITE;
};

struct Game
{
    static constexpr auto* SCRIPTS_DIR = "scripts";

    entt::registry       registry{};
    Lua::LuaWrapper      lua{};
    assets::AssetManager assets{};

    void Initialize()
    {
        lua.SetScriptsPath(util::FindDirectory(SCRIPTS_DIR).string());

        // setting up raylib
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
        InitWindow(800, 600, "Raylib time!");

        // Bind a few functions
        binding::lua_register_functions(lua);

        // Bind a couple of key constants (so Lua doesn’t need magic numbers)
        lua.SetGlobal("KEY_LEFT", KEY_LEFT);
        lua.SetGlobal("KEY_RIGHT", KEY_RIGHT);
        lua.SetGlobal("KEY_UP", KEY_UP);
        lua.SetGlobal("KEY_DOWN", KEY_DOWN);

        binding::l_bind_ecs(lua, registry);
    }

    void DrawPlayerTest(const PlayerTest& player) const
    {
        if (const Texture2D* tex = assets.TryGetTexture(player.texture))
            DrawTexturePro(
                *tex,
                player.srcRect,
                player.dstRect,
                player.origin,
                player.rotation,
                player.tint);
    }
};

int main(void)
{
    Game game;
    game.Initialize();

    // setting up lua

    Lua::LuaWrapper& lua = game.lua;

    register_meta();

    // Optional init (now it can exist)
    CallLuaVoid0(lua, "init");

    if (!lua.RequireModule("circle"))
    {
        std::cerr << "Failed to get module circle: " << lua.Error() << "\n";
        CloseWindow();
        return EXIT_FAILURE;
    }

    // -- Initialize game --

    {
        auto& reg    = game.registry;
        auto  e      = reg.create();
        auto& player = reg.emplace_or_replace<PlayerTest>(e);

        player.texture = game.assets.RegisterTexture("test_tiles.png");

        if (!game.assets.LoadTextureFile(player.texture))
        {
            std::cerr << "Failed to load player texture\n";
            CloseWindow();
            return EXIT_FAILURE;
        }
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Let Lua update state
        if (!CallLuaVoid1Number(lua, "update", dt))
            break;

        BeginDrawing();

        // Lua decides background and drawing
        if (!CallLuaVoid0(lua, "draw"))
        {
            EndDrawing();
            break;
        }

        {
            auto view = game.registry.view<PlayerTest>();
            for (auto entity : view)
            {
                const auto& player = view.get<PlayerTest>(entity);
                game.DrawPlayerTest(player);
            }
        }

        EndDrawing();
    }

    CloseWindow();
}
