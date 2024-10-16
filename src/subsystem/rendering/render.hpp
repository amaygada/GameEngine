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
        void presentScene(const unordered_map<int, std::vector<Entity *>> &entityMap, int client_id, bool use_custom_renderer);
        void cleanup();
        // ~Renderer();
};

class ModularRenderer {
    public:
        Timeline *rendererTimeline;
        
        ModularRenderer();
        virtual void renderEntity(Entity *entity) = 0;
        virtual ~ModularRenderer() = default;  
};

class DefaultRenderer : public ModularRenderer{
    public:
        void renderEntity(Entity *entity) override;
};