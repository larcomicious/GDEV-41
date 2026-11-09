
#pragma once

#include "ground.h"
#include <raymath.h>

class SegmentGround : public Ground {
private:
    Vector2 start;
    Vector2 end;

public:
    SegmentGround(Vector2 start, Vector2 end) : start(start), end(end) {}

    GroundSample sample(Vector2 position) const override {
        Vector2 ab = Vector2Subtract(end, start);
        float ab_length_squared = Vector2DotProduct(ab, ab);

        if (ab_length_squared <= 0.0001f)
            return {start, {0.0, -1.0f}, {1.0f, 0.0f}};
        
        Vector2 ap = Vector2Subtract(position, start);

        float raw_t = Vector2DotProduct(ap, ab) / ab_length_squared;

        float t = Clamp(raw_t, 0.0f, 1.0f);
        
        Vector2 closest_point = Vector2Add(start, Vector2Scale(ab, t));

        Vector2 tangent = Vector2Normalize(ab);
    

        // calculate upward facing normal
        Vector2 normal = Vector2Normalize({tangent.y, -tangent.x});

        bool valid = raw_t >= 0.0f && raw_t <= 1.0f;

        return {closest_point, normal, tangent, -1, valid};
    }

    Vector2 getStart() const {
        return start;
    }

    Vector2 getEnd() const {
        return end;
    }
};
