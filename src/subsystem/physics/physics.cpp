#include "physics.hpp"

PhysicsSubsystem::PhysicsSubsystem(Timeline *physicsTimeline) {
    this->physicsSubsystemTimeline = physicsTimeline;
}

void PhysicsSubsystem::doPhysics(std::vector<Entity> &E) {
    for (auto &entity : E) {
        if (entity.physicsHandler != nullptr){
            if (entity.physicsHandler->input_allowed) entity.physicsHandler->handleInput(&entity);
            else entity.physicsHandler->updatePhysics(&entity, 0, 0, 0, PHYS_GRAVITY_CONSTANT, -1);
        }
    }

    this->customPhysics(E);
}

ModularPhysicsHandler::ModularPhysicsHandler(bool input_allowed) {
    this->physicsTimeline = new Timeline(gameTimeline, 1);
    if (input_allowed) {
        this->input_allowed = true;    
        for (int i = 0; i < 512; i++) {
            this->PState[i] = 0;
        }
    }
}

// Apply gravity to entity
void DefaultGravityPhysicsHandler::updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    // if timer is paused, resume it
    if (this->physicsTimeline->isParentPaused()) return;

    if (this->start_time == -1) {
        this->start_time = physicsTimeline->getTime();
    }

    int timeValue = int(this->start_time - physicsTimeline->getTime());

    int current_loc = entity->y;
    int to_be_loc = (0.5 * acceleration_y * (timeValue * timeValue));
    int locDifference = to_be_loc - current_loc;

    if (entity->y + entity->h + locDifference >= SCREEN_HEIGHT) {
        entity->y = SCREEN_HEIGHT - entity->h;
    }
    else {
        entity->y += locDifference;
    }
}

// Handle input for entity
void DefaultMovementPhysicsHandler::handleInput(Entity *entity){
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    bool parent_paused = this->physicsTimeline->isParentPaused();

    if(parent_paused) return;

    // If the 'D' key is pressed
    if (state[SDL_SCANCODE_D] && !this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->resume();
        this->updatePhysics(entity, 1, 0, 1, 0, 1);
    }else if (state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        this->updatePhysics(entity, 1, 0, 1, 0, 1);
    }else if (!state[SDL_SCANCODE_D] && this->PState[SDL_SCANCODE_D]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
    }

    // If the 'A' key is pressed
    if (state[SDL_SCANCODE_A] && !this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->resume();
        this->updatePhysics(entity, 1, 0, 1, 0, -1);
    }else if (state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        this->updatePhysics(entity, 1, 0, 1, 0, -1);
    }else if (!state[SDL_SCANCODE_A] && this->PState[SDL_SCANCODE_A]) {
        this->physicsTimeline->pause();
        this->start_time = -1;
    }

    // If the 'W' key is pressed
    if (state[SDL_SCANCODE_W] && !this->PState[SDL_SCANCODE_W]) {
        entity->physicsHandler->physicsTimeline->resume();
        entity->physicsHandler->updatePhysics(entity, 0, 1, 0, 1, -1);
    }else if (state[SDL_SCANCODE_W] && this->PState[SDL_SCANCODE_W]) {
        entity->physicsHandler->updatePhysics(entity, 0, 1, 0, 1, -1);
    }else if (!state[SDL_SCANCODE_W] && this->PState[SDL_SCANCODE_W]) {
        entity->physicsHandler->physicsTimeline->pause();
        entity->physicsHandler->start_time = -1;
    }

    // If the 'S' key is pressed
    if (state[SDL_SCANCODE_S] && !this->PState[SDL_SCANCODE_S]) {
        entity->physicsHandler->physicsTimeline->resume();
        entity->physicsHandler->updatePhysics(entity, 0, 1, 0, 1, 1);
    }else if (state[SDL_SCANCODE_S] && this->PState[SDL_SCANCODE_S]) {
        entity->physicsHandler->updatePhysics(entity, 0, 1, 0, 1, 1);
    }else if (!state[SDL_SCANCODE_S] && this->PState[SDL_SCANCODE_S]) {
        entity->physicsHandler->physicsTimeline->pause();
        entity->physicsHandler->start_time = -1;
    }

    for (int i = 0; i < 512; i++) {
        this->PState[i] = state[i];
    }
}

// Apply movement to entity
void DefaultMovementPhysicsHandler::updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {
    // if timer is paused, resume it
    if (this->physicsTimeline->isParentPaused()) return;

    if (this->physicsTimeline->isPaused()) return;

    if (this->start_time == -1) {
        this->start_time = physicsTimeline->getTime();
    }

    int timeValue = int(physicsTimeline->getTime() - this->start_time);

    int current_loc = entity->x;
    int to_be_loc = velocity_x + (0.5 * acceleration_x * (timeValue * timeValue));

    int locDifference = direction * (to_be_loc);

    if (entity->x + entity->w + locDifference >= SCREEN_WIDTH) {
        entity->x = SCREEN_WIDTH - entity->w;
    }else if(entity->x + locDifference <= 0){
        entity->x = 0;
    }else {
        entity->x += locDifference;
    }

    current_loc = entity->y;
    to_be_loc = velocity_y + (0.5 * acceleration_y * (timeValue * timeValue));
    locDifference = direction * (to_be_loc);

    if (entity->y + entity->h + locDifference >= SCREEN_HEIGHT) {
        entity->y = SCREEN_HEIGHT - entity->h;
    }else if(entity->y + locDifference <= 0){
        entity->y = 0;
    }else {
        entity->y += locDifference;
    }

}