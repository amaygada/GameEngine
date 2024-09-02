#pragma once
#include <SDL2/SDL.h>
#include <string>

// Struct to hold the reference to SDL window and renderer
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} App;

// Global variable for the SDL application
extern App *app;

class Entity {
public:
    int x, y;           // Position
    int w, h;           // Width and Height
    SDL_Color color;    // Color of the object

    // Default constructor
    Entity() : x(0), y(0), w(0), h(0), color({0, 0, 0, 255}) {}

    // Constructor to initialize the entity
    Entity(int x, int y, int w, int h, SDL_Color color)
        : x(x), y(y), w(w), h(h), color(color) {}

    // Method to draw the entity
    void draw(SDL_Renderer *renderer) {
        SDL_Rect rect = {x, y, w, h};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
};

// Global variable for the static shape
// extern Entity staticShape;