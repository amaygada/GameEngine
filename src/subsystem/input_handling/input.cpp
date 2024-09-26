#include "input.hpp"

ModularInputHandler::ModularInputHandler(){
    // memset(PState, 0, sizeof(PState));
    for (int i = 0; i < 512; i++) {
        this->PState[i] = 0;
    }
}

InputSubsystem::InputSubsystem(Timeline *inputSubsystemTimeline) {
    this->inputSubsystemTimeline = inputSubsystemTimeline;
    for (int i = 0; i < 512; i++) {
        this->PState[i] = 0;
    }
}

void InputSubsystem::doInput(unordered_map<int, Entity*> &entity_map) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_ESCAPE]){
        exit(0);
    }else if (state[SDL_SCANCODE_T] && state[SDL_SCANCODE_LSHIFT]){
        SDL_RenderSetLogicalSize(app->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    }else if (state[SDL_SCANCODE_T]){
        SDL_RenderSetLogicalSize(app->renderer, 0, 0);
    }

    if (state[SDL_SCANCODE_SPACE] && !this->PState[SDL_SCANCODE_SPACE]) {
        if(gameTimeline->isPaused()) gameTimeline->resume();
        else gameTimeline->pause();
    }else if (state[SDL_SCANCODE_SPACE] && this->PState[SDL_SCANCODE_SPACE]) {

    }else if (!state[SDL_SCANCODE_SPACE] && this->PState[SDL_SCANCODE_SPACE]) {

    }

    this->customInput(entity_map);

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }

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