#pragma once

#include "Game/Game.h"
#include "Editor/Editor.h"

static constexpr auto LUA_SCRIPTS_LOCATION = "scripts";

class App
{
  public:
    /// @brief The main mode of the application, either running the game or the editor.
    enum class Mode : uint8_t
    {
        MainMenu,
        Game,
        Editor
    };

    enum class State : uint8_t
    {
        Initialize,
        Running,
        Paused,
        Exiting,
    };

  public:
    bool Initialize();
    void Shutdown();
    int  Run();

  private:
    void SetupWindow();
    bool InitializeLua();
    void LoadInitialScene();

    Scene& ActiveScene() const { return *m_Scenes[m_SceneActiveIndex]; }

    void Update();
    void Draw();
    void SwitchMode(Mode newMode) { m_Mode = newMode; }

  private:
    Mode  m_Mode  = Mode::Editor;
    State m_State = State::Initialize;

    game::Game     m_Game;
    editor::Editor m_Editor;

    Lua::LuaWrapper m_Lua;

    std::vector<std::unique_ptr<Scene>> m_Scenes;
    int                                 m_SceneActiveIndex = 0;
};
