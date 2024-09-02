#pragma once
#include "struct.hpp"
#include <SDL2/SDL.h>
#include <vector>     // Include the vector header

// window context
extern App *app;

// Prepare scene
void prepareScene(std::vector<Entity> &E);

// Present scene
void presentScene(void);