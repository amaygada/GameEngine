#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"
#include "./../../utils/event.hpp"
#include "./../../utils/app.hpp"
#include <unordered_map>
#include <vector>


class CollisionSubsystem {
    public:
        Timeline *collisionSubsystemTimeline;
        int64_t start_time = -1;
        
        CollisionSubsystem(Timeline *collisionTimeline);
        void doCollision(std::vector<Entity*> &E, std::unordered_map<int, std::vector<Entity *>> &entity_map);
        virtual void customCollision(std::unordered_map<int, std::vector<Entity *>> &entity_map){};
};

// Abstract class for modular entity pattern handling
class ModularCollisionHandler {
    public:
        Timeline *collisionHandlerTimeline;
        int64_t start_time = -1;
        
        ModularCollisionHandler();
        virtual void triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) = 0;
        virtual ~ModularCollisionHandler() = default;
};

// Class for moving the entity towards the next point in the path, looping continuously through the path
class DefaultCollisionHandler : public ModularCollisionHandler{
    public:
        void triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) override;
};

extern EventManager *eventManager;