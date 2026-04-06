#include "App.h"

#include "Common/Components.h"

#include "LuaBindings.h"

#include "utility/fileutils.h"

bool App::Initialize()
{
    SetupWindow();

    if (!InitializeLua())
        return false;

    LoadInitialScene();

    if (!m_Game.Initialize(ActiveScene()))
        return false;

    m_Editor.Initialize(ActiveScene());

    // app is ready to start run loop
    m_State = State::Running;

    return true;
}

void App::Shutdown() {}

int App::Run()
{
    if (m_State == State::Initialize && !Initialize())
    {
        std::cerr << "App initialization failed. Exiting...\n";
        return EXIT_FAILURE;
    }

    while (!WindowShouldClose() && !IsExiting()) // TODO: maybe add some other exit condition
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

    {
        auto& reg = ActiveScene().GetRegistry();
        auto  e   = reg.create();

        auto texId = m_Assets.RegisterTexture("test_tiles.png");
        if (!m_Assets.TryGetTexture(texId))
            m_Assets.LoadTextureFile(texId);

        reg.emplace<components::Renderable>(e);
        reg.emplace<components::Sprite2D>(
            e, components::Sprite2D{
                   .texture = texId,
                   .src     = { 0, 0, 16, 16 },
                   .dst     = { 100, 100, 16, 16 },
               });
    }
}

void App::Update()
{
    if (m_State != State::Running)
        return;

    if (m_Mode == Mode::MainMenu)
    {
        // TODO: main menu stuff
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

    auto renderView = scene.GetRegistry().view<components::Renderable>();

    for (auto entity : renderView)
    {
        // TODO: render the entity based on its components (e.g. Sprite, Transform2D, etc.)
        if (auto sprite = scene.GetRegistry().try_get<components::Sprite2D>(entity))
        {
            // Draw the sprite
            if (const auto tex = m_Assets.TryGetTexture(sprite->texture))
                DrawTexturePro(*tex, sprite->src, sprite->dst, sprite->origin, sprite->rotation, sprite->tint);
        }
    }

    EndDrawing();
}
