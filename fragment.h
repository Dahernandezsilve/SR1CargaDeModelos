#pragma once
#include "glm/glm.hpp"

struct Fragment {
    glm::ivec2 position; // X and Y coordinates of the pixel (in screen space)
    Color color;
    float z;
    // Other interpolated attributes (e.g., color, texture coordinates, normals) can be added here
};