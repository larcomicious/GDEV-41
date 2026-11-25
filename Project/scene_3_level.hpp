#pragma once

#include <cmath>

#include <raylib.h>
#include <raymath.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"

#include "level_config.hpp"

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
    enum class LevelState {
        READY,
        PLAYING,
        PAUSED,
        COMPLETED,
        FAILED
    }; 

    static constexpr float TIMESTEP = 1.0f / 120.0f;
    static constexpr float CAMERA_SPEED = 5.0f;

    static constexpr float START_COUNTDOWN = 3.0f;

    const LevelConfig config;
    GameSession& session;
    
    Motorcycle motorcycle;
    Course course;
    Track ground;
    Camera2D camera = {0};
    Texture background_far;
    Texture background_mid;
    Texture background_near;
    static constexpr float PARALLAX_FAR = 0.1f;
    static constexpr float PARALLAX_MID = 0.25f;
    static constexpr float PARALLAX_NEAR = 0.5f;

    motorcycle_collision::MotorcycleGroundContacts ground_contacts{};

    Font ui_font;
    Sound engine_sound;
    float engine_volume = 0.0f;

    static constexpr float ENGINE_MAX_VOLUME = 0.2f;
    static constexpr float ENGINE_FADE_IN_SPEED = 1.0f;
    static constexpr float ENGINE_FADE_OUT_SPEED = 0.5f;
    
    LevelState level_state = LevelState::READY;
    
    int jump_count = 0;
    float accumulator = 0.0f;
    float elapsed_time = 0.0f;
    float countdown_timer = START_COUNTDOWN;

    void initializeCourse() {
        course = config.course_factory();
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

    void drawBackgroundLayer(const Texture& texture, float parallax) const {
        float screen_w = (float)GetScreenWidth();
        float screen_h = (float)GetScreenHeight();

        float scale = screen_h / texture.height;
        float tile_width = std::ceil(texture.width * scale);

        float scroll = -camera.target.x * camera.zoom * parallax;

        float start_x = std::fmod(scroll, tile_width);
        if (start_x > 0.0f)
            start_x -= tile_width;

        Rectangle source = {0.0f, 0.0f, (float)texture.width, (float)texture.height};

        for (float x = std::floor(start_x); x < screen_w; x += tile_width) {
            Rectangle dest = {x, 0.0f, tile_width, screen_h};
            DrawTexturePro(texture, source, dest, {0.0f, 0.0f}, 0.0f, WHITE);
        }
    }

    void checkLevelComplete() {
        if (motorcycle.front_wheel.position.x >= course.finishX) {
            finishLevel(LevelOutcome::COMPLETED, "Level completed eyyy");
            return;
        }

        if (config.use_fall_limit) {
            if (motorcycle.body.position.y >= config.fall_limit_y || motorcycle.front_wheel.position.y >= config.fall_limit_y || motorcycle.rear_wheel.position.y >= config.fall_limit_y) {
                finishLevel(LevelOutcome::FAILED, "try NOT to fall bro");
                return;
            }
        }

        if (config.time_limit_seconds > 0.0f && elapsed_time >= config.time_limit_seconds) {
            finishLevel(LevelOutcome::FAILED, "u too slow bro");
            return;
        }
    }

    void finishLevel(LevelOutcome outcome, const char* reason) {
        if (level_state == LevelState::COMPLETED || level_state == LevelState::FAILED) 
            return;

        level_state = (outcome == LevelOutcome::COMPLETED) ? LevelState::COMPLETED : LevelState::FAILED;

        session.last_result_level = config.level_number;
        session.last_result_time = elapsed_time;
        session.last_outcome = outcome;
        session.last_result_reason = reason;

        if (outcome == LevelOutcome::COMPLETED) { // add outcome when player winz
            session.last_completed_level = config.level_number;
            session.last_completion_time = elapsed_time;
        }

        SceneManager* manager = GetSceneManager();

        if (manager != nullptr) {
            manager->SwitchScene(SCENE_LEADERBOARD);
        }
    }

    void resetLevel() {
        motorcycle.initialize();

        jump_count = 0;
        accumulator = 0.0f;
        elapsed_time = 0.0f;

        countdown_timer = START_COUNTDOWN;
        level_state = LevelState::READY;

        initializeCamera();
    }

    void updateEngineSound(bool accelerating, float delta_time) {
        float target = accelerating ? ENGINE_MAX_VOLUME : 0.0f;
        float speed = accelerating ? ENGINE_FADE_IN_SPEED : ENGINE_FADE_OUT_SPEED;

        if (engine_volume < target)
            engine_volume = Clamp(engine_volume + speed * delta_time, 0.0f, target);
        else
            engine_volume = Clamp(engine_volume - speed * delta_time, target, ENGINE_MAX_VOLUME);

        if (accelerating && !IsSoundPlaying(engine_sound))
            PlaySound(engine_sound);

        if (engine_volume <= 0.0f && IsSoundPlaying(engine_sound))
            StopSound(engine_sound);

        SetSoundVolume(engine_sound, engine_volume);
    }

public:
    LevelScene(const LevelConfig& config, GameSession& session) : config(config), session(session) {}

    void Begin() override {
        engine_sound = ResourceManager::GetInstance()->GetSound("Engine_EpidemicSound.wav");
        engine_volume = 0.0f;
        SetSoundVolume(engine_sound, 0.0f);
        

        ui_font = ResourceManager::GetInstance()->GetFont("KNYuanmo-Regular.ttf");

        background_far = ResourceManager::GetInstance()->GetTexture("background_far.png");
        background_mid = ResourceManager::GetInstance()->GetTexture("background_mid.png");
        background_near = ResourceManager::GetInstance()->GetTexture("background_near.png");

        initializeCourse();
        resetLevel();

    }

    void End() override {
    }

    void Update() override {
        float delta_time = GetFrameTime();

        MotorcycleInput input = readMotorcycleInput();

        updateEngineSound(input.throttle > 0.0f && level_state == LevelState::PLAYING, delta_time);

        if (input.reset) {
            resetLevel();
            return;
        }

        // countdown
        if (level_state == LevelState::READY) {
            countdown_timer -= delta_time;

            if (countdown_timer <= 0.0f) {
                countdown_timer = 0.0f;
                level_state = LevelState::PLAYING;
            }
            return;
        }

        // if paused
        if (level_state == LevelState::PAUSED) {
            if (IsKeyPressed(KEY_P)) 
                level_state = LevelState::READY;
            return;
        }

        // if not playing, dont do anything lol
        if (level_state != LevelState::PLAYING)
            return;
        
        // pausing
        if (IsKeyPressed(KEY_P)) {
            countdown_timer = START_COUNTDOWN;
            level_state = LevelState::PAUSED;
        }

        elapsed_time += delta_time;

        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {
           updateMotorcyclePhysics(
    motorcycle,
    input,
    jump_count,
    ground_contacts.rear_wheel,
    TIMESTEP
);

solveChassisAlignment(motorcycle);
solveBodyFrameLimit(motorcycle);

handleGroundCollision(
    motorcycle,
    ground,
    ground_contacts
);

            // friction
            applyGroundFriction(motorcycle.front_wheel, motorcycle.front_wheel_on_ground, ground_contacts.front_wheel, TIMESTEP);
            applyGroundFriction(motorcycle.rear_wheel, motorcycle.rear_wheel_on_ground, ground_contacts.rear_wheel, TIMESTEP);

            if (motorcycle.front_wheel_on_ground && motorcycle.rear_wheel_on_ground) 
                jump_count = 0;

            accumulator -= TIMESTEP;
        }

        checkLevelComplete();
    
        updateCamera(delta_time);
    }

    void Draw() override {
        ClearBackground(BLACK);

        drawBackgroundLayer(background_far, PARALLAX_FAR);
        drawBackgroundLayer(background_mid, PARALLAX_MID);
        drawBackgroundLayer(background_near, PARALLAX_NEAR);

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,150});

        BeginMode2D(camera);

        // terrain
        for (const SegmentGround& segment : ground.getSegments()) {
            DrawLineEx(segment.getStart(), segment.getEnd(), 10.0f, {201, 151, 22, 255});
        }
        // start
        DrawLineEx({course.startPosition.x, course.startPosition.y - 120.0f}, course.startPosition, 10.0f, GREEN);
        // finish
        DrawLineEx({course.finishX, 380.0f}, {course.finishX, 500.0f}, 10.0f, RED);

        motorcycle.draw();
        EndMode2D();

        // UI
        DrawTextEx(ui_font, TextFormat("LEVEL %d", config.level_number), {20, 20}, 24, 2, YELLOW);
        DrawTextEx(ui_font, TextFormat("TIME: %.2f", elapsed_time), {20, 55}, 22, 2, YELLOW);
        if (config.time_limit_seconds > 0.0f) 
            DrawTextEx(ui_font, TextFormat("/ %.2f", config.time_limit_seconds), {150, 55}, 22, 2, YELLOW);

        DrawTextEx(ui_font, "RIGHT/D: Accelerate    LEFT/A: Reverse    SHIFT: Nitro", {20, 90}, 18, 2, YELLOW);
        DrawTextEx(ui_font, "P: Pause    R: Restart", {20, 115}, 18, 2, YELLOW);

        // debug
        // DrawText(motorcycle.front_wheel_on_ground ? "FRONT: GROUNDED" : "FRONT: AIRBORNE", 20, 100, 18, motorcycle.front_wheel_on_ground ? GREEN : RED);
        // DrawText(motorcycle.rear_wheel_on_ground ? "REAR: GROUNDED" : "REAR: AIRBORNE", 20, 125, 18, motorcycle.rear_wheel_on_ground ? GREEN : RED);

        // state overlays
        if (level_state == LevelState::READY) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.45f));
            DrawTextEx(ui_font, TextFormat("STARTING IN: %d", static_cast<int>(countdown_timer + 0.99f)), {(float)GetScreenWidth() / 2 - 180, (float)GetScreenHeight() / 2 - 30}, 48, 2, WHITE);
        }

        if (level_state == LevelState::PAUSED) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.55f) );
            DrawTextEx(ui_font, "PAUSED", {(float)GetScreenWidth() / 2 - 100, (float)GetScreenHeight() / 2 - 50}, 48, 2, WHITE);
            DrawTextEx(ui_font, "P: RESUME    R: RESTART", {(float)GetScreenWidth() / 2 - 150, (float)GetScreenHeight() / 2 + 20}, 22, 2, WHITE);
        }
    }
};
