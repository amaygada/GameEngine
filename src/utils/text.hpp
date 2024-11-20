#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

#include "app.hpp"

class Text {
    SDL_Color text_color;
    TTF_Font* font;
    SDL_Rect textRect;
    std::string text;
    SDL_Surface* textSurface;
    SDL_Texture* textTexture;

    public:
        Text(std::string text, int x, int y, int w, int h, SDL_Color text_color);
        std::string getText();
        void setText(std::string text);
        void renderText();
        void cleanup();
};