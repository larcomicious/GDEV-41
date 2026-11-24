#pragma once

struct GameSession {
    int selected_level = 1;
    int last_completed_level = 0;

    float last_completion_time = 0.0f;
};