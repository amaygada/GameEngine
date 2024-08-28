#include "main.hpp"
#include <memory>

int main(int argc, char *argv[]){
    // Initialize SDL
    initSDL();

    while(1){
        //Prep the scene
        prepareScene();

        //Present the scene
        presentScene();

        // Insert 16ms delay for a budget frame limiter
        SDL_Delay(16);
    }

    return 0;
}