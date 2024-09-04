#pragma once
#include "struct.hpp"
#include "defs.hpp"
#include <SDL2/SDL.h>
#include <iostream>
using namespace std;

// making the instance of the SDL App struct available to struct
extern App *app;

// function to initialize the SDL window and renderer
void initSDL();

void getWindowSize(int *window_width, int *window_height);