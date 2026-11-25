#pragma once

#include <raylib.h>
#include <cctype>
#include <string>

#include "scene_manager.hpp"
#include "scene_ids.hpp"
#include "game_session.hpp"
#include "leaderboard.hpp"

class LeaderboardScene : public Scene {
    static constexpr int MAX_NAME_LENGTH = 10;

    GameSession& session;
    Leaderboard leaderboard;

    Texture leaderboard_screen;
    Font ui_font;

    std::string name;
    bool entering_name = false;
    int highlight_index = -1;

    void handleNameInput() {
        // typed characters (letters, digits, underscore)
        int key = GetCharPressed();
        while (key > 0) {
            bool allowed = (key >= 'A' && key <= 'Z') ||
                           (key >= 'a' && key <= 'z') ||
                           (key >= '0' && key <= '9') ||
                           key == '_';

            if (allowed && name.size() < MAX_NAME_LENGTH)
                name.push_back((char)std::toupper(key));

            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !name.empty())
            name.pop_back();

        if (IsKeyPressed(KEY_ENTER) && !name.empty()) {
            highlight_index = leaderboard.add(session.last_result_level, name, session.last_result_time);
            entering_name = false;
        }
    }

public:
    explicit LeaderboardScene(GameSession& session)
        : session(session) {
    }

    void Begin() override {
        leaderboard_screen = ResourceManager::GetInstance()->GetTexture("screen_leaderboard.png");
        ui_font = ResourceManager::GetInstance()->GetFont("KNYuanmo-Regular.ttf");

        leaderboard.load();

        name.clear();
        highlight_index = -1;

        // throw away any keys typed before this screen opened
        while (GetCharPressed() > 0) {}

        // only ask for a name if the level was completed and the time makes the top list
        entering_name = session.last_outcome == LevelOutcome::COMPLETED &&
                        leaderboard.qualifies(session.last_result_level, session.last_result_time);
    }

    void End() override {
    }

    void Update() override {
        if (entering_name) {
            handleNameInput();
            return;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(SCENE_LEVEL_SELECT);
            }
        }
    }

    void Draw() override {
        bool completed = session.last_outcome == LevelOutcome::COMPLETED;
        
        if (completed)
            DrawTextureV(leaderboard_screen, {0, 0}, YELLOW);
        else
            DrawTextureV(leaderboard_screen, {0, 0}, RED);
        
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,150});

        DrawTextEx(ui_font, completed ? "LEVEL COMPLETE!" : "LEVEL FAILED", {350, 160}, 40, 2, completed ? GREEN : RED);    
        DrawTextEx(ui_font, TextFormat("LEVEL: %d", session.last_result_level), {420, 260}, 28, 2, YELLOW);


        DrawTextEx(ui_font, TextFormat("TIME: %.2f SECONDS", session.last_result_time), {365, 320}, 28, 2, YELLOW);
        DrawTextEx(ui_font, session.last_result_reason, {420, 390}, 24, 2, YELLOW);

        if (entering_name) {
            DrawTextEx(ui_font, "EYYY NEW TOP TIME! ENTER YOUR NAME:", {340, 440}, 22, 2, YELLOW);
            DrawRectangleLines(380, 480, 320, 44, YELLOW);

            const char* cursor = ((int)(GetTime() * 2.0) % 2 == 0) ? "_" : "";
            DrawTextEx(ui_font, TextFormat("%s%s", name.c_str(), cursor), {392, 488}, 28, 2, YELLOW);

            DrawTextEx(ui_font, "[ENTER] CONFIRM", {410, 650}, 22, 2, YELLOW);
            return;
        }

        // best times for this level
        DrawTextEx(ui_font, TextFormat("TOP SCORERS - LEVEL %d", session.last_result_level), {390, 430}, 22, 2, YELLOW);

        const auto& entries = leaderboard.getEntries(session.last_result_level);

        if (entries.empty())
            DrawTextEx(ui_font, "NO TIMES YET", {440, 470}, 20, 2, YELLOW);

        for (size_t i = 0; i < entries.size(); i++) {
            Color color = ((int)i == highlight_index) ? GREEN : YELLOW;
            int y = 470 + (int)i * 30;

            DrawTextEx(ui_font, TextFormat("%d. %s", (int)i + 1, entries[i].name.c_str()), {380, (float)y}, 22, 2, color);
            DrawTextEx(ui_font, TextFormat("%.2f s", entries[i].time), {620, (float)y}, 22, 2, color);
        }

        DrawTextEx(ui_font, "[ENTER] LEVEL SELECT", {375, 650}, 22, 2, YELLOW);
    }
};