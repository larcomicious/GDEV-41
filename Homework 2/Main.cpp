#include <raylib.h>
#include <raymath.h>


struct Particle {
    bool isActive = false;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    float defLifeTime;
    Color color;
};

const float FPS(144);

void initParticle(Particle &p, bool keyMode);
float GetRandomFloat(float min, float max);

int main() {
    InitWindow(800, 600, "Hello Raylib");
    SetTargetFPS(FPS);

    Particle* particles = new Particle[1000];
    int x_rate = 25;
    int y_rate = 25;

    float key_emission = 0.0f;
    float mouse_emission = 0.0f;

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        key_emission += dt;
        mouse_emission += dt;

        if (IsKeyDown(KEY_RIGHT))
            x_rate =  (x_rate > 50) ? 50 : (x_rate + 1);
        if (IsKeyDown(KEY_LEFT))
            x_rate =  (x_rate < 0) ? 0 : (x_rate - 1);
        if (IsKeyDown(KEY_UP))
            y_rate =  (y_rate > 50) ? 0 : (y_rate + 1);
        if (IsKeyDown(KEY_DOWN))
            y_rate =  (y_rate < 0) ? 0 : (y_rate - 1);
        
        // emission
        for (int i = 0; i < 1000; i++)
        {
            if (!particles[i].isActive) {
                if (IsKeyDown(KEY_SPACE) && key_emission >= (1.0f/x_rate)) {
                    initParticle(particles[i], true);
                    key_emission -= (1.0f/x_rate);
                }
                else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && mouse_emission >= (1.0f/y_rate)) {
                    initParticle(particles[i], false);
                    mouse_emission -= (1.0f/y_rate);
                }
                break;
            }
        }

        // update
        for (int i = 0; i < 1000; i++)
        {
            if (particles[i].isActive) {
                Vector2 normDir = Vector2Normalize(particles[i].direction);
                Vector2 step = Vector2Scale(normDir, particles[i].speed * dt);
                particles[i].position = Vector2Add(particles[i].position, step);
                particles[i].lifeTime -= dt;
                particles[i].isActive = (particles[i].lifeTime <= 0) ? false : true;
                particles[i].color.a = (particles[i].isActive) ? 
                    (unsigned char) (255 * (particles[i].lifeTime /  particles[i].defLifeTime))
                    : 0;
                }
        }
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < 1000; i++)
        {
            DrawCircle(particles[i].position.x, particles[i].position.y, 5, particles[i].color);
        }
        EndDrawing();
    }

    delete[] particles;
    particles = nullptr;
    CloseWindow();
    return 0;
}

void initParticle(Particle &p, bool keyMode) {
    p.isActive = true;
    p.position = keyMode ? Vector2{400, 600} : GetMousePosition();
    p.direction = {GetRandomFloat(-1,1), keyMode ? -1 : GetRandomFloat(-1,1)};
    p.speed = GetRandomValue(50,100);
    p.lifeTime = p.defLifeTime = keyMode ? GetRandomFloat(2.0,5.0) : GetRandomFloat(0.5,2);
    p.color = (Color){
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        (unsigned char)GetRandomValue(0,255),
        255
    };
}

float GetRandomFloat(float min, float max)
{
    float scale = (float)GetRandomValue(0, 100) / 100.0f;
    return min + scale * (max - min);
}