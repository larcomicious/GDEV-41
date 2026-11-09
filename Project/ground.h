#pragma once

#include "raylib.h"

struct GroundSample {
    Vector2 point;
    Vector2 normal;
    Vector2 tangent;

    int segment_index = -1;
    bool valid = false;
};

class Ground {
public:
    virtual GroundSample sample(Vector2 position) const = 0;

    virtual ~Ground() = default;
};
