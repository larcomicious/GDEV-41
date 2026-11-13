
#pragma once

#include "track.hpp"
#include "raymath.h"

class TrackBuilder {
private:
    Track track;

    Vector2 currentPoint;
    Vector2 previousTangent;

    bool hasPreviousDirection = false;

public:
    TrackBuilder(Vector2 start)
        : currentPoint(start) {
    }

    // Add a straight section ending at the specified point.
    void addStraightTo(Vector2 end) {
        track.addSegment(currentPoint, end);

        previousTangent = Vector2Subtract(end, currentPoint);
        hasPreviousDirection = true;

        currentPoint = end;
    }

    // Add a cubic Bezier curve.
    void addBezierTo(
        Vector2 control1,
        Vector2 control2,
        Vector2 end,
        int subdivisions = 5
    ) {
        track.addBezier(
            currentPoint,
            control1,
            control2,
            end,
            subdivisions
        );

        previousTangent = Vector2Subtract(end, control2);
        hasPreviousDirection = true;

        currentPoint = end;
    }

    // Add a Bezier curve that continues the previous section's direction.
    void addSmoothBezierTo(
        Vector2 control2,
        Vector2 end,
        int subdivisions = 5
    ) {
        if (!hasPreviousDirection)
            return;

        Vector2 control1 = Vector2Add(
            currentPoint,
            previousTangent
        );

        track.addBezier(
            currentPoint,
            control1,
            control2,
            end,
            subdivisions
        );

        previousTangent = Vector2Subtract(end, control2);
        currentPoint = end;
    }

    const Track& getTrack() const {
        return track;
    }
};
