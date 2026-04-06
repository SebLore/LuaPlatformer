// Game.cpp
#include "Game.h"

#include "Common/Components.h"

#include "LevelEditor/LevelLoader.h"

#include <iostream>

namespace game
{
    namespace
    {
        // constants, maybe put in lua later
        constexpr float kMoveSpeed = 220.0f;
        constexpr float kJumpSpeed = -520.0f;
        constexpr float kGravity   = 1600.0f;

        bool RectHitsSolid(const level::LevelData& level, Rectangle rect)
        {
            const int tileSize = level.map.TileSize();

            const int left   = static_cast<int>(rect.x) / tileSize;
            const int right  = static_cast<int>(rect.x + rect.width - 1.0f) / tileSize;
            const int top    = static_cast<int>(rect.y) / tileSize;
            const int bottom = static_cast<int>(rect.y + rect.height - 1.0f) / tileSize;

            for (int ty = top; ty <= bottom; ++ty)
            {
                for (int tx = left; tx <= right; ++tx)
                    if (level.IsTileSolid(tx, ty))
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

    bool Game::Initialize(Scene& scene)
    {
        if (!LoadGameState())
        {
            std::cerr << "Failed to load game state. Exiting...\n";
            m_State = GameState::Exiting;
            return false;
        }
        m_Scene = &scene;
        m_State = GameState::Running;
        return true;
    }

    bool Game::LoadGameState() { return SetupInitialGameState(); }

    bool Game::SetupInitialGameState()
    {
        return true;
    }

    void Game::HandleInput()
    {
        // Reserved for app-level hotkeys later (editor/game switch, pause, etc.)
    }

    void Game::Update() { const float dt = GetFrameTime(); }
} // namespace game
