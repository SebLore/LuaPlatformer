#pragma once

//#include <memory>
//#include <utility>
//#include <vector>

#if defined(_DEBUG)
#include <iostream>
#endif

#include "entt/entity/registry.hpp"
#include <raylib.h>

#include "Common/Components.h"

namespace systems
{
    /// @brief Unique system for drawing the scene
    class RenderSystem
    {
      public:
        void Draw(entt::registry& registry)
        {
            auto view = registry.view<components::Transform, components::Collider, components::Renderable>();

            for (auto entity : view)
            {
                const auto& transform  = view.get<components::Transform>(entity);
                const auto& collider   = view.get<components::Collider>(entity);
                const auto& renderable = view.get<components::Renderable>(entity);

                DrawRectangle(
                    static_cast<int>(transform.position.x), static_cast<int>(transform.position.y),
                    static_cast<int>(collider.size.x), static_cast<int>(collider.size.y), renderable.color);
            }
        }
    };

    // base class interface
    class ISystem
    {
      public:
        virtual ~ISystem()                                   = default;
        virtual void OnUpdate(entt::registry&, float /*dt*/) = 0;
    };

    // update position based on velocity
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

                transform.position.x += velocity.x * dt;
                transform.position.y += velocity.y * dt;
            }
        }
    };

    /// special system to handle behaviours. needs to keep track of the lua state to create and resume coroutines
    class BehaviourSystem : public ISystem
    {
      public:
        explicit BehaviourSystem(lua_State* L) : m_L(L) {}

        void OnUpdate(entt::registry& registry, float dt) override
        {
            auto view = registry.view<components::Behaviour>();

            for (auto entity : view)
            {
                auto& behaviour = view.get<components::Behaviour>(entity);

                if (behaviour.finished)
                    continue;

                // create coroutine on first update
                if (behaviour.threadRef == LUA_NOREF)
                {
                    if (!CreateCoroutine(behaviour))
                    {
                        behaviour.finished = true;
                        continue;
                    }
                }

                ResumeCoroutine(behaviour, dt);
            }
        }

      private:
        static int ResumeLua(lua_State* thread, lua_State* parent, int argumentCount)
        {
            int resultCount = 0;

            return lua_resume(thread, parent, argumentCount, &resultCount);
        }

        bool CreateCoroutine(components::Behaviour& behaviour);
        void ResumeCoroutine(components::Behaviour& behaviour, float dt);

      private:
        lua_State* m_L;
    };

    // this basically follows the 6-7 (haha) steps outlined in 13 in the provided ecs doc
    inline bool BehaviourSystem::CreateCoroutine(components::Behaviour& behaviour)
    {
        // get behaviour table, put on stack
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, behaviour.luaRef);

        if (!lua_istable(m_L, -1))
        {
            lua_pop(m_L, 1);
            return false;
        }

        // fetch onupdate behaviour
        lua_getfield(m_L, -1, "OnUpdate");

        // Lua stack should be
        // behaviour table
        // OnUpdate

        if (!lua_isfunction(m_L, -1))
        {
            lua_pop(m_L, 2);

            TraceLog(LOG_WARNING, "Behaviour '%s' has no OnUpdate", behaviour.scriptPath);

            return false;
        }

        // create lua coroutine
        lua_State* thread = lua_newthread(m_L);

        // Lua stack:
        // behaviour table
        // OnUpdate
        // thread

        // store thread in Lua registry so it isn't garbage collected
        behaviour.threadRef = luaL_ref(m_L, LUA_REGISTRYINDEX);

        // stack:
        // behaviour table
        // OnUpdate

        // move OnUpdate function to coroutine.
        // xmove moves value from top of state to the thread and pops it from the main lua thread
        lua_xmove(m_L, thread, 1);

        // next copy behaviour table and move it over as "self"
        lua_pushvalue(m_L, -1);
        lua_xmove(m_L, thread, 1);

        // finally remove behaviour from main lua
        lua_pop(m_L, 1);

        return true;
    }

    inline void BehaviourSystem::ResumeCoroutine(components::Behaviour& behaviour, float dt)
    {
        // get coroutine from Lua registry
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, behaviour.threadRef);

        lua_State* thread = lua_tothread(m_L, -1);

        lua_pop(m_L, 1);

        if (thread == nullptr)
        {
            behaviour.finished = true;
            return;
        }

        lua_pushnumber(thread, dt);

        int argumentCount = 1;

        if (!behaviour.started)
        {
            // First resume is
            // OnUpdate
            // self
            // dt
            //
            // so number of arguments is 2, OnUpdate(self, dt)
            argumentCount = 2;

            behaviour.started = true;
        }

        const int status = ResumeLua(thread, m_L, argumentCount);

        if (status == LUA_YIELD)
            return;

        if (status == LUA_OK)
        {
            behaviour.finished = true;

            luaL_unref(m_L, LUA_REGISTRYINDEX, behaviour.threadRef);

            behaviour.threadRef = LUA_NOREF;

            return;
        }

        // if we are here something is wrong
        const char* error = lua_tostring(thread, -1);

        TraceLog(LOG_ERROR, "Behaviour '%s' failed: %s", behaviour.scriptPath, error ? error : "unknown Lua error");
        lua_pop(thread, 1);
        luaL_unref(m_L, LUA_REGISTRYINDEX, behaviour.threadRef);

        behaviour.threadRef = LUA_NOREF;
        behaviour.finished  = true;
    }

    class GravitySystem : public ISystem
    {
      public:
        void OnUpdate(entt::registry& registry, float dt) override
        {
            auto view = registry.view<components::Player, components::Velocity>();

            for (auto entity : view)
            {
                auto& velocity = view.get<components::Velocity>(entity);

                velocity.y += m_G * dt;
            }
        }

      private:
        const float m_G = 1200.0f;
    };

    class CollisionSystem : public ISystem
    {
      public:
        void OnUpdate(entt::registry& registry, float dt) override
        {
            // get players and solid colliding objects like terrain
            auto players =
                registry.view<components::Player, components::Transform, components::Velocity, components::Collider>();
            auto solids = registry.view<components::Solid, components::Transform, components::Collider>();

            for (auto playerEntity : players)
            {
                auto&       player    = players.get<components::Player>(playerEntity);
                auto&       transform = players.get<components::Transform>(playerEntity);
                auto&       velocity  = players.get<components::Velocity>(playerEntity);
                const auto& collider  = players.get<components::Collider>(playerEntity);

                // start by assuming player isn't colliding
                player.grounded = false;

                Rectangle playerRect{ transform.position.x, transform.position.y, collider.size.x, collider.size.y };

                for (auto solidEntity : solids)
                {
                    const auto& solidTransform = solids.get<components::Transform>(solidEntity);
                    const auto& solidCollider  = solids.get<components::Collider>(solidEntity);
                    Rectangle   solidRect{ solidTransform.position.x, solidTransform.position.y, solidCollider.size.x,
                                         solidCollider.size.y };

                    if (!CheckCollisionRecs(playerRect, solidRect))
                        continue;

                    if (velocity.y >= 0.0f)
                    {
                        transform.position.y = solidTransform.position.y - collider.size.y;
                        velocity.y           = 0.0f;
                        player.grounded      = true;

                        // player should follow moving solid objects like platforms
                        if (auto* solidVelocity = registry.try_get<components::Velocity>(solidEntity))
                            transform.position.x += solidVelocity->x * dt;

                        playerRect.x = transform.position.x;
                        playerRect.y = transform.position.y;
                    }
                }
            }
        }
    };

    // could possibly be handled in lua
    class PlayerInputSystem : public ISystem
    {
      public:
        void OnUpdate(entt::registry& registry, float /*dt*/) override
        {
            auto view = registry.view<components::Player, components::Velocity>();

            for (auto entity : view)
            {
                static constexpr float moveSpeed = 300.0f;
                static constexpr float jumpSpeed = 550.0f;

                auto& player   = view.get<components::Player>(entity);
                auto& velocity = view.get<components::Velocity>(entity);

                // left-right move
                velocity.x = 0.0f;

                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
                    velocity.x = -moveSpeed;
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
                    velocity.x = moveSpeed;

                // jumping
                if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player.grounded)
                {
                    velocity.y      = -jumpSpeed;
                    player.grounded = false;
                }
            }
        }
    };

    class InteractionSystem : public ISystem
    {
      public:
        void OnUpdate(
            entt::registry& registry,
            float /*dt*/
            ) override
        {
            auto players = registry.view<components::Player, components::Transform, components::Collider>();
            auto hazards = registry.view<components::Hazard, components::Transform, components::Collider>();
            auto goals   = registry.view<components::Goal, components::Transform, components::Collider>();

            for (auto playerEntity : players)
            {
                auto& player = players.get<components::Player>(playerEntity);

                const auto&     playerTransform = players.get<components::Transform>(playerEntity);
                const auto&     playerCollider  = players.get<components::Collider>(playerEntity);
                const Rectangle playerRect{ playerTransform.position.x, playerTransform.position.y,
                                            playerCollider.size.x, playerCollider.size.y };

                // hazards kill the player on touch (TODO: have do damage instead if there is time)
                for (auto hazardEntity : hazards)
                {
                    const auto& transform = hazards.get<components::Transform>(hazardEntity);
                    const auto& collider  = hazards.get<components::Collider>(hazardEntity);

                    const Rectangle hazardRect{ transform.position.x, transform.position.y, collider.size.x,
                                                collider.size.y };

                    if (CheckCollisionRecs(playerRect, hazardRect))
                    {
                        if (!player.dead)
                        {
                            player.dead = true;

                            TraceLog(LOG_INFO, "Player hit a hazard");
                        }

                        break;
                    }
                }

                // Player cannot touch goal when dead
                if (player.dead)
                    continue;

                // goal
                for (auto goalEntity : goals)
                {
                    const auto& transform = goals.get<components::Transform>(goalEntity);
                    const auto& collider  = goals.get<components::Collider>(goalEntity);

                    const Rectangle goalRect{ transform.position.x, transform.position.y, collider.size.x,
                                              collider.size.y };

                    if (CheckCollisionRecs(playerRect, goalRect))
                    {
                        if (!player.reachedGoal)
                        {
                            player.reachedGoal = true;

                            TraceLog(LOG_INFO, "Player reached the goal");
                        }

                        break;
                    }
                }
            }
        }
    };

    // draw ui like text
    class UIRenderSystem
    {
      public:
        void Draw(entt::registry& registry)
        {
            auto view = registry.view<components::Transform, components::UIText>();

            for (auto entity : view)
            {
                const auto& transform = view.get<components::Transform>(entity);
                const auto& text      = view.get<components::UIText>(entity);

                DrawText(
                    text.text, static_cast<int>(transform.position.x), static_cast<int>(transform.position.y),
                    text.fontSize, text.color);
            }
        }
    };
} // namespace systems
