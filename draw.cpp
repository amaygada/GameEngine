#include "draw.hpp"
#include <vector>

// Entity default constructor
Entity::Entity() : x(0), y(0), w(0), h(0), color({0, 0, 0, 255}) {
    inputHandler = nullptr;
}

// Entity parametric constructor
Entity::Entity(int x, int y, int w, int h, SDL_Color color): x(x), y(y), w(w), h(h), color(color) {
    inputHandler = nullptr;
}

// Draw an Entity
void Entity::draw(SDL_Renderer *renderer) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

// To get the bounds of a rectangle
SDL_Rect Entity::getBoundingBox() const {
    SDL_Rect rect = {x, y, w, h};
    return rect;
}

// Method to check if this entity collides with another entity
bool Entity::checkCollision(const Entity &other) const {
    SDL_Rect thisRect = getBoundingBox();
    SDL_Rect otherRect = other.getBoundingBox();
    return SDL_HasIntersection(&thisRect, &otherRect);
}

// Function to prepare the scene by clearing the screen and drawing an Entity
void prepareScene(std::vector<Entity> &E) {
    SDL_SetRenderDrawColor(app->renderer, BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_O);  // Background color
    SDL_RenderClear(app->renderer);

    // Draw the vector of entities passed as a parameter
    for (auto &object : E) {
        object.draw(app->renderer);
    }
}

// Function to present the scene
void presentScene(void) {
    SDL_RenderPresent(app->renderer);
}