    #include <iostream>
    #include <raylib.h>
    #include <raymath.h>

    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const float FPS = 60;
    const float TIMESTEP = 1 / 120.0f; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
    const float FRICTION = 0.5;
    const int BALL_COUNT = 10;
    const int POCKET_COUNT = 4;
    const int WALL_COUNT = 4;
    const Color BALL_COLOR = SKYBLUE;
    const Color INDICATOR_COLOR = GREEN;
    const Color TABLE_COLOR = {30, 110, 70, 255};
    const Color RAIL_COLOR = {75, 45, 25, 255};
    const Color BALL_OUTLINE_COLOR = {20, 20, 20, 255};
    int ELASTICITY = 1;
    bool dragging = false;
    bool canDrag = true;
    bool showGuide = false;
    float cueStickWidth = 5.0f;

    const float VELOCITY_TOLERANCE = 1.0f;
    const float MAX_SPEED = 1500.0f;

    const float MAX_PULL_DISTANCE = 150.0f;

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
        int number = 0;
        
        bool consumed = false;
        // bool enabled = true;
    };

    // List of Functions
    void initCircle(Circle &b);
    void circleCollision(Circle &circleA, Circle &circleB);
    void AABBIntersection(Circle &ball, Rectangle &wall);
    void resetGame(Circle* balls);
    Vector2 ColNorm(Vector2 PosA, Vector2 PosB);
    Vector2 RelVelA(Vector2 VelA, Vector2 VelB);
    Color GetRandomColor(void);
    bool CheckIfAllBallsStopped(Circle* balls);
    void limitVelocity(Circle& ball);

    int main() {
        Circle* balls = new Circle[BALL_COUNT];
        Circle* pockets = new Circle[POCKET_COUNT];
        Rectangle* walls = new Rectangle[WALL_COUNT];

        // initialize ball s
        for (int i = 0; i < BALL_COUNT; i++) {
            initCircle(balls[i]);

            balls[i].type = (i == 0) ? cue_ball : non_cue_ball;
            balls[i].number = i;
            if (i == 0)
                balls[i].color = WHITE;
            else if (i == 9)
                balls[i].color = BLACK;
            else
                balls[i].color = BALL_COLOR;
        }

        balls[0].position = balls[0].default_position = {WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 };

        float rackX = 2.0f * (WINDOW_WIDTH / 3);
        float rackY = WINDOW_HEIGHT / 2;
        float spacing = balls[1].radius * 2.05f;
        
        balls[1].position = balls[1].default_position = {rackX, rackY};
        balls[2].position = balls[2].default_position = {rackX + spacing, rackY - spacing / 2};
        balls[3].position = balls[3].default_position = {rackX + spacing, rackY + spacing / 2};
        balls[4].position = balls[4].default_position = {rackX + spacing * 2, rackY - spacing};
        balls[5].position = balls[5].default_position = {rackX + spacing * 2, rackY};
        balls[6].position = balls[6].default_position = {rackX + spacing * 2, rackY + spacing};
        balls[7].position = balls[7].default_position = {rackX + spacing * 3, rackY - spacing * 0.5f};
        balls[8].position = balls[8].default_position = {rackX + spacing * 3, rackY + spacing * 0.5f};
        balls[9].position = balls[9].default_position = {rackX + spacing * 4, rackY};

        balls[1].color = YELLOW;
        balls[2].color = BLUE;
        balls[3].color = RED;
        balls[4].color = VIOLET;
        balls[5].color = ORANGE;
        balls[6].color = LIME;
        balls[7].color = BROWN;
        balls[8].color = BLACK;
        balls[9].color = GOLD;

        // initialize pockets
        for (int i = 0; i < POCKET_COUNT; i++) {
            initCircle(pockets[i]);
            pockets[i].type = pocket;
            pockets[i].radius = 25.0f;
        }
        pockets[0].position = {pockets[0].radius, pockets[0].radius};
        pockets[1].position = {WINDOW_WIDTH - pockets[1].radius, pockets[1].radius};
        pockets[2].position = {pockets[2].radius, WINDOW_HEIGHT - pockets[2].radius};
        pockets[3].position = {WINDOW_WIDTH - pockets[3].radius, WINDOW_HEIGHT - pockets[3].radius};

        // walls
        float& pocket_radius = pockets[0].radius;
        walls[0] = {0, pocket_radius * 2, pocket_radius, WINDOW_HEIGHT - pocket_radius * 4};     
        walls[1] = {pocket_radius * 2, 0, WINDOW_WIDTH - pocket_radius * 4, pocket_radius};     
        walls[2] = {WINDOW_WIDTH - pocket_radius, pocket_radius * 2, pocket_radius, WINDOW_HEIGHT - pocket_radius * 4};     
        walls[3] = {pocket_radius * 2, WINDOW_HEIGHT - pocket_radius , WINDOW_WIDTH - pocket_radius * 4, pocket_radius};     
        
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Gimena & Tan");
        SetTargetFPS(FPS);

        float accumulator = 0;

        while (!WindowShouldClose()) {
            float delta_time = GetFrameTime();
            Vector2 forces = Vector2Zero();


            if(IsKeyPressed(KEY_SPACE)) {
                ELASTICITY = (ELASTICITY) ? 0 : 1;
            }

            if(IsKeyPressed(KEY_R)) {
                resetGame(balls);
            }

            canDrag = CheckIfAllBallsStopped(balls);
            if (CheckCollisionPointCircle(GetMousePosition(), balls[0].position, balls[0].radius) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && canDrag) {
                dragging = true;
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && dragging) {
                Vector2 pullVector = Vector2Subtract(balls[0].position,GetMousePosition());

                float pullDistance = Vector2Length(pullVector);

                pullDistance = Clamp(pullDistance,0.0f,MAX_PULL_DISTANCE);

                if (pullDistance > 0.0f)
                {
                    Vector2 shotDirection = Vector2Normalize(pullVector);
                    float shotStrength = pullDistance / MAX_PULL_DISTANCE;
                    float shotSpeed = shotStrength * MAX_SPEED;
                    balls[0].velocity = Vector2Add(balls[0].velocity,Vector2Scale(shotDirection,shotSpeed));
                    limitVelocity(balls[0]);
                }
                dragging = false;
            }


            // PHYSICS
            accumulator += delta_time;
            while(accumulator >= TIMESTEP) {       
                for (int i = 0; i < BALL_COUNT; i++)
                {
                    if (balls[i].consumed)
                        continue;

                    // Computes for velocity using v(t + dt) = v(t) + (a(t) * dt)
                    balls[i].velocity = Vector2Add(balls[i].velocity, Vector2Scale(balls[i].acceleration, TIMESTEP));
                    balls[i].velocity = Vector2Subtract(balls[i].velocity, Vector2Scale(balls[i].velocity, FRICTION * balls[i].inverse_mass * TIMESTEP));

                    // Computes for change in position using x(t + dt) = x(t) + (v(t + dt) * dt)
                    balls[i].position = Vector2Add(balls[i].position, Vector2Scale(balls[i].velocity, TIMESTEP));

                    for(int j = 0; j < BALL_COUNT; j++)
                    {
                        if (i == j) continue;

                        circleCollision(balls[i], balls[j]);
                        limitVelocity(balls[j]);
                    }

                    for(int j = 0; j < POCKET_COUNT; j++)
                    {
                        circleCollision(balls[i], pockets[j]);
                    }

                    for(int j = 0; j < WALL_COUNT; j++) {
                        AABBIntersection(balls[i], walls[j]);
                    }
                    limitVelocity(balls[i]);
                }
                accumulator -= TIMESTEP;
            }

            BeginDrawing();
            ClearBackground(RAIL_COLOR);
            DrawRectangle(20,20,WINDOW_WIDTH - 40,WINDOW_HEIGHT - 40,TABLE_COLOR);

            for (int i = 0; i < POCKET_COUNT; i++) {
                DrawCircleV(pockets[i].position, pockets[i].radius + 3.0f, DARKBROWN);
                DrawCircleV(pockets[i].position, pockets[i].radius, BLACK);
            }
            for (int i = 0; i < WALL_COUNT; i++) {
                DrawRectangleRec(walls[i], RAIL_COLOR);
            }

            if (canDrag && !dragging) {
                DrawCircleV(balls[0].position, balls[0].radius + 10.0f, INDICATOR_COLOR);
                DrawCircleV(balls[0].position, balls[0].radius + 5.0f, TABLE_COLOR);
            }
            
            for (int i = 0; i < BALL_COUNT; i++) {
                if (balls[i].consumed)
                    continue;

                DrawCircleV(balls[i].position,balls[i].radius + 2.0f,BALL_OUTLINE_COLOR);
                DrawCircleV(balls[i].position,balls[i].radius,balls[i].color);
                DrawCircleV(balls[i].position,balls[i].radius * 0.42f,WHITE);

            }

            if (dragging) {
                DrawLineEx(balls[0].position, GetMousePosition(), cueStickWidth, YELLOW);
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
        b.radius = 20.0f;
        b.color = BLACK;
        b.mass = 1.0f;
        b.inverse_mass = 1 / b.mass;
        b.acceleration = Vector2Zero();
        b.velocity = Vector2Zero();
    }

    void circleCollision(Circle &circleA, Circle &circleB) {

        // if ((circleA.type == cue_stick_ball && circleB.type != cue_ball) || (circleA.type != cue_ball && circleB.type == cue_stick_ball))
        //     return;
        
        // if (circleA.type == cue_stick_ball && (!circleA.enabled || (circleA.enabled && dragging))) 
        //     return;
        
        // if (circleB.type == cue_stick_ball && (!circleB.enabled || (circleB.enabled && dragging)))
        //     return;
        
        if (circleA.consumed || circleB.consumed)
        return;

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
                
                // if (circleA.type == cue_stick_ball)
                //     circleA.enabled = false;
                
                // if (circleB.type == cue_stick_ball)
                //     circleB.enabled = false;
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

    void AABBIntersection(Circle& ball, Rectangle& wall)
    {
        Vector2 min = {wall.x, wall.y};
        Vector2 max = {wall.x + wall.width, wall.y + wall.height};
        Vector2 p = ball.position;

        Vector2 q = {Clamp(p.x, min.x, max.x), Clamp(p.y, min.y, max.y)};
        Vector2 normal;
        float dist;
        Vector2 diff = Vector2Subtract(p, q);
        float distanceSqr = Vector2LengthSqr(diff);

        if (distanceSqr > 0.000001f)
        {
            dist = sqrtf(distanceSqr);
            normal = Vector2Scale(diff, 1.0f / dist);
            float pen = ball.radius - dist;

            if (pen <= 0.0f)
                return;

            // push bol out of wol
            ball.position = Vector2Add(ball.position, Vector2Scale(normal, pen));
        }
        else
        {
            float leftDistance = p.x - min.x;
            float rightDistance = max.x - p.x;
            float topDistance = p.y - min.y;
            float bottomDistance = max.y - p.y;

            float smallest = leftDistance;
            normal = {-1.0f, 0.0f};

            if (rightDistance < smallest){
                smallest = rightDistance;
                normal = {1.0f, 0.0f};
            }
            if (topDistance < smallest){
                smallest = topDistance;
                normal = {0.0f, -1.0f};
            }
            if (bottomDistance < smallest){
                smallest = bottomDistance;
                normal = {0.0f, 1.0f};
            }

            // displace by smallest
            float pen = ball.radius + smallest;
            ball.position = Vector2Add(ball.position, Vector2Scale(normal, pen));
        }

        float velnormal = Vector2DotProduct(ball.velocity, normal);
        if (velnormal >= 0.0f)
            return;

        ball.velocity = Vector2Subtract(ball.velocity,Vector2Scale(normal,(1.0f + ELASTICITY) * velnormal));
    }

    bool CheckIfAllBallsStopped(Circle* balls) {
        for (int i = 0; i < BALL_COUNT; i++) {
            if (balls[i].consumed) continue;

            if (Vector2LengthSqr(balls[i].velocity) > VELOCITY_TOLERANCE) {
                return false;
            }
        }
        return true;
    }

    void limitVelocity(Circle& ball)
    {
        float speedSqr = Vector2LengthSqr(ball.velocity);

        if (speedSqr > MAX_SPEED * MAX_SPEED)
        {
            ball.velocity = Vector2Scale(Vector2Normalize(ball.velocity), MAX_SPEED);
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