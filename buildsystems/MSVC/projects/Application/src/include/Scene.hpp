#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <entt/entt.hpp>
#include <lua.hpp>

#include "Game/Systems/SystemBase.h"

class Scene
{
  public:
    Scene()  = default;
    ~Scene() = default;

    // --------------------------------------------------
    // Lua module
    // --------------------------------------------------

    static void lua_openscene(lua_State* L, Scene* scene);

    // --------------------------------------------------
    // Entities
    // --------------------------------------------------

    int GetEntityCount() const;

    entt::entity CreateEntity();

    bool IsEntity(entt::entity entity) const;

    void RemoveEntity(entt::entity entity);

    // --------------------------------------------------
    // Components
    // --------------------------------------------------

    template <typename T, typename... Args> void SetComponent(entt::entity entity, Args&&... args)
    {
        m_registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T> bool HasComponent(entt::entity entity) const { return m_registry.all_of<T>(entity); }

    template <typename T> T& GetComponent(entt::entity entity) { return m_registry.get<T>(entity); }

    template <typename T> void RemoveComponent(entt::entity entity) { m_registry.remove<T>(entity); }

    // --------------------------------------------------
    // Systems
    // --------------------------------------------------

    template <typename T, typename... Args> void AddSystem(Args&&... args)
    {
        m_Systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void Update(float dt);

    // --------------------------------------------------
    // Rendering
    // --------------------------------------------------

    void Draw();

  private:
    // --------------------------------------------------
    // Lua helper
    // --------------------------------------------------

    static Scene* lua_GetSceneUpValue(lua_State* L);

    // --------------------------------------------------
    // Lua entity functions
    // --------------------------------------------------

    static int lua_GetEntityCount(lua_State* L);
    static int lua_CreateEntity(lua_State* L);
    static int lua_IsEntity(lua_State* L);
    static int lua_RemoveEntity(lua_State* L);

    // --------------------------------------------------
    // Lua component functions
    // --------------------------------------------------

    static int lua_HasComponent(lua_State* L);
    static int lua_GetComponent(lua_State* L);
    static int lua_SetComponent(lua_State* L);
    static int lua_RemoveComponent(lua_State* L);

  private:
    entt::registry m_registry;

    std::vector<std::unique_ptr<systems::ISystem>> m_Systems;

    systems::RenderSystem   m_RenderSystem;
    systems::UIRenderSystem m_UIRenderSystem;
};
