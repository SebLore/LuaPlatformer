// Game.cpp
#include "Game.h"

#include "Components.h"
#include "LevelEditor/LevelLoader.h"
#include "LuaBindings.h"
#include "utility/fileutils.h"

#include <iostream>

#include "Tags.h"

namespace game
{
    namespace
    {
        constexpr float kMoveSpeed = 220.0f;
        constexpr float kJumpSpeed = -520.0f;
        constexpr float kGravity   = 1600.0f;

        bool RectHitsSolid(const LevelData& level, Rectangle rect)
        {
            const int tileSize = level.map.TileSize();

            const int left   = static_cast<int>(rect.x) / tileSize;
            const int right  = static_cast<int>(rect.x + rect.width - 1.0f) / tileSize;
            const int top    = static_cast<int>(rect.y) / tileSize;
            const int bottom = static_cast<int>(rect.y + rect.height - 1.0f) / tileSize;

            for (int ty = top; ty <= bottom; ++ty)
            {
                for (int tx = left; tx <= right; ++tx)
                    if (level.IsSolidForCollision(tx, ty))
                        return true;
            }

            return false;
        }
    } // namespace

    Game::~Game()
    {
        if (IsWindowReady())
            CloseWindow();
    }

    bool Game::Initialize()
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
        InitWindow(800, 600, "Raylib time!");
        SetTargetFPS(60);

        if (!LoadScripts())
        {
            std::cerr << "Failed to load lua scripts. Exiting...\n";
            m_State = GameState::Exiting;
            return false;
        }

        if (!LoadGameState())
        {
            std::cerr << "Failed to load game state. Exiting...\n";
            m_State = GameState::Exiting;
            return false;
        }

        m_State = GameState::Running;
        return true;
    }

    void Game::Run()
    {
        while (!WindowShouldClose() && m_State != GameState::Exiting)
        {
            HandleInput();
            Update();
            Draw();
        }
    }

    bool Game::LoadScripts()
    {
        m_Lua.SetScriptsPath(util::FindDirectory(SCRIPTS_DIR).string());

        binding::lua_register_functions(m_Lua);
        binding::lua_bind_ecs(m_Lua, m_Registry);

        // Bind keys so Lua logic can use them later
        m_Lua.SetGlobal("KEY_A", KEY_A);
        m_Lua.SetGlobal("KEY_D", KEY_D);
        m_Lua.SetGlobal("KEY_W", KEY_W);
        m_Lua.SetGlobal("KEY_S", KEY_S);
        m_Lua.SetGlobal("KEY_SPACE", KEY_SPACE);

        m_Lua.SetGlobal("KEY_LEFT", KEY_LEFT);
        m_Lua.SetGlobal("KEY_RIGHT", KEY_RIGHT);
        m_Lua.SetGlobal("KEY_UP", KEY_UP);
        m_Lua.SetGlobal("KEY_DOWN", KEY_DOWN);

        // Optional global init() if some bootstrap script created it.
        if (!CallLuaVoid0(m_Lua, "init"))
        {
            std::cerr << "Lua init() failed: " << m_Lua.Error() << "\n";
            return false;
        }

        return true;
    }

    bool Game::LoadGameState() { return SetupInitialGameState(); }

    bool Game::SetupInitialGameState()
    {
        const char* texpath = "test_tiles.png";
        m_TilesTexture = m_Assets.RegisterTexture(texpath);
        if (!m_Assets.LoadTextureFile(m_TilesTexture))
        {
            std::cerr << "Failed to load tiles texture: " << texpath << "\n";
            return false;
        }

        if (!LoadLevelFromLua(m_Lua, "testlevel", m_Level))
        {
            std::cerr << "Failed to load level from lua: " << m_Lua.Error() << "\n";
            return false;
        }

        SpawnPlayer();
        return true;
    }

    void Game::HandleInput()
    {
        // Reserved for app-level hotkeys later (editor/game switch, pause, etc.)
    }

    void Game::Update()
    {
        const float dt = GetFrameTime();

        // Optional Lua update(dt). Missing function is treated as success by your helper.
        if (!CallLuaVoid1Number(m_Lua, "update", dt))
        {
            std::cerr << "Lua update(dt) failed: " << m_Lua.Error() << "\n";
            m_State = GameState::Exiting;
            return;
        }

        // Temporary C++ player behavior.
        // Later, remove this when movement/behavior is fully Lua-driven.
        UpdatePlayers(dt);
    }

    void Game::Draw()
    {
        BeginDrawing();
        ClearBackground(Color{ 20, 20, 30, 255 });

        DrawWorld();
        DrawDebug();
        DrawUI();

        EndDrawing();
    }

    void Game::DrawWorld()
    {
        DrawLevel();
        DrawPlayers();
    }

    void Game::DrawDebug()
    {
        if (m_State == GameState::Editor)
            DrawSpawnMarker();
    }

    void Game::DrawUI()
    {
        // no UI for now
    }

    void Game::DrawLevel() const
    {
        const Texture2D* tileset = m_Assets.TryGetTexture(m_TilesTexture);

        const Rectangle src{ 0.0f, 0.0f, 16.0f, 16.0f }; // first 16x16 quadrant = tile "1"

        int height = m_Level.map.Height();
        int width  = m_Level.map.Width();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const std::uint8_t tile = m_Level.map.Get(x, y);

                if (tile == 0)
                    continue;

                const float     size = static_cast<float>(m_Level.map.TileSize());
                const Rectangle dst{ static_cast<float>(x) * size, static_cast<float>(y) * size, size, size };
                if (tileset)
                    DrawTexturePro(*tileset, src, dst, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
                else
                    DrawRectangleRec(dst, GREEN);
            }
        }
    }

    void Game::DrawSpawnMarker() const
    {
        DrawRectangleRec(m_Level.playerSpawn, Color{ 255, 0, 0, 120 });
        DrawRectangleLinesEx(m_Level.playerSpawn, 1.0f, RED);
    }

    void Game::DrawPlayers() const
    {
        auto view =
            m_Registry.view<PlayerTag, components::Position, components::BoxCollider, components::DebugRenderRect>();

        for (auto entity : view)
        {
            const auto& pos      = view.get<components::Position>(entity);
            const auto& collider = view.get<components::BoxCollider>(entity);
            const auto& render   = view.get<components::DebugRenderRect>(entity);

            DrawRectangle(static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(collider.size.x),
                static_cast<int>(collider.size.y), render.color);
        }
    }

    void Game::UpdatePlayers(float dt)
    {
        auto view = m_Registry.view<
            PlayerTag, components::PlayerControlled, components::Position, components::Velocity,
            components::BoxCollider, components::Grounded>();

        for (auto entity : view)
        {
            auto& pos      = view.get<components::Position>(entity);
            auto& vel      = view.get<components::Velocity>(entity);
            auto& collider = view.get<components::BoxCollider>(entity);
            auto& grounded = view.get<components::Grounded>(entity);

            float moveX = 0.0f;
            if (IsKeyDown(KEY_A))
                moveX -= 1.0f;
            if (IsKeyDown(KEY_D))
                moveX += 1.0f;

            vel.x = moveX * kMoveSpeed;

            if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE)) && grounded.value)
            {
                vel.y          = kJumpSpeed;
                grounded.value = false;
            }

            vel.y += kGravity * dt;

            // Horizontal move
            pos.x += vel.x * dt;
            {
                Rectangle rect{ pos.x, pos.y, collider.size.x, collider.size.y };

                if (RectHitsSolid(m_Level, rect))
                {
                    pos.x -= vel.x * dt;
                    vel.x  = 0.0f;
                }
            }

            // Vertical move
            pos.y          += vel.y * dt;
            grounded.value  = false;
            {
                Rectangle rect{ pos.x, pos.y, collider.size.x, collider.size.y };

                if (RectHitsSolid(m_Level, rect))
                {
                    if (vel.y > 0.0f)
                        grounded.value = true;

                    pos.y -= vel.y * dt;
                    vel.y  = 0.0f;
                }
            }
        }
    }

    entt::entity Game::FindPlayer() const
    {
        auto view = m_Registry.view<PlayerTag>();

        for (auto entity : view)
            return entity;

        return entt::null;
    }

    entt::entity Game::GetOrCreatePlayer()
    {
        if (const entt::entity player = FindPlayer(); player != entt::null && m_Registry.valid(player))
            return player;

        const entt::entity player = m_Registry.create();
        m_Registry.emplace<PlayerTag>(player);
        return player;
    }

    void Game::SpawnPlayer()
    {
        const entt::entity player = GetOrCreatePlayer();

        m_Registry.emplace_or_replace<components::Position>(
            player, Vector2{ m_Level.playerSpawn.x, m_Level.playerSpawn.y });

        m_Registry.emplace_or_replace<components::Velocity>(player, Vector2{ 0.0f, 0.0f });

        m_Registry.emplace_or_replace<components::BoxCollider>(
            player, Vector2{ m_Level.playerSpawn.width, m_Level.playerSpawn.height });

        m_Registry.emplace_or_replace<components::Grounded>(player, false);
        m_Registry.emplace_or_replace<components::PlayerControlled>(player);
        m_Registry.emplace_or_replace<components::DebugRenderRect>(player, BLUE);
    }

} // namespace game
