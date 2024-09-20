#include "app.hpp"

App *app = new App();

// GLOBAL TIMELINE
Timeline *globalTimeline = new Timeline();

// GAME TIMELINE
Timeline *gameTimeline = new Timeline(globalTimeline, 1E9/GAMETIME_FREQ);

// ENGINE SUBSYSTEM TIMELINES
Timeline *inputSubsystemTimeline = new Timeline(globalTimeline, 1e8);
Timeline *physicsSubsystemTimeline = new Timeline(globalTimeline, 1e9/GAMETIME_FREQ);
Timeline *collisionSubsystemTimeline = new Timeline(globalTimeline, 1e9/GAMETIME_FREQ);

// Engine Subsystem objects
InputSubsystem *inputSubsystem = new InputSubsystem(inputSubsystemTimeline);
PhysicsSubsystem *physicsSubsystem = new PhysicsSubsystem(physicsSubsystemTimeline);
CollisionSubsystem *collisionSubsystem = new CollisionSubsystem(collisionSubsystemTimeline);
AnimationSubsystem *animationSubsystem = new AnimationSubsystem(gameTimeline);