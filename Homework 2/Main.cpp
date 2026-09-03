#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main() {
    
    // globals here

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particles Yay!");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // logic here
        
        EndDrawing();
    }
    CloseWindow();
    return 0;
}