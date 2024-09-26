#pragma once
#include "./../../utils/entity.hpp"
#include "./../../utils/defs.hpp"
#include "./../../utils/app.hpp"
#include <SDL2/SDL.h>
#include <unordered_map>
#include <iostream>
using namespace std;

class Entity; //forward declaration due to circular dependency

class InputSubsystem {
public:
    int PState[512];
    Timeline *inputSubsystemTimeline;

    InputSubsystem(Timeline *inputSubsystemTimeline);
    void doInput(std::unordered_map<int, std::vector<Entity *>> &entity_map);
    virtual void customInput(std::unordered_map<int, std::vector<Entity *>> &entity_map){};
};

// Abstract class for modular input handling
class ModularInputHandler{
public:
    int PState[512];
    ModularInputHandler();
    virtual void handleInput(Entity *entity)=0;
    virtual ~ModularInputHandler() = default;
};