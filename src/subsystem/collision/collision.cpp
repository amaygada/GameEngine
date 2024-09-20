#include "collision.hpp"

CollisionSubsystem::CollisionSubsystem(Timeline *collisionTimeline) {
    this->collisionSubsystemTimeline = collisionTimeline;
}

void CollisionSubsystem::doCollision(std::vector<Entity> &E) {
    for (size_t i = 0; i < E.size(); ++i) {
        for (size_t j = i + 1; j < E.size(); ++j) {
            if (E[i].checkCollision(E[j])) {
                std::cout << "Collision detected between Entity " << i << " and Entity " << j << std::endl;
                exit(0);
            }
        }
    }

    this->customCollision(E);
}

