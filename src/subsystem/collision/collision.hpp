#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"
#include <unordered_map>


class CollisionSubsystem {
    public:
        Timeline *collisionSubsystemTimeline;
        CollisionSubsystem(Timeline *collisionTimeline);
        void doCollision(std::unordered_map<int, Entity*> &entity_map);
        virtual void customCollision(std::unordered_map<int, Entity*> &entity_map){};
};