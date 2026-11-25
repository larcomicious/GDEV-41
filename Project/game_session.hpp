#pragma once

enum class LevelOutcome {
    NONE,
    COMPLETED,
    FAILED
};

struct GameSession {
    int selected_level = 1;

    int last_completed_level = 0;
    int last_result_level = 0;
    float last_result_time = 0.0f;

    LevelOutcome last_outcome = LevelOutcome::NONE;
    const char* last_result_reason = "";

    float last_completion_time = 0.0f;
};