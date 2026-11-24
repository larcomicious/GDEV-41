
#pragma once

#include <raylib.h>
#include <raymath.h>

#include "motorcycle.hpp"
#include "ground.hpp"

namespace motorcycle_collision {

    constexpr float contact_tolerance = 0.5f;
    constexpr float ground_friction = 5.0f;
}

inline void resolveWheelGroundCollision(
        Ball& wheel,
        bool& on_ground,
        const Ground& ground
    ) {
        GroundSample sample = ground.sample(wheel.position);

        if (!sample.valid) {
            on_ground = false;
            return;
        }

        Vector2 normal = sample.normal;
        Vector2 to_wheel = Vector2Subtract(wheel.position, sample.point);

        float distance = Vector2DotProduct(to_wheel, normal);

        on_ground = distance <= wheel.radius + motorcycle_collision::contact_tolerance;

        // Resolve clipping
        if (distance < wheel.radius) {
            float penetration = wheel.radius - distance;

            wheel.position = Vector2Add(
                wheel.position,
                Vector2Scale(normal, penetration)
            );

            // Remove velocity directed into the ground
            float normal_velocity = Vector2DotProduct(
                wheel.velocity, normal
            );

            if (normal_velocity < 0.0f) {
                wheel.velocity = Vector2Subtract(
                    wheel.velocity,
                    Vector2Scale(normal, normal_velocity)
                );
            }
        }
    }

    inline void handleGroundCollision(
        Motorcycle& motorcycle,
        const Ground& ground
    ) {
        resolveWheelGroundCollision(
            motorcycle.front_wheel,
            motorcycle.front_wheel_on_ground,
            ground
        );

        resolveWheelGroundCollision(
            motorcycle.rear_wheel,
            motorcycle.rear_wheel_on_ground,
            ground
        );

        resolveWheelGroundCollision(
            motorcycle.body,
            motorcycle.body_on_ground,
            ground
        );
    }

    inline void applyGroundFriction(
        Ball& wheel,
        bool on_ground,
        const Ground& ground,
        float timestep
    ) {
        if (!on_ground)
            return;

        GroundSample sample = ground.sample(wheel.position);

        Vector2 tangent = sample.tangent;

        // Project velocity onto the ground tangent
        float tangent_velocity = Vector2DotProduct(
            wheel.velocity, tangent
        );

        Vector2 tangential_component = Vector2Scale(
            tangent, tangent_velocity
        );

        // Preserve velocity perpendicular to the ground
        Vector2 normal_component = Vector2Subtract(
            wheel.velocity, tangential_component
        );

        // Apply tangential damping
        float friction_factor =
            1.0f - motorcycle_collision::ground_friction * wheel.inverse_mass * timestep;

        if (friction_factor < 0.0f)
            friction_factor = 0.0f;

        tangential_component = Vector2Scale(
            tangential_component, friction_factor
        );

        wheel.velocity = Vector2Add(
            tangential_component, normal_component
        );
    }

