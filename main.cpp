#include <SDL2/SDL.h>
#include <cstdint>
#include <ctime>
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <array>

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

void line(const glm::vec3& start, const glm::vec3& end) {
    SDL_RenderDrawLine(renderer, static_cast<int>(start.x), static_cast<int>(start.y),
                       static_cast<int>(end.x), static_cast<int>(end.y));
}

void triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    line(A, B);
    line(B, C);
    line(C, A);
}

std::vector<glm::vec3> vertices = {
        {1.125986f, 0.781798f, -1.058748f},
        {0.074502f, -0.860990f, -1.501048f},
        {1.568286f, 0.015753f, 0.735004f},
        {0.516802f, -1.627034f, 0.292704f},
        {-0.516802f, 1.627034f, -0.292704f},
        {-1.568286f, -0.015753f, -0.735004f},
        {-0.074502f, 0.860990f, 1.501048f},
        {-1.125986f, -0.781798f, 1.058748f}
};

std::vector<std::array<int, 3>> faces = {
        { {5, 3, 1} },
        { {3, 8, 4} },
        { {7, 6, 8} },
        { {2, 8, 6} },
        { {1, 4, 2} },
        { {5, 2, 6} },
        { {5, 7, 3} },
        { {3, 7, 8} },
        { {7, 5, 6} },
        { {2, 4, 8} },
        { {1, 3, 4} },
        { {5, 1, 2} }
};

void render() {
    glm::vec3 projection(400.0f, 400.0f, 0.0f);
    float scale = 100.0f;

    for (const auto& face : faces) {
        const glm::vec3& A = vertices[face[0] - 1];
        const glm::vec3& B = vertices[face[1] - 1];
        const glm::vec3& C = vertices[face[2] - 1];

        glm::vec3 A_projected = projection + A * scale;
        glm::vec3 B_projected = projection + B * scale;
        glm::vec3 C_projected = projection + C * scale;

        triangle(A_projected, B_projected, C_projected);
    }
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

        render(); // Llamada a la función render para mostrar el cubo triangulado
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
