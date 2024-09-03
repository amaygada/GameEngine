#include "main.hpp"
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {

    // Initialize the static shape
    SDL_Color shapeColor = {SCREEN_BACKGROUND};  // Red color
    Entity staticShape(50, 50, 100, 100, shapeColor);
    staticShape.inputHandler = new DefaultEntityInputHandler();
    staticShape.physicsHandler = new DefaultGravityPhysicsHandler();
    std::vector<Entity> E;
    E.push_back(staticShape);

    // Initialize SDL
    initSDL();

    bool running = true;  // Variable to control the main loop
    double physicsTime = 0; // Variable to contain physics timer - temporary for now

    while (running) {
        // Prepare the scene with the entities
        prepareScene(E);

        // Process input
        doInput();

        for (auto &object : E) {
            if (object.inputHandler != nullptr) object.inputHandler->handleInput(&object);

            if (object.physicsHandler != nullptr) object.physicsHandler->updatePhysics(&object, PHYS_GRAVITY_CONSTANT, &physicsTime);
        }

        // Present the scene
        presentScene();

        // Insert 16ms delay for a budget frame limiter
        SDL_Delay(16);

        physicsTime += GRAV_INCREMENT_TIMER;
    }

    // Cleanup and exit
    SDL_Quit();
    return 0;
}