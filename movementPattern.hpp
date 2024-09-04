#pragma once
#include "draw.hpp"
#include <vector>

class Entity; // Forward declaration due to circular dependency

// Abstract class for modular entity pattern handling
class ModularPatternHandler {
    public:
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
