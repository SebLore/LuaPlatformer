#include "Scene/Scene.h"

void Scene::Update(float dt)
{
    for (const auto& sys : GetSystems())
        sys->OnUpdate(m_Registry, dt);
}

void Scene::Render()
{
    for (const auto& sys : GetSystems())
        sys->OnRender(m_Registry);
}

entt::entity Scene::Create() { return m_Registry.create(); }

Scene::Systems& Scene::GetSystems() { return m_Systems; }

const Scene::Systems& Scene::GetSystems() const { return m_Systems; }
