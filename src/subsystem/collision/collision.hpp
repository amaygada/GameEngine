#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"
#include <unordered_map>


class CollisionSubsystem {
    public:
        Timeline *collisionSubsystemTimeline;
        int64_t start_time = -1;
        
        CollisionSubsystem(Timeline *collisionTimeline);
        void doCollision(std::unordered_map<int, std::vector<Entity *>> &entity_map);
        virtual void customCollision(std::unordered_map<int, std::vector<Entity *>> &entity_map){};
};