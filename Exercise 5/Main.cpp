#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <utility>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float FRICTION = 0.0f;
const float SPEED = 25.0f;
const float CELL_SIZE = 60.0f;
constexpr int GRID_ROWS = WINDOW_HEIGHT / 60 + 1;
constexpr int GRID_COLS = WINDOW_WIDTH / 60 + 1;
int ELASTICITY = 1;

struct GridCell {
    Vector2 position;
    float width;
    float height;
    std::vector<int> objects;
};

struct Circle {
    Vector2 position;
    float radius;
    float mass;
    float inverse_mass;
    Vector2 velocity;
    Color color;
    std::vector<Vector2> occupiedCells; 
};

float GetRandomFloat(float min, float max)
{
    float scale = (float)GetRandomValue(0, 100) / 100.0f;
    return min + scale * (max - min);
}

void initCircle(Circle &c) {
    c.position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    c.radius = GetRandomFloat(5.0f, 10.0f);
    c.mass = 1.0f;
    c.inverse_mass = 1 / c.mass;
    c.color = {
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        255
    };
    c.velocity = Vector2Scale({GetRandomFloat(-10,10), GetRandomFloat(-10,10)}, SPEED);
    c.occupiedCells = std::vector<Vector2> {};
}

void spawnCircle(std::vector<Circle> &circles, int num)
{
    for (int i = 0; i < num; i++) {
        Circle newCircle;
        initCircle(newCircle);
        circles.push_back(newCircle);
    }
}

void spawnBigCircle(std::vector<Circle> &circles)
{
    Circle newCircle;
    initCircle(newCircle);
    newCircle.radius = 25.0f;
    newCircle.mass = 10.0f;
    newCircle.inverse_mass = 1 / newCircle.mass;
    circles.push_back(newCircle);
}

void circleCollision(Circle &circleA, Circle &circleB) {

    // pre-collision computations
    Vector2 collisionNorm = Vector2Subtract(circleA.position, circleB.position);
    Vector2 relVelA = Vector2Subtract(circleA.velocity, circleB.velocity);

    float distance = Vector2Distance(circleA.position, circleB.position);
    float colDotVel = Vector2DotProduct(collisionNorm, relVelA);

    bool isOverlap = (circleA.radius + circleB.radius) >= distance;
    bool isColliding = (colDotVel < 0);

    // collision computaitons
    if (isOverlap && isColliding) {
        float impulse_num = (1 + ELASTICITY) * Vector2DotProduct(relVelA, collisionNorm);
        float impulse_den = Vector2DotProduct(collisionNorm, collisionNorm) * (circleA.inverse_mass + circleB.inverse_mass);

        float impulse = -(impulse_num / impulse_den);

        circleA.velocity = Vector2Add(circleA.velocity,
                            Vector2Scale(collisionNorm, (impulse * circleA.inverse_mass))
                            );

        circleB.velocity = Vector2Subtract(circleB.velocity,
                            Vector2Scale(collisionNorm, (impulse * circleB.inverse_mass))
                            );
    }
}

void wallCollision(Circle &c)
{
    if(c.position.x + c.radius >= WINDOW_WIDTH || c.position.x - c.radius <= 0) {
        c.velocity.x *= -1;
    }
    
    if(c.position.y + c.radius >= WINDOW_HEIGHT || c.position.y - c.radius <= 0) {
        c.velocity.y *= -1;
    }
}

void generateGrid(GridCell grid[GRID_ROWS][GRID_COLS])
{
    float cellWidth = CELL_SIZE;
    float cellHeight = CELL_SIZE;

    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            grid[row][col].position = {col * cellWidth, row * cellHeight};
            grid[row][col].width = cellWidth;
            grid[row][col].height = cellHeight;
            grid[row][col].objects.clear();
        }
    }
}

void clearGridCells(GridCell grid[GRID_ROWS][GRID_COLS])
{
    for (int row = 0; row < GRID_ROWS; row++)
    {
        for (int col = 0; col < GRID_COLS; col++)
        {
            grid[row][col].objects.clear();
        }
    }
}

void reassignGridCells(Circle &c, int circleIndex, GridCell grid[GRID_ROWS][GRID_COLS])
{
    c.occupiedCells.clear();
    
    Vector2 min = {c.position.x - c.radius, c.position.y - c.radius};
    Vector2 max = {c.position.x + c.radius, c.position.y + c.radius};

    Vector2 new_min = {
        static_cast<float>(static_cast<int>(min.x / CELL_SIZE)),
        static_cast<float>(static_cast<int>(min.y / CELL_SIZE))};
    Vector2 new_max = {
        static_cast<float>(static_cast<int>(max.x / CELL_SIZE)),
        static_cast<float>(static_cast<int>(max.y / CELL_SIZE))};

    
    for (int row = static_cast<int>(new_min.y); row <= static_cast<int>(new_max.y); row++)
    {
        for (int col = static_cast<int>(new_min.x); col <= static_cast<int>(new_max.x); col++)
        {
            if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
            {
                grid[row][col].objects.push_back(circleIndex);
                Vector2 cellPos = {col, row};
                c.occupiedCells.push_back(cellPos);
            }
        }
    }
}

int main() {
    std::vector<Circle> circles;
    int space_count = 0;
    GridCell grid[GRID_ROWS][GRID_COLS];

    generateGrid(grid);
    bool showGrid = false;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Exercise 5 - Gimena & Tan");
    SetTargetFPS(FPS);

    float accumulator = 0;

    // checker for comparison in between two or more cells
    std::unordered_set<int> seenBalls;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        Vector2 forces = Vector2Zero();

        if(IsKeyPressed(KEY_SPACE)) {
            if (space_count % 10 == 0 && space_count != 0)
                spawnBigCircle(circles);
            else
                spawnCircle(circles, 25);
            
            space_count++;
        }

        if(IsKeyPressed(KEY_G)) {
            showGrid = (showGrid) ? false : true;
        }

        
        clearGridCells(grid);


        // PHYSICS
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {       
            for (int i = 0; i < circles.size(); i++)
            {
                circles[i].velocity = Vector2Subtract(circles[i].velocity, Vector2Scale(circles[i].velocity, FRICTION * circles[i].inverse_mass * TIMESTEP));
                circles[i].position = Vector2Add(circles[i].position, Vector2Scale(circles[i].velocity, TIMESTEP));

                int circle_index = &circles[i] - &circles[0];
                
                // reassign grid cells
                seenBalls.clear();
                reassignGridCells(circles[i], circle_index, grid);
                
                for (int j = 0; j < circles[i].occupiedCells.size(); j++) {
                    Vector2 gridCoords = circles[i].occupiedCells[j];

                    GridCell &cur_grid = grid[static_cast<int>(gridCoords.y)][static_cast<int>(gridCoords.x)];

                    for (int k = 0; k < cur_grid.objects.size(); k++) {
                        int idx = cur_grid.objects[k];
                        
                        if (i == idx) continue;                        
                        if (seenBalls.find(idx) != seenBalls.end()) continue; 

                        circleCollision(circles[i], circles[idx]);
                        seenBalls.insert(idx);
                    }

                }

                wallCollision(circles[i]);
            }
            accumulator -= TIMESTEP; 
        }

        BeginDrawing();
        ClearBackground(WHITE);
        for (int i = 0; i < circles.size(); i++) {
            DrawCircleV(circles[i].position, circles[i].radius, circles[i].color);
        }
        std::string space_label = "Space Pressed: " + std::to_string(space_count);
        std::string circle_count = "Circles: " + std::to_string(circles.size());
        std::string grid_instruction = "Press G to toggle grid";
        DrawText(space_label.c_str(), 10, 10, 24, DARKGRAY);
        DrawText(circle_count.c_str(), 10, 40, 24, DARKGRAY);
        DrawText(grid_instruction.c_str(), 10, 70, 24, DARKGRAY);

        // Bonus Stuffs
        // draw columns
        if (showGrid)
        {
            for (int col = 0; col < GRID_COLS; col++)
            {
                DrawLine(col * CELL_SIZE, 0, col * CELL_SIZE, WINDOW_HEIGHT, LIGHTGRAY);
            }

            for (int row = 0; row < GRID_ROWS; ++row)
            {
                // draw rows
                DrawLine(0, row * CELL_SIZE, WINDOW_WIDTH, row * CELL_SIZE, LIGHTGRAY);
                
                // draw labels
                for (int col = 0; col < GRID_COLS; ++col)
                {
                    std::string cell_label = "(" + std::to_string(col) + "," + std::to_string(row) + ")";
                    DrawText(cell_label.c_str(), grid[row][col].position.x + 2, grid[row][col].position.y + 3, 12, DARKGRAY);
                
                    std::string occupants = std::to_string(grid[row][col].objects.size());
                    
                    Color occupants_color = (grid[row][col].objects.size() > 1) ? GREEN : RED;
                    DrawText(occupants.c_str(), grid[row][col].position.x + (CELL_SIZE / 2) - 8, grid[row][col].position.y + (CELL_SIZE / 2) - 10, 24, occupants_color);
                }
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}