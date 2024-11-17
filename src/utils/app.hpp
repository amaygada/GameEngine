#pragma once
#include "timer.hpp"
#include "defs.hpp"
#include "./../subsystem/event_manager/event_manager.hpp"
#include "./../subsystem/input_handling/input.hpp"
#include "./../subsystem/physics/physics.hpp"
#include "./../subsystem/animation/animation.hpp"
#include "./../subsystem/collision/collision.hpp"

#include <SDL2/SDL.h>
#include <string>


// Struct to hold the reference to SDL window and renderer
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool quit;
    bool replay;
    bool record;
    int replayIndex;
    int displacement;
} App;

// Global variable for the SDL application
extern App *app;