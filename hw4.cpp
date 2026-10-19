#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
bool PAUSED = false;

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
    QuadNode* parent;
    QuadNode* children[4];
    std::vector<Circle*> objects;
    std::string id;
    bool active;

    bool isLeaf() {
        for (int i = 0; i < 4; i++) {
            if (children[i] != nullptr)
                return false;
        }
        
        return true;
    }

    QuadNode() {
        parent = nullptr;
        for (int i = 0; i < 4; i++) {
            children[i] = nullptr;
        }
        active = false;
    }

    // ~QuadNode() {
    //     for (QuadNode* child : children) {
    //         if (child != nullptr) {
    //             delete child;
    //             child = nullptr;
    //         }
    //     }
    // };
};

struct Circle {
    Vector2 position;
    float radius;
    float mass;
    float inverse_mass;
    Vector2 velocity;
    Color color;
    QuadNode* node;
};

float getRandomFloat(float min, float max) {
    float scale = (float)GetRandomValue(0, 100) / 100.0f;
    return min + scale * (max - min);
}

void initCircle(Circle* c, QuadNode* root) {
    c->position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    c->radius = getRandomFloat(5.0f, 10.0f);
    // c.radius = 50.0f;
    c->mass = 1.0f;
    c->inverse_mass = 1 / c->mass;
    c->color = {
        (unsigned char)GetRandomValue(0, 255),
        (unsigned char)GetRandomValue(0, 255),
        (unsigned char)GetRandomValue(0, 255),
        255};
    c->velocity = Vector2Scale({getRandomFloat(-10, 10), getRandomFloat(-10, 10)}, SPEED);
    c->node = root;
}

void spawnCircle(std::vector<Circle*> &circles, int num, QuadNode* node) {
    for (int i = 0; i < num; i++) {
        Circle* newCircle = new Circle();
        initCircle(newCircle, node);
        circles.push_back(newCircle);
    }
}

void spawnBigCircle(std::vector<Circle*> &circles, QuadNode* node) {
    Circle* newCircle = new Circle();
    initCircle(newCircle, node);
    newCircle->radius = 25.0f;
    newCircle->mass = 10.0f;
    newCircle->inverse_mass = 1 / newCircle->mass;
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

    child->id = parent.id + std::to_string(index);
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
    root.parent = nullptr;
    root.id = "R";

    for (int i = 0; i < 4; i++) {
        root.children[i] = generateChild(root, i);
    }
}

bool canFitObject(Circle* circle, QuadNode* child_node) {
    if (child_node == nullptr)
        return false;

    Vector2 min = child_node->min;
    Vector2 max = child_node->max;

    Vector2 circleMin = {circle->position.x - circle->radius, circle->position.y - circle->radius};
    Vector2 circleMax = {circle->position.x + circle->radius, circle->position.y + circle->radius};

    return (circleMin.x >= min.x && circleMax.x <= max.x && circleMin.y >= min.y && circleMax.y <= max.y);
}

QuadNode* findNode(Circle* circle, QuadNode* cur_node, int depth = 0) {
    if (depth >= MAX_RECURSION_DEPTH)
        return cur_node;

    for (int i = 0; i < 4; i++) {
        QuadNode* child_node = cur_node->children[i];
        if (child_node != nullptr && canFitObject(circle, child_node))
            return findNode(circle, child_node, depth + 1);
    }

    return cur_node;
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

void collideWithAncestorObjects(QuadNode* node, Circle* cur_circle) {
    if (node == nullptr) return;

    for (Circle* obj : node->objects) {
        circleCollision(*cur_circle, *obj);
    }

    collideWithAncestorObjects(node->parent, cur_circle);
}

void collideWithDescendantObjects(QuadNode* node, Circle* cur_circle) {
    if (node == nullptr) return;

    for (QuadNode* child : node->children) {
        if (child == nullptr) return;
        for (Circle* obj : child->objects) {
            circleCollision(*cur_circle, *obj);
        }
        collideWithDescendantObjects(child, cur_circle);
    }
}

void handleCollisionWithQueryCandidates(Circle* cur_circle) {
    // collide with ancestors
    QuadNode* cur_node = cur_circle->node;
    
    collideWithAncestorObjects(cur_node, cur_circle);
    collideWithDescendantObjects(cur_node, cur_circle);

}

// debuger helper
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

void drawActiveNodes(QuadNode* node) {
    if (node->active) {
        // std::cout << "Node ID: " << node.id << ", Objects: ";
        // for (Circle *obj : node.objects) {
        //     std::cout << obj << " ";
        // }
        // std::cout << std::endl;
        Vector2 min = node->min;
        Vector2 max = node->max;
        Rectangle rect = {min.x, min.y, max.x - min.x, max.y - min.y};
        DrawRectangleLinesEx(rect, 2, GREEN);
    }

    for (QuadNode *child : node->children) {
        if (child != nullptr) 
            drawActiveNodes(child);
    }
}

int main() {
    std::vector<Circle*> circles;
    int space_count = 0;

    QuadNode root;

    generateQuadTree(root);

    std::cout << "node count " << NODE_COUNT << std::endl;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "HW 4 - Gimena & Tan");
    SetTargetFPS(FPS);

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            if (space_count % 10 == 0 && space_count != 0) {
                spawnBigCircle(circles, &root);
                space_count = -1;
            }
            else 
                spawnCircle(circles, 25, &root);
            
            space_count++;

        }
        if (IsKeyPressed(KEY_ENTER)) 
            PAUSED = !PAUSED;
        

        if (!PAUSED) {
            accumulator += delta_time;
            while (accumulator >= TIMESTEP) {
                
                for (int i = 0; i < circles.size(); i++) {
                    circles[i]->velocity = Vector2Subtract(circles[i]->velocity, Vector2Scale(circles[i]->velocity, FRICTION * circles[i]->inverse_mass * TIMESTEP));
                    circles[i]->position = Vector2Add(circles[i]->position, Vector2Scale(circles[i]->velocity, TIMESTEP));

                    int circle_index = &circles[i] - &circles[0];

                    QuadNode* old_node = circles[circle_index]->node;
                    QuadNode* start_node = old_node;

                    // check if ball can still be contained within current node
                    while (start_node->parent != nullptr && !canFitObject(circles[circle_index], start_node)) {
                        start_node = start_node->parent;
                    }

                    // if yes, check if can be in its children
                    QuadNode* new_node = findNode(circles[circle_index], start_node);

                    // find iterator for the index on where the circle lies (ty geeksforgeeks) return end if not in the range
                    auto it = std::find(old_node->objects.begin(), old_node->objects.end(), circles[circle_index]);

                    // circle moved to a diff node
                    if (new_node != old_node) {
                        if (it != old_node->objects.end()) {
                            old_node->objects.erase(it);
                        }
                            // the circle left the vector so erase it using iterator

                        old_node->active = !old_node->objects.empty();

                        new_node->objects.push_back(circles[circle_index]);
                        new_node->active = true;
                        circles[circle_index]->node = new_node;
                    }
                    // else if (it == old_node->objects.end()) {
                    //     // the circle was not in the range of old node so this is new circle, pushback
                    //     new_node->objects.push_back(&circles[circle_index]);
                    //     new_node->active = true;
                    // }

                    
                    // TODO: (done look up)
                    // check if ball can still be contained within current node
                        // if yes, check if can be in its children (use/change canFitObject() method for this :3)
                        // if no, go up to parent
                            // after going up to parent, check again if can fit in other children of parent

                    // then insert circle:
                    // insertObject(&circles[circle_index], &root); // TODO: remove
                    
                    // get candidate objects
                    // std::vector<Circle*> candidate_objects = queryCandidateObjects(circles[circle_index]);
                    // for (int j = 0; j < candidate_objects.size(); j++) {
                    //     if (circles[i] == candidate_objects[j]) continue;

                    //     circleCollision(*circles[i], *candidate_objects[j]);
                    // }

                    handleCollisionWithQueryCandidates(circles[circle_index]);

                    wallCollision(*circles[i]);
                }

                accumulator -= TIMESTEP;
            }

        }
        
        BeginDrawing();
        ClearBackground(WHITE);
        for (int i = 0; i < circles.size(); i++) {
            DrawCircleV(circles[i]->position, circles[i]->radius, circles[i]->color);
        }
        // std::string space_label = "Space Pressed: " + std::to_string(space_count);
        std::string circle_count = "Circles: " + std::to_string(circles.size());
        // std::string grid_instruction = "Press G to toggle grid";
        // DrawText(space_label.c_str(), 10, 10, 24, DARKGRAY);
        DrawText(circle_count.c_str(), 10, 40, 24, DARKGRAY);
        // DrawText(grid_instruction.c_str(), 10, 70, 24, DARKGRAY);

        if (PAUSED)
            DrawText("PAUSED", 10, 70, 24, DARKGRAY);
            
        // draw quad tree
        drawActiveNodes(&root);

        DrawText(std::to_string(GetFPS()).c_str(), 10, 10, 24, RED);
        

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