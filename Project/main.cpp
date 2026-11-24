#include <raylib.h>
#include <raymath.h>
#include "segment_ground.hpp"
#include "track.hpp"
#include "course.hpp"

#include "motorcycle.hpp"
#include "motorcycle_controller.hpp"
#include "motorcycle_physics.hpp"
#include "motorcycle_collision.hpp"

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / 120.0f;

const float CAMERA_SPEED = 5.0f;


// global variables
Motorcycle motorcycle;
Course course;
Track ground;
Camera2D camera = {0};

void initializeCourse() {
    Course& c = course;
    Track& g = ground;

    c = createBeginnerCourse();

    g = c.track;
}

void initializeCamera() {
    Camera2D& c = camera;
    Motorcycle& m = motorcycle;

    c.offset = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 20.0f};

    c.target = m.position;

    c.rotation = 0.0f;
    c.zoom = 0.5f;
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
            motorcycleCenter.x = (trackStart + trackEnd) / 2.0f;
        else 
            motorcycleCenter.x = Clamp(motorcycleCenter.x, minCameraX, maxCameraX);
    }

    camera.target = Vector2Lerp(camera.target, motorcycleCenter, Clamp(CAMERA_SPEED * deltaTime, 0.0f, 1.0f));
}


int main() {
    motorcycle.initialize();
    initializeCourse();
    initializeCamera();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project");
    SetTargetFPS(FPS);

    int jump_count = 0;

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        MotorcycleInput input = readMotorcycleInput();

        if (input.reset)
            motorcycle.initialize();

        // physics loop
        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {

            updateMotorcyclePhysics(motorcycle, input, jump_count, TIMESTEP);

            // ground collision
            handleGroundCollision(motorcycle, ground);
            solveBodyFrameLimit(motorcycle);

            // ground friction
            applyGroundFriction(motorcycle.front_wheel, motorcycle.front_wheel_on_ground, ground, TIMESTEP);
            applyGroundFriction(motorcycle.rear_wheel, motorcycle.rear_wheel_on_ground, ground, TIMESTEP);

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

        BeginDrawing();
        ClearBackground(SKYBLUE);

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
    

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
