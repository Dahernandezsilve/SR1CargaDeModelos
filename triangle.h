#pragma once
#include "glm/glm.hpp"
#include <SDL2/SDL.h>
#include "line.h"

void triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, SDL_Renderer* renderer) {
    line(A, B, renderer);
    line(B, C, renderer);
    line(C, A, renderer);
}

