#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/defs.hpp"
#include "./../../utils/app.hpp"
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
using namespace std;

class Entity; //forward declaration due to circular dependency

class InputSubsystem {
public:
    int PState[512];
    Timeline *inputSubsystemTimeline;
    int64_t start_time = -1;

    InputSubsystem(Timeline *inputSubsystemTimeline);
    void doInput(vector<Entity*>& E);
    virtual void customInput(vector<Entity*>& E){};
};

// Abstract class for modular input handling
class ModularInputHandler{
public:
    int PState[512];
    ModularInputHandler();
    virtual void handleInput(Entity *entity)=0;
    virtual ~ModularInputHandler() = default;
};