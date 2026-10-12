#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float MAX_HALF_WIDTH = fmaxf(WINDOW_HEIGHT, WINDOW_WIDTH) / 2.0f;
const int MAX_RECURSION_DEPTH = 4;
int NODE_COUNT = 1;

const float FRICTION = 0.0f;
const float SPEED = 25.0f;
int ELASTICITY = 1;

struct Circle;

struct QuadNode {
    Vector2 min;
    Vector2 max;
    float half_width;
    QuadNode *parent;
    QuadNode *children[4];
    std::vector<Circle *> objects;
    std::string id;
    bool active;

    QuadNode() {
        parent = nullptr;
        for (int i = 0; i < 4; i++)
        {
            children[i] = nullptr;
        }
        active = false;
    }

    ~QuadNode() {
        for (QuadNode *child : children)
        {
            if (child != nullptr)
            {
                delete child;
                child = nullptr;
            }
        }
    };
};

struct Circle {
    Vector2 position;
    float radius;
    float mass;
    float inverse_mass;
    Vector2 velocity;
    Color color;
    QuadNode *node;
};

float getRandomFloat(float min, float max) {
    float scale = (float)GetRandomValue(0, 100) / 100.0f;
    return min + scale * (max - min);
}

void initCircle(Circle &c) {
    c.position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    c.radius = getRandomFloat(5.0f, 10.0f);
    c.mass = 1.0f;
    c.inverse_mass = 1 / c.mass;
    c.color = {
        (unsigned char)GetRandomValue(0, 255),
        (unsigned char)GetRandomValue(0, 255),
        (unsigned char)GetRandomValue(0, 255),
        255};
    c.velocity = Vector2Scale({getRandomFloat(-10, 10), getRandomFloat(-10, 10)}, SPEED);
    c.node = nullptr;
}

void spawnCircle(std::vector<Circle> &circles, int num) {
    for (int i = 0; i < num; i++) {
        Circle newCircle;
        initCircle(newCircle);
        circles.push_back(newCircle);
    }
}

void spawnBigCircle(std::vector<Circle> &circles) {
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
                                      Vector2Scale(collisionNorm, (impulse * circleA.inverse_mass)));

        circleB.velocity = Vector2Subtract(circleB.velocity,
                                           Vector2Scale(collisionNorm, (impulse * circleB.inverse_mass)));
    }
}

void wallCollision(Circle &c) {
    if (c.position.x + c.radius >= WINDOW_WIDTH || c.position.x - c.radius <= 0) {
        c.velocity.x *= -1;
    }

    if (c.position.y + c.radius >= WINDOW_HEIGHT || c.position.y - c.radius <= 0) {
        c.velocity.y *= -1;
    }
}

QuadNode* generateChild(QuadNode& parent, int index, int depth = 0) {
    if (depth >= MAX_RECURSION_DEPTH)
        return nullptr;

    QuadNode* child = new QuadNode();
    child->parent = &parent;

    // midpoint of parent
    float midX = (parent.min.x + parent.max.x) / 2.0f;
    float midY = (parent.min.y + parent.max.y) / 2.0f;

    switch (index) {
        case 0:
            child->min = parent.min;
            child->max = {midX, midY};
            break;
        case 1:
            child->min = {midX, parent.min.y};
            child->max = {parent.max.x, midY};
            break;
        case 2:
            child->min = {parent.min.x, midY};
            child->max = {midX, parent.max.y};
            break;
        case 3:
            child->min = {midX, midY};
            child->max = parent.max;
            break;
    }

    child->half_width = (child->max.x - child->min.x) / 2.0f;

    child->id = std::to_string(depth) + std::to_string(index);
    NODE_COUNT++;

    for (int i = 0; i < 4; i++) {
        child->children[i] = generateChild(*child, i, depth + 1);
    }

    return child;
}


void generateQuadTree(QuadNode &root) {
    Vector2 center = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0F};
    root.min = {center.x - MAX_HALF_WIDTH, center.y - MAX_HALF_WIDTH};
    root.max = {center.x + MAX_HALF_WIDTH, center.y + MAX_HALF_WIDTH};
    root.half_width = MAX_HALF_WIDTH;
    root.id = "R";

    for (int i = 0; i < 4; i++) {
        root.children[i] = generateChild(root, i);
    }
}

bool canFitObject(Circle *circle, QuadNode *child_node) {
    if (child_node == nullptr)
        return false;

    Vector2 min = child_node->min;
    Vector2 max = child_node->max;

    Vector2 circleMin = {circle->position.x - circle->radius, circle->position.y - circle->radius};
    Vector2 circleMax = {circle->position.x + circle->radius, circle->position.y + circle->radius};

    return (circleMin.x >= min.x && circleMax.x <= max.x && circleMin.y >= min.y && circleMax.y <= max.y);
}

// void insertObject(Circle* circle, QuadNode* cur_node, int depth = 0) {
//     if (depth >= MAX_RECURSION_DEPTH)
//         return;

//     circle->node = cur_node;
//     cur_node->objects.push_back(circle - &circle[0]);
//     cur_node->active = true;

//     for (int i = 0; i < 4; i++) {
//         QuadNode* child_node = cur_node->children[i];
//         if (canFitObject(circle, child_node)) {
//             insertObject(circle, child_node, depth + 1);
//             // std::cout << "inserted at depth " << depth + 1 << std::endl;
//             return;
//         }
//     }
// }

void insertObject(Circle *circle, QuadNode *cur_node, int depth = 0) {
    if (depth >= MAX_RECURSION_DEPTH) {
        circle->node = cur_node;
        cur_node->objects.push_back(circle);
        cur_node->active = true;
        return;
    }

    for (int i = 0; i < 4; i++) {
        QuadNode *child_node = cur_node->children[i];

        if (child_node != nullptr && canFitObject(circle, child_node)) {
            insertObject(circle, child_node, depth + 1);
            return;
        }
    }

    circle->node = cur_node;
    cur_node->objects.push_back(circle);
    cur_node->active = true;
}

void clearQuadNodeObjects(QuadNode &node) {
    node.objects.clear();
    node.active = false;
    for (QuadNode *child : node.children) {
        if (child != nullptr) {
            clearQuadNodeObjects(*child);
        }
    }
}

void printObjectsInQuadNode(const QuadNode &node) {
    if (node.active) {
        std::cout << "Node ID: " << node.id << ", Objects: ";
        for (Circle *obj : node.objects) {
            std::cout << obj << " ";
        }
        std::cout << std::endl;
    }

    for (const QuadNode *child : node.children) {
        if (child != nullptr) 
            printObjectsInQuadNode(*child);
    }
}

int main() {
    std::vector<Circle> circles;
    int space_count = 0;

    QuadNode root;

    generateQuadTree(root);

    std::cout << "node count " << NODE_COUNT << std::endl;

    // CloseWindow();

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HW 4 - Gimena & Tan");
    SetTargetFPS(FPS);

    float accumulator = 0;

    // checker for comparison in between two or more cells

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            if (space_count % 10 == 0 && space_count != 0)
                spawnBigCircle(circles);
            else
                spawnCircle(circles, 1);

            space_count++;
        }

        // clearGridCells(grid);

        // PHYSICS
        accumulator += delta_time;
        while (accumulator >= TIMESTEP) {
            clearQuadNodeObjects(root);

            for (int i = 0; i < circles.size(); i++) {
                circles[i].velocity = Vector2Subtract(circles[i].velocity, Vector2Scale(circles[i].velocity, FRICTION * circles[i].inverse_mass * TIMESTEP));
                circles[i].position = Vector2Add(circles[i].position, Vector2Scale(circles[i].velocity, TIMESTEP));

                int circle_index = &circles[i] - &circles[0];
               
                // insert circle

                insertObject(&circles[circle_index], &root);

        

                // reassign grid cells
                // seenBalls.clear();
                // reassignGridCells(circles[i], circle_index, grid);

                // for (int j = 0; j < circles[i].occupiedCells.size(); j++) {
                //     Vector2 gridCoords = circles[i].occupiedCells[j];

                //     GridCell &cur_grid = grid[static_cast<int>(gridCoords.y)][static_cast<int>(gridCoords.x)];

                //     for (int k = 0; k < cur_grid.objects.size(); k++) {
                //         int idx = cur_grid.objects[k];

                //         if (i == idx) continue;
                //         if (seenBalls.find(idx) != seenBalls.end()) continue;

                //         circleCollision(circles[i], circles[idx]);
                //         seenBalls.insert(idx);
                //     }

                // }

                for (int j = 0; j < circles.size(); j++) {
                    if (i == j)
                        continue;
                    circleCollision(circles[i], circles[j]);
                }

                wallCollision(circles[i]);
            }

            printObjectsInQuadNode(root);
            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        for (int i = 0; i < circles.size(); i++) {
            DrawCircleV(circles[i].position, circles[i].radius, circles[i].color);
        }
        // std::string space_label = "Space Pressed: " + std::to_string(space_count);
        std::string circle_count = "Circles: " + std::to_string(circles.size());
        // std::string grid_instruction = "Press G to toggle grid";
        // DrawText(space_label.c_str(), 10, 10, 24, DARKGRAY);
        DrawText(circle_count.c_str(), 10, 40, 24, DARKGRAY);
        // DrawText(grid_instruction.c_str(), 10, 70, 24, DARKGRAY);


        // draw quad tree




        // // Bonus Stuffs
        // // draw columns
        // if (showGrid)
        // {
        //     for (int col = 0; col < GRID_COLS; col++)
        //     {
        //         DrawLine(col * CELL_SIZE, 0, col * CELL_SIZE, WINDOW_HEIGHT, LIGHTGRAY);
        //     }

        //     for (int row = 0; row < GRID_ROWS; ++row)
        //     {
        //         // draw rows
        //         DrawLine(0, row * CELL_SIZE, WINDOW_WIDTH, row * CELL_SIZE, LIGHTGRAY);

        //         // draw labels
        //         for (int col = 0; col < GRID_COLS; ++col)
        //         {
        //             std::string cell_label = "(" + std::to_string(col) + "," + std::to_string(row) + ")";
        //             DrawText(cell_label.c_str(), grid[row][col].position.x + 2, grid[row][col].position.y + 3, 12, DARKGRAY);

        //             std::string occupants = std::to_string(grid[row][col].objects.size());

        //             Color occupants_color = (grid[row][col].objects.size() > 1) ? GREEN : RED;
        //             DrawText(occupants.c_str(), grid[row][col].position.x + (CELL_SIZE / 2) - 8, grid[row][col].position.y + (CELL_SIZE / 2) - 10, 24, occupants_color);
        //         }
        //     }
        // }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}