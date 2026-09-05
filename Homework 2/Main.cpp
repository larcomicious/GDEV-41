#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 144
#define PARTICLE_RADIUS 5

struct InputKeys{
    int xUp;
    int xDown;
    int yUp;
    int yDown;
    int mouseButton;
    int emitKey;
};

struct Particle {
    bool isActive = false;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    float defLifeTime;
    Color color;
};

void initParticle(Particle &p, bool keyMode);
float GetRandomFloat(float min, float max);

InputKeys loadConfig(std::string file) {
    std::ifstream configFile(file);

    if (!configFile.is_open()) {
        std::cout << "Error: config file could not open."<< std::endl;
        return InputKeys{KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, MOUSE_BUTTON_LEFT, KEY_SPACE};
    }

    InputKeys inputKeys;
    if (configFile >> inputKeys.xUp >> inputKeys.xDown >> inputKeys.yUp >> inputKeys.yDown
            >> inputKeys.mouseButton >> inputKeys.emitKey) {
        std::cout << "Successfully read config.\n";
    } else {
        std::cout << "Failed to read 6 consecutive keys from the file.\n";
        return InputKeys{KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, MOUSE_BUTTON_LEFT, KEY_SPACE};
    }

    return inputKeys;
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Raylib");
    SetTargetFPS(FPS);

    InputKeys keys = loadConfig("config.ini");

    Particle* particles = new Particle[1000];
    int x_rate = 25;
    int y_rate = 25;

    float key_emission = 0.0f;
    float mouse_emission = 0.0f;

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        bool isKeyEmitting = IsKeyDown(keys.emitKey) || IsMouseButtonDown(keys.emitKey);
        bool isMouseEmitting = IsMouseButtonDown(keys.mouseButton) || IsKeyDown(keys.mouseButton);
        
        key_emission += isKeyEmitting ? dt : 0;
        mouse_emission += isMouseEmitting ? dt : 0;

        if (IsKeyDown(keys.xUp)) {
            x_rate =  (x_rate >= 50) ? 50 : (x_rate + 1);
            std::cout << "x_rate: " << x_rate << std::endl; 
        }
        if (IsKeyDown(keys.xDown)){
            x_rate =  (x_rate <= 1) ? 1 : (x_rate - 1);
            std::cout << "x_rate: " << x_rate << std::endl; 
        }
        if (IsKeyDown(keys.yUp)){
            y_rate =  (y_rate >= 50) ? 50 : (y_rate + 1);
            std::cout << "y_rate: " << y_rate << std::endl; 
        }
        if (IsKeyDown(keys.yDown)){
            y_rate =  (y_rate <= 1) ? 1 : (y_rate - 1);
            std::cout << "y_rate: " << y_rate << std::endl; 
        }
        
        // emission
        for (int i = 0; i < 1000; i++)
        {
            if (!particles[i].isActive) {
                if (isKeyEmitting && key_emission >= (1.0f/x_rate)) {
                    initParticle(particles[i], true);
                    key_emission -= (1.0f/x_rate);
                }
                else if (isMouseEmitting && mouse_emission >= (1.0f/y_rate)) {
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
                    (unsigned char) (255 * (particles[i].lifeTime/particles[i].defLifeTime))
                    : 0;
                }
        }
        BeginDrawing();
        ClearBackground(WHITE);

        for (int i = 0; i < 1000; i++)
        {
            DrawCircleV(particles[i].position, PARTICLE_RADIUS, particles[i].color);
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