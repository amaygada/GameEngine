#include "init.hpp"
#include <vector>

void initSDL(){
    int rendererFlags, windowFlags;

    // Naming the window
    std::string windowName = "HW1 Engine";

    // Creating the window
    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = SDL_WINDOW_RESIZABLE;

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

}

void getWindowSize(int *window_width, int *window_height){
    SDL_GetWindowSize(app->window, window_width, window_height);
}