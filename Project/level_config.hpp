
#pragma once

#include "course.hpp"

using CourseFactory = Course (*)();

struct LevelConfig {
    int level_number;
    // const char* level_name;

    CourseFactory course_factory; // stores function pointer to the course factory

    float time_limit_seconds = 0.0f; // 0 = no time limit

    // optional failure conditions
    bool use_fall_limit = false;
    float fall_limit_y = 0.0f;
};
