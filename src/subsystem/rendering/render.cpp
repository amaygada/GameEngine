#include "render.hpp"

void Renderer::init(string windowName="Engine") {
    int rendererFlags, windowFlags;
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

void Renderer::getWindowSize(int *window_width, int *window_height){
    SDL_GetWindowSize(app->window, window_width, window_height);
}

// Function to prepare the scene by clearing the screen and drawing an Entity
// attach timer here
void Renderer::prepareScene() {
    SDL_SetRenderDrawColor(app->renderer, BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_O);  // Background color
    SDL_RenderClear(app->renderer);
}

// Function to present the scene
// attach timer here
void Renderer::presentScene(const unordered_map<int, std::vector<Entity *>> &entity_map) {
    // Draw the entities from the entity_map
    for (const auto pair : entity_map) {
        std::vector<Entity *> entities = pair.second;
        for (Entity *entity : entities) {
            entity->draw(app->renderer);
        }
    }
    SDL_RenderPresent(app->renderer);  // Present the final rendered scene
}

void Renderer::cleanup() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

Renderer *renderer;