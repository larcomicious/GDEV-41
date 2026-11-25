
#pragma once

#include <vector>
#include <limits>
#include <algorithm>

#include "ground.hpp"
#include "segment_ground.hpp"

class Track : public Ground {
private:
    static constexpr int local_search_radius = 6;

    std::vector<SegmentGround> segments;

    int active_chain_id = 0;

    Vector2 lastBezierP2{};
    Vector2 lastBezierP3{};
    bool hasPreviousBezier = false;

    GroundSample findClosestSample(
        Vector2 position,
        int first_segment,
        int last_segment,
        int required_chain,
        float max_distance,
        float max_backface_penetration
    ) const {

        float closest_distance_squared =
            std::numeric_limits<float>::max();

        GroundSample closest_sample{};

        for (int i = first_segment; i <= last_segment; i++) {

            const SegmentGround& segment = segments[i];

            // Restrict searches to the same connected terrain chain.
            if (required_chain >= 0 &&
                segment.getChainID() != required_chain) {
                continue;
            }

            GroundSample candidate = segment.sample(position);

            if (!candidate.valid)
                continue;

            candidate.segment_index = i;

            if (candidate.distance > max_distance)
                continue;

            // Smooth the normal at connected segment joints.
            Vector2 joined_normal{};

            bool has_joined_normal = false;

            if (candidate.at_start_endpoint && i > 0) {
                const SegmentGround& previous = segments[i - 1];

                if (previous.getChainID() == segment.getChainID()) {
                    joined_normal = Vector2Add(
                        previous.getSurfaceNormal(),
                        segment.getSurfaceNormal()
                    );

                    has_joined_normal = true;
                }
            }
            else if (candidate.at_end_endpoint &&
                     i + 1 < static_cast<int>(segments.size())) {

                const SegmentGround& next = segments[i + 1];

                if (next.getChainID() == segment.getChainID()) {
                    joined_normal = Vector2Add(
                        segment.getSurfaceNormal(),
                        next.getSurfaceNormal()
                    );

                    has_joined_normal = true;
                }
            }

            if (has_joined_normal &&
                Vector2Length(joined_normal) > 0.0001f) {

                joined_normal = Vector2Normalize(joined_normal);

                Vector2 offset = Vector2Subtract(
                    position,
                    candidate.point
                );

                candidate.normal = joined_normal;

                candidate.separation = Vector2DotProduct(
                    offset,
                    joined_normal
                );

                candidate.signed_distance = candidate.separation;
            }

            // Reject surfaces approached from their backside.
            if (candidate.signed_distance <
                -max_backface_penetration) {
                continue;
            }

            float distance_squared =
                candidate.distance * candidate.distance;

            if (distance_squared < closest_distance_squared) {
                closest_distance_squared = distance_squared;
                closest_sample = candidate;
            }
        }

        return closest_sample;
    }

public:
    void beginNewChain() {
        active_chain_id++;
    }

    void addSegment(Vector2 start, Vector2 end) {
        segments.emplace_back(
            start,
            end,
            active_chain_id
        );
    }

    void addBezier(
        Vector2 p0,
        Vector2 p1,
        Vector2 p2,
        Vector2 p3,
        int subdivisions = 5
    ) {
        if (subdivisions < 1)
            subdivisions = 1;

        Vector2 previous = p0;

        for (int i = 1; i <= subdivisions; i++) {
            float t = static_cast<float>(i) / subdivisions;
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

    void addSmoothBezier(
        Vector2 p2,
        Vector2 p3,
        int subdivisions = 5
    ) {
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
        return sample(
            position,
            -1,
            std::numeric_limits<float>::max(),
            8.0f
        );
    }

    GroundSample sample(
        Vector2 position,
        int preferred_segment_index
    ) const override {
        return sample(
            position,
            preferred_segment_index,
            std::numeric_limits<float>::max(),
            8.0f
        );
    }

    GroundSample sample(
        Vector2 position,
        int preferred_segment_index,
        float max_distance,
        float max_backface_penetration
    ) const override {

        if (segments.empty())
            return {};

        int segment_count = static_cast<int>(segments.size());

        // First, search near the previously contacted segment.
        if (preferred_segment_index >= 0 &&
            preferred_segment_index < segment_count) {

            int first_segment = std::max(
                0,
                preferred_segment_index - local_search_radius
            );

            int last_segment = std::min(
                segment_count - 1,
                preferred_segment_index + local_search_radius
            );

            int preferred_chain =
                segments[preferred_segment_index].getChainID();

            GroundSample local_sample = findClosestSample(
                position,
                first_segment,
                last_segment,
                preferred_chain,
                max_distance,
                max_backface_penetration
            );

            if (local_sample.valid)
                return local_sample;
        }

        // If local search fails, reacquire from the entire track.
        return findClosestSample(
            position,
            0,
            segment_count - 1,
            -1,
            max_distance,
            max_backface_penetration
        );
    }

    const std::vector<SegmentGround>& getSegments() const {
        return segments;
    }
};
