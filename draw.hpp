#pragma once
#include "struct.hpp"
#include <SDL2/SDL.h>

// window context
extern App *app;

// Prepare scene
void prepareScene(void);

// Present scene
void presentScene(void);