#include "Scene.hpp"

#include <iostream>

#include "Common/BaseComponents.h"

// =========
// PUBLIC
// =========

entt::entity Scene::CreateEntity() { return m_registry.create(); }

int Scene::EntityCount() { return static_cast<int>(m_registry.storage<entt::entity>().free_list()); }

bool Scene::IsEntity(int entity) const { return m_registry.valid(static_cast<entt::entity>(entity)); }

void Scene::RemoveEntity(int entity)
{
    if (IsEntity(entity))
        m_registry.destroy(static_cast<entt::entity>(entity));
}

void Scene::lua_openscene(lua_State* L, Scene* scene)
{
    lua_newtable(L);

    static const luaL_Reg methods[] = { { "GetEntityCount", lua_GetEntityCount },
                                        { "CreateEntity", lua_CreateEntity },
                                        { "IsEntity", lua_IsEntity },
                                        { "RemoveEntity", lua_RemoveEntity },

                                        { "HasComponent", lua_HasComponent },
                                        { "GetComponent", lua_GetComponent },
                                        { "SetComponent", lua_SetComponent },
                                        { "RemoveComponent", lua_RemoveComponent },

                                        { nullptr, nullptr } };

    // This becomes upvalue #1 for every
    // function in methods.
    lua_pushlightuserdata(L, scene);

    luaL_setfuncs(L, methods, 1);

    lua_setglobal(L, "scene");
}

void Scene::Draw() { m_RenderSystem.Draw(m_registry); }

void Scene::Update(float dt)
{
    for (auto& system : m_Systems)
        system->OnUpdate(m_registry, dt);
}

void Scene::DebugTransforms()
{
    {

        auto view = m_registry.view<components::Transform>();

        for (auto entity : view)
        {
            const auto& transform = view.get<components::Transform>(entity);

            std::cout << "Entity " << entt::to_integral(entity) << " Transform: " << transform.position.x << ", "
                      << transform.position.y << '\n';
        }
    }

    {
        auto view = m_registry.view<components::Velocity>();

        for (auto entity : view)
        {
            const auto& velocity = view.get<components::Velocity>(entity);

            std::cout << "Entity " << entt::to_integral(entity) << " velocity: " << velocity.x << ", " << velocity.y
                      << '\n';
        }
    }
}

// ===================
// Lua binding
// ===================

// get the Scene pointer from the upvalue of the Lua function
Scene* Scene::lua_GetSceneUpValue(lua_State* L)
{
    auto* scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (scene == nullptr)
    {
        luaL_error(L, "scene: missing Scene pointer upvalue");

        return nullptr;
    }

    return scene;
}

int Scene::lua_CreateEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const entt::entity entity = scene->CreateEntity();

    lua_pushinteger(L, static_cast<lua_Integer>(entt::to_integral(entity)));

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

        scene->SetComponent<components::Transform>(entity, x, y);

        std::cout << "Added Transform to entity " << id << " at (" << x << ", " << y << ")\n";
    }
    else if (std::string(componentType) == "velocity")
    {
        // Arguments 3 and 4
        const float vx = static_cast<float>(luaL_checknumber(L, 3));
        const float vy = static_cast<float>(luaL_checknumber(L, 4));

        scene->SetComponent<components::Velocity>(entity, vx, vy);

        std::cout << "Added Velocity to entity " << id << " at (" << vx << ", " << vy << ")\n";
    }

    else if (std::string(componentType) == "behaviour")
    {
        if (scene->HasComponents<BehaviourComponent>(entity))
            scene->RemoveComponent<BehaviourComponent>(entity);

        const char* path = lua_tostring(L, 3);

        // Returns the behaviour table on top of the stack
        luaL_dofile(L, path);

        // luaL_ref pops the value of the stack , so we push
        // the table again before luaL_ref
        lua_pushvalue(L, -1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);

        // Populate the behaviour table with the information
        // the behaviour should know about
        lua_pushinteger(L, entity);
        lua_setfield(L, -2, "ID");

        lua_pushstring(L, path);
        lua_setfield(L, -2, " path ");

        // Let the behaviour construct itself . It may be good
        // practice to check if the method exist before calling it
        lua_getfield(L, -1, " OnCreate ");
        lua_pushvalue(L, -2); // Push the table as argument
        lua_pcall(L, 1, 0, 0);

        // scene -> SetComponent < Behaviour >( entity , path , ref );
        return 1;
    }

    return 0;
}

int Scene::lua_GetEntityCount(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    lua_pushinteger(L, scene->EntityCount());

    return 1;
}
int Scene::lua_IsEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id = luaL_checkinteger(L, 1);

    lua_pushboolean(L, scene->IsEntity(id));

    return 1;
}
int Scene::lua_RemoveEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id = luaL_checkinteger(L, 1);

    scene->RemoveEntity(id);

    return 0;
}

int Scene::lua_HasComponent(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id = luaL_checkinteger(L, 1);

    const char* type = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(id);

    if (!scene->IsEntity(entity))
    {
        lua_pushboolean(L, false);
        return 1;
    }

    bool result = false;

    const std::string componentType(type);

    if (componentType == "transform")
        result = scene->HasComponent<components::Transform>(entity);
    else if (componentType == "velocity")
        result = scene->HasComponent<components::Velocity>(entity);
    else if (componentType == "behaviour")
        result = scene->HasComponent<components::Behaviour>(entity);

    lua_pushboolean(L, result);

    return 1;
}
