#pragma once
#include "./../../utils/app.hpp"
#include "./../../utils/defs.hpp"
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

// window context
extern App *app;

class Renderer {
    public:
        // Renderer();
        void init(string windowName);
        void getWindowSize(int *window_width, int *window_height);
        void prepareScene(void);
        void presentScene(const unordered_map<int, Entity*> &entityMap);
        void cleanup();
        // ~Renderer();
};