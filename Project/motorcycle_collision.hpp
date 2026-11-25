
#pragma once

#include <raylib.h>
#include <raymath.h>

#include "motorcycle.hpp"
#include "ground.hpp"

namespace motorcycle_collision {

    constexpr float contact_tolerance = 0.5f;
    constexpr float sample_search_margin = 8.0f;
    constexpr float max_backface_penetration = 8.0f;

    constexpr float ground_friction = 5.0f;

    struct MotorcycleGroundContacts {
        GroundSample front_wheel{};
        GroundSample rear_wheel{};
        GroundSample body{};
    };
}


// Resolve one circular object against the ground.
inline GroundSample resolveWheelGroundCollision(
    Ball& wheel,
    bool& on_ground,
    int& preferred_segment_index,
    const Ground& ground
) {

    float search_distance =
        wheel.radius +
        motorcycle_collision::contact_tolerance +
        motorcycle_collision::sample_search_margin;

    GroundSample sample = ground.sample(
        wheel.position,
        preferred_segment_index,
        search_distance,
        motorcycle_collision::max_backface_penetration
    );

    if (!sample.valid) {
        on_ground = false;
        return sample;
    }

    // Remember the segment, even when the wheel is just above it.
    preferred_segment_index = sample.segment_index;

    // The wheel must actually overlap or nearly touch the terrain.
    if (sample.distance >
            wheel.radius + motorcycle_collision::contact_tolerance ||
        sample.separation >
            wheel.radius + motorcycle_collision::contact_tolerance) {

        on_ground = false;
        return sample;
    }

    on_ground = true;

    // Resolve penetration.
    if (sample.separation < wheel.radius) {

        float penetration = wheel.radius - sample.separation;

        wheel.position = Vector2Add(
            wheel.position,
            Vector2Scale(sample.normal, penetration)
        );

        // Remove velocity directed into the surface.
        float normal_velocity = Vector2DotProduct(
            wheel.velocity,
            sample.normal
        );

        if (normal_velocity < 0.0f) {
            wheel.velocity = Vector2Subtract(
                wheel.velocity,
                Vector2Scale(sample.normal, normal_velocity)
            );
        }
    }

    return sample;
}


// Resolve all motorcycle components and preserve their contact samples.
inline void handleGroundCollision(
    Motorcycle& motorcycle,
    const Ground& ground,
    motorcycle_collision::MotorcycleGroundContacts& contacts
) {

    contacts.front_wheel = resolveWheelGroundCollision(
        motorcycle.front_wheel,
        motorcycle.front_wheel_on_ground,
        motorcycle.front_ground_segment_index,
        ground
    );

    contacts.rear_wheel = resolveWheelGroundCollision(
        motorcycle.rear_wheel,
        motorcycle.rear_wheel_on_ground,
        motorcycle.rear_ground_segment_index,
        ground
    );

    contacts.body = resolveWheelGroundCollision(
        motorcycle.body,
        motorcycle.body_on_ground,
        motorcycle.body_ground_segment_index,
        ground
    );
}


// Friction uses the exact contact sample selected during collision.
inline void applyGroundFriction(
    Ball& wheel,
    bool on_ground,
    const GroundSample& sample,
    float timestep
) {

    if (!on_ground || !sample.valid)
        return;

    Vector2 tangent = sample.tangent;

    float tangent_velocity = Vector2DotProduct(
        wheel.velocity,
        tangent
    );

    Vector2 tangential_component = Vector2Scale(
        tangent,
        tangent_velocity
    );

    Vector2 normal_component = Vector2Subtract(
        wheel.velocity,
        tangential_component
    );

    float friction_factor =
        1.0f -
        motorcycle_collision::ground_friction *
        wheel.inverse_mass *
        timestep;

    if (friction_factor < 0.0f)
        friction_factor = 0.0f;

    tangential_component = Vector2Scale(
        tangential_component,
        friction_factor
    );

    wheel.velocity = Vector2Add(
        tangential_component,
        normal_component
    );
}
