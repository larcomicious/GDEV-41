
#pragma once

#include "ground.hpp"
#include <raymath.h>

class SegmentGround : public Ground {
private:
    Vector2 start;
    Vector2 end;

    int chain_id;

public:
    SegmentGround(
        Vector2 start,
        Vector2 end,
        int chain_id = 0
    )
        : start(start), end(end), chain_id(chain_id) {
    }

    GroundSample sample(Vector2 position) const override {
        GroundSample result{};

        Vector2 ab = Vector2Subtract(end, start);
        float length_squared = Vector2DotProduct(ab, ab);

        if (length_squared <= 0.0001f)
            return result;

        Vector2 ap = Vector2Subtract(position, start);

        float raw_t = Vector2DotProduct(ap, ab) / length_squared;
        float t = Clamp(raw_t, 0.0f, 1.0f);

        Vector2 closest_point = Vector2Add(
            start,
            Vector2Scale(ab, t)
        );

        Vector2 tangent = Vector2Normalize(ab);

        // Surface normal for the direction this segment was constructed.
        Vector2 surface_normal = Vector2Normalize({
            tangent.y,
            -tangent.x
        });

        Vector2 offset = Vector2Subtract(
            position,
            closest_point
        );

        float distance = Vector2Length(offset);

        float signed_distance = Vector2DotProduct(
            offset,
            surface_normal
        );

        bool at_start = raw_t <= 0.0f;
        bool at_end = raw_t >= 1.0f;
        bool at_endpoint = at_start || at_end;

        Vector2 contact_normal = surface_normal;
        float separation = signed_distance;

        // Open endpoint: use a radial contact normal.
        if (at_endpoint && distance > 0.0001f) {
            contact_normal = Vector2Scale(
                offset,
                1.0f / distance
            );

            separation = distance;
        }

        result.point = closest_point;
        result.normal = contact_normal;
        result.tangent = tangent;

        result.valid = true;

        result.distance = distance;
        result.signed_distance = signed_distance;
        result.separation = separation;

        result.at_endpoint = at_endpoint;
        result.at_start_endpoint = at_start;
        result.at_end_endpoint = at_end;

        return result;
    }

    Vector2 getStart() const {
        return start;
    }

    Vector2 getEnd() const {
        return end;
    }

    int getChainID() const {
        return chain_id;
    }

    Vector2 getSurfaceNormal() const {
        Vector2 tangent = Vector2Normalize(
            Vector2Subtract(end, start)
        );

        return Vector2Normalize({
            tangent.y,
            -tangent.x
        });
    }
};
