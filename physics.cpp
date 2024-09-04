#include "physics.hpp"

// Apply gravity to entity
void DefaultGravityPhysicsHandler::updatePhysics(Entity *entity, double factor, double *time) {
    long timeValue = *time;
    long prevTimeValue = (*time) - 1;

    int locDifference = (0.5 * factor * (timeValue * timeValue)) - (0.5 * factor * (prevTimeValue * prevTimeValue));

    if (entity->y + entity->h + locDifference >= SCREEN_HEIGHT) {
        entity->y = SCREEN_HEIGHT - entity->h;
    }
    else {
        entity->y += locDifference;
    }
}
