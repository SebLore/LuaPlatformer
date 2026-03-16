#pragma once

#include <raylib.h>

#include <cstdint>

namespace components
{
    struct PositionI
    {
        int x = 0, y = 0;
    };

    struct Transform
    {
        Vector2 position{ 0, 0 };
    };

    struct Velocity : Vector2
    {
    };

    struct Position : Vector2
    {
    };

    struct AABBCollider
    {
        Vector2 halfExtents{ 16, 16 };
        bool    isStatic = true;
        bool    isOneWay = false; // one-way platform behavior
    };

    struct PhysicsBody
    {
        bool  dynamic      = true;
        bool  grounded     = false;
        float gravityScale = 1.0f;
    };

    struct Circle
    {
        float x = 0;
        float y = 0;
        float r = 0;
    };

    struct PlayerController
    {
        // speed measured in pixels per second TODO: use units instead, like meters where 1 m = 100 px
        float moveSpeed = 220.0f;
        float jumpSpeed = 420.0f;

        // boolean values packed to single bits
        uint8_t jumping : 1 = 0; // whether jump input is active
        uint8_t canJump : 1 =
            1; // whether player is allowed to jump (is grounded)

        // note: jumpSpeed is treated as a scalar for
        // positive magnitude; in raylib, +Y is down, so jump sets v.y negative
    };

    struct OneWayPlatformMotion
    {
        Vector2 basePos{ 0, 0 };
        Vector2 offset{ 0, -80 }; // moves along this offset (e.g., up/down)
        float   periodSeconds = 2.0f;
        float   t             = 0.0f;
    };

    struct Drawable
    {
        Texture*  texture = nullptr;
        Rectangle srcRect = { 0, 0, 0, 0 };
        Vector2   scale{ 1.0f, 1.0f };
        Vector2   origin{ 0, 0 };
        Color     tint{ WHITE };
        int       layer = 0; // rendering layer
    };
} // namespace components
namespace cmp = components; // alias for convenience
