#include "main.hpp"
#include "struct.hpp"
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {

    // Initialize the static shape
    SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
    Entity staticShape(50, 50, 100, 100, shapeColor);
    std::vector<Entity> E;
    E.push_back(staticShape);

    // Initialize SDL
    initSDL();

    bool running = true;  // Variable to control the main loop

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle quit events
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // Handle keydown events
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;  // Exit the loop if Escape is pressed
                }
            }
        }

        // Prepare the scene with the entity
        prepareScene(E);

        // Present the scene
        presentScene();

        // Insert 16ms delay for a budget frame limiter
        SDL_Delay(16);
    }

    // Cleanup and exit
    SDL_Quit();
    return 0;
}