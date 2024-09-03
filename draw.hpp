#pragma once
#include "struct.hpp"
#include "defs.hpp"
#include "input.hpp"

#include <SDL2/SDL.h>
#include <vector>

// window context
extern App *app;

class ModularInputHandler; // Forward declaration due to circular dependency

class Entity {
public:
    int x, y;           // Position
    int w, h;           // Width and Height
    SDL_Color color;    // Color of the object
    ModularInputHandler* inputHandler;

    // Default constructor
    Entity();

    // Constructor to initialize the entity
    Entity(int x, int y, int w, int h, SDL_Color color);

    // Method to draw the entity
    void draw(SDL_Renderer *renderer);

    // Method to get the bounding box of the entity
    SDL_Rect getBoundingBox() const;

    // Method to check if this entity collides with another entity
    bool checkCollision(const Entity &other) const;
    
};

// Prepare scene
void prepareScene(std::vector<Entity> &E);

// Present scene
void presentScene(void);