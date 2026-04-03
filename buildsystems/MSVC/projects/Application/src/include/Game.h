#pragma once

#include <cstdint>

#include <entt/entt.hpp>
#include <raylib.h>

#include "Assets/AssetManager.h"
#include "LevelEditor/LevelData.h"
#include "LuaWrapper.h"

namespace game
{
    enum class GameState : std::uint8_t
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
        Game() = default;
        ~Game();

        bool Initialize();
        void Run();

        void HandleInput();
        void Update();
        void Draw();

      private:
        bool LoadScripts();
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
        static constexpr const char* SCRIPTS_DIR = "scripts";

      private:
        GameState            m_State = GameState::Ready;
        entt::registry       m_Registry{};
        Lua::LuaWrapper      m_Lua{};
        assets::AssetManager m_Assets{};

        LevelData m_Level{};
        TextureId m_TilesTexture{};
    };
} // namespace game
