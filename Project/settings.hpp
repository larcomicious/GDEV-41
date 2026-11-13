
#pragma once

namespace settings {

    inline constexpr int window_height = 720;
    inline constexpr int target_fps = 60;

    // physics
    inline constexpr float fixed_timestep = 1.0f / 120.0f;
    inline constexpr float gravity_y = 1000.0f;

    // movement
    inline constexpr float engine_force = 6000.0f;
    inline constexpr float jump_velocity = 500.0f;
    inline constexpr float turbo_multiplier = 3.0f;
    inline constexpr float airborne_engine_multiplier = 0.35f;

    // Suspension
    inline constexpr float spring_rest_length = 80.0f;
    inline constexpr float spring_k = 3000.0f;
    inline constexpr float spring_b = 100.0f;
    inline constexpr float wheel_distance = 100.0f;

    // Ground
    inline constexpr float ground_friction = 15.0f;

    // Air control
    inline constexpr float air_rotation_strength = 80000.0f;
    inline constexpr float air_rotation_damping = 20000.0f;
    inline constexpr float air_rotation_control = 60000.0f;

    // Gameplay
    inline constexpr int max_jump_count = 2;

    // Camera
    inline constexpr float camera_speed = 5.0f;
}
