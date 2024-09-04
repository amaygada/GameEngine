#pragma once
#include "draw.hpp"

class Entity; // Forward declaration due to circular dependency

// Abstract class for modular physics handling
class ModularPhysicsHandler {
    public:
        virtual void updatePhysics(Entity *entity, double factor, double *time) = 0;
        virtual ~ModularPhysicsHandler() = default;
};

// Class for handling gravity physics on an Entity
class DefaultGravityPhysicsHandler : public ModularPhysicsHandler{
    public:
        void updatePhysics(Entity *entity, double factor, double *time) override;
};
