
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
    TrackBuilder(Vector2 start) : currentPoint(start) {
    }

    // add a straight segment with  specific enpoint.
    void addStraightTo(Vector2 end) {
        track.addSegment(currentPoint, end);

        previousTangent = Vector2Subtract(end, currentPoint);
        hasPreviousDirection = true;

        currentPoint = end;
    }

    // for gaps
    void skipTo(Vector2 end) {
        track.beginNewChain();

        currentPoint = end;

        previousTangent = Vector2Zero();
        hasPreviousDirection = false;
    }

    // for cubic bezier curves
    void addBezierTo(Vector2 control1, Vector2 control2, Vector2 end, int subdivisions = 5) {
        track.addBezier(currentPoint, control1, control2, end, subdivisions);

        previousTangent = Vector2Subtract(end, control2);
        hasPreviousDirection = true;

        currentPoint = end;
    }

    // new bezier curve that continues the with dir of previous section
    void addSmoothBezierTo(Vector2 control2, Vector2 end, int subdivisions = 5) {
        if (!hasPreviousDirection)
            return;

        Vector2 control1 = Vector2Add(currentPoint, previousTangent);

        track.addBezier(currentPoint, control1, control2,end, subdivisions);

        previousTangent = Vector2Subtract(end, control2);
        currentPoint = end;
    }

    const Track& getTrack() const {
        return track;
    }
};
