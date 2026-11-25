#pragma once

#include <raylib.h>
#include "ground.hpp"
#include <raymath.h>

class FlatGround : public Ground {
private:
    float ground_y;

public:
    FlatGround(float y) : ground_y(y) {}

    GroundSample sample(Vector2 position) const override {

    GroundSample result{};

    result.point = {position.x, ground_y};
    result.normal = {0.0f, -1.0f};
    result.tangent = {1.0f, 0.0f};

    Vector2 offset = Vector2Subtract(position, result.point);
    result.distance = Vector2Length(offset);
    result.signed_distance = Vector2DotProduct(offset, result.normal);

    result.separation = result.signed_distance;
    result.valid = true;

    return result;
}
};
