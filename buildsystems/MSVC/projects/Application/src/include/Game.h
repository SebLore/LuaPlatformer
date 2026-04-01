#pragma once

#include <raylib.h>

#include "Components.h"

#include <entt/entt.hpp>
#include <raylib.h>

#include "LuaBindings.h"
#include "utility/fileutils.h"

#include <LuaWrapper.h>
#include "Assets/AssetManager.h"

namespace game
{
    enum class GameState : uint8_t
    {
        Ready,
        Running,
        Paused,
        GameOver,
        Editor,
        Exiting
    };

    class Game
    {
      public:
        ~Game() { CloseWindow(); }
        bool Initialize()
        {
            // setting up raylib
            SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
            InitWindow(800, 600, "Raylib time!");

            if (!LoadScripts())
            {
                std::cerr << "Failed load lua scripts. Exiting...\n";
                state = GameState::Exiting;
                return false;
            }
            if (!LoadGameState())
            {
                std::cerr << "Failed to load game state. Exiting...\n";
                state = GameState::Exiting;
                return false;
            }

            return true;
        }
        void Run()
        {
            while (!WindowShouldClose() && state != GameState::Exiting)
            {
                HandleInput();
                Update();
                Draw();
            }
            CloseWindow();
        }

      public:
        entt::registry       registry{};
        Lua::LuaWrapper      lua{};
        assets::AssetManager assets{};

      private:
        bool LoadScripts()
        {
            lua.SetScriptsPath(util::FindDirectory(SCRIPTS_DIR).string());
            binding::lua_register_functions(lua);

            // Bind a couple of key constants (so Lua doesn't need magic numbers)
            lua.SetGlobal("KEY_LEFT", KEY_LEFT);
            lua.SetGlobal("KEY_RIGHT", KEY_RIGHT);
            lua.SetGlobal("KEY_UP", KEY_UP);
            lua.SetGlobal("KEY_DOWN", KEY_DOWN);

            binding::lua_bind_ecs(lua, registry);

            // Optional init (now it can exist)
            CallLuaVoid0(lua, "init");

            if (!lua.RequireModule("circle"))
            {
                std::cerr << "Failed to get module circle: " << lua.Error() << "\n";
                return false;
            }

            return true;
        }

        bool LoadGameState() { return SetupInitialGameState(); }

        void DrawPlayerTest(const components::PlayerTest& player) const
        {
            if (const Texture2D* tex = assets.TryGetTexture(player.texture))
                DrawTexturePro(*tex, player.srcRect, player.dstRect, player.origin, player.rotation, player.tint);
        }

        void HandleInput()
        {
            // no op for now TODO: implement controls if needed
        }
        void Update()
        {
            float dt = GetFrameTime();

            // If lua update fails, break the loop and close the game
            if (!CallLuaVoid1Number(lua, "update", dt))
                state = GameState::Exiting;
        }
        void Draw()
        {
            BeginDrawing();

            // Lua decides background and drawing
            if (!CallLuaVoid0(lua, "draw"))
            {
                EndDrawing();
                state = GameState::Exiting;
                return;
            }

            {
                auto view = registry.view<components::PlayerTest>();
                for (auto entity : view)
                {
                    const auto& player = view.get<components::PlayerTest>(entity);
                    DrawPlayerTest(player);
                }
            }

            EndDrawing();
        }

        bool SetupInitialGameState()
        {
            // -- Initialize game state --
            {
                auto& reg    = this->registry;
                auto  e      = reg.create();
                auto& player = reg.emplace_or_replace<components::PlayerTest>(e);

                player.texture = assets.RegisterTexture("test_tiles.png");
                if (!assets.LoadTextureFile(player.texture))
                {
                    std::cerr << "Failed to load player texture\n";
                    return false;
                }
            }

            return true;
        }

      private:
        static constexpr auto* SCRIPTS_DIR = "scripts";

        GameState state = GameState::Ready;
    };
} // namespace game
