#include "collision.hpp"

CollisionSubsystem::CollisionSubsystem(Timeline *collisionTimeline) {
    this->collisionSubsystemTimeline = collisionTimeline;
}

void CollisionSubsystem::doCollision(vector<Entity*> &E, std::unordered_map<int, std::vector<Entity *>> &entity_map) {
    if(this->start_time == -1) {
        this->start_time = collisionSubsystemTimeline->getTime();
    }
    int64_t currentTime = collisionSubsystemTimeline->getTime();
    if (currentTime - this->start_time < 1) return;
    this->start_time = currentTime;
    
    for(Entity *entity : E) {
        if (entity->collisionHandler != nullptr) {
            entity->collisionHandler->triggerPostCollide(entity, entity_map);
        }
    }

    this->customCollision(entity_map);
}

ModularCollisionHandler::ModularCollisionHandler() {
    this->collisionHandlerTimeline = new Timeline(gameTimeline, 1);
}

void DefaultCollisionHandler::triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entity_map) {
    if (this->collisionHandlerTimeline->isParentPaused()) return;

    for (auto it1 = entity_map.begin(); it1 != entity_map.end(); ++it1) {
        std::vector<Entity *> E1 = it1->second;
        for (Entity *e1 : E1) {
            if(e1 == entity) continue;
            if (e1->checkCollision(*entity)) {
                // Custom collision handling logic
                std:cout << "Collision detected between Entity " << it1->first << " and Entity " << entity << std::endl;
            }
        }   
    }
}