#include "entity.hpp"

// Entity default constructor
Entity::Entity() : x(0), y(0), w(0), h(0), color({0, 0, 0, 255}), attachedClientID(-1) {
    inputHandler = nullptr;
    physicsHandler = nullptr;
    patternHandler = nullptr;
}

// Entity parametric constructor
Entity::Entity(int x, int y, int w, int h, SDL_Color color): x(x), y(y), w(w), h(h), color(color), attachedClientID(-1) {
    inputHandler = nullptr;
    physicsHandler = nullptr;
    patternHandler = nullptr;
}

// Sets the attached client ID of the entity
void Entity::setID(int ID) {

    attachedClientID = ID;

}

// Clones the fields of this entity to the given one's
void Entity::setFields(Entity *entity) {

    this->x = entity->x;
    this->y = entity->y;
    this->w = entity->w;
    this->h = entity->h;

    this->color = entity->color;

    this->attachedClientID = entity->attachedClientID;

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
bool Entity::checkCollision(const Entity *other) const {
    SDL_Rect thisRect = getBoundingBox();
    SDL_Rect otherRect = other->getBoundingBox();
    return SDL_HasIntersection(&thisRect, &otherRect);
}

// Method to update the dimensions while rescaling the window screen size
void Entity::updateDimensions(int newx, int newy, int neww, int newh) {
    x = newx; y = newy;
    w = neww; h = newh;
}