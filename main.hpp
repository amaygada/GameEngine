#pragma once
#include "src/utils/app.hpp"
#include "src/utils/entity.hpp"
#include "src/subsystem/input_handling/input.hpp"
#include "src/subsystem/physics/physics.hpp"
#include "src/subsystem/rendering/render.hpp"
#include "src/subsystem/collision/collision.hpp"
#include "src/subsystem/animation/animation.hpp"
#include "src/utils/timer.hpp"
#include "src/subsystem/connection/p2pclient.hpp"
#include "src/subsystem/connection/server.hpp"
#include "src/subsystem/connection/client.hpp"
#include <memory>
#include <vector>
#include <mutex>

#include "src/subsystem/event_manager/event_base.hpp"

// window context
extern App *app;

extern Renderer *renderer;
extern Timeline *globalTimeline;
extern Timeline *gameTimeline;
extern Timeline *inputTimeline;

int main(int argc, char *argv[]);

extern InputSubsystem *inputSubsystem;
extern PhysicsSubsystem *physicsSubsystem;
extern AnimationSubsystem *animationSubsystem;
extern CollisionSubsystem *collisionSubsystem;

extern std::mutex entity_mutex;

class JumpPhysicsHandler : public ModularPhysicsHandler{
    private:
        int y_initial = -1;
    public:
        JumpPhysicsHandler(bool input_allowed) : ModularPhysicsHandler(input_allowed) {}
        void handleInput(Entity *entity) override;
        void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) override;
};

class XPhysicsHandler : public ModularPhysicsHandler{
    public:
        XPhysicsHandler(bool input_allowed) : ModularPhysicsHandler(input_allowed) {}
        void handleInput(Entity *entity) override;
        void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) override;
};

class CharacterCollisionHandler : public ModularCollisionHandler{    
    public:
        void triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) override;
};

class BulletMovementHandler : public ModularPatternHandler{
    public:
        Server *server;
        BulletMovementHandler(Server *server) : ModularPatternHandler() {
            this->server = server;
        }
        void moveToPath(Entity *entity, int factor) override;
};

class BulletEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class SpawnEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class SideScrollingEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class GoRightEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class GoLeftEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class PlayerExitEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

extern EventManager *eventManager;