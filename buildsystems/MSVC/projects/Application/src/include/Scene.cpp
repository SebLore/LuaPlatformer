#include "Scene.hpp"

#include <iostream>

#include "Common/BaseComponents.h"

entt::entity Scene::CreateEntity() { return m_registry.create(); }

void Scene::lua_openscene(lua_State* L, Scene* scene)
{
    // Create a new table for the scene
    lua_newtable(L);
    // ----------------------------
    // scene.CreateEntity
    // ----------------------------

    lua_pushlightuserdata(L, scene);

    lua_pushcclosure(L, lua_CreateEntity, 1);

    lua_setfield(L, -2, "CreateEntity");

    // ----------------------------
    // scene.SetComponent
    // ----------------------------

    lua_pushlightuserdata(L, scene);

    lua_pushcclosure(L, lua_SetComponent, 1);

    lua_setfield(L, -2, "SetComponent");

    // ----------------------------

    lua_setglobal(L, "scene");
}

void Scene::Draw()
{
    auto view = m_registry.view<Transform>();

    for (auto entity : view)
    {
        const auto& transform = view.get<Transform>(entity);

        DrawRectangle(
            static_cast<int>(transform.translation.x), static_cast<int>(transform.translation.y), 50, 50, RED);
    }
}

void Scene::DebugTransforms()
{
    auto view = m_registry.view<Transform>();

    for (auto entity : view)
    {
        const auto& transform = view.get<Transform>(entity);

        std::cout << "Entity " << entt::to_integral(entity) << " Transform: " << transform.translation.x << ", "
                  << transform.translation.y << '\n';
    }
}

int Scene::lua_CreateEntity(lua_State* L)
{
    // Get the scene instance from the upvalue
    Scene* scene        = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    // Create a new entity
    entt::entity entity = scene->CreateEntity();
    // Push the entity ID onto the Lua stack
    lua_pushinteger(L, static_cast<lua_Integer>(entity));
    // Return 1 value (the entity ID)
    return 1;
}

int Scene::lua_SetComponent(lua_State* L)
{
    Scene* scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    // Argument 1: entity ID
    const lua_Integer id = luaL_checkinteger(L, 1);

    // Argument 2: component type
    const char* componentType = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(id);

    if (std::string(componentType) == "transform")
    {
        // Arguments 3 and 4
        const float x = static_cast<float>(luaL_checknumber(L, 3));

        const float y = static_cast<float>(luaL_checknumber(L, 4));

        scene->SetComponent<Transform>(entity, x, y);

        std::cout << "Added Transform to entity " << id << " at (" << x << ", " << y << ")\n";
    }

    return 0;
}
