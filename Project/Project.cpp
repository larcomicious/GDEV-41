#include <iostream>
#include <raylib.h>
#include <raymath.h>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / 120.0f;

const Vector2 GRAVITY = {0, 1000};

const float ENGINE_FORCE = 1500.0f;

const float SPRING_REST_LENGTH = 100.0f;
const float SPRING_K = 1000.0f;
const float SPRING_B = 1.0f;

const float GROUND_Y = 600.0f;

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

    void Draw() {
        // springs
        DrawLineEx(body.position, front_wheel.position, 6.0f, DARKGRAY);
        DrawLineEx(body.position, rear_wheel.position, 6.0f, DARKGRAY);
        DrawLineEx(front_wheel.position, rear_wheel.position, 8.0f, DARKGRAY);
        
        DrawCircleV(front_wheel.position, front_wheel.radius, front_wheel.color);
        DrawCircleV(rear_wheel.position, rear_wheel.radius, rear_wheel.color);
        DrawCircleV(body.position, body.radius, body.color);
    }
};

void initializeMotorcycle(Motorcycle& motorcycle) {
    // body
    motorcycle.body.position = {300, 300};
    motorcycle.body.radius = 25;
    motorcycle.body.color = RED;
    motorcycle.body.mass = 10.0f;
    motorcycle.body.inverse_mass = 1.0f / motorcycle.body.mass;
    motorcycle.body.acceleration = Vector2Zero();
    motorcycle.body.velocity = Vector2Zero();

    // wheels
    motorcycle.front_wheel.position = {350, 380};
    motorcycle.front_wheel.radius = 25;
    motorcycle.front_wheel.color = BLACK;
    motorcycle.front_wheel.mass = 30.0f;
    motorcycle.front_wheel.inverse_mass = 1.0f / motorcycle.front_wheel.mass;
    motorcycle.front_wheel.acceleration = Vector2Zero();
    motorcycle.front_wheel.velocity = Vector2Zero();

    motorcycle.rear_wheel.position = {250, 380};
    motorcycle.rear_wheel.radius = 25;
    motorcycle.rear_wheel.color = BLACK;
    motorcycle.rear_wheel.mass = 30.0f;
    motorcycle.rear_wheel.inverse_mass = 1.0f / motorcycle.rear_wheel.mass;
    motorcycle.rear_wheel.acceleration = Vector2Zero();
    motorcycle.rear_wheel.velocity = Vector2Zero();

    // springs
    motorcycle.front_spring.spring_start = motorcycle.body.position;
    motorcycle.front_spring.spring_end = motorcycle.front_wheel.position;
    motorcycle.front_spring.rest_length = SPRING_REST_LENGTH;
    motorcycle.front_spring.b = SPRING_B;
    motorcycle.front_spring.k = SPRING_K;

    motorcycle.rear_spring.spring_start = motorcycle.body.position;
    motorcycle.rear_spring.spring_end = motorcycle.rear_wheel.position;
    motorcycle.rear_spring.rest_length = SPRING_REST_LENGTH;
    motorcycle.rear_spring.b = SPRING_B;
    motorcycle.rear_spring.k = SPRING_K;
}

void handlePhysicsMovement(Motorcycle& motorcycle) {
    motorcycle.body.velocity = Vector2Add(motorcycle.body.velocity, Vector2Scale(motorcycle.body.acceleration, TIMESTEP));
    motorcycle.body.position = Vector2Add(motorcycle.body.position, Vector2Scale(motorcycle.body.velocity, TIMESTEP));

    motorcycle.front_wheel.velocity = Vector2Add(motorcycle.front_wheel.velocity, Vector2Scale(motorcycle.front_wheel.acceleration, TIMESTEP));
    motorcycle.front_wheel.position = Vector2Add(motorcycle.front_wheel.position, Vector2Scale(motorcycle.front_wheel.velocity, TIMESTEP));

    motorcycle.rear_wheel.velocity = Vector2Add(motorcycle.rear_wheel.velocity, Vector2Scale(motorcycle.rear_wheel.acceleration, TIMESTEP));
    motorcycle.rear_wheel.position = Vector2Add(motorcycle.rear_wheel.position, Vector2Scale(motorcycle.rear_wheel.velocity, TIMESTEP));

    motorcycle.front_spring.spring_start = motorcycle.body.position;
    motorcycle.front_spring.spring_end = motorcycle.front_wheel.position;

    motorcycle.rear_spring.spring_start = motorcycle.body.position;
    motorcycle.rear_spring.spring_end = motorcycle.rear_wheel.position;
}

void handleGroundCollision(Motorcycle& motorcycle) {
    if (motorcycle.front_wheel.position.y + motorcycle.front_wheel.radius > GROUND_Y) {
        motorcycle.front_wheel.position.y = GROUND_Y - motorcycle.front_wheel.radius;

        if (motorcycle.front_wheel.velocity.y > 0)
            motorcycle.front_wheel.velocity.y = 0;
    }

    if (motorcycle.rear_wheel.position.y + motorcycle.rear_wheel.radius > GROUND_Y) {
        motorcycle.rear_wheel.position.y = GROUND_Y - motorcycle.rear_wheel.radius;

        if (motorcycle.rear_wheel.velocity.y > 0)
            motorcycle.rear_wheel.velocity.y = 0;
    }
}


int main() {
    Motorcycle motorcycle;
    initializeMotorcycle(motorcycle);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project");
    SetTargetFPS(FPS);

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        // force accumulation
        Vector2 body_forces = Vector2Zero();
        Vector2 front_wheel_forces = Vector2Zero();
        Vector2 rear_wheel_forces = Vector2Zero();

        Vector2 body_gravity = Vector2Scale(GRAVITY, motorcycle.body.mass);
        body_forces = Vector2Add(body_forces, body_gravity);

        Vector2 front_gravity = Vector2Scale(GRAVITY, motorcycle.front_wheel.mass);
        front_wheel_forces = Vector2Add(front_wheel_forces, front_gravity);

        Vector2 rear_gravity = Vector2Scale(GRAVITY, motorcycle.rear_wheel.mass);
        rear_wheel_forces = Vector2Add(rear_wheel_forces, rear_gravity);

        // suspension forces
        Vector2 D = Vector2Subtract(motorcycle.front_spring.spring_end, motorcycle.front_spring.spring_start);
        Vector2 D_norm = Vector2Normalize(D);
        Vector2 front_spring_force = Vector2Scale(D_norm, -motorcycle.front_spring.k * (Vector2Length(D) - motorcycle.front_spring.rest_length));

        front_wheel_forces = Vector2Add(front_wheel_forces, front_spring_force);
        body_forces = Vector2Subtract(body_forces, front_spring_force);

        Vector2 D_rear = Vector2Subtract(motorcycle.rear_spring.spring_end, motorcycle.rear_spring.spring_start);
        Vector2 D_rear_norm = Vector2Normalize(D_rear);
        Vector2 rear_spring_force = Vector2Scale(D_rear_norm, -motorcycle.rear_spring.k * (Vector2Length(D_rear) - motorcycle.rear_spring.rest_length));

        rear_spring_force = Vector2Subtract(rear_spring_force, Vector2Scale(motorcycle.rear_wheel.velocity, motorcycle.rear_spring.b));
        rear_wheel_forces = Vector2Add(rear_wheel_forces, rear_spring_force);
        body_forces = Vector2Subtract(body_forces, rear_spring_force);

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
            motorcycle.body.position = {300, 300};
            motorcycle.body.velocity = Vector2Zero();

            motorcycle.front_wheel.position = {350, 380};
            motorcycle.front_wheel.velocity = Vector2Zero();

            motorcycle.rear_wheel.position = {250, 380};
            motorcycle.rear_wheel.velocity = Vector2Zero();
        }
        
        motorcycle.body.acceleration = Vector2Scale(body_forces, motorcycle.body.inverse_mass);
        motorcycle.front_wheel.acceleration = Vector2Scale(front_wheel_forces, motorcycle.front_wheel.inverse_mass);
        motorcycle.rear_wheel.acceleration = Vector2Scale(rear_wheel_forces, motorcycle.rear_wheel.inverse_mass);

        // physics loop
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {

            handlePhysicsMovement(motorcycle);

            handleGroundCollision(motorcycle);

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
