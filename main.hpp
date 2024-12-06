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
#include "src/subsystem/event_manager/event_base.hpp"
#include "src/utils/text.hpp"

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <map>

extern App *app;
extern Renderer *renderer;
extern Timeline *globalTimeline;
extern Timeline *gameTimeline;
extern Timeline *inputTimeline;
extern InputSubsystem *inputSubsystem;
extern PhysicsSubsystem *physicsSubsystem;
extern AnimationSubsystem *animationSubsystem;
extern CollisionSubsystem *collisionSubsystem;
extern std::mutex entity_mutex;
extern EventManager *eventManager;

int main(int argc, char *argv[]);

// PHYSICS HANDLERS
class XPhysicsHandler : public ModularPhysicsHandler{
    public:
        XPhysicsHandler(bool input_allowed) : ModularPhysicsHandler(input_allowed) {}
        void handleInput(Entity *entity) override;
        void updatePhysics(Entity *entity, double velocity_x, double velocity_y, double acceleration_x, double acceleration_y, int direction) {};
};

// ANIMATION HANDLERS
class BulletMovementHandler : public ModularPatternHandler{
    public:
        void moveToPath(Entity *entity, int factor) override;
};

class EnemyBulletMovementHandler : public ModularPatternHandler{
    public:
        int enemyId;
        void moveToPath(Entity *entity, int factor) override;
};

// COLLISION HANDLERS
class CharacterCollisionHandler : public ModularCollisionHandler{    
    public:
        void triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) override;
};

class CharacterBulletCollisionHandler : public ModularCollisionHandler{    
    public:
        void triggerPostCollide(Entity *entity, std::unordered_map<int, std::vector<Entity *>> &entityMap) override;
};

// CUSTOM SERVER CLASS
class CustomServer : public Server {
    public:
        void handleCustomRequest(string message) override;
};


// EVENT HANDLERS
class GoRightEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class GoLeftEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class ShootBulletCharacterEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

class ShootBulletEnemyEventHandler : public EventHandler {
    public:
        void onEvent(Event e) override;
};

void InputSubsystem::customInput(vector<Entity*>& E){};

class EnemyMovementHandler : public ModularPatternHandler {
public:
    int direction = 1; // 1 for right, -1 for left
    int stepDown = 10; // Pixels to move down when changing direction

    void moveToPath(Entity* entity, int factor) override;
};