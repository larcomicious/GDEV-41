
#pragma once

#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"

class LevelSelectScene : public Scene {
    GameSession& session;
    Texture level_select_screen;

    void selectLevel(int level, int scene_id) {
        session.selected_level = level;

        SceneManager* manager = GetSceneManager();

        if (manager != nullptr) {
            manager->SwitchScene(scene_id);
        }
    }

public:
    explicit LevelSelectScene(GameSession& session) : session(session) {}

    void Begin() override {
        level_select_screen = ResourceManager::GetInstance()->GetTexture("screen_level_select.png");
    }

    void End() override {
    }

    void Update() override {
        if (IsKeyPressed(KEY_ONE)) {
            selectLevel(1, SCENE_LEVEL_1);
        }
        else if (IsKeyPressed(KEY_TWO)) {
            selectLevel(2, SCENE_LEVEL_2);
        }
        else if (IsKeyPressed(KEY_THREE)) {
            selectLevel(3, SCENE_LEVEL_3);
        }
    }

    void Draw() override {
        DrawTextureV(level_select_screen, {0, 0}, WHITE);

        // DrawText("SELECT LEVEL", 390, 150, 40, BLACK);

        // DrawText("[1] LEVEL 1", 420, 280, 30, DARKGRAY);
        // DrawText("[2] LEVEL 2", 420, 350, 30, DARKGRAY);
        // DrawText("[3] LEVEL 3", 420, 420, 30, DARKGRAY);

        // DrawText(
        //     "Press 1, 2, or 3 to select a level :3",
        //     335, 540, 22, BLACK
        // );

        // DrawText(
        //     "[ESC] Quit",
        //     480, 600, 18, GRAY
        // );
    }
};
