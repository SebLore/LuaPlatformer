#include "Game/App.h"

#include <raylib.h>

#include "Game/LuaBindInput.h"
#include "Game/Scene.h"

App::App() = default;

App::~App() { Shutdown(); }

bool App::Initialize()
{
    // init raylib
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(1280, 720, "Lua ECS Platformer");
    SetTargetFPS(60);

    // run relative to the exe
    if (!ChangeDirectory(GetApplicationDirectory()))
    {
        TraceLog(LOG_ERROR, "Failed to set application directory");
        return false;
    }

    if (!InitializeLua())
        return false;

    // init scene and add systems
    m_Scene = std::make_unique<Scene>();
    {
        m_Scene->AddSystem<systems::BehaviourSystem>(m_L);
        m_Scene->AddSystem<systems::GravitySystem>();
        m_Scene->AddSystem<systems::MovementSystem>();
        m_Scene->AddSystem<systems::CollisionSystem>();
        m_Scene->AddSystem<systems::InteractionSystem>();
    }

    Scene::lua_openscene(m_L, m_Scene.get());

    if (!LoadGame())
        return false;

    return true;
}

bool App::InitializeLua()
{
    m_L = luaL_newstate();

    if (m_L == nullptr)
    {
        TraceLog(LOG_ERROR, "Failed to create Lua state.");

        return false;
    }

    luaL_openlibs(m_L);

    LuaInput::Open(m_L);

    return true;
}

bool App::LoadGame() const
{
    const int status = luaL_dofile(m_L, "scripts/game.lua");

    if (status != LUA_OK)
    {
        const char* error = lua_tostring(m_L, -1);

        TraceLog(LOG_ERROR, "Lua error: %s", error != nullptr ? error : "unknown");

        lua_pop(m_L, 1);

        return false;
    }

    return true;
}

int App::Run() const
{
    while (!WindowShouldClose())
    {
        // update
        const float dt = GetFrameTime();
        m_Scene->Update(dt);

        // draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        m_Scene->Draw();

        EndDrawing();
    }

    return 0;
}

void App::Shutdown()
{
    // want to ensure scene is destroyed before lua is to avoid unreleased lua references
    m_Scene.reset();

    if (m_L != nullptr)
    {
        lua_close(m_L);
        m_L = nullptr;
    }

    if (IsWindowReady())
        CloseWindow();
}
