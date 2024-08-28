#pragma once
#include <SDL2/SDL.h>
#include <string>

// struct to hold the reference to SDL window and renderer
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} App;