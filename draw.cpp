#include "draw.hpp"

void drawEntity(Entity &entity) {
    SDL_Rect rect = {entity.x, entity.y, entity.w, entity.h};
    SDL_SetRenderDrawColor(app->renderer, entity.color.r, entity.color.g, entity.color.b, entity.color.a);
    SDL_RenderFillRect(app->renderer, &rect);
}

void prepareScene(void) {
    SDL_SetRenderDrawColor(app->renderer, 96, 128, 255, 255);
    SDL_RenderClear(app->renderer);

    // Draw the static shape
    drawEntity(staticShape);
}

void presentScene(void) {
    SDL_RenderPresent(app->renderer);
}