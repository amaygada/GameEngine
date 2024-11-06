#include "app.hpp"

App *app = new App();

// GLOBAL TIMELINE
Timeline *globalTimeline = new Timeline();

// GAME TIMELINE
Timeline *gameTimeline = new Timeline(globalTimeline, 1E9/GAMETIME_FREQ);

// ENGINE SUBSYSTEM TIMELINES
Timeline *inputSubsystemTimeline = new Timeline(globalTimeline, 1e9/INPUT_SUBSYSTEM_FREQ);
Timeline *physicsSubsystemTimeline = new Timeline(globalTimeline, 1e9/PHYSICS_SUBSYSTEM_FREQ);
Timeline *collisionSubsystemTimeline = new Timeline(globalTimeline, 1e9/COLLISION_SUBSYSTEM_FREQ);
Timeline *animationSubsystemTimeline = new Timeline(globalTimeline, 1e9/ANIMATION_SUBSYSTEM_FREQ);
// EVENT MANAGER TIMELINE
Timeline *eventManagerTimeline = new Timeline(globalTimeline, 1e9/EVENT_MANAGER_FREQ);

// Engine Subsystem objects
InputSubsystem *inputSubsystem = new InputSubsystem(inputSubsystemTimeline);
PhysicsSubsystem *physicsSubsystem = new PhysicsSubsystem(physicsSubsystemTimeline);
CollisionSubsystem *collisionSubsystem = new CollisionSubsystem(collisionSubsystemTimeline);
AnimationSubsystem *animationSubsystem = new AnimationSubsystem(animationSubsystemTimeline);

EventManager *eventManager = new EventManager(eventManagerTimeline);

// Mutex to lock entity updates
std::mutex entity_mutex;