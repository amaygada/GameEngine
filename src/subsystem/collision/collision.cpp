#include "collision.hpp"

CollisionSubsystem::CollisionSubsystem(Timeline *collisionTimeline) {
    this->collisionSubsystemTimeline = collisionTimeline;
}

void CollisionSubsystem::doCollision(std::unordered_map<int, std::vector<Entity *>> &entity_map) {
    // Use two iterators to compare each entity with the others
    for (auto it1 = entity_map.begin(); it1 != entity_map.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != entity_map.end(); ++it2) {

            std::vector<Entity *> E1 = it1->second;
            std::vector<Entity *> E2 = it2->second;

            for (Entity *e1 : E1) {

                for (Entity *e2 : E2) {

                    if (e1->checkCollision(*e2)) {

                        // std::cout << "Collision detected between Entity " << it1->first << " and Entity " << it2->first << std::endl;

                    }

                }

            }

        }
        
    }

    this->customCollision(entity_map);
}