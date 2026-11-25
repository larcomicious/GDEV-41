
#pragma once

#include <raylib.h>

struct GroundSample {
    Vector2 point{};
    Vector2 normal{};
    Vector2 tangent{};

    int segment_index = -1;
    bool valid = false;

    float distance = 0.0f;
    float signed_distance = 0.0f;
    float separation = 0.0f;

    bool at_endpoint = false;
    bool at_start_endpoint = false;
    bool at_end_endpoint = false;
};

class Ground {
public:
    virtual GroundSample sample(Vector2 position) const = 0;

    virtual GroundSample sample(Vector2 position, int preferred_segment_index) const {
        return sample(position);
    }

    // one-sided sampling thats bounded
    virtual GroundSample sample(Vector2 position, int preferred_segment_index, float max_distance, float max_backface_penetration) const {
        GroundSample result = sample(position, preferred_segment_index);

        if (!result.valid || result.distance > max_distance || result.signed_distance < -max_backface_penetration) 
            result.valid = false;

        return result;
    }

    virtual ~Ground() = default;
};
