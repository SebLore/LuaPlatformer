#pragma once

#include "Game.h"
#include "Editor.h"

enum class AppMode
{
    Game,
    Editor
};

class App
{
  public:
    bool Initialize();
    void Shutdown();
    int  Run();

  private:
    void Update();
    void Draw();
    void SwitchMode(AppMode newMode);

  private:
    AppMode m_Mode = AppMode::Editor;

    game::Game     m_Game;
    editor::Editor m_Editor;

    //Project      m_Project; // shared editable data
    //RuntimeWorld m_Runtime; // optional derived runtime state
};
