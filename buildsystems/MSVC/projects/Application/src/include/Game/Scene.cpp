#include "Scene.h"

#include <string>

#include "Common/Components.h"

int Scene::GetEntityCount() const { return static_cast<int>(m_registry.storage<entt::entity>()->free_list()); }

entt::entity Scene::CreateEntity() { return m_registry.create(); }

bool Scene::IsEntity(entt::entity entity) const { return m_registry.valid(entity); }

void Scene::RemoveEntity(entt::entity entity)
{
    if (!IsEntity(entity))
        return;

    auto* behaviour = m_registry.try_get<components::Behaviour>(entity);

    // clear lua refs before removing the entity
    if (behaviour != nullptr && m_L != nullptr)
    {
        if (behaviour->threadRef != LUA_NOREF && behaviour->threadRef != LUA_REFNIL)
        {
            luaL_unref(m_L, LUA_REGISTRYINDEX, behaviour->threadRef);
            behaviour->threadRef = LUA_NOREF;
        }

        if (behaviour->luaRef != LUA_NOREF && behaviour->luaRef != LUA_REFNIL)
        {
            luaL_unref(m_L, LUA_REGISTRYINDEX, behaviour->luaRef);
            behaviour->luaRef = LUA_NOREF;
        }
    }

    m_registry.destroy(entity);
}

// ==| Systems |====================================
void Scene::Update(float dt)
{
    for (auto& system : m_Systems)
        system->OnUpdate(m_registry, dt);
}

void Scene::Draw()
{
    m_RenderSystem.Draw(m_registry);
    m_UIRenderSystem.Draw(m_registry);
}

// ==| Lua module bindings |========================================
Scene* Scene::lua_GetSceneUpValue(lua_State* L)
{
    auto* scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (scene == nullptr)
    {
        luaL_error(L, "Scene pointer missing from Lua upvalue");
        return nullptr;
    }

    return scene;
}

void Scene::lua_openscene(lua_State* L, Scene* scene)
{
    scene->m_L = L;

    lua_newtable(L);

    static constexpr luaL_Reg methods[] = { { "GetEntityCount", lua_GetEntityCount },
                                            { "CreateEntity", lua_CreateEntity },
                                            { "IsEntity", lua_IsEntity },
                                            { "RemoveEntity", lua_RemoveEntity },

                                            { "HasComponent", lua_HasComponent },
                                            { "GetComponent", lua_GetComponent },
                                            { "SetComponent", lua_SetComponent },
                                            { "RemoveComponent", lua_RemoveComponent },

                                            { nullptr, nullptr } };

    // scene pointer is upvalue #1 on the stack
    lua_pushlightuserdata(L, scene);
    luaL_setfuncs(L, methods, 1);

    lua_setglobal(L, "scene");
}

// ==| entity bindings |============================

int Scene::lua_GetEntityCount(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    lua_pushinteger(L, static_cast<lua_Integer>(scene->GetEntityCount()));

    return 1;
}

int Scene::lua_CreateEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const entt::entity entity = scene->CreateEntity();

    lua_pushinteger(L, static_cast<lua_Integer>(entt::to_integral(entity)));

    return 1;
}

int Scene::lua_IsEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer  id     = luaL_checkinteger(L, 1);
    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    lua_pushboolean(L, scene->IsEntity(entity));

    return 1;
}

int Scene::lua_RemoveEntity(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer  id     = luaL_checkinteger(L, 1);
    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    scene->RemoveEntity(entity);

    return 0;
}

// ==| component bindings |=========================

int Scene::lua_HasComponent(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id   = luaL_checkinteger(L, 1);
    const std::string type = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    if (!scene->IsEntity(entity))
    {
        lua_pushboolean(L, false);
        return 1;
    }

    bool hasComponent = false;

    if (type == "transform")
        hasComponent = scene->HasComponent<components::Transform>(entity);
    else if (type == "velocity")
        hasComponent = scene->HasComponent<components::Velocity>(entity);
    else if (type == "behaviour")
        hasComponent = scene->HasComponent<components::Behaviour>(entity);
    else if (type == "collider")
        hasComponent = scene->HasComponent<components::Collider>(entity);
    else if (type == "renderable")
        hasComponent = scene->HasComponent<components::Renderable>(entity);
    else if (type == "solid")
        hasComponent = scene->HasComponent<components::Solid>(entity);
    else if (type == "player")
        hasComponent = scene->HasComponent<components::Player>(entity);
    else if (type == "hazard")
        hasComponent = scene->HasComponent<components::Hazard>(entity);
    else if (type == "goal")
        hasComponent = scene->HasComponent<components::Goal>(entity);
    else if (type == "ui_text")
        hasComponent = scene->HasComponent<components::UIText>(entity);

    lua_pushboolean(L, hasComponent);

    return 1;
}

int Scene::lua_GetComponent(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id   = luaL_checkinteger(L, 1);
    const std::string type = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    if (!scene->IsEntity(entity))
    {
        lua_pushnil(L);
        return 1;
    }

    if (type == "transform" && scene->HasComponent<components::Transform>(entity))
    {
        const auto& transform = scene->GetComponent<components::Transform>(entity);

        lua_newtable(L);

        lua_pushnumber(L, transform.position.x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, transform.position.y);
        lua_setfield(L, -2, "y");

        return 1;
    }

    if (type == "velocity" && scene->HasComponent<components::Velocity>(entity))
    {
        const auto& velocity = scene->GetComponent<components::Velocity>(entity);

        lua_newtable(L);

        lua_pushnumber(L, velocity.x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, velocity.y);
        lua_setfield(L, -2, "y");

        return 1;
    }

    if (type == "behaviour" && scene->HasComponent<components::Behaviour>(entity))
    {
        const auto& behaviour = scene->GetComponent<components::Behaviour>(entity);

        // behaviour table lives in the lua registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, behaviour.luaRef);

        return 1;
    }

    if (type == "collider" && scene->HasComponent<components::Collider>(entity))
    {
        const auto& collider = scene->GetComponent<components::Collider>(entity);

        lua_newtable(L);

        lua_pushnumber(L, collider.size.x);
        lua_setfield(L, -2, "width");

        lua_pushnumber(L, collider.size.y);
        lua_setfield(L, -2, "height");

        return 1;
    }

    if (type == "renderable" && scene->HasComponent<components::Renderable>(entity))
    {
        const auto& renderable = scene->GetComponent<components::Renderable>(entity);

        lua_newtable(L);

        lua_pushinteger(L, renderable.color.r);
        lua_setfield(L, -2, "r");

        lua_pushinteger(L, renderable.color.g);
        lua_setfield(L, -2, "g");

        lua_pushinteger(L, renderable.color.b);
        lua_setfield(L, -2, "b");

        lua_pushinteger(L, renderable.color.a);
        lua_setfield(L, -2, "a");

        return 1;
    }

    if (type == "player" && scene->HasComponent<components::Player>(entity))
    {
        const auto& player = scene->GetComponent<components::Player>(entity);

        lua_newtable(L);

        lua_pushboolean(L, player.grounded);
        lua_setfield(L, -2, "grounded");

        lua_pushboolean(L, player.dead);
        lua_setfield(L, -2, "dead");

        lua_pushboolean(L, player.reachedGoal);
        lua_setfield(L, -2, "reachedGoal");

        return 1;
    }

    lua_pushnil(L);

    return 1;
}

int Scene::lua_SetComponent(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id   = luaL_checkinteger(L, 1);
    const std::string type = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    if (!scene->IsEntity(entity))
        return luaL_error(L, "scene.SetComponent: invalid entity");

    if (type == "transform")
    {
        const float x = static_cast<float>(luaL_checknumber(L, 3));
        const float y = static_cast<float>(luaL_checknumber(L, 4));

        scene->SetComponent<components::Transform>(entity, components::Transform{ Vector2{ x, y } });

        return 0;
    }

    if (type == "velocity")
    {
        const float x = static_cast<float>(luaL_checknumber(L, 3));
        const float y = static_cast<float>(luaL_checknumber(L, 4));

        scene->SetComponent<components::Velocity>(entity, components::Velocity{ x, y });

        return 0;
    }

    if (type == "behaviour")
    {
        const char* path = luaL_checkstring(L, 3);

        // remove old refs if we're replacing one
        if (scene->HasComponent<components::Behaviour>(entity))
        {
            auto& oldBehaviour = scene->GetComponent<components::Behaviour>(entity);

            if (oldBehaviour.threadRef != LUA_NOREF && oldBehaviour.threadRef != LUA_REFNIL)
                luaL_unref(L, LUA_REGISTRYINDEX, oldBehaviour.threadRef);

            if (oldBehaviour.luaRef != LUA_NOREF && oldBehaviour.luaRef != LUA_REFNIL)
                luaL_unref(L, LUA_REGISTRYINDEX, oldBehaviour.luaRef);

            scene->RemoveComponent<components::Behaviour>(entity);
        }

        const int status = luaL_dofile(L, path);

        if (status != LUA_OK)
        {
            const char*       luaError = lua_tostring(L, -1);
            const std::string error    = luaError ? luaError : "unknown Lua error";

            lua_pop(L, 1);

            return luaL_error(L, "Failed to load behaviour '%s': %s", path, error.c_str());
        }

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);

            return luaL_error(L, "Behaviour '%s' must return a table", path);
        }

        // keep a ref so the table stays alive
        lua_pushvalue(L, -1);
        const int ref = luaL_ref(L, LUA_REGISTRYINDEX);

        lua_pushinteger(L, static_cast<lua_Integer>(entt::to_integral(entity)));
        lua_setfield(L, -2, "ID");

        lua_pushstring(L, path);
        lua_setfield(L, -2, "path");

        lua_getfield(L, -1, "OnCreate");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);

            if (lua_pcall(L, 1, 0, 0) != LUA_OK)
            {
                const char*       luaError = lua_tostring(L, -1);
                const std::string error    = luaError ? luaError : "unknown Lua error";

                lua_pop(L, 1);
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
                lua_pop(L, 1);

                return luaL_error(L, "Behaviour '%s' OnCreate failed: %s", path, error.c_str());
            }
        }
        else
        {
            lua_pop(L, 1);
        }

        scene->SetComponent<components::Behaviour>(entity, path, ref);

        lua_pop(L, 1);

        return 0;
    }

    if (type == "collider")
    {
        const float width  = static_cast<float>(luaL_checknumber(L, 3));
        const float height = static_cast<float>(luaL_checknumber(L, 4));

        scene->SetComponent<components::Collider>(entity, components::Collider{ Vector2{ width, height } });

        return 0;
    }

    if (type == "renderable")
    {
        const int r = static_cast<int>(luaL_checkinteger(L, 3));
        const int g = static_cast<int>(luaL_checkinteger(L, 4));
        const int b = static_cast<int>(luaL_checkinteger(L, 5));
        const int a = static_cast<int>(luaL_optinteger(L, 6, 255));

        scene->SetComponent<components::Renderable>(
            entity, components::Renderable{ Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                                                   static_cast<unsigned char>(b), static_cast<unsigned char>(a) } });

        return 0;
    }

    if (type == "solid")
    {
        scene->SetComponent<components::Solid>(entity);
        return 0;
    }

    if (type == "player")
    {
        scene->SetComponent<components::Player>(entity);
        return 0;
    }

    if (type == "hazard")
    {
        scene->SetComponent<components::Hazard>(entity);
        return 0;
    }

    if (type == "goal")
    {
        scene->SetComponent<components::Goal>(entity);
        return 0;
    }

    if (type == "ui_text")
    {
        const char* text     = luaL_checkstring(L, 3);
        const int   fontSize = static_cast<int>(luaL_checkinteger(L, 4));
        const int   r        = static_cast<int>(luaL_checkinteger(L, 5));
        const int   g        = static_cast<int>(luaL_checkinteger(L, 6));
        const int   b        = static_cast<int>(luaL_checkinteger(L, 7));

        scene->SetComponent<components::UIText>(
            entity, text, fontSize,
            Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255 });

        return 0;
    }

    return luaL_error(L, "scene.SetComponent: unknown component '%s'", type.c_str());
}

int Scene::lua_RemoveComponent(lua_State* L)
{
    Scene* scene = lua_GetSceneUpValue(L);

    const lua_Integer id   = luaL_checkinteger(L, 1);
    const std::string type = luaL_checkstring(L, 2);

    const entt::entity entity = static_cast<entt::entity>(static_cast<entt::id_type>(id));

    if (!scene->IsEntity(entity))
        return 0;

    if (type == "transform" && scene->HasComponent<components::Transform>(entity))
    {
        scene->RemoveComponent<components::Transform>(entity);
        return 0;
    }

    if (type == "velocity" && scene->HasComponent<components::Velocity>(entity))
    {
        scene->RemoveComponent<components::Velocity>(entity);
        return 0;
    }

    if (type == "behaviour" && scene->HasComponent<components::Behaviour>(entity))
    {
        auto& behaviour = scene->GetComponent<components::Behaviour>(entity);

        if (behaviour.threadRef != LUA_NOREF && behaviour.threadRef != LUA_REFNIL)
            luaL_unref(L, LUA_REGISTRYINDEX, behaviour.threadRef);

        if (behaviour.luaRef != LUA_NOREF && behaviour.luaRef != LUA_REFNIL)
            luaL_unref(L, LUA_REGISTRYINDEX, behaviour.luaRef);

        scene->RemoveComponent<components::Behaviour>(entity);

        return 0;
    }

    if (type == "ui_text" && scene->HasComponent<components::UIText>(entity))
    {
        scene->RemoveComponent<components::UIText>(entity);
        return 0;
    }

    if (type == "collider" && scene->HasComponent<components::Collider>(entity))
    {
        scene->RemoveComponent<components::Collider>(entity);
        return 0;
    }

    if (type == "solid" && scene->HasComponent<components::Solid>(entity))
    {
        scene->RemoveComponent<components::Solid>(entity);
        return 0;
    }

    if (type == "player" && scene->HasComponent<components::Player>(entity))
    {
        scene->RemoveComponent<components::Player>(entity);
        return 0;
    }

    if (type == "renderable" && scene->HasComponent<components::Renderable>(entity))
    {
        scene->RemoveComponent<components::Renderable>(entity);
        return 0;
    }

    if (type == "hazard" && scene->HasComponent<components::Hazard>(entity))
    {
        scene->RemoveComponent<components::Hazard>(entity);
        return 0;
    }

    if (type == "goal" && scene->HasComponent<components::Goal>(entity))
    {
        scene->RemoveComponent<components::Goal>(entity);
        return 0;
    }

    return 0;
}
