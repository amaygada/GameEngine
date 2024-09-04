#pragma once
#include "draw.hpp"
#include <SDL2/SDL.h>
using namespace std;

void doInput(void);

class Entity; //forward declaration due to circular dependency

// Abstract class for modular input handling
class ModularInputHandler{
public:
    virtual void handleInput(Entity *entity)=0;
    virtual ~ModularInputHandler() = default;
};

class DefaultEntityInputHandler : public ModularInputHandler{
public:
    void handleInput(Entity *entity) override;
};