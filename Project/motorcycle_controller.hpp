#pragma once

#include <raylib.h>

struct MotorcycleInput {
    float throttle = 0.0f;
    float rotation = 0.0f;

    bool nitro_active = false;
    bool jump_pressed = false;
    bool reset = false;
};

inline MotorcycleInput readMotorcycleInput() {

    MotorcycleInput input;

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        input.throttle += 1.0f;

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        input.throttle -= 1.0f;

    // air rotation
    if (IsKeyDown(KEY_W))
        input.rotation -= 1.0f;

    if (IsKeyDown(KEY_S))
        input.rotation += 1.0f;

    input.nitro_active = IsKeyDown(KEY_LEFT_SHIFT);

    input.jump_pressed = IsKeyPressed(KEY_SPACE);

    input.reset = IsKeyPressed(KEY_R);

    return input;
}
