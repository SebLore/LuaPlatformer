#include "App.h"

#include "LuaBindings.h"

bool App::Initialize()
{
    LoadInitialScene();



    if (!m_Game.Initialize(ActiveScene()))
        return false;
    m_Editor.Initialize(ActiveScene());

    return true;
}

void App::Shutdown()
{
}

int App::Run()
{
    if (m_State == State::Initialize)
        if (!Initialize())
        {
            std::cerr << "App initialization failed. Exiting...\n";
            return EXIT_FAILURE;
        }

    while (!WindowShouldClose()) // TODO: maybe add some other exit condition
    {
        Update();
        Draw();
    }
    return EXIT_SUCCESS;
}

void App::SetupWindow()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(800, 600, "Raylib time!");
    SetTargetFPS(60);
}

bool App::InitializeLua()
{
    m_Lua.SetScriptsPath(util::FindDirectory(LUA_SCRIPTS_LOCATION).string());
    binding::lua_register_functions(m_Lua);

    m_Lua.SetGlobal("KEY_A", KEY_A);
    m_Lua.SetGlobal("KEY_D", KEY_D);
    m_Lua.SetGlobal("KEY_W", KEY_W);
    m_Lua.SetGlobal("KEY_S", KEY_S);
    m_Lua.SetGlobal("KEY_SPACE", KEY_SPACE);

    m_Lua.SetGlobal("KEY_LEFT", KEY_LEFT);
    m_Lua.SetGlobal("KEY_RIGHT", KEY_RIGHT);
    m_Lua.SetGlobal("KEY_UP", KEY_UP);
    m_Lua.SetGlobal("KEY_DOWN", KEY_DOWN);

    // Optional global init() if some bootstrap script created it
    if (!CallLuaVoid0(m_Lua, "init"))
    {
        std::cerr << "Lua init() failed: " << m_Lua.Error() << "\n";
        return false;
    }

    return true;
}

void App::LoadInitialScene()
{
    m_Scenes.clear();

    // create a default scene, later load or configure from lua
    m_Scenes.push_back(std::make_unique<Scene>());
    m_SceneActiveIndex = 0;
}

void App::Update()
{
    if (m_State != State::Running)
        return;

    if (m_Mode == Mode::MainMenu)
    {
        // TODO: mainmenu stuff
    }
    else if (m_Mode == Mode::Game)
    {
        m_Game.Update();
    }
    else if (m_Mode == Mode::Editor)
    {
        m_Editor.Update();
    }
}

void App::Draw()
{
    if (m_State != State::Running)
        return;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw scene
    Scene& scene = ActiveScene();

    auto renderView = scene.GetRegistry().view<Renderable>();

    EndDrawing();
}
