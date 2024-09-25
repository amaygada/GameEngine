#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"


class CollisionSubsystem {
    public:
        Timeline *collisionSubsystemTimeline;
        CollisionSubsystem(Timeline *collisionTimeline);
        void doCollision(std::vector<Entity *> &E);
        virtual void customCollision(std::vector<Entity *> &E){};
};
