#include "text.hpp"

Text::Text(std::string text, int x, int y, int w, int h, SDL_Color text_color) {
    this->text = text;
    this->textRect.x = x;
    this->textRect.y = y;
    this->textRect.w = w;
    this->textRect.h = h;
    this->text_color = text_color;
}

std::string Text::getText() {
    return this->text;
}

void Text::setText(std::string text) {
    this->text = text;
    this->textSurface = TTF_RenderText_Solid(font, text.c_str(), text_color);
    this->textTexture = SDL_CreateTextureFromSurface(app->renderer, textSurface);
    SDL_FreeSurface(textSurface);
}

void Text::renderText() {
    SDL_Color textColor = {0, 0, 0, 255}; // White color
    textSurface = TTF_RenderText_Solid(app->font, text.c_str(), textColor);
    textTexture = SDL_CreateTextureFromSurface(app->renderer, textSurface);
    SDL_FreeSurface(textSurface); // Free the surface once we have the texture
    SDL_RenderCopy(app->renderer, textTexture, NULL, &textRect);
}

void Text::cleanup() {
    // SDL_FreeSurface(textSurface);
    // SDL_DestroyTexture(textTexture);
    // TTF_CloseFont(font);
}