#include "input.hpp"
#include <iostream>

void doInput(void)
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_ESCAPE]){
        exit(0);
    }else if (state[SDL_SCANCODE_T] && state[SDL_SCANCODE_LSHIFT]){
        SDL_RenderSetLogicalSize(app->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    }else if (state[SDL_SCANCODE_T]){
        SDL_RenderSetLogicalSize(app->renderer, 0, 0);
    }

    // in code because removing this breaks the program
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;

        default:
            break;
        }
    }
}

void DefaultEntityInputHandler::handleInput(Entity *entity) {
    // Get the current state of the keyboard
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_W]) {
        // Move the entity up
        if( entity->y > 0) entity->y -= 1;
    }

    // If the 'S' key is pressed
    if (state[SDL_SCANCODE_S]) {
        // Move the entity down
        if( entity->y < SCREEN_HEIGHT - entity->h) entity->y += 1;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A]) {
        // Move the entity left
        if( entity->x > 0) entity->x -= 1;
    }

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D]) {
        // Move the entity right
        if( entity->x < SCREEN_WIDTH - entity->w) entity->x += 1;
    }
}