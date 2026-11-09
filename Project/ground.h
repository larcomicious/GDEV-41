#pragma once

#include "raylib.h"

struct GroundSample {
    Vector2 point;
    Vector2 normal;
    Vector2 tangent;
};

class Ground {
public:
    virtual GroundSample sample(Vector2 position) const = 0;

    virtual ~Ground() = default;
};
