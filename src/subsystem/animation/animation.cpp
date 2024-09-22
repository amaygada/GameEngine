#include "animation.hpp"

AnimationSubsystem::AnimationSubsystem(Timeline *animationTimeline) {
    this->animationSubsystemTimeline = animationTimeline;
}

void AnimationSubsystem::doAnimation(std::vector<Entity *> &E) {
    for (Entity *entity : E) {
        if (entity->patternHandler != nullptr) {
            entity->patternHandler->moveToPath(entity, 1);
        }
    }

    this->customAnimation(E);
}

// Construct the pattern handler with the given path (pattern) to follow
DefaultPatternHandler::DefaultPatternHandler(std::vector<SDL_Rect> path) {
    for (SDL_Rect point : path) {
        this->path.push_back(point);
    }
}

// Move the entity towards its current target in the path
void DefaultPatternHandler::moveToPath(Entity *entity, int factor) {
    if (path.size() == 0) {
        return;
    }

    // Get the current target of the path
    SDL_Rect current;
    // Construct a bounding box of the entity
    SDL_Rect entityBounds = {entity->x, entity->y, entity->w, entity->h};

    const int pathSize = path.size();
    for (int i = 0; i < pathSize; i++) {
        current = path.at(0);

        // If the entity happens to already be intersecting with its current target...
        if (SDL_HasIntersection(&entityBounds, &current)) {
            // Remove the current target
            path.erase(path.begin());
            // Add it to the end of the path (this is a never-ending, looping path)
            path.push_back(current);
            // Set the current target to be the next point in the path
            current = path.at(0);
        }
        // If we have a point in the path that the entity is not intersecting with, break out of the loop!
        else {
            break;
        }
    }

    // Calculate the direction of the current target
    int xDirection = current.x - entityBounds.x;
    int yDirection = current.y - entityBounds.y; // TODO make sure it's not the other way around because of opposite y-coordinate system

    int futureX = entityBounds.x;
    int futureY = entityBounds.y;
    if (xDirection > 0) {
        futureX += factor;
    }
    else if (xDirection < 0) {
        futureX -= factor;
    }
    if (yDirection > 0) {
        futureY += factor;
    }
    else if (yDirection < 0) {
        futureY -= factor;
    }

    entity->x = futureX;
    entity->y = futureY;
}
