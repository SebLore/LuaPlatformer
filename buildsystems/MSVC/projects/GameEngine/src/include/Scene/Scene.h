#pragma once

#include <entt/entt.hpp>

#include <memory>

#include "ECS.hpp"
#include "Camera.h"


class Scene
{
    using Systems = std::vector<std::unique_ptr<ISystem>>;
    struct TrackedEntities
    {
        entt::entity MainCamera;
        entt::entity Player;
    };

  public:
    Scene()                   = default;
    ~Scene()                  = default;
    Scene(Scene&&)            = default;
    Scene& operator=(Scene&&) = default;

    void Update(float dt);
    void Render();

    entt::entity Create();
    entt::registry& GetRegistry() { return m_Registry; }

    template <class TSystem, class... Args>
    static TSystem& AddSystem(Systems& out, Args&&... args);

    template <class TSystem, class... Args> TSystem& AddSystem(Args&&... args);

    Systems&       GetSystems();
    const Systems& GetSystems() const;

    // --| ECS Components |------------------------------------------------------------------------
    template <class TComponent, class... Args>
    TComponent& RegisterComponent(entt::entity e, Args&&... args);


    // public access
    TrackedEntities& GetTrackedEntities() { return m_TrackedEntities; }
    const TrackedEntities& GetTrackedEntities()const { return m_TrackedEntities; }
  public:
    // no copying only move
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;

  private:
    entt::registry m_Registry;

    TrackedEntities m_TrackedEntities;

    Systems m_Systems;
};

template <class TSystem, class... Args>
TSystem& Scene::AddSystem(Systems& out, Args&&... args)
{
    auto sys = std::make_unique<TSystem>(std::forward<Args>(args)...);

    TSystem& ref = *sys;
    out.emplace_back(std::move(sys));

    return ref;
}

template <class TSystem, class... Args>
TSystem& Scene::AddSystem(Args&&... args)
{
    return AddSystem<TSystem>(m_Systems, std::forward<Args>(args)...);
}

template <class TComponent, class... Args>
TComponent& Scene::RegisterComponent(entt::entity e, Args&&... args)
{
    assert(m_Registry.valid(e) && "RegisterComponent: invalid entity");

    return m_Registry.emplace_or_replace<TComponent>(
        e,
        std::forward<Args>(args)...);
}
