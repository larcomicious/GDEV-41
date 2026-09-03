#include <raylib.h>
#include <raymath.h>
#include <cstdlib> // for rand
#include <algorithm> // for clamp

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PARTICLE_COUNT 1000
#define PARTICLE_LIFETIME 3.0f

struct Particle {
    bool isActive = false;
    Vector2 position = {0, 0};
    Vector2 direction = {0, 0};
    float speed = 50;
    float lifeTime = PARTICLE_LIFETIME;
    Color color = Color{255, 255, 255, 255}; // default to white

};

void emitParticle(Particle& particle) {
    particle.isActive = true;
}

int emissionRate = 10;

void processInput() {
    if (IsKeyDown(KEY_UP)) 
        emissionRate = std::clamp(emissionRate++, 1, 50);
    if (IsKeyDown(KEY_DOWN)) 
        emissionRate = std::clamp(emissionRate--, 1, 50);

}
Particle* particles = new Particle[PARTICLE_COUNT];

void updateParticles(float delta) {
    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        Particle& p = particles[i];

        if (!p.isActive) continue;

        p.position += p.direction * p.speed;
        p.lifeTime -= delta;

        if (p.lifeTime <= 0) {
            p.isActive = false;
            p.lifeTime = PARTICLE_LIFETIME;
        }
    }
}


int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particles Yay!");

    while (!WindowShouldClose()) {
        
        processInput();
        updateParticles(GetFrameTime());

        BeginDrawing();
        ClearBackground(BLACK);

        // logic here
        
        EndDrawing();
    }

    delete[] particles;
    particles = nullptr;

    CloseWindow();
    return 0;
}