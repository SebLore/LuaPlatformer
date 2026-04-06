#pragma once

#include <cstdint>

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

      private:
        GameState            m_State = GameState::Ready;
        Scene* m_Scene = nullptr;
    };
} // namespace game
