#include "collision.hpp"

CollisionSubsystem::CollisionSubsystem(Timeline *collisionTimeline) {
    this->collisionSubsystemTimeline = collisionTimeline;
}

void CollisionSubsystem::doCollision(std::unordered_map<int, Entity*> &entity_map) {
    // Use two iterators to compare each entity with the others
    for (auto it1 = entity_map.begin(); it1 != entity_map.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != entity_map.end(); ++it2) {
            if (it1->second->checkCollision(*(it2->second))) {
                std::cout << "Collision detected between Entity " << it1->first << " and Entity " << it2->first << std::endl;
                exit(0);
            }
        }
    }

    this->customCollision(entity_map);
}


