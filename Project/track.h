
#pragma once

#include <vector>
#include <limits>

#include "ground.h"
#include "segment_ground.h"

class Track : public Ground {
private:
    std::vector<SegmentGround> segments;

public:
    void addSegment(Vector2 start, Vector2 end) {
        segments.emplace_back(start, end);
    }

    GroundSample sample(Vector2 position) const override {

        if (segments.empty()) {
            return {position, {0.0f, -1.0f}, {1.0f, 0.0f}, -1};
        }

        float closest_distance_squared = std::numeric_limits<float>::max();

        GroundSample closest_sample{};

        for (int i = 0; i < segments.size(); i++) {

            GroundSample candidate = segments[i].sample(position);

            if (!candidate.valid)
                continue;

            float dx = position.x - candidate.point.x;
            float dy = position.y - candidate.point.y;
            float distance_squared = dx * dx + dy * dy;

            if (distance_squared < closest_distance_squared) {
                closest_distance_squared = distance_squared;
                candidate.segment_index = i;
                closest_sample = candidate;
            }
        }

        if (closest_sample.segment_index == -1) { 
            return {position, {0.0f, -1.0f}, {1.0f, 0.0f}, -1, false};
        }

        return closest_sample;
    }

    const std::vector<SegmentGround>& getSegments() const {
        return segments;
    }
};
