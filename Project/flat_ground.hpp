#pragma once

#include "ground.hpp"

class FlatGround : public Ground {
private:
    float ground_y;

public:
    FlatGround(float y) : ground_y(y) {}

    GroundSample sample(Vector2 position) const override {
        return {{position.x, ground_y}, {0.0f, -1.0f}, {1.0f, 0.0f}};
    }
};
