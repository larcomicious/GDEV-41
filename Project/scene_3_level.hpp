#pragma once

#include <raylib.h>
#include <raymath.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"

#include "segment_ground.hpp"
#include "track.hpp"
#include "course.hpp"

#include "motorcycle.hpp"
#include "motorcycle_controller.hpp"
#include "motorcycle_physics.hpp"
#include "motorcycle_collision.hpp"

class LevelScene : public Scene {
public:
    using CourseFactory = Course (*)(); // type alias for a function pointer 

private:
    static constexpr float TIMESTEP = 1.0f / 120.0f;
    static constexpr float CAMERA_SPEED = 5.0f;

    int level_number;
    CourseFactory course_factory; // stores function pointer to the course factory
    GameSession& session;

    Motorcycle motorcycle;
    Course course;
    Track ground;
    Camera2D camera = {0};

    int jump_count = 0;
    float accumulator = 0.0f;
    float elapsed_time = 0.0f;

    void initializeCourse() {
        course = course_factory();
        ground = course.track;
    }

    void initializeCamera() {
        camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f - 20.0f};
        camera.target = motorcycle.position;
        camera.rotation = 0.0f;
        camera.zoom = 0.5f;
    }

    void updateCamera(float deltaTime) {
        Vector2 motorcycleCenter = {(motorcycle.front_wheel.position.x + motorcycle.rear_wheel.position.x) / 2.0f, 
            (motorcycle.front_wheel.position.y + motorcycle.rear_wheel.position.y) / 2.0f};

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

    void completeLevel() {
        session.last_completed_level = level_number;
        session.last_completion_time = elapsed_time;

        SceneManager* manager = GetSceneManager();

        if (manager != nullptr) {
            manager->SwitchScene(SCENE_LEADERBOARD);
        }
    }

public:
    LevelScene(int level_number, CourseFactory course_factory, GameSession& session) : level_number(level_number), course_factory(course_factory), session(session) {}

    void Begin() override {
        motorcycle.initialize();

        initializeCourse();
        initializeCamera();

        jump_count = 0;
        accumulator = 0.0f;
        elapsed_time = 0.0f;
    }

    void End() override {
    }

    void Update() override {
        float delta_time = GetFrameTime();

        MotorcycleInput input = readMotorcycleInput();

        if (input.reset) {
            motorcycle.initialize();
            jump_count = 0;
            elapsed_time = 0.0f;
        }

        elapsed_time += delta_time;

        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {
            updateMotorcyclePhysics(motorcycle, input,jump_count, TIMESTEP);

            handleGroundCollision(motorcycle, ground);
            solveBodyFrameLimit(motorcycle);

            applyGroundFriction(motorcycle.front_wheel, motorcycle.front_wheel_on_ground, ground,TIMESTEP);
            applyGroundFriction(motorcycle.rear_wheel, motorcycle.rear_wheel_on_ground, ground, TIMESTEP);

            if (motorcycle.front_wheel_on_ground && motorcycle.rear_wheel_on_ground) 
                jump_count = 0;

            accumulator -= TIMESTEP;
        }
        
        // basic level complete check
        if (motorcycle.front_wheel.position.x >= course.finishX) {
            completeLevel();
            return;
        }

        updateCamera(delta_time);
    }

    void Draw() override {
        ClearBackground(SKYBLUE);
        BeginMode2D(camera);

        // terrain
        for (const SegmentGround& segment : ground.getSegments()) {
            DrawLineEx(
                segment.getStart(),
                segment.getEnd(),
                5.0f,
                DARKGREEN
            );
        }
        // start
        DrawLineEx(
            {course.startPosition.x, course.startPosition.y - 120.0f},
            course.startPosition,
            5.0f,
            GREEN
        );
        // finish
        DrawLineEx(
            {course.finishX, 380.0f},
            {course.finishX, 500.0f},
            5.0f,
            RED
        );

        motorcycle.draw();
        EndMode2D();

        // UI
        DrawText(TextFormat("LEVEL %d", level_number), 20, 20, 24, BLACK);
        DrawText("RIGHT/D: Accelerate    LEFT/A: Reverse    SHIFT: Nitro    R: Reset", 20, 55, 18, BLACK);
        
        // debug
        // DrawText(motorcycle.front_wheel_on_ground ? "FRONT: GROUNDED" : "FRONT: AIRBORNE", 20, 100, 18, motorcycle.front_wheel_on_ground ? GREEN : RED);
        // DrawText(motorcycle.rear_wheel_on_ground ? "REAR: GROUNDED" : "REAR: AIRBORNE", 20, 125, 18, motorcycle.rear_wheel_on_ground ? GREEN : RED);
    }
};
