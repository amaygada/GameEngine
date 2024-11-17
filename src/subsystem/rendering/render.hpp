#pragma once
#include "./../../utils/app.hpp"
#include "./../../utils/defs.hpp"
#include "./../../utils/entity.hpp"
#include "./../../utils/timer.hpp"
#include "./../../subsystem/event_manager/event_base.hpp"
#include "./../../utils/event.hpp"
#include "./../../subsystem/connection/message.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

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

extern EventManager *eventManager;