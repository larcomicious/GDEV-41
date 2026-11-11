
#pragma once

#include <vector>
#include <limits>

#include "ground.h"
#include "segment_ground.h"

class Track : public Ground {
private:
Vector2 lastBezierP2;
Vector2 lastBezierP3;
bool hasPreviousBezier = false;

public:
std::vector<SegmentGround> segments;
    void addSegment(Vector2 start, Vector2 end) {
        segments.emplace_back(start, end);
    }

    void addBezier(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, int subdivisions = 5) {
        if (subdivisions < 1)
            subdivisions = 1;

        Vector2 previous = p0;

        for (int i = 1; i <= subdivisions; i++) {
            float t = (float)i / subdivisions;
            float u = 1.0f - t;

            Vector2 current = {
                u * u * u * p0.x +
                3.0f * u * u * t * p1.x +
                3.0f * u * t * t * p2.x +
                t * t * t * p3.x,

                u * u * u * p0.y +
                3.0f * u * u * t * p1.y +
                3.0f * u * t * t * p2.y +
                t * t * t * p3.y
            };

            addSegment(previous, current);
            previous = current;
        }

        lastBezierP2 = p2;
        lastBezierP3 = p3;
        hasPreviousBezier = true;
    }

    void addSmoothBezier(Vector2 p2, Vector2 p3, int subdivisions = 5) {
        if (!hasPreviousBezier)
            return;

        Vector2 p0 = lastBezierP3;

        Vector2 tangent = Vector2Subtract(
            lastBezierP3,
            lastBezierP2
        );

        Vector2 p1 = Vector2Add(p0, tangent);

        addBezier(p0, p1, p2, p3, subdivisions);
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
