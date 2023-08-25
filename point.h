#pragma once
#include <SDL2/SDL.h>

void point(int x, int y, SDL_Renderer* renderer) {
    SDL_RenderDrawPoint(renderer, x, y);
}