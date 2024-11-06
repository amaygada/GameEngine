#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/defs.hpp"
#include "./../../utils/timer.hpp"
#include "./../../utils/event.hpp"
#include "./../event_manager/event_base.hpp"
#include <vector>

class Entity; // Forward declaration due to circular dependency

class PhysicsSubsystem{
    public:
        Timeline *physicsSubsystemTimeline;
        int64_t start_time = -1;
        
        PhysicsSubsystem(Timeline *physicsTimeline);
        void doPhysics(std::vector<Entity*>& E);
        virtual void customPhysics(std::vector<Entity*>& E){};
};


// Abstract class for modular physics handling
class ModularPhysicsHandler {
    public:
        Timeline *physicsTimeline;
        int PState[512];
        int64_t start_time = -1;
        bool input_allowed = false;

        ModularPhysicsHandler(bool input_allowed);
        virtual void handleInput(Entity *entity){};
        virtual void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) = 0;
        virtual ~ModularPhysicsHandler() = default;
};

// Class for handling gravity physics on an Entity
class DefaultGravityPhysicsHandler : public ModularPhysicsHandler{
    public:
        DefaultGravityPhysicsHandler(bool input_allowed) : ModularPhysicsHandler(input_allowed) {}
        void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) override;
};

class DefaultMovementPhysicsHandler : public ModularPhysicsHandler{
    public:
        DefaultMovementPhysicsHandler(bool input_allowed) : ModularPhysicsHandler(input_allowed) {}
        void handleInput(Entity *entity) override;
        void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) override;
};

extern Timeline *gameTimeline;
extern std::mutex entity_mutex;

extern Event *defaultPhysicsEventRight;
extern Event *defaultPhysicsEventLeft;
extern Event *defaultPhysicsEventUp;
extern Event *defaultPhysicsEventDown;
extern EventManager *eventManager;