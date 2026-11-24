#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_1_title.hpp"
#include "scene_2_game.hpp"

int main() {
    InitWindow(1080, 720, "Project - Gimena Tan");
    SetTargetFPS(60);

    SceneManager scene_manager;

    TitleScene title_scene;
    title_scene.SetSceneManager(&scene_manager);

    GameScene game_scene;
    game_scene.SetSceneManager(&scene_manager);

    scene_manager.RegisterScene(&title_scene, 0);
    scene_manager.RegisterScene(&game_scene, 1);

    scene_manager.SwitchScene(0);

    while(!WindowShouldClose()) {
        Scene* active_scene = scene_manager.GetActiveScene();

        BeginDrawing();
        ClearBackground(WHITE);

        if (active_scene != nullptr) {
            active_scene->Update();
            active_scene->Draw();
        }

        EndDrawing();
    }

    Scene* active_scene = scene_manager.GetActiveScene();
    if (active_scene != nullptr) {
        active_scene->End();
    }

    ResourceManager::GetInstance()->UnloadAllTextures();

    CloseWindow();
    return 0;
}