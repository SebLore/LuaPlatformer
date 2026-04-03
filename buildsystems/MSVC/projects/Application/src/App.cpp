#include "App.h"

bool App::Initialize()
{
    if (!m_Game.Initialize())
        return false;
    //if (!m_Editor.Initialize())
    //    return false;
    return true;
}

void App::Shutdown()
{
    //m_Game.Shutdown();
    //m_Editor.Shutdown();
}

int App::Run()
{
    if (!Initialize())
        return EXIT_FAILURE;

    while (!WindowShouldClose())
    {
        Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Draw();
        EndDrawing();
    }

    Shutdown();
    return EXIT_SUCCESS;
}

void App::Update()
{
    switch (m_Mode)
    {
    case AppMode::Editor:
        m_Editor.Update();
        break;
    case AppMode::Game:
        m_Game.Update();
        break;
    }
}

void App::Draw()
{
    switch (m_Mode)
    {
    case AppMode::Editor:
        m_Editor.Draw();
        break;
    case AppMode::Game:
        m_Game.Draw();
        break;
    }
}
