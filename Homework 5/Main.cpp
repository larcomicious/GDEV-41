#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <iostream>
#include "entt.hpp"

const float FPS = 60;
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const float SPRING_STIFFNESS = 8.0f;   // how hard it pulls toward the mouse
const float DAMPING_FACTOR   = 1.5f;   // how quickly the motion dies out
const float MAX_SPEED        = 700.0f; // safety cap

enum AttractionType {
    normal,
    attracted,
    repelled
};

struct PositionComponent {
    Vector2 position;
    Vector2 direction;
    float speed;
};

struct ColorComponent {
    Color color;
};

struct AsteroidComponent {
    float radius;
};

struct AttractionComponent {
    AttractionType type;
};

struct DestructionComponent {
    bool selfDestruct;
    float destructTime;
    int spawnChildren;
};

struct SpawnRequest {
    Vector2 position;
    int count;
};


float GetRandomFloat(float min, float max)
{
    float scale = (float)GetRandomValue(0, 100) / 100.0f;
    return min + scale * (max - min);
}

void createAsteroid(entt::registry& registry, bool fromDestroy, Vector2 parentPosition = {0, 0}) {

    // default values
    entt::entity asteroid = registry.create();

    PositionComponent& transform = registry.emplace<PositionComponent>(asteroid);
    if (fromDestroy)
    {
        transform.position = parentPosition;
    }
    else
    {
        transform.position = {(float)GetRandomValue(60, WINDOW_WIDTH - 60), (float)GetRandomValue(60, WINDOW_HEIGHT - 60)};
    }

    Vector2 dir = {GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)};
    if (Vector2Length(dir) < 0.01f)
        dir = {1, 0};
    transform.direction = Vector2Normalize(dir);
    // transform.direction = {GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)};
    transform.speed = GetRandomFloat(120.0f, 240.0f);

    AsteroidComponent& ast_comp = registry.emplace<AsteroidComponent>(asteroid);
    ast_comp.radius = (float)GetRandomValue(20,50);

    AttractionComponent& attr_comp = registry.emplace<AttractionComponent>(asteroid);
    attr_comp.type = (GetRandomValue(1, 100) > 50) ? normal : ((GetRandomValue(1, 100) > 50) ? attracted : repelled);

    ColorComponent& color_comp = registry.emplace<ColorComponent>(asteroid);
    switch (attr_comp.type)
    {
        case AttractionType::attracted:
            color_comp.color = BLUE;            
            break;
        case AttractionType::repelled:
            color_comp.color = RED;            
            break;
        default:
            color_comp.color = GRAY;
            break;
    }

    DestructionComponent& dest_comp = registry.emplace<DestructionComponent>(asteroid);
    dest_comp.selfDestruct = (GetRandomValue(1, 100) > 35) ? false : true;
    dest_comp.destructTime = (dest_comp.selfDestruct) ? GetRandomValue(2, 5) : 0;
    dest_comp.spawnChildren = (GetRandomValue(1, 100) > 10) ? 0 : GetRandomValue(2, 5);
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 05 - Gimena Tan");
    entt::registry registry;

    // start with 3 asteroids :)
    createAsteroid(registry, false);
    createAsteroid(registry, false);
    createAsteroid(registry, false);

    SetTargetFPS(FPS);
    float timer = 0.0f;          
    const float interval = 3.0f;
    
    while(!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        timer += delta_time;
        if (timer >= interval)
        {
            createAsteroid(registry, false);
            timer -= interval;
        }

        std::vector<entt::entity> toDestroy;
        std::vector<SpawnRequest> toSpawn;
        Vector2 mousePosition = GetMousePosition();
        bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        auto move_circle = registry.view<AsteroidComponent, PositionComponent, DestructionComponent, AttractionComponent>();
        for (auto entity : move_circle) {
            PositionComponent& transform = registry.get<PositionComponent>(entity);
            AsteroidComponent& asteroid = registry.get<AsteroidComponent>(entity);
            DestructionComponent& destruction = registry.get<DestructionComponent>(entity);
            AttractionComponent& attraction = registry.get<AttractionComponent>(entity);

            // asteroid movement
            switch (attraction.type)
            {
                case AttractionType::attracted:
                {
                    // spring logic (Hooke's Law - with Spring Damper System)

                    // dir - length of spring
                    // rest length = mouse position, current length = asteroid position so it is attracted to the mouse
                    // (mouse - asteroid) is the same as -(asteroid - mouse), no need for negative later on k 
                    Vector2 dir = Vector2Subtract(mousePosition, transform.position);
                    Vector2 vel = Vector2Scale(transform.direction, transform.speed);

                    // Fs = -k Δl - bv
                    Vector2 accel = Vector2Subtract(
                        Vector2Scale(dir, SPRING_STIFFNESS),
                        Vector2Scale(vel, DAMPING_FACTOR));

                    // apply force
                    vel = Vector2Add(vel, Vector2Scale(accel, delta_time));

                    float newSpeed = Vector2Length(vel);
                    if (newSpeed > 0.001f)
                    {
                        transform.direction = Vector2Scale(vel, 1.0f / newSpeed);
                        transform.speed = fminf(newSpeed, MAX_SPEED);
                    }
                    else
                    {
                        transform.speed = 0.0f;
                    }
                    break;
                }
                case AttractionType::repelled:
                {
                    Vector2 dir = Vector2Subtract(transform.position, mousePosition);
                    if (CheckCollisionPointCircle(mousePosition, transform.position, asteroid.radius * 2.0f)
                        && Vector2Length(dir) > 0.001f)
                        transform.direction = Vector2Normalize(dir);
                    break;
                }
                default:
                    break;
            }
            
            transform.position = Vector2Add(transform.position, Vector2Scale(transform.direction, transform.speed * delta_time));

            // wall collision
            if (transform.position.x - asteroid.radius <= 0) {
                transform.position.x = asteroid.radius;
                transform.direction.x = fabsf(transform.direction.x);
            }
            else if (transform.position.x + asteroid.radius >= WINDOW_WIDTH) {
                transform.position.x = WINDOW_WIDTH - asteroid.radius;
                transform.direction.x = -fabsf(transform.direction.x);
            }

            if (transform.position.y - asteroid.radius <= 0) {
                transform.position.y = asteroid.radius;
                transform.direction.y = fabsf(transform.direction.y); 
            }
            else if (transform.position.y + asteroid.radius >= WINDOW_HEIGHT) {
                transform.position.y = WINDOW_HEIGHT - asteroid.radius;
                transform.direction.y = -fabsf(transform.direction.y);
            }

            if(destruction.selfDestruct) destruction.destructTime -= delta_time;

            bool timeoutAsteroid = (destruction.selfDestruct) && (destruction.destructTime <= 0);
            bool shootAsteroid = clicked && CheckCollisionPointCircle(mousePosition, transform.position, asteroid.radius);

            if (timeoutAsteroid || shootAsteroid) {
                toSpawn.push_back({transform.position, destruction.spawnChildren});
                toDestroy.push_back(entity);
            }
        }

        for (auto entity : toDestroy) {
            registry.destroy(entity);
        }
        
        for (const auto& newAsteroids : toSpawn) {
            for (int i = 0; i < newAsteroids.count; i++) {
                createAsteroid(registry, true, newAsteroids.position);
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        auto all_circles = registry.view<PositionComponent, AsteroidComponent, ColorComponent>();
        for (auto entity : all_circles) {
            PositionComponent& pos = registry.get<PositionComponent>(entity);
            AsteroidComponent& circle = registry.get<AsteroidComponent>(entity);
            ColorComponent& color = registry.get<ColorComponent>(entity);

            DrawCircleV(pos.position, circle.radius, color.color);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}