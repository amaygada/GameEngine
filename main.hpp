#pragma once
#include "struct.hpp"
#include "draw.hpp"
#include "init.hpp"
#include "input.hpp"
#include <memory>

// window context
extern App *app;

int main(int argc, char *argv[]);

class MyEntityInputHandler : public ModularInputHandler{
public:
    void handleInput(Entity *entity) override;
};