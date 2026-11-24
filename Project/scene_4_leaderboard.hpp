
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
        DrawText("LEVEL COMPLETE!", 350, 160, 40, BLACK);

        DrawText(TextFormat("LEVEL: %d", session.last_completed_level), 420, 290, 28, DARKGRAY);
        DrawText(TextFormat("TIME: %.2f SECONDS", session.last_completion_time), 365, 350, 28, DARKGRAY);
        DrawText("PRESS ENTER TO RETURN TO LEVEL SELECT", 270, 500, 22, BLACK);
    }
};
