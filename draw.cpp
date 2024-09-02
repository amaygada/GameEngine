#include "draw.hpp"
#include <vector>
#include "struct.hpp"

// Function to draw an Entity using the draw method of the Entity class
void drawEntity(Entity &entity) {
    entity.draw(app->renderer);
}

// Function to prepare the scene by clearing the screen and drawing an Entity
void prepareScene(std::vector<Entity> &E) {
    SDL_SetRenderDrawColor(app->renderer, 96, 128, 255, 255);  // Background color
    SDL_RenderClear(app->renderer);

    // Draw the vector of entities passed as a parameter
    for (auto &object : E) {
        drawEntity(object);
    }
}

// Function to present the scene
void presentScene(void) {
    SDL_RenderPresent(app->renderer);
}
