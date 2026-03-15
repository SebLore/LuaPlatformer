#pragma once

#include "ISystem.h"

#include <raylib.h>

// components
#include <ECS/Components/BaseComponents.h>

class RenderSystem : public ISystem
{
  public:
    RenderSystem() = default;

    void OnRender(entt::registry& registry) override;
    void OnUpdate(entt::registry&, float) override {} // no-op
};

inline void RenderSystem::OnRender(entt::registry& registry)
{
    // get a view of all entities registered as Drawable:

    for (const auto& e : registry.view<cmp::Position, cmp::Drawable>())
    {
        const auto& [position] = registry.get<cmp::Position>(e);
        const auto& d          = registry.get<cmp::Drawable>(e);

        if (const Texture2D* tex = d.texture)
        {
            DrawTexturePro(
                *tex,
                d.srcRect,
                { position.x,
                  position.y,
                  d.srcRect.width * d.scale.x,
                  d.srcRect.height * d.scale.y },
                { 0, 0 },
                0,
                WHITE);
            //DrawTexturePro(*tex, d.srcRect, Rectangle{ p.position.x, p.position.y, d.srcRect.width, d.srcRect.height }, d.origin, 0, d.tint);
        }
        else
        {
            DrawRectanglePro(
                d.srcRect,
                { d.origin.x + position.x, d.origin.y + position.y },
                0,
                d.tint);
        }
    }
}
