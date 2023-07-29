#include <SDL2/SDL.h>
#include <cstdint>
#include <ctime>
#include "glm/glm.hpp"
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

class Color {
public:
    uint8_t r, g, b, a;
};

Color clearColor = {0, 0, 0, 255};
Color currentColor = {255, 255, 255, 255};

SDL_Renderer* renderer;

void point(int x, int y) {
    SDL_RenderDrawPoint(renderer, x, y);
}

void render() {
    point(10, 10);
}

Color getRandomColor() {
    return {static_cast<uint8_t>(rand() % 256),
            static_cast<uint8_t>(rand() % 256),
            static_cast<uint8_t>(rand() % 256),
            255};
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    srand(time(nullptr));

    SDL_Window* window = SDL_CreateWindow("Pixel Drawer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);


    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);

        render(); // Llamada a la función render para mostrar los puntos
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
