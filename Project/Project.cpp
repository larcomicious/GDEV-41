#include <iostream>
#include <raylib.h>
#include <raymath.h>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / 120.0f;

const Vector2 GRAVITY = {0, 1000};

const float ENGINE_FORCE = 8000.0f;

const float SPRING_REST_LENGTH = 100.0f;
const float SPRING_K = 1000.0f;
const float SPRING_B = 1.0f;

const float GROUND_Y = 600.0f;
const float GROUND_FRICTION = 15.0f;

const float WHEEL_DISTANCE = 100.0f;
const float WHEEL_TO_WHEEL_STIFFNESS = 1000.0f;
const float WHEEL_TO_WHEEL_DAMPNESS = 100.0f;

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

struct Motorcycle {
    Ball body;
    Ball front_wheel;
    Ball rear_wheel;

    Spring front_spring;
    Spring rear_spring;
    Spring wheel_to_wheel_suspension;

    void Draw() {
        // springs
        // DrawLineEx(body.position, front_wheel.position, 6.0f, DARKGRAY);
        // DrawLineEx(body.position, rear_wheel.position, 6.0f, DARKGRAY);
        DrawLineEx(front_wheel.position, rear_wheel.position, 8.0f, DARKGRAY);
        
        DrawCircleV(front_wheel.position, front_wheel.radius, front_wheel.color);
        DrawCircleV(rear_wheel.position, rear_wheel.radius, rear_wheel.color);
        DrawCircleV(body.position, body.radius, body.color);
    }
};

// global variables
Motorcycle motorcycle;
bool onGround;

void initializeMotorcycle() {
    Motorcycle& m = motorcycle;
    // body
    m.body.position = {300, 300};
    m.body.radius = 25;
    m.body.color = RED;
    m.body.mass = 10.0f;
    m.body.inverse_mass = 1.0f / m.body.mass;
    m.body.acceleration = Vector2Zero();
    m.body.velocity = Vector2Zero();

    // wheels
    m.front_wheel.position = {350, 380};
    m.front_wheel.radius = 25;
    m.front_wheel.color = BLACK;
    m.front_wheel.mass = 10.0f;
    m.front_wheel.inverse_mass = 1.0f / m.front_wheel.mass;
    m.front_wheel.acceleration = Vector2Zero();
    m.front_wheel.velocity = Vector2Zero();

    m.rear_wheel.position = {250, 380};
    m.rear_wheel.radius = 25;
    m.rear_wheel.color = BLACK;
    m.rear_wheel.mass = 10.0f;
    m.rear_wheel.inverse_mass = 1.0f / m.rear_wheel.mass;
    m.rear_wheel.acceleration = Vector2Zero();
    m.rear_wheel.velocity = Vector2Zero();

    // springs
    m.front_spring.spring_start = m.body.position;
    m.front_spring.spring_end = m.front_wheel.position;
    m.front_spring.rest_length = SPRING_REST_LENGTH;
    m.front_spring.b = SPRING_B;
    m.front_spring.k = SPRING_K;

    m.rear_spring.spring_start = m.body.position;
    m.rear_spring.spring_end = m.rear_wheel.position;
    m.rear_spring.rest_length = SPRING_REST_LENGTH;
    m.rear_spring.b = SPRING_B;
    m.rear_spring.k = SPRING_K;

    m.wheel_to_wheel_suspension.spring_start = m.rear_wheel.position;
    m.wheel_to_wheel_suspension.spring_end = m.front_wheel.position;
    m.wheel_to_wheel_suspension.rest_length = WHEEL_DISTANCE;
    m.wheel_to_wheel_suspension.k = WHEEL_TO_WHEEL_STIFFNESS;
    m.wheel_to_wheel_suspension.b = WHEEL_TO_WHEEL_DAMPNESS;

}

void handlePhysicsMovement() {
    Motorcycle& m = motorcycle;
    // m.body.velocity = Vector2Add(m.body.velocity, Vector2Scale(m.body.acceleration, TIMESTEP));
    // m.body.position = Vector2Add(m.body.position, Vector2Scale(m.body.velocity, TIMESTEP));

    m.front_wheel.velocity = Vector2Add(m.front_wheel.velocity, Vector2Scale(m.front_wheel.acceleration, TIMESTEP));
    m.front_wheel.position = Vector2Add(m.front_wheel.position, Vector2Scale(m.front_wheel.velocity, TIMESTEP));

    m.rear_wheel.velocity = Vector2Add(m.rear_wheel.velocity, Vector2Scale(m.rear_wheel.acceleration, TIMESTEP));
    if (onGround)
        m.rear_wheel.velocity = Vector2Subtract(m.rear_wheel.velocity, Vector2Scale(m.rear_wheel.velocity, GROUND_FRICTION * m.rear_wheel.inverse_mass * TIMESTEP));
    m.rear_wheel.position = Vector2Add(m.rear_wheel.position, Vector2Scale(m.rear_wheel.velocity, TIMESTEP));

    m.wheel_to_wheel_suspension.spring_start = m.rear_wheel.position;
    m.wheel_to_wheel_suspension.spring_end = m.front_wheel.position;
    // m.front_spring.spring_start = m.body.position;
    // m.front_spring.spring_end = m.front_wheel.position;

    // m.rear_spring.spring_start = m.body.position;
    // m.rear_spring.spring_end = m.rear_wheel.position;
}

void handleGroundCollision() {
    Motorcycle& m = motorcycle;

    if (m.front_wheel.position.y + m.front_wheel.radius > GROUND_Y) {
        m.front_wheel.position.y = GROUND_Y - m.front_wheel.radius;

        if (m.front_wheel.velocity.y > 0)
            m.front_wheel.velocity.y = 0;
    }

    if (m.rear_wheel.position.y + m.rear_wheel.radius > GROUND_Y) {
        onGround = true;
        std::cout << "on ground" << std::endl;
        m.rear_wheel.position.y = GROUND_Y - m.rear_wheel.radius;
        
        if (m.rear_wheel.velocity.y > 0)
            m.rear_wheel.velocity.y = 0;
    }
    else {
        onGround = false;
        std::cout << "not on ground" << std::endl;
    }
}

Vector2 calculateSpringForce(Vector2 spring_start, Vector2 spring_end, float k, float rest_length) {
    Vector2 d = Vector2Subtract(spring_end, spring_start);
    Vector2 d_norm = Vector2Normalize(d);

    return Vector2Scale(d_norm, - k * (Vector2Length(d) - rest_length));

    // return Vector2Subtract(Vector2Scale(d_norm, - k * (Vector2Length(d) - rest_length)), Vector2Scale(velocity, b));
}


int main() {
    initializeMotorcycle();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project");
    SetTargetFPS(FPS);

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        // force accumulation
        Vector2 body_forces = Vector2Zero();
        Vector2 front_wheel_forces = Vector2Zero();
        Vector2 rear_wheel_forces = Vector2Zero();

        // Vector2 body_gravity = Vector2Scale(GRAVITY, motorcycle.body.mass);
        // body_forces = Vector2Add(body_forces, body_gravity);

        Vector2 front_gravity = Vector2Scale(GRAVITY, motorcycle.front_wheel.mass);
        front_wheel_forces = Vector2Add(front_wheel_forces, front_gravity);

        Vector2 rear_gravity = Vector2Scale(GRAVITY, motorcycle.rear_wheel.mass);
        rear_wheel_forces = Vector2Add(rear_wheel_forces, rear_gravity);

        // suspension forces
        // Vector2 D = Vector2Subtract(motorcycle.front_spring.spring_end, motorcycle.front_spring.spring_start);
        // Vector2 D_norm = Vector2Normalize(D);
        // Vector2 front_spring_force = Vector2Scale(D_norm, -motorcycle.front_spring.k * (Vector2Length(D) - motorcycle.front_spring.rest_length));

        // front_wheel_forces = Vector2Add(front_wheel_forces, front_spring_force);
        // body_forces = Vector2Subtract(body_forces, front_spring_force);

        // Vector2 D_rear = Vector2Subtract(motorcycle.rear_spring.spring_end, motorcycle.rear_spring.spring_start);
        // Vector2 D_rear_norm = Vector2Normalize(D_rear);
        // Vector2 rear_spring_force = Vector2Scale(D_rear_norm, -motorcycle.rear_spring.k * (Vector2Length(D_rear) - motorcycle.rear_spring.rest_length));

        // rear_spring_force = Vector2Subtract(rear_spring_force, Vector2Scale(motorcycle.rear_wheel.velocity, motorcycle.rear_spring.b));
        // rear_wheel_forces = Vector2Add(rear_wheel_forces, rear_spring_force);
        // body_forces = Vector2Subtract(body_forces, rear_spring_force);

        // wheel to wheel suspension
        Vector2 wheel_to_wheel_suspension = calculateSpringForce(motorcycle.rear_wheel.position, motorcycle.front_wheel.position, WHEEL_TO_WHEEL_STIFFNESS, WHEEL_DISTANCE);
        Vector2 wheel_axis = Vector2Normalize(Vector2Subtract(motorcycle.front_wheel.position, motorcycle.rear_wheel.position));
        Vector2 relative_wheel_velocity = Vector2Subtract(motorcycle.front_wheel.velocity, motorcycle.rear_wheel.velocity);
        float axial_relative_speed = Vector2DotProduct(relative_wheel_velocity, wheel_axis);
        Vector2 damping_force = Vector2Scale(wheel_axis, -WHEEL_TO_WHEEL_DAMPNESS * axial_relative_speed);
        Vector2 front_suspension_force = Vector2Add(wheel_to_wheel_suspension, damping_force);

        front_wheel_forces = Vector2Add(front_wheel_forces, front_suspension_force);
        rear_wheel_forces = Vector2Subtract(rear_wheel_forces, front_suspension_force);

        if (IsKeyDown(KEY_RIGHT))
        {
            rear_wheel_forces.x += ENGINE_FORCE;
        }

        if (IsKeyDown(KEY_LEFT))
        {
            rear_wheel_forces.x -= ENGINE_FORCE;
        }

        if (IsKeyPressed(KEY_R))
        {
            initializeMotorcycle();
        }
        
        // motorcycle.body.acceleration = Vector2Scale(body_forces, motorcycle.body.inverse_mass);
        motorcycle.front_wheel.acceleration = Vector2Scale(front_wheel_forces, motorcycle.front_wheel.inverse_mass);
        motorcycle.rear_wheel.acceleration = Vector2Scale(rear_wheel_forces, motorcycle.rear_wheel.inverse_mass);

        // physics loop
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {

            handlePhysicsMovement();

            handleGroundCollision();

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawRectangle(0, GROUND_Y, WINDOW_WIDTH, WINDOW_HEIGHT - GROUND_Y, DARKGREEN);
        
        motorcycle.Draw();

        DrawText("RIGHT: Accelerate    LEFT: Reverse    R: Reset", 20, 20, 20, BLACK);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
