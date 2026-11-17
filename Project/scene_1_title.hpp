#ifndef TITLE_SCENE
#define TITLE_SCENE

#include <raylib.h>

#include "scene_manager.hpp"

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
                GetSceneManager()->SwitchScene(1);
            }
        }
    }

    void Draw() override {
        DrawTexturePro(raylib_logo, {0, 0, 256, 256}, {300, 100, 200, 200}, {0, 0}, 0.0f, WHITE);
        DrawText("Press ENTER", 300, 325, 30, BLACK);
    }
};

#endif