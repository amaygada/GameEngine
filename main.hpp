#pragma once
#include "src/utils/app.hpp"
#include "src/utils/entity.hpp"
#include "src/subsystem/input_handling/input.hpp"
#include "src/subsystem/physics/physics.hpp"
#include "src/subsystem/rendering/render.hpp"
#include "src/subsystem/collision/collision.hpp"
// #include "src/subsystem/animation/animation.hpp"
#include "src/utils/timer.hpp"
#include "src/utils/serverClient.hpp"
#include <memory>

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
