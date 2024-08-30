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

// Struct to represent an entity (e.g., a shape)
typedef struct {
    int x, y;           // Position
    int w, h;           // Width and Height
    SDL_Color color;    // Color of the object
} Entity;

// Global variable for the static shape
extern Entity staticShape;
