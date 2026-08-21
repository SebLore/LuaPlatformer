#pragma once

#include <memory>
#include <utility>
#include <vector>

#if defined(_DEBUG)
#include <iostream>
#endif
#include "entt/entity/registry.hpp"
#include <raylib.h>
#include "Common/BaseComponents.h"

namespace systems
{
    /// @brief Unique system for drawing the scene
    class RenderSystem
    {
      public:
        void Draw(entt::registry& registry)
        {
            auto view = registry.view<components::Transform>();

            for (auto entity : view)
            {
                const auto& transform = view.get<components::Transform>(entity);

                DrawRectangle(
                    static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), 50, 50, RED);
            }
        }
    };

    class ISystem
    {
      public:
        virtual ~ISystem()                                   = default;
        virtual void OnUpdate(entt::registry&, float /*dt*/) = 0;
    };

    class MovementSystem : public ISystem
    {
      public:
        MovementSystem() = default;
        void OnUpdate(entt::registry& registry, float dt) override
        {
            auto view = registry.view<components::Transform, components::Velocity>();
            for (auto entity : view)
            {
                auto&       transform = view.get<components::Transform>(entity);
                const auto& velocity  = view.get<components::Velocity>(entity);

                std::cout << "entity: " << entt::to_integral(entity) << '\n'
                          << "position before update: " << transform.position.x << ", " << transform.position.y << '\n';

                transform.position.x += velocity.x * dt;
                transform.position.y += velocity.y * dt;
                std::cout << "entity: " << entt::to_integral(entity)
                          << " position after update: " << transform.position.x << ", " << transform.position.y << '\n';
            }
        }
    };
} // namespace systems
