#pragma once

#include <vector>
#include <memory>
#include <utility> // for std::forward
#include <iostream>

#include <entt/entt.hpp>
#include <lua.hpp>
#include "Game/Systems/SystemBase.h"

class Scene
{
  public:
    Scene()  = default;
    ~Scene() = default;

    static void lua_openscene(lua_State* L, Scene* scene);

    // Components
    template <typename T, typename... Args> void SetComponent(entt::entity entity, Args&&... args)
    {
        m_registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T> bool HasComponent(entt::entity entity) { return m_registry.all_of<T>(entity); }

    template <typename T> T& GetComponent(entt::entity entity) { return m_registry.get<T>(entity); }

    void DebugTransforms();

    void Draw();
    void Update(float dt);

    template <typename T, typename... Args> void AddSystem(Args&&... args)
    {
        std::cout << "size: " << m_Systems.size() << '\n';
        std::cout << "capacity: " << m_Systems.capacity() << '\n';

        auto sys = std::make_unique<T>(std::forward<Args>(args)...);

        m_Systems.emplace_back(std::move(sys));
    }

    int          EntityCount();
    entt::entity CreateEntity();
    bool         IsEntity(int entity) const;
    void         RemoveEntity(int entity);

  private:
    static int    lua_CreateEntity(lua_State* L);
    static Scene* lua_GetSceneUpValue(lua_State* L);

    static int lua_HasComponent(lua_State* L);
    static int lua_GetComponent(lua_State* L);
    static int lua_RemoveComponent(lua_State* L);
    static int lua_SetComponent(lua_State* L);
    static int lua_GetEntityCount(lua_State* L);
    static int lua_IsEntity(lua_State* L);
    static int lua_RemoveEntity(lua_State* L);

  private:
    entt::registry m_registry;

    std::vector<std::unique_ptr<systems::ISystem>> m_Systems = {};
    systems::RenderSystem                          m_RenderSystem;
};
