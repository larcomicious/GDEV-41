#include <iostream>
#include <raylib.h>
#include <raymath.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float FRICTION = 0.5;
const int BALL_COUNT = 5;
const int POCKET_COUNT = 4;
int ELASTICITY = 1;

enum CircleType {
    cue_ball,
    non_cue_ball,
    pocket
};

struct Circle {
    Vector2 position;
    Vector2 default_position;
    float radius;
    Color color;

    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass
    Vector2 acceleration;
    Vector2 velocity;

    CircleType type;
    bool consumed = false;
};

// List of Functions
void initCircle(Circle &b);
void circleCollision(Circle &circleA, Circle &circleB);
void resetGame(Circle* balls);
Vector2 ColNorm(Vector2 PosA, Vector2 PosB);
Vector2 RelVelA(Vector2 VelA, Vector2 VelB);
Color GetRandomColor(void);

int main() {

    Circle* balls = new Circle[BALL_COUNT];
    Circle* pockets = new Circle[POCKET_COUNT];

    // initialize ball s
    for (int i = 0; i < BALL_COUNT; i++) {
        initCircle(balls[i]);
        balls[i].type = (i == 0) ? cue_ball : non_cue_ball;
        balls[i].color = (i == 0) ? WHITE : BLUE;
    }

    balls[0].position = balls[0].default_position = {WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 };
    balls[1].position = balls[1].default_position = {2.0f * (WINDOW_WIDTH / 3) - (balls[1].radius * 1.7f), WINDOW_HEIGHT / 2 };
    balls[2].position = balls[2].default_position = {2.0f * (WINDOW_WIDTH / 3) + (balls[2].radius * 1.7f), WINDOW_HEIGHT / 2 };
    balls[3].position = balls[3].default_position = {2.0f * (WINDOW_WIDTH / 3), (WINDOW_HEIGHT / 2) - (balls[3].radius * 1.3f)};
    balls[4].position = balls[4].default_position = {2.0f * (WINDOW_WIDTH / 3), (WINDOW_HEIGHT / 2) + (balls[4].radius * 1.3f)};

    // initialize pockets
    for (int i = 0; i < POCKET_COUNT; i++) {
        initCircle(pockets[i]);
        pockets[i].type = pocket;
        pockets[i].radius = 30.0f;
    }
    pockets[0].position = {pockets[0].radius, pockets[0].radius};
    pockets[1].position = {WINDOW_WIDTH - pockets[1].radius, pockets[1].radius};
    pockets[2].position = {pockets[2].radius, WINDOW_HEIGHT - pockets[2].radius};
    pockets[3].position = {WINDOW_WIDTH - pockets[3].radius, WINDOW_HEIGHT - pockets[3].radius};

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Gimena & Tan");
    SetTargetFPS(FPS);

    float accumulator = 0;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        Vector2 forces = Vector2Zero();

        if(IsKeyDown(KEY_W)) {
            forces = Vector2Add(forces, {0, -200});
        }
        if(IsKeyDown(KEY_A)) {
            forces = Vector2Add(forces, {-200, 0});
        }
        if(IsKeyDown(KEY_S)) {
            forces = Vector2Add(forces, {0, 200});
        }
        if(IsKeyDown(KEY_D)) {
            forces = Vector2Add(forces, {200, 0});
        }
        if(IsKeyPressed(KEY_SPACE)) {
            ELASTICITY = (ELASTICITY) ? 0 : 1;
        }
        if(IsKeyPressed(KEY_R)) {
            resetGame(balls);
        }

        balls[0].acceleration = Vector2Scale(forces, balls[0].inverse_mass);

        // PHYSICS
        accumulator += delta_time;
        while(accumulator >= TIMESTEP) {            
            for (int i = 0; i < BALL_COUNT; i++)
            {
                // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt)
                balls[i].velocity = Vector2Add(balls[i].velocity, Vector2Scale(balls[i].acceleration, TIMESTEP));
                balls[i].velocity = Vector2Subtract(balls[i].velocity, Vector2Scale(balls[i].velocity, FRICTION * balls[i].inverse_mass * TIMESTEP));

                // Computes for change in position using x(t + dt) = x(t) + (v(t + dt) * dt)
                balls[i].position = Vector2Add(balls[i].position, Vector2Scale(balls[i].velocity, TIMESTEP));

                for(int j = 0; j < BALL_COUNT; j++)
                {
                    if (i == j) continue;
                    circleCollision(balls[i], balls[j]);
                }

                for(int j = 0; j < POCKET_COUNT; j++)
                {
                    circleCollision(balls[i], pockets[j]);
                }
                
                if(balls[i].position.x + balls[i].radius >= WINDOW_WIDTH || balls[i].position.x - balls[i].radius <= 0) {
                    balls[i].velocity.x *= -1;
                }
                
                if(balls[i].position.y + balls[i].radius >= WINDOW_HEIGHT || balls[i].position.y - balls[i].radius <= 0) {
                    balls[i].velocity.y *= -1;
                }
            }
            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(GREEN);
        for (int i = 0; i < BALL_COUNT; i++) {
            if (!balls[i].consumed)
                DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
        }
        for (int i = 0; i < POCKET_COUNT; i++) {
            DrawCircleV(pockets[i].position, pockets[i].radius, pockets[i].color);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

Vector2 ColNorm(Vector2 PosA, Vector2 PosB) {
    return Vector2Subtract(PosA, PosB);
}

Vector2 RelVelA(Vector2 VelA, Vector2 VelB) {
    return Vector2Subtract(VelA, VelB);
}

Color GetRandomColor(void) {
    Color color;
    color.r = (unsigned char)GetRandomValue(0, 255);
    color.g = (unsigned char)GetRandomValue(0, 255);
    color.b = (unsigned char)GetRandomValue(0, 255);
    color.a = 255; 
    return color;
}

void initCircle(Circle &b) {
    // defaults
    b.position = {0,0};
    b.default_position = {0,0};
    b.radius = 25.0f;
    b.color = BLACK;
    b.mass = 1.0f;
    b.inverse_mass = 1 / b.mass;
    b.acceleration = Vector2Zero();
    b.velocity = Vector2Zero();
}

void circleCollision(Circle &circleA, Circle &circleB) {

    // pre-collision computations
    Vector2 collisionNorm = ColNorm(circleA.position, circleB.position);
    Vector2 relVelA = RelVelA(circleA.velocity, circleB.velocity);

    float distance = Vector2Distance(circleA.position, circleB.position);
    float colDotVel = Vector2DotProduct(collisionNorm, relVelA);

    bool isOverlap = (circleA.radius + circleB.radius) >= distance;
    bool isColliding = (colDotVel < 0);

    // collision computaitons
    if (isOverlap && isColliding) {
        
        if ((circleA.type != pocket) && (circleB.type != pocket))
        {
            // ball on ball collision scenarip
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
        else if (((circleA.type == pocket) && (circleB.type == cue_ball)) || ((circleA.type == cue_ball) && (circleB.type == pocket)))
        {
            // cueball pocket scenarios
            circleA.position = (circleA.type == cue_ball) ? circleA.default_position : circleA.position;
            circleB.position = (circleB.type == cue_ball) ? circleB.default_position : circleB.position;

            // reset speed safety net
            circleA.velocity = (circleA.type == cue_ball) ? Vector2Zero() : circleA.velocity;
            circleB.velocity = (circleB.type == cue_ball) ? Vector2Zero() : circleB.velocity;
        }
        else if (((circleA.type == pocket) && (circleB.type == non_cue_ball)) || ((circleA.type == non_cue_ball) && (circleB.type == pocket)))
        {
            // non-cue-ball pocket scenarios
            // i placed it outside
            circleA.position = (circleA.type == non_cue_ball) ? Vector2Scale({WINDOW_WIDTH, WINDOW_HEIGHT}, 2) : circleA.position;
            circleB.position = (circleB.type == non_cue_ball) ? Vector2Scale({WINDOW_WIDTH, WINDOW_HEIGHT}, 2) : circleB.position;

            circleA.consumed = (circleA.type == non_cue_ball) ? true : false;
            circleB.consumed = (circleB.type == non_cue_ball) ? true : false;
        }

    }
}

void resetGame(Circle* balls)
{
    for (int i = 0; i < BALL_COUNT; i++)
    {
        balls[i].position = balls[i].default_position;
        balls[i].velocity = Vector2Zero();
        balls[i].consumed = false;
    }
}