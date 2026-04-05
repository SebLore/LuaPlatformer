#pragma once

#include <cstdint>

#include <entt/entt.hpp>
#include <raylib.h>

#include "Assets/AssetManager.h"
#include "LevelEditor/LevelData.h"
#include "LuaWrapper.h"
#include "Scene/Scene.h"

namespace game
{
    enum class GameState : std::uint8_t
    {
        Ready,
        Running,
        Paused,
        GameOver,
        Exiting
    };

    class Game
    {
      public:
        Game() = default;
        ~Game();

        bool Initialize(Scene &scene);
        void Run();

        void HandleInput();
        void Update();
        void Draw();

        bool Exiting()const { return m_State == GameState::Exiting; }

      private:
        bool LoadGameState();
        bool SetupInitialGameState();

        void DrawWorld();
        void DrawDebug();
        void DrawUI();

        void DrawLevel() const;
        void DrawSpawnMarker() const;
        void DrawPlayers() const;

        void UpdatePlayers(float dt);

        entt::entity FindPlayer() const;
        entt::entity GetOrCreatePlayer();
        void         SpawnPlayer();

      private:
        GameState            m_State = GameState::Ready;
        Scene* m_Scene = nullptr;
    };
} // namespace game
