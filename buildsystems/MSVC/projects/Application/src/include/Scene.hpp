#pragma once

#include <entt/entt.hpp>

#include <lua.hpp>

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


    entt::entity             CreateEntity();

  private:
    static int lua_CreateEntity(lua_State* L);
    static int lua_SetComponent(lua_State* L);

  private:
    entt::registry m_registry;
};
