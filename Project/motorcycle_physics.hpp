
#pragma once

#include <raylib.h>
#include <raymath.h>
#include <cmath>

#include "motorcycle.hpp"
#include "motorcycle_controller.hpp"

namespace motorcycle_physics {
    constexpr float gravity_y = 1000.0f;

    constexpr float engine_force = 6000.0f;
    constexpr float jump_velocity = 500.0f;
    constexpr float turbo_multiplier = 3.0f;
    constexpr float airborne_engine_multiplier = 0.35f;

    constexpr float wheel_distance = 100.0f;

    constexpr float air_rotation_strength = 80000.0f;
    constexpr float air_rotation_damping = 20000.0f;
    constexpr float air_rotation_control = 60000.0f;

    constexpr int max_jump_count = 2;
}

inline void addDirectionForce(Vector2& forces, const Vector2& direction, float force) {
    forces = Vector2Add(forces, Vector2Scale(direction, force));
}

inline void applySpringDamper(const Ball& a, const Ball& b, const Spring& spring, Vector2& force_a, Vector2& force_b) {
    Vector2 direction = Vector2Subtract(b.position, a.position);
    float distance = Vector2Length(direction);

    if (distance < 0.0001f)
        return;

    Vector2 axis = Vector2Scale(direction, 1.0f / distance);

    Vector2 relative_velocity = Vector2Subtract(b.velocity, a.velocity);
    float axial_speed = Vector2DotProduct(relative_velocity, axis);

    float spring_magnitude = spring.k * (distance - spring.rest_length);
    float damping_magnitude = spring.b * axial_speed;
    float total_magnitude = spring_magnitude + damping_magnitude;

    Vector2 force_on_b = Vector2Scale(axis, -total_magnitude);

    force_b = Vector2Add(force_b, force_on_b);
    force_a = Vector2Subtract(force_a, force_on_b);
}

inline void solveDistanceConstraint(Ball& a, Ball& b, float rest_length) {
    Vector2 direction = Vector2Subtract(b.position, a.position);
    float distance = Vector2Length(direction);

    if (distance < 0.0001f)
        return;

    float inverse_mass_sum = a.inverse_mass + b.inverse_mass;

    if (inverse_mass_sum <= 0.0f)
        return;

    Vector2 axis = Vector2Scale(direction, 1.0f / distance);

    float error = distance - rest_length;

    Vector2 correction = Vector2Scale(axis, error / inverse_mass_sum);

    a.position = Vector2Add(a.position, Vector2Scale(correction, a.inverse_mass));
    b.position = Vector2Subtract(b.position, Vector2Scale(correction, b.inverse_mass));

    Vector2 relative_velocity = Vector2Subtract(b.velocity, a.velocity);
    float relative_speed = Vector2DotProduct(relative_velocity, axis);

    Vector2 velocity_correction = Vector2Scale(axis, relative_speed / inverse_mass_sum);
    a.velocity = Vector2Add(a.velocity, Vector2Scale(velocity_correction, a.inverse_mass));
    b.velocity = Vector2Subtract(b.velocity, Vector2Scale(velocity_correction, b.inverse_mass));
}

inline void applyAirStabilization(const Motorcycle& m, Vector2& front_wheel_forces, Vector2& rear_wheel_forces, float rotation_input) {
    Vector2 wheel_axis = Vector2Subtract(m.front_wheel.position, m.rear_wheel.position);
    float distance = Vector2Length(wheel_axis);

    if (distance < 0.0001f)
        return;

    Vector2 axis = Vector2Normalize(wheel_axis);

    float angle = std::atan2f(axis.y, axis.x);

    Vector2 relative_velocity = Vector2Subtract(m.front_wheel.velocity, m.rear_wheel.velocity);
    float angular_velocity = (wheel_axis.x * relative_velocity.y - wheel_axis.y * relative_velocity.x) / (distance * distance);

    float torque;

    // player controls rotation
    if (rotation_input != 0.0f) {
        torque = motorcycle_physics::air_rotation_control * rotation_input - motorcycle_physics::air_rotation_damping * angular_velocity;
    }
    // no input, return to horizontal.
    else {
        torque = -motorcycle_physics::air_rotation_strength * angle - motorcycle_physics::air_rotation_damping * angular_velocity;
    }

    float force_magnitude = torque / distance;

    Vector2 perpendicular = {-axis.y, axis.x};
    Vector2 rotational_force = Vector2Scale(perpendicular, force_magnitude);

    front_wheel_forces = Vector2Add(front_wheel_forces, rotational_force);
    rear_wheel_forces = Vector2Subtract(rear_wheel_forces, rotational_force);
}


inline void updateMotorcyclePhysics(Motorcycle& motorcycle, MotorcycleInput& input, int jump_count, float timestep) {
    const Vector2 gravity = {0.0f, motorcycle_physics::gravity_y};

    Vector2 body_forces = Vector2Scale(gravity, motorcycle.body.mass);
    Vector2 front_wheel_forces = Vector2Scale(gravity, motorcycle.front_wheel.mass);
    Vector2 rear_wheel_forces = Vector2Scale(gravity, motorcycle.rear_wheel.mass);

    bool is_airborne = !motorcycle.front_wheel_on_ground && !motorcycle.rear_wheel_on_ground;

    applySpringDamper(motorcycle.body, motorcycle.front_wheel, motorcycle.front_spring, body_forces, front_wheel_forces);
    applySpringDamper(motorcycle.body, motorcycle.rear_wheel, motorcycle.rear_spring, body_forces, rear_wheel_forces);

    float movement_multiplier = is_airborne ? motorcycle_physics::airborne_engine_multiplier : 1.0f;
    float base_engine_force = input.nitro_active ? motorcycle_physics::engine_force * motorcycle_physics::turbo_multiplier : motorcycle_physics::engine_force;
    float total_engine_force = input.throttle * base_engine_force * movement_multiplier;

    if (is_airborne) {

        float total_mass = motorcycle.body.mass + motorcycle.front_wheel.mass + motorcycle.rear_wheel.mass;

        if (total_mass > 0.0f) {
            addDirectionForce(body_forces, motorcycle.forward, total_engine_force * motorcycle.body.mass / total_mass);
            addDirectionForce(front_wheel_forces, motorcycle.forward, total_engine_force * motorcycle.front_wheel.mass / total_mass);
            addDirectionForce(rear_wheel_forces, motorcycle.forward, total_engine_force * motorcycle.rear_wheel.mass / total_mass);
        }

        applyAirStabilization(motorcycle, front_wheel_forces, rear_wheel_forces, input.rotation);
    }
    else {
        // grounded, rear-wheel only
        addDirectionForce(rear_wheel_forces, motorcycle.forward, total_engine_force);
    }

    // if (is_airborne) {
    //     applyAirStabilization(motorcycle, front_wheel_forces, rear_wheel_forces, input.rotation);
    // }

    if (input.jump_pressed && jump_count < motorcycle_physics::max_jump_count) {
        motorcycle.body.velocity.y = -motorcycle_physics::jump_velocity;
        motorcycle.front_wheel.velocity.y = -motorcycle_physics::jump_velocity;
        motorcycle.rear_wheel.velocity.y = -motorcycle_physics::jump_velocity;

        jump_count++;
        input.jump_pressed = false;
    }

    motorcycle.body.acceleration = Vector2Scale(body_forces, motorcycle.body.inverse_mass);
    motorcycle.front_wheel.acceleration = Vector2Scale(front_wheel_forces, motorcycle.front_wheel.inverse_mass);
    motorcycle.rear_wheel.acceleration = Vector2Scale(rear_wheel_forces, motorcycle.rear_wheel.inverse_mass);

    motorcycle.body.velocity = Vector2Add(motorcycle.body.velocity, Vector2Scale(motorcycle.body.acceleration, timestep));
    motorcycle.body.position = Vector2Add(motorcycle.body.position, Vector2Scale(motorcycle.body.velocity, timestep));

    motorcycle.front_wheel.velocity = Vector2Add(motorcycle.front_wheel.velocity, Vector2Scale(motorcycle.front_wheel.acceleration, timestep));
    motorcycle.front_wheel.position = Vector2Add(motorcycle.front_wheel.position, Vector2Scale(motorcycle.front_wheel.velocity, timestep));
    
    motorcycle.rear_wheel.velocity = Vector2Add(motorcycle.rear_wheel.velocity, Vector2Scale(motorcycle.rear_wheel.acceleration, timestep));
    motorcycle.rear_wheel.position = Vector2Add(motorcycle.rear_wheel.position, Vector2Scale(motorcycle.rear_wheel.velocity, timestep));

    motorcycle.updateOrientation();

    solveDistanceConstraint(motorcycle.rear_wheel, motorcycle.front_wheel, motorcycle_physics::wheel_distance);
}
