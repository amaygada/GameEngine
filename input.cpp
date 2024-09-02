#include "input.hpp"

void doInput(void)
{

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
        entity->y -= 1;
    }

    // If the 'S' key is pressed
    if (state[SDL_SCANCODE_S]) {
        // Move the entity down
        entity->y += 1;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A]) {
        // Move the entity left
        entity->x -= 1;
    }

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D]) {
        // Move the entity right
        entity->x += 1;
    }
}