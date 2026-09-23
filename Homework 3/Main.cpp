    #include <iostream>
    #include <raylib.h>
    #include <raymath.h>

    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const float FPS = 60;
    const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
    const float FRICTION = 0.5;
    const int BALL_COUNT = 6;
    const int POCKET_COUNT = 4;
    const int WALL_COUNT = 4;
    const Color WALL_COLOR = DARKBROWN;
    int ELASTICITY = 1;
    bool dragging = false;
    bool canDrag = true;
    bool showGuide = false;

    const float MAX_IMPULSE = 100;
    const float VELOCITY_TOLERANCE = 1.0f;

    enum CircleType {
        cue_stick_ball,
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

        bool enabled = true;
    };

    struct Spring {
        Vector2 spring_start;
        Vector2 spring_end;
        float rest_length;
        float max_length;
        float b;
        float k;
        float width = 5.0f;
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

    int main() {

        Circle* balls = new Circle[BALL_COUNT];
        Circle* pockets = new Circle[POCKET_COUNT];
        Rectangle* walls = new Rectangle[WALL_COUNT];

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
        
        // initialize spring/cue stick
        balls[5].position = balls[0].position;
        balls[5].color = YELLOW;
        balls[5].type = cue_stick_ball;
        balls[5].enabled = false;
        
        Spring cue_stick;
        cue_stick.spring_start = balls[0].position;
        cue_stick.spring_end = balls[5].position;
        // cue_stick.rest_length = Vector2Distance(cue_stick.spring_start, Vector2Scale(cue_stick.spring_end, 1.1));
        cue_stick.rest_length = 20.0f;
        cue_stick.b = 1.0f;
        cue_stick.k = 100.0f;

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

            // if(IsKeyDown(KEY_W)) {
            //     forces = Vector2Add(forces, {0, -200});
            // }
            // if(IsKeyDown(KEY_A)) {
            //     forces = Vector2Add(forces, {-200, 0});
            // }
            // if(IsKeyDown(KEY_S)) {
            //     forces = Vector2Add(forces, {0, 200});
            // }
            // if(IsKeyDown(KEY_D)) {
            //     forces = Vector2Add(forces, {200, 0});
            // }
            if(IsKeyPressed(KEY_SPACE)) {
                ELASTICITY = (ELASTICITY) ? 0 : 1;
            }
            if (IsKeyPressed(KEY_G)) {
                showGuide = (showGuide) ? 0 : 1;
            }

            if(IsKeyPressed(KEY_R)) {
                resetGame(balls);
            }

            balls[0].acceleration = Vector2Scale(forces, balls[0].inverse_mass);

            Vector2 spring_force;

            Vector2 D = Vector2Subtract(cue_stick.spring_end, cue_stick.spring_start);
            Vector2 D_norm = Vector2Normalize(D);
            
            spring_force = Vector2Scale(D_norm, -cue_stick.k * (Vector2Length(D) - cue_stick.rest_length));
            spring_force = Vector2Subtract(spring_force, Vector2Scale(balls[5].velocity, cue_stick.b));
            forces = Vector2Add(forces, spring_force);
            // std::cout << balls[1].velocity.x << ", " << balls[1].velocity.y << std::endl;
            // std::cout << Vector2Length(balls[1].velocity) << std::endl;
            
            canDrag = CheckIfAllBallsStopped(balls);

            // std::cout <<"can drag: " << canDrag << std::endl;

            
            if (CheckCollisionPointCircle(GetMousePosition(), balls[0].position, balls[0].radius) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && canDrag) {
                
                dragging = true;
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                dragging = false;
            }

            // if(CheckCollisionPointCircle(GetMousePosition(), balls[0].position, cue_stick.max_length)) {
            //     if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            //         balls[5].position = GetMousePosition();
            //         balls[5].enabled = true;
            //         forces = Vector2Zero();
            //     }
            // }

            
            if(dragging) {
                balls[5].position = GetMousePosition();
                balls[5].velocity = Vector2Zero();
                balls[5].acceleration = Vector2Zero();
                balls[5].enabled = true;
                forces = Vector2Zero();
            }
            

            balls[5].acceleration = Vector2Scale(forces, balls[5].inverse_mass);
            // std::cout << balls[0].velocity.x << ", " << balls[0].velocity.x << std::endl;
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

                    if (balls[i].type == cue_stick_ball) continue;
                    
                    // if(balls[i].position.x + balls[i].radius >= WINDOW_WIDTH || balls[i].position.x - balls[i].radius <= 0) {
                    //     balls[i].velocity.x *= -1;
                    // }
                    
                    // if(balls[i].position.y + balls[i].radius >= WINDOW_HEIGHT || balls[i].position.y - balls[i].radius <= 0) {
                    //     balls[i].velocity.y *= -1;
                    // }
                    for(int j = 0; j < WALL_COUNT; j++) {
                        AABBIntersection(balls[i], walls[j]);
                    }
                }

                balls[5].velocity = Vector2Add(balls[5].velocity, Vector2Scale(balls[5].acceleration, TIMESTEP));
                balls[5].position = Vector2Add(balls[5].position, Vector2Scale(balls[5].velocity, TIMESTEP));

                cue_stick.spring_start = balls[0].position;
                cue_stick.spring_end = balls[5].position;
                accumulator -= TIMESTEP;
            }

            BeginDrawing();
            ClearBackground(GREEN);
            if (canDrag) {
                DrawCircleV(balls[0].position, balls[0].radius + 10.0f, LIGHTGRAY);
                DrawCircleV(balls[0].position, balls[0].radius + 5.0f, GREEN);
            }
            for (int i = 0; i < BALL_COUNT; i++) {
                if (!balls[i].consumed && (balls[i].type != cue_stick_ball || showGuide))
                    DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
            }
            for (int i = 0; i < POCKET_COUNT; i++) {
                DrawCircleV(pockets[i].position, pockets[i].radius, pockets[i].color);
            }
            for (int i = 0; i < WALL_COUNT; i++) {
                DrawRectangleRec(walls[i], WALL_COLOR);
            }

            if (dragging) {
                
                DrawLineEx(balls[0].position, GetMousePosition(), cue_stick.width, YELLOW);
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

        if ((circleA.type == cue_stick_ball && circleB.type != cue_ball) || (circleA.type != cue_ball && circleB.type == cue_stick_ball))
            return;
        
        if (circleA.type == cue_stick_ball && (!circleA.enabled || (circleA.enabled && dragging))) 
            return;
        
        if (circleB.type == cue_stick_ball && (!circleB.enabled || (circleB.enabled && dragging)))
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
                impulse = Clamp(impulse, -MAX_IMPULSE, MAX_IMPULSE);

                circleA.velocity = Vector2Add(circleA.velocity,
                                    Vector2Scale(collisionNorm, (impulse * circleA.inverse_mass))
                                    );

                circleB.velocity = Vector2Subtract(circleB.velocity,
                                    Vector2Scale(collisionNorm, (impulse * circleB.inverse_mass))
                                    );
                
                if (circleA.type == cue_stick_ball)
                    circleA.enabled = false;
                
                if (circleB.type == cue_stick_ball)
                    circleB.enabled = false;
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

    void AABBIntersection(Circle& ball, Rectangle& wall) {
        Vector2 min = {wall.x, wall.y};
        Vector2 max = {wall.x + wall.width, wall.y + wall.height};
        Vector2 p = ball.position;
        
        Vector2 q = {Clamp(p.x, min.x, max.x), Clamp(p.y, min.y, max.y)};

        Vector2 normal = ColNorm(p, q);
        Vector2 relVelA = RelVelA(ball.velocity, {0, 0});

        
        bool isOverlap = Vector2Distance(p, q) <= ball.radius;
        bool isColliding = Vector2DotProduct(normal, relVelA) < 0;

        if (isOverlap && isColliding) {
                float impulse_num = (1 + ELASTICITY) * Vector2DotProduct(relVelA, normal);
                float impulse_den = Vector2DotProduct(normal, normal) * (ball.inverse_mass + 0); // coz static?

                float impulse = -(impulse_num / impulse_den);
                impulse = Clamp(impulse, -MAX_IMPULSE, MAX_IMPULSE);

                ball.velocity = Vector2Add(ball.velocity,
                                    Vector2Scale(normal, (impulse * ball.inverse_mass))
                                    );

                // circleB.velocity = Vector2Subtract(circleB.velocity,
                //                     Vector2Scale(collisionNorm, (impulse * circleB.inverse_mass))
                //                     );
                
                // if (circleA.type == cue_stick_ball)
                //     circleA.enabled = false;
                
                // if (circleB.type == cue_stick_ball)
                //     circleB.enabled = false;
            }

    } 

    bool CheckIfAllBallsStopped(Circle* balls) {
        for (int i = 0; i < BALL_COUNT; i++) {
            if (balls[i].type == cue_stick_ball) continue;
            if (balls[i].consumed) continue;

            if (Vector2LengthSqr(balls[i].velocity) > VELOCITY_TOLERANCE) {
                return false;
            }
        }
        return true;
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