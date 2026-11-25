
#include <raylib.h>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"

#include "scene_1_title.hpp"
#include "scene_2_level_select.hpp"
#include "scene_3_level.hpp"
#include "scene_4_leaderboard.hpp"

int main() {
    InitWindow(1080, 720, "Project - Gimena Tan");
    SetTargetFPS(60);

    SceneManager scene_manager;

    GameSession session; // shared session

    // configs
    // const LevelConfig level1_config = {1, &createLevelOneCourse, 5.0f, true, 1500.0f};
    const LevelConfig level1_config = {1, &createLevelOneCourse, 0.0f, true, 1500.0f};
    const LevelConfig level2_config = {2, &createLevelTwoCourse, 0.0f, true, 5000.0f};
    const LevelConfig level3_config = {3, &createLevelThreeCourse, 0.0f, true, 8000.0f};

    TitleScene title_scene;
    LevelSelectScene level_select_scene(session);

    LevelScene level1_scene(level1_config, session);
    LevelScene level2_scene(level2_config, session);
    LevelScene level3_scene(level3_config, session);

    LeaderboardScene leaderboard_scene(session);

    title_scene.SetSceneManager(&scene_manager);
    level_select_scene.SetSceneManager(&scene_manager);
    level1_scene.SetSceneManager(&scene_manager);
    level2_scene.SetSceneManager(&scene_manager);
    level3_scene.SetSceneManager(&scene_manager);
    leaderboard_scene.SetSceneManager(&scene_manager);

    scene_manager.RegisterScene(&title_scene, SCENE_TITLE);
    scene_manager.RegisterScene(&level_select_scene, SCENE_LEVEL_SELECT);
    scene_manager.RegisterScene(&level1_scene, SCENE_LEVEL_1);
    scene_manager.RegisterScene(&level2_scene, SCENE_LEVEL_2);
    scene_manager.RegisterScene(&level3_scene, SCENE_LEVEL_3);
    scene_manager.RegisterScene(&leaderboard_scene, SCENE_LEADERBOARD);

    scene_manager.SwitchScene(SCENE_TITLE);

    while (!WindowShouldClose()) {
        Scene* active_scene = scene_manager.GetActiveScene();

        BeginDrawing();
        ClearBackground(WHITE);

        if (active_scene != nullptr) {
            active_scene->Update();
            active_scene->Draw();

            // if (scene_manager.GetActiveScene() == active_scene) {
            //     active_scene->Draw();
            // }
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
