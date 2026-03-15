#pragma once

#include <raylib.h>

struct CameraConfig
{
    Vector2 Offset;
    Vector2 Target;
    float Rotation;
    float Zoom;

    Vector2 Origin;
    float ZoomBase;
    float ZoomMin;
    float ZoomMax;
    float ZoomStep;

    struct FollowEntityPosition {
        entt::entity e;
        bool enable;
    } FollowEPos;
};
