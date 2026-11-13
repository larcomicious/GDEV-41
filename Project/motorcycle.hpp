#pragma once

#include <raylib.h>

struct Ball {
    Vector2 position;
    float radius;
    Color color;

    float mass;
    float inverse_mass;

    Vector2 acceleration;
    Vector2 velocity;
};

struct Spring {
    Vector2 spring_start;
    Vector2 spring_end;

    float rest_length;
    float b;
    float k;
};

namespace {
    constexpr float SPRING_REST_LENGTH = 80.0f;
    constexpr float SPRING_K = 3000.0f;
    constexpr float SPRING_B = 100.0f;
    constexpr float WHEEL_DISTANCE = 100.0f;
    constexpr float BODY_TO_WHEEL_HEIGHT = 100.0f;
}

struct Motorcycle {
    Vector2 position;
    Vector2 local_up;
    Vector2 forward;

    Ball body;
    Ball front_wheel;
    Ball rear_wheel;

    Spring front_spring;
    Spring rear_spring;

    bool front_wheel_on_ground;
    bool rear_wheel_on_ground;
    bool body_on_ground;

    bool debug = true;

    void initialize() {
        //body
        body.position = {300, 300};
        body.radius = 30;
        body.color = RED;
        body.mass = 10.0f;
        body.inverse_mass = 1.0f / body.mass;
        body.acceleration = Vector2Zero();
        body.velocity = Vector2Zero();

        // wheels
        front_wheel.position = {350, 380};
        front_wheel.radius = 25;
        front_wheel.color = BLACK;
        front_wheel.mass = 5.0f;
        front_wheel.inverse_mass = 1.0f / front_wheel.mass;
        front_wheel.acceleration = Vector2Zero();
        front_wheel.velocity = Vector2Zero();

        rear_wheel.position = {250, 380};
        rear_wheel.radius = 25;
        rear_wheel.color = BLACK;
        rear_wheel.mass = 5.0f;
        rear_wheel.inverse_mass = 1.0f / rear_wheel.mass;
        rear_wheel.acceleration = Vector2Zero();
        rear_wheel.velocity = Vector2Zero();

        // suspensions
        front_spring.spring_start = body.position;
        front_spring.spring_end = front_wheel.position;
        front_spring.rest_length = SPRING_REST_LENGTH;
        front_spring.b = SPRING_B;
        front_spring.k = SPRING_K;

        rear_spring.spring_start = body.position;
        rear_spring.spring_end = rear_wheel.position;
        rear_spring.rest_length = SPRING_REST_LENGTH;
        rear_spring.b = SPRING_B;
        rear_spring.k = SPRING_K;

        front_wheel_on_ground = false;
        rear_wheel_on_ground = false;
        body_on_ground = false;

        position = body.position;
        forward = Vector2Normalize(Vector2Subtract(front_wheel.position, rear_wheel.position));
        local_up = Vector2Normalize({forward.y, -forward.x});
        // m.wheel_to_wheel_suspension.spring_start = m.rear_wheel.position;
        // m.wheel_to_wheel_suspension.spring_end = m.front_wheel.position;
        // m.wheel_to_wheel_suspension.rest_length = WHEEL_DISTANCE;
        // m.wheel_to_wheel_suspension.k = WHEEL_TO_WHEEL_STIFFNESS;
        // m.wheel_to_wheel_suspension.b = WHEEL_TO_WHEEL_DAMPNESS;
    }
    void updateOrientation() {
        position = body.position;

        forward = Vector2Normalize(Vector2Subtract(front_wheel.position, rear_wheel.position));
        local_up = Vector2Normalize({forward.y, -forward.x});
    }
    void draw() const {
        DrawLineEx(position, front_wheel.position, 5.0f, GRAY);
        DrawLineEx(position, rear_wheel.position, 5.0f, GRAY);

        DrawCircleV(front_wheel.position, front_wheel.radius, front_wheel.color);
        DrawCircleV(rear_wheel.position, rear_wheel.radius, rear_wheel.color);
        DrawCircleV(body.position, body.radius, body.color);

        if (debug) {
            DrawLineEx(position, Vector2Add(position, Vector2Scale(forward, 100.0f)), 5.0f, GREEN);
            DrawLineEx(position, Vector2Add(position, Vector2Scale(local_up, 100.0f)), 5.0f, BLUE);
        }
    }
};
