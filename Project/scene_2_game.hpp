#ifndef GAME_SCENE
#define GAME_SCENE

#include <raylib.h>
#include <raymath.h>

#include "scene_manager.hpp"
#include "segment_ground.hpp"
#include "track.hpp"
#include "course.hpp"

#include "motorcycle.hpp"
#include "motorcycle_controller.hpp"
#include "motorcycle_physics.hpp" 

class GameScene : public Scene {
    static constexpr int WINDOW_WIDTH = 1080;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr float TIMESTEP = 1 / 120.0f;

    static constexpr float GROUND_Y = 600.0f;
    // FlatGround ground(GROUND_Y);
    // SegmentGround ground({100.0f, 700.0f}, {900.0f, 550.0f});
    static constexpr float GROUND_FRICTION = 15.0f;

    static constexpr float CAMERA_SPEED = 5.0f;

    Motorcycle motorcycle;
    Course course;
    Track ground; // rename
    Camera2D camera = {0};

    int jump_count = 0;
    float accumulator = 0.0f;

    void initializeCourse() {
        Course& c = course;
        Track& g = ground;

        c = createBeginnerCourse();

        g = c.track;
    }

    void initializeCamera() {
        Camera2D& c = camera;
        Motorcycle& m = motorcycle;

        c.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f - 20.0f};

        c.target = m.position;

        c.rotation = 0.0f;
        c.zoom = 0.5f;
    }

    void handlePhysicsMovement() {
        Motorcycle& m = motorcycle;
        m.body.velocity = Vector2Add(m.body.velocity, Vector2Scale(m.body.acceleration, TIMESTEP));
        m.body.position = Vector2Add(m.body.position, Vector2Scale(m.body.velocity, TIMESTEP));

        m.front_wheel.velocity = Vector2Add(m.front_wheel.velocity, Vector2Scale(m.front_wheel.acceleration, TIMESTEP));
        m.front_wheel.position = Vector2Add(m.front_wheel.position, Vector2Scale(m.front_wheel.velocity, TIMESTEP));

        m.rear_wheel.velocity = Vector2Add(m.rear_wheel.velocity, Vector2Scale(m.rear_wheel.acceleration, TIMESTEP));

        m.rear_wheel.position = Vector2Add(m.rear_wheel.position, Vector2Scale(m.rear_wheel.velocity, TIMESTEP));

        m.updateOrientation();
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
        resolveWheelGroundCollision(m.body, m.body_on_ground, ground);
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

    void updateCamera(float deltaTime) {
        Vector2 motorcycleCenter = {(motorcycle.front_wheel.position.x + motorcycle.rear_wheel.position.x) / 2.0f, (motorcycle.front_wheel.position.y + motorcycle.rear_wheel.position.y) / 2.0f};


        const auto& segments = ground.getSegments();

        if (!segments.empty()) {
            float trackStart = segments.front().getStart().x;
            float trackEnd = segments.back().getEnd().x;

            float halfViewWidth = GetScreenWidth() / (2.0f * camera.zoom);

            float minCameraX = trackStart + halfViewWidth;
            float maxCameraX = trackEnd - halfViewWidth;

            if (minCameraX > maxCameraX) 
                camera.target.x = (trackStart + trackEnd) / 2.0f;

            else 
                motorcycleCenter.x = Clamp(motorcycleCenter.x, minCameraX, maxCameraX);
        }

        camera.target = Vector2Lerp(camera.target, motorcycleCenter, Clamp(CAMERA_SPEED * deltaTime, 0.0f, 1.0f));
    }


public:
    void Begin() override {
        motorcycle.initialize();
        initializeCourse();
        initializeCamera();


        jump_count = 0;
        accumulator = 0;
    }

    void End() override {}

    void Update() override {
        float delta_time = GetFrameTime();
        
        MotorcycleInput input = readMotorcycleInput();

        if (input.reset)
            motorcycle.initialize();

        // physics loop
        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {

            updateMotorcyclePhysics(motorcycle, input, jump_count, TIMESTEP);

            // ground collision
            handleGroundCollision(ground);

            // ground friction
            applyGroundFriction(motorcycle.front_wheel, motorcycle.front_wheel_on_ground, ground);
            applyGroundFriction(motorcycle.rear_wheel, motorcycle.rear_wheel_on_ground, ground);

            if (motorcycle.front_wheel_on_ground && motorcycle.rear_wheel_on_ground) 
                jump_count = 0;


            accumulator -= TIMESTEP;
        
            // GroundSample front_sample =
            //     ground.sample(motorcycle.front_wheel.position);

            // GroundSample rear_sample =
            //     ground.sample(motorcycle.rear_wheel.position);

            // if (motorcycle.front_wheel_on_ground) {
            //     TraceLog(
            //         LOG_INFO,
            //         "Front Wheel Segment: %d",
            //         front_sample.segment_index
            //     );
            // }

            // if (motorcycle.rear_wheel_on_ground) {
            //     TraceLog(
            //         LOG_INFO,
            //         "Rear Wheel Segment: %d",
            //         rear_sample.segment_index
            //     );
            // }
        }


        // update camera
        updateCamera(delta_time);
    }

    void Draw() override {
        BeginMode2D(camera); // for raylib camera
        
        // draw terrain
        for (const SegmentGround& segment : ground.getSegments()) {
            DrawLineEx(segment.getStart(), segment.getEnd(), 5.0f, DARKGREEN);
        }

        // finish 
        DrawLineEx({course.startPosition.x, course.startPosition.y - 120.0f}, course.startPosition, 5.0f, GREEN);
        // start
        DrawLineEx({course.finishX, 380.0f}, {course.finishX, 500.0f}, 5.0f, RED);
                
        // draw motorcycle
        motorcycle.draw();

        EndMode2D(); 
        // now draw ui

        DrawText("RIGHT: Accelerate    LEFT: Reverse    R: Reset", 20, 20, 20, BLACK);
        // debug
        DrawText(motorcycle.front_wheel_on_ground ? "FRONT: GROUNDED" : "FRONT: AIRBORNE", 20, 80, 18, motorcycle.front_wheel_on_ground ? GREEN : RED);
        DrawText(motorcycle.rear_wheel_on_ground ? "REAR: GROUNDED" : "REAR: AIRBORNE", 20, 105, 18, motorcycle.rear_wheel_on_ground ? GREEN : RED);
    }
};

#endif