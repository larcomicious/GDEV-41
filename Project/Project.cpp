#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include "segment_ground.h"
#include "track.h"

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / 120.0f;

const Vector2 GRAVITY = {0, 1000};

const float ENGINE_FORCE = 8000.0f;
const float JUMP_FORCE = 250000.0f;
const float TURBO_MULTIPLIER = 3.0f;

const float SPRING_REST_LENGTH = 100.0f;
const float SPRING_K = 3000.0f;
const float SPRING_B = 80.0f;
const float WHEEL_DISTANCE = 100.0f;

const float GROUND_Y = 600.0f;
// FlatGround ground(GROUND_Y);
// SegmentGround ground({100.0f, 700.0f}, {900.0f, 550.0f});
const float GROUND_FRICTION = 15.0f;

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

    bool front_wheel_on_ground;
    bool rear_wheel_on_ground;

    void Draw() {
        DrawLineEx(body.position, front_wheel.position, 5.0f, GRAY);
        DrawLineEx(body.position, rear_wheel.position, 5.0f, GRAY);
        DrawLineEx(front_wheel.position, rear_wheel.position, 8.0f, DARKGRAY);
        
        DrawCircleV(front_wheel.position, front_wheel.radius, front_wheel.color);
        DrawCircleV(rear_wheel.position, rear_wheel.radius, rear_wheel.color);
        DrawCircleV(body.position, body.radius, body.color);

        // debug
        DrawText(front_wheel_on_ground ? "FRONT: GROUNDED" : "FRONT: AIRBORNE", 20, 80, 18, front_wheel_on_ground ? GREEN : RED);
        DrawText(rear_wheel_on_ground ? "REAR: GROUNDED" : "REAR: AIRBORNE", 20, 105, 18, rear_wheel_on_ground ? GREEN : RED);
    }
};

// global variables
Motorcycle motorcycle;
Track ground;

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

    m.front_wheel_on_ground = false;
    m.rear_wheel_on_ground = false;

    // m.wheel_to_wheel_suspension.spring_start = m.rear_wheel.position;
    // m.wheel_to_wheel_suspension.spring_end = m.front_wheel.position;
    // m.wheel_to_wheel_suspension.rest_length = WHEEL_DISTANCE;
    // m.wheel_to_wheel_suspension.k = WHEEL_TO_WHEEL_STIFFNESS;
    // m.wheel_to_wheel_suspension.b = WHEEL_TO_WHEEL_DAMPNESS;

}

void initializeTrack() {
    Track& g = ground;
    g.addSegment({100.0f, 500.0f}, {300.0f, 500.0f});
    g.addSegment({300.0f, 500.0f}, {450.0f, 400.0f});
    g.addSegment({450.0f, 400.0f}, {600.0f, 350.0f});
    g.addSegment({600.0f, 350.0f}, {800.0f, 450.0f});
    g.addSegment({800.0f, 450.0f}, {1000.0f, 450.0f});
}

void handlePhysicsMovement() {
    Motorcycle& m = motorcycle;
    m.body.velocity = Vector2Add(m.body.velocity, Vector2Scale(m.body.acceleration, TIMESTEP));
    m.body.position = Vector2Add(m.body.position, Vector2Scale(m.body.velocity, TIMESTEP));

    m.front_wheel.velocity = Vector2Add(m.front_wheel.velocity, Vector2Scale(m.front_wheel.acceleration, TIMESTEP));
    m.front_wheel.position = Vector2Add(m.front_wheel.position, Vector2Scale(m.front_wheel.velocity, TIMESTEP));

    m.rear_wheel.velocity = Vector2Add(m.rear_wheel.velocity, Vector2Scale(m.rear_wheel.acceleration, TIMESTEP));
    m.rear_wheel.position = Vector2Add(m.rear_wheel.position, Vector2Scale(m.rear_wheel.velocity, TIMESTEP));

    // m.wheel_to_wheel_suspension.spring_start = m.rear_wheel.position;
    // m.wheel_to_wheel_suspension.spring_end = m.front_wheel.position;
    // m.front_spring.spring_start = m.body.position;
    // m.front_spring.spring_end = m.front_wheel.position;

    // m.rear_spring.spring_start = m.body.position;
    // m.rear_spring.spring_end = m.rear_wheel.position;
}

void solveDistanceConstraint(Ball& a, Ball& b, float rest_length) {

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

void resolveWheelGroundCollision(Ball& wheel, bool& on_ground, const Ground& ground) {
    const float CONTACT_TOLERANCE = 0.01f;

    GroundSample sample = ground.sample(wheel.position);
    if (!sample.valid) {
        on_ground = false;
        return;
    }

    Vector2 normal = sample.normal;

    Vector2 to_wheel = Vector2Subtract(wheel.position, sample.point);

    float distance = Vector2DotProduct(to_wheel, normal);

    on_ground = distance <= wheel.radius + CONTACT_TOLERANCE;

     // resolve clipping
    if (distance < wheel.radius) {
        float penetration = wheel.radius - distance;

        wheel.position = Vector2Add(wheel.position, Vector2Scale(normal, penetration));

        // remove velocity directed into the ground
        float normal_velocity = Vector2DotProduct(wheel.velocity, normal);

        if (normal_velocity < 0.0f)
            wheel.velocity = Vector2Subtract(wheel.velocity, Vector2Scale(normal, normal_velocity));
    }
}

void handleGroundCollision(const Ground& ground) {
    Motorcycle& m = motorcycle;

    resolveWheelGroundCollision(m.front_wheel, m.front_wheel_on_ground, ground);
    resolveWheelGroundCollision(m.rear_wheel, m.rear_wheel_on_ground, ground);
}

void applyGroundFriction(Ball& wheel, bool on_ground, const Ground& ground) {

    if (!on_ground)
        return;

    GroundSample sample = ground.sample(wheel.position);

    Vector2 tangent = sample.tangent;

    // project velocity onto the ground tangent
    float tangent_velocity = Vector2DotProduct(wheel.velocity, tangent);

    Vector2 tangential_component = Vector2Scale(tangent, tangent_velocity);

    // preserve velocity perpendicular to the ground
    Vector2 normal_component = Vector2Subtract(wheel.velocity, tangential_component);

    // Apply friction to tangential velocity.
    float friction_factor = 1.0f - GROUND_FRICTION * wheel.inverse_mass * TIMESTEP;

    if (friction_factor < 0.0f)
        friction_factor = 0.0f;

    tangential_component = Vector2Scale(tangential_component, friction_factor);

    wheel.velocity = Vector2Add(tangential_component, normal_component);
}

Vector2 calculateSpringForce(Vector2 spring_start, Vector2 spring_end, float k, float rest_length) {
    Vector2 d = Vector2Subtract(spring_end, spring_start);
    Vector2 d_norm = Vector2Normalize(d);

    return Vector2Scale(d_norm, - k * (Vector2Length(d) - rest_length));
}

void applySpringDamper(const Ball& a, const Ball& b, const Spring& spring, Vector2& force_a, Vector2& force_b) {
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


int main() {
    initializeMotorcycle();
    initializeTrack();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project");
    SetTargetFPS(FPS);

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_R))
        {
            initializeMotorcycle();
        }

        // physics loop
        accumulator += delta_time;

        while (accumulator >= TIMESTEP) {

            Vector2 body_forces = Vector2Scale(GRAVITY, motorcycle.body.mass);
            Vector2 front_wheel_forces = Vector2Scale(GRAVITY, motorcycle.front_wheel.mass);
            Vector2 rear_wheel_forces = Vector2Scale(GRAVITY, motorcycle.rear_wheel.mass);

            // front suspension
            applySpringDamper(motorcycle.body, motorcycle.front_wheel, motorcycle.front_spring, body_forces, front_wheel_forces);
            // fear suspension
            applySpringDamper(motorcycle.body, motorcycle.rear_wheel, motorcycle.rear_spring, body_forces, rear_wheel_forces);

            if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)))
                rear_wheel_forces.x += IsKeyDown(KEY_LEFT_SHIFT) ? ENGINE_FORCE * TURBO_MULTIPLIER : ENGINE_FORCE;

            if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)))
                rear_wheel_forces.x -= IsKeyDown(KEY_LEFT_SHIFT) ? ENGINE_FORCE * TURBO_MULTIPLIER : ENGINE_FORCE;
            
            if (IsKeyPressed(KEY_SPACE)) {
                body_forces.y -= JUMP_FORCE;
                front_wheel_forces.y -= JUMP_FORCE;
                rear_wheel_forces.y -= JUMP_FORCE;
            }

            motorcycle.body.acceleration = Vector2Scale(body_forces, motorcycle.body.inverse_mass);
            motorcycle.front_wheel.acceleration = Vector2Scale(front_wheel_forces, motorcycle.front_wheel.inverse_mass);
            motorcycle.rear_wheel.acceleration = Vector2Scale(rear_wheel_forces, motorcycle.rear_wheel.inverse_mass);

            handlePhysicsMovement();
            solveDistanceConstraint(motorcycle.rear_wheel, motorcycle.front_wheel, WHEEL_DISTANCE);
            handleGroundCollision(ground);
            
            applyGroundFriction(motorcycle.front_wheel, motorcycle.front_wheel_on_ground, ground);
            applyGroundFriction(motorcycle.rear_wheel, motorcycle.rear_wheel_on_ground, ground);

            accumulator -= TIMESTEP;

            GroundSample front_sample =
                ground.sample(motorcycle.front_wheel.position);

            GroundSample rear_sample =
                ground.sample(motorcycle.rear_wheel.position);

            if (motorcycle.front_wheel_on_ground) {
                TraceLog(
                    LOG_INFO,
                    "Front Wheel Segment: %d",
                    front_sample.segment_index
                );
            }

            if (motorcycle.rear_wheel_on_ground) {
                TraceLog(
                    LOG_INFO,
                    "Rear Wheel Segment: %d",
                    rear_sample.segment_index
                );
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        
        motorcycle.Draw();
        for (const SegmentGround& segment : ground.getSegments())
            DrawLineEx(segment.getStart(), segment.getEnd(), 5.0f, DARKGREEN);

        DrawText("RIGHT: Accelerate    LEFT: Reverse    R: Reset", 20, 20, 20, BLACK);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
