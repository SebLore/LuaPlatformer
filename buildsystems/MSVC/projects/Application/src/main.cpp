/**
 * @file main.cpp
 * @brief Main entry point for the application.
 *
 * Initializes the game, runs the main loop, and handles cleanup.
 */

#include <raylib.h>

#include "Scene.hpp"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(1280, 720, "Lua ECS Platformer");
    SetTargetFPS(60);

    lua_State* L = luaL_newstate();

    if (L == nullptr)
    {
        TraceLog(LOG_ERROR, "Failed to create Lua state.");
        CloseWindow();
        return 1;
    }

    luaL_openlibs(L);

    Scene scene;
    Scene::lua_openscene(L, &scene);

    int status = luaL_dofile(L, "scripts/test.lua");

    if (status != LUA_OK)
    {
        const char* error = lua_tostring(L, -1);
        std::cout << "Lua error: " << (error ? error : "unknown") << '\n';

        lua_pop(L, 1);
    }
    else
        scene.DebugTransforms();

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText("Raylib + Lua + EnTT are running.", 40, 40, 30, BLACK);
        scene.Draw();

        EndDrawing();
    }

    lua_close(L);

    CloseWindow();

    return 0;
}
