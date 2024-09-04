#include "main.hpp"
#include <memory>
#include <vector>

void MyEntityInputHandler::handleInput(Entity *entity) {
    // Get the current state of the keyboard
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_UP]) {
        // Move the entity up
        if( entity->y > 0) entity->y -= 1;
    }

    // If the 'S' key is pressed
    if (state[SDL_SCANCODE_DOWN]) {
        // Move the entity down
        if( entity->y < SCREEN_HEIGHT - entity->h) entity->y += 1;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_LEFT]) {
        // Move the entity left
        if( entity->x > 0) entity->x -= 1;
    }

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_RIGHT]) {
        // Move the entity right
        if( entity->x < SCREEN_WIDTH - entity->w) entity->x += 1;
    }
}

int main(int argc, char *argv[]) {

    // Initialize the controllable shape
    SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
    Entity shape1(1000, 0, 100, 500, shapeColor);
    shape1.physicsHandler = new DefaultGravityPhysicsHandler();
    std::vector<Entity> E;
    E.push_back(shape1);

    // Initialize the static shape
    SDL_Color shapeColor2 = {0, 255, 0, 255};  // Green color
    Entity shape2(SCREEN_WIDTH-300, SCREEN_HEIGHT-300, 100, 100, shapeColor2);
    shape2.inputHandler = new MyEntityInputHandler();
    E.push_back(shape2);

    SDL_Color shapeColor5 = {0, 0, 255, 255};  // Green color
    Entity shape5(300, SCREEN_HEIGHT-300, 100, 100, shapeColor5);
    shape5.inputHandler = new DefaultEntityInputHandler();
    E.push_back(shape5);

    // Initialize the pattern-following shape
    SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
    Entity shape3( 10, 10, 100, 100, shapeColor3);
    std::vector<SDL_Rect> shape3Path = {};
    shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
    shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
    shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
    shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
    shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
    shape3Path.push_back({10, 10, 1, 1});
    shape3.patternHandler = new DefaultPatternHandler(shape3Path);
    E.push_back(shape3);

    // Initialize the pattern-following shape
    SDL_Color shapeColor4 = {255, 255, 0, 255};  // Yellow color
    Entity shape4( 100, 400, 100, 100, shapeColor4);
    std::vector<SDL_Rect> shape4Path = {};
    shape4Path.push_back({SCREEN_WIDTH/3, 400, 1, 1});
    shape4Path.push_back({SCREEN_WIDTH/2, 400, 1, 1});
    shape4Path.push_back({SCREEN_WIDTH-100,400, 1, 1});
    shape4Path.push_back({SCREEN_WIDTH/2, 400, 1, 1});
    shape4Path.push_back({SCREEN_WIDTH/3, 400, 1, 1});
    shape4Path.push_back({10, 400, 1, 1});
    shape4.patternHandler = new DefaultPatternHandler(shape4Path);
    E.push_back(shape4);

    // Initialize SDL
    initSDL();

    bool running = true;  // Variable to control the main loop
    double physicsTime = 0; // Variable to contain physics timer - temporary for now

    int window_width, window_height;

    while (running) {

        getWindowSize(&window_width, &window_height);

        // Prepare the scene with the entities
        prepareScene(E);

        // Process input
        doInput();

        for (auto &object : E) {
            if (object.inputHandler != nullptr) object.inputHandler->handleInput(&object);
            if (object.physicsHandler != nullptr) object.physicsHandler->updatePhysics(&object, PHYS_GRAVITY_CONSTANT, &physicsTime);
            if (object.patternHandler != nullptr) object.patternHandler->moveToPath(&object, PATTERN_MOVEMENT_CONSTANT);
        }

        // Check for collisions between entities
        for (size_t i = 0; i < E.size(); ++i) {
            for (size_t j = i + 1; j < E.size(); ++j) {
                if (E[i].checkCollision(E[j])) {
                    std::cout << "Collision detected between Entity " << i << " and Entity " << j << std::endl;
                    exit(0);
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