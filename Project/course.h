
#pragma once

#include "track_builder.h"

struct Course {
    Track track;
    Vector2 startPosition;
    float finishX;
};

inline Course createBeginnerCourse() {
    TrackBuilder builder({100.0f, 500.0f});

    // 1. Starting platform
    builder.addStraightTo({450.0f, 500.0f});

    // 2. Gentle rolling hills
    builder.addBezierTo(
        {600.0f, 500.0f},
        {650.0f, 400.0f},
        {800.0f, 420.0f},
        30
    );

    builder.addSmoothBezierTo(
        {1000.0f, 350.0f},
        {1200.0f, 430.0f},
        30
    );

    // 3. Steeper uphill section
    builder.addSmoothBezierTo(
        {1450.0f, 550.0f},
        {1650.0f, 450.0f},
        30
    );

    // 4. Uneven terrain
    builder.addStraightTo({1800.0f, 450.0f});

    builder.addBezierTo(
        {1900.0f, 450.0f},
        {1900.0f, 330.0f},
        {2050.0f, 350.0f},
        30
    );

    builder.addSmoothBezierTo(
        {2250.0f, 300.0f},
        {2400.0f, 430.0f},
        30
    );

    // 5. Final downhill stretch
    builder.addSmoothBezierTo(
        {2600.0f, 550.0f},
        {2800.0f, 500.0f},
        30
    );

    // 6. Finish platform
    builder.addStraightTo({3200.0f, 500.0f});

    return {
        builder.getTrack(),
        {150.0f, 500.0f},
        3100.0f
    };
}
