#include "init.hpp"

// Define the global staticShape variable
Entity staticShape;

void initSDL(void){
    int rendererFlags, windowFlags;

    // Naming the window
    std::string windowName = "Static Shape Example";

    // Creating the window
    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = 0;

    // handle unsuccessful initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        exit(1);
    }

    // attempt to create the window
    app->window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

    // If the window pointer is null, we have an error
    if (!app->window) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // attempt to create the renderer
    app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

    // if the renderer pointer is null, we have an error
    if (!app->renderer) {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        exit(1);
    }

    // Initialize the static shape
    staticShape = {100, 100, 100, 100, {255, 0, 0, 255}}; // Red square at (100, 100) with 100x100 size

}