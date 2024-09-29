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

void InputSubsystem::doInput(vector<Entity*>& E) {\
    if(this->start_time == -1) this->start_time = inputSubsystemTimeline->getTime();
    int64_t currentTime = inputSubsystemTimeline->getTime();
    if (currentTime - this->start_time < 1) return;
    this->start_time = currentTime;

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

    if (state[SDL_SCANCODE_J]) { // 0.5 speed
        gameTimeline->changeTic(1E9/GAMETIME_FREQ * 2.0);
    }
    else if (state[SDL_SCANCODE_K]) { // 1.0 speed (default speed)
        gameTimeline->changeTic(1E9/GAMETIME_FREQ);
    }
    else if (state[SDL_SCANCODE_L]) { // 2.0 speed
        gameTimeline->changeTic(1E9/GAMETIME_FREQ * 0.5);
    }

    this->customInput(E);

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