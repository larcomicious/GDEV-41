
#pragma once

#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"

class LeaderboardScene : public Scene {
    GameSession& session;

public:
    explicit LeaderboardScene(GameSession& session)
        : session(session) {
    }

    void Begin() override {
    }

    void End() override {
    }

    void Update() override {
        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(SCENE_LEVEL_SELECT);
            }
        }
    }

    void Draw() override {
        bool completed = session.last_outcome == LevelOutcome::COMPLETED;
        DrawText(completed ? "LEVEL COMPLETE!" : "LEVEL FAILED", 350, 160, 40, completed ? GREEN : RED);
        DrawText(TextFormat("LEVEL: %d", session.last_result_level), 420, 260, 28, DARKGRAY);

        DrawText(TextFormat("TIME: %.2f SECONDS", session.last_result_time), 365, 320, 28, DARKGRAY);
        DrawText(session.last_result_reason, 420, 390, 24, DARKGRAY);

        DrawText("[ENTER] LEVEL SELECT", 375, 500, 22, BLACK);
    }
};
