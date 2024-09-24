#pragma once
#include "./../../utils/entity.hpp"
#include <SDL2/SDL.h>
#include <vector>

class Entity; // Forward declaration due to circular dependency

class AnimationSubsystem {
    public:
        Timeline *animationSubsystemTimeline;
        AnimationSubsystem(Timeline *animationTimeline);
        void doAnimation(std::vector<Entity> &E);
        void customAnimation(std::vector<Entity> &E){};
};

// Abstract class for modular entity pattern handling
class ModularPatternHandler {
    public:
        Timeline *patternHandlerTimeline;
        int64_t start_time = -1;
        
        ModularPatternHandler();
        virtual void moveToPath(Entity *entity, int factor) = 0;
        virtual ~ModularPatternHandler() = default;  
        std::vector<SDL_Rect> path;
};

// Class for moving the entity towards the next point in the path, looping continuously through the path
class DefaultPatternHandler : public ModularPatternHandler{
    public:
        void moveToPath(Entity *entity, int factor) override;
        DefaultPatternHandler(std::vector<SDL_Rect> path);
};
