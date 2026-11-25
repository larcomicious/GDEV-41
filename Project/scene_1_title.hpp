#ifndef TITLE_SCENE
#define TITLE_SCENE

#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"

class TitleScene : public Scene {
    Texture title_screen;

public:
    void Begin() override {
        title_screen = ResourceManager::GetInstance()->GetTexture("screen_title.png");
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
        // DrawTexturePro(title_screen, {0, 0, 256, 256}, {300, 100, 200, 200}, {0, 0}, 0.0f, WHITE);

        DrawTextureV(title_screen, {0, 0}, WHITE);

        // DrawText("MOTORCYCLE GAME", 340, 380, 40, BLACK);
        // DrawText("PRESS ENTER TO START", 390, 460, 24, DARKGRAY);
    }
};

#endif