#include "main.hpp"
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {

    // Initialize the static shape
    SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
    Entity shape1(0, 0, 100, 100, shapeColor);
    shape1.inputHandler = new DefaultEntityInputHandler();
    shape1.physicsHandler = new DefaultGravityPhysicsHandler();
    std::vector<Entity> E;
    E.push_back(shape1);

    SDL_Color shapeColor2 = {0, 255, 0, 255};  // Green color
    Entity shape2(SCREEN_WIDTH-100, SCREEN_HEIGHT-100, 100, 100, shapeColor2);
    E.push_back(shape2);

    // Initialize SDL
    initSDL();

    bool running = true;  // Variable to control the main loop
    double physicsTime = 0; // Variable to contain physics timer - temporary for now


    int window_width, window_height;

    while (running) {

        SDL_GetWindowSize(app->window, &window_width, &window_height);
        // Prepare the scene with the entities
        prepareScene(E);
        cout << window_width << " " << window_height << endl;

        // Process input
        doInput();

        for (auto &object : E) {
            if (object.inputHandler != nullptr) object.inputHandler->handleInput(&object);
            if (object.physicsHandler != nullptr) object.physicsHandler->updatePhysics(&object, PHYS_GRAVITY_CONSTANT, &physicsTime);
        }

        // Check for collisions between entities
        for (size_t i = 0; i < E.size(); ++i) {
            for (size_t j = i + 1; j < E.size(); ++j) {
                if (E[i].checkCollision(E[j])) {
                    std::cout << "Collision detected between Entity " << i << " and Entity " << j << std::endl;

                    // ToDo: helper functions/methods to react to collisions and map the Entity behaviour come here
                }
            }
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