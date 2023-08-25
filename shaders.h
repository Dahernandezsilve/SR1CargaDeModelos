#pragma once
#include "glm/glm.hpp"
#include "uniform.h"
#include "fragment.h"
#include "color.h"

void printMatrix(const glm::mat4& myMatrix) {
    // Imprimir los valores de la matriz
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << myMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// Función para imprimir un vec4
void printVec4(const glm::vec4& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
}

// Función para imprimir un vec3
void printVec3(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
}


Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    glm::vec4 transformedVertex = uniforms.viewport * uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);
    // printMatrix(uniforms.viewport);
    // printMatrix(uniforms.projection);
    // printMatrix(uniforms.view);
    // nice printMatrix(uniforms.model);
    // printVec4(transformedVertex);
    glm::vec3 vertexRedux;
    vertexRedux.x = transformedVertex.x / transformedVertex.w;
    vertexRedux.y = transformedVertex.y / transformedVertex.w;
    vertexRedux.z = transformedVertex.z / transformedVertex.w;
    // Calculate the color for the fragment based on some criteria (e.g., lighting, textures, etc.)
    Color fragmentColor(255, 0, 0, 255); // Example color (red)
    glm::vec3 normal = glm::normalize(glm::mat3(uniforms.model) * vertex.normal);
    // Create a fragment and assign its attributes
    Fragment fragment;
    fragment.position = glm::ivec2(transformedVertex.x, transformedVertex.y);
    fragment.color = fragmentColor;
    // printVec3(vertexRedux);
    // Return the transformed vertex as a vec3
    return Vertex {vertexRedux, normal};
}


Color fragmentShader(const Fragment& fragment) {
    // Example: Assign a constant color to each fragment
    // You can modify this function to implement more complex shading
    // based on the fragment's attributes (e.g., depth, interpolated normals, texture coordinates, etc.)

    return fragment.color;
}