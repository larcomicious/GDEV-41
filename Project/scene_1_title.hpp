#ifndef TITLE_SCENE
#define TITLE_SCENE

#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"

class TitleScene : public Scene {
    Texture raylib_logo;

public:
    void Begin() override {
        raylib_logo = ResourceManager::GetInstance()->GetTexture("Raylib_logo.png");
    }

    void End() override {}

    void Update() override {
        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(SCENE_LEVEL_SELECT);
            }
        }
    }

    void Draw() override {
        DrawTexturePro(raylib_logo, {0, 0, 256, 256}, {300, 100, 200, 200}, {0, 0}, 0.0f, WHITE);

        DrawText("MOTORCYCLE GAME", 340, 380, 40, BLACK);
        DrawText("PRESS ENTER TO START", 390, 460, 24, DARKGRAY);
    }
};

#endif