#include "main.hpp"
#include <memory>

int main(int argc, char *argv[]){
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

        // Prep the scene
        prepareScene();

        // Present the scene
        presentScene();

        // Insert 16ms delay for a budget frame limiter
        SDL_Delay(16);
    }

    // Cleanup and exit
    SDL_Quit();
    return 0;
}