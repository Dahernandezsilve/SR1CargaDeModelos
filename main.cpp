#include <SDL2/SDL.h>
#include <ctime>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "color.h"
#include <vector>
#include "loadObj.h"
#include "vertexArray.h"
#include "uniform.h"
#include "shaders.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

Color clearColor = {0, 0, 0, 255};
Color currentColor = {255, 255, 255, 255};
Color colorA(255, 0, 0, 255); // Red color
Color colorB(0, 255, 255, 255); // Green color
Color colorC(0, 24, 255, 255); // Blue color
glm::vec3 L = glm::vec3(0, 0, 200.0f); // Dirección de la luz en el espacio del ojo

Uniforms uniforms;
SDL_Renderer* renderer;

std::array<double, WINDOW_WIDTH * WINDOW_HEIGHT> zbuffer;

Color interpolateColor(const glm::vec3& barycentricCoord, const Color& colorA, const Color& colorB, const Color& colorC) {
    float u = barycentricCoord.x;
    float v = barycentricCoord.y;
    float w = barycentricCoord.z;

    // Realiza una interpolación lineal para cada componente del color
    uint8_t r = static_cast<uint8_t>(u * colorA.r + v * colorB.r + w * colorC.r);
    uint8_t g = static_cast<uint8_t>(u * colorA.g + v * colorB.g + w * colorC.g);
    uint8_t b = static_cast<uint8_t>(u * colorA.b + v * colorB.b + w * colorC.b);
    uint8_t a = static_cast<uint8_t>(u * colorA.a + v * colorB.a + w * colorC.a);

    return Color(r, g, b, a);
}

bool isBarycentricCoordInsideTriangle(const glm::vec3& barycentricCoord) {
    return barycentricCoord.x >= 0 && barycentricCoord.y >= 0 && barycentricCoord.z >= 0 &&
           barycentricCoord.x <= 1 && barycentricCoord.y <= 1 && barycentricCoord.z <= 1 &&
           glm::abs(1 - (barycentricCoord.x + barycentricCoord.y + barycentricCoord.z)) < 0.00001f;
}

glm::vec3 calculateBarycentricCoord(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& P) {
    float denominator = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    float u = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
    float v = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
    float w = 1 - u - v;
    return glm::vec3(u, v, w);
}

std::vector<Fragment> triangle(const Vertex& a, const Vertex& b, const Vertex& c) {
    std::vector<Fragment> fragments;

    // Calculate the bounding box of the triangle
    int minX = static_cast<int>(std::min({a.position.x, b.position.x, c.position.x}));
    int minY = static_cast<int>(std::min({a.position.y, b.position.y, c.position.y}));
    int maxX = static_cast<int>(std::max({a.position.x, b.position.x, c.position.x}));
    int maxY = static_cast<int>(std::max({a.position.y, b.position.y, c.position.y}));

    // Iterate over each point in the bounding box
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // Central point of the pixel
            glm::vec3 barycentricCoord = calculateBarycentricCoord(a.position, b.position, c.position, pixelPosition);

            if (isBarycentricCoordInsideTriangle(barycentricCoord)) {
                Color p {0, 0, 0};
                // Interpolate attributes (color, depth, etc.) using barycentric coordinates
                Color interpolatedColor = interpolateColor(barycentricCoord, p, p, p);

                // Calculate the interpolated Z value using barycentric coordinates
                float interpolatedZ = barycentricCoord.x * a.position.z + barycentricCoord.y * b.position.z + barycentricCoord.z * c.position.z;

                // Create a fragment with the position, interpolated attributes, and Z coordinate
                Fragment fragment;
                fragment.position = glm::ivec2(x, y);
                fragment.color = interpolatedColor;
                fragment.z = interpolatedZ;

                fragments.push_back(fragment);
            }
        }
    }

    return fragments;
}

void render(const std::vector<Vertex>& vertexArray,  const Uniforms& uniforms) {
    std::vector<Vertex> transformedVertexArray;
    for (const auto& vertex : vertexArray) {
        auto transformedVertex = vertexShader(vertex, uniforms);
        transformedVertexArray.push_back(transformedVertex);
    }

    // Clear z-buffer at the beginning of each frame
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<double>::max());


    for (size_t i = 0; i < transformedVertexArray.size(); i += 3) {
        const Vertex& a = transformedVertexArray[i];
        const Vertex& b = transformedVertexArray[i + 1];
        const Vertex& c = transformedVertexArray[i + 2];

        glm::vec3 A = a.position;
        glm::vec3 B = b.position;
        glm::vec3 C = c.position;

        // Calculate the bounding box of the triangle
        int minX = static_cast<int>(std::min({A.x, B.x, C.x}));
        int minY = static_cast<int>(std::min({A.y, B.y, C.y}));
        int maxX = static_cast<int>(std::max({A.x, B.x, C.x}));
        int maxY = static_cast<int>(std::max({A.y, B.y, C.y}));

        // Iterate over each point in the bounding box
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // Central point of the pixel
                glm::vec3 barycentricCoord = calculateBarycentricCoord(A, B, C, pixelPosition);

                if (isBarycentricCoordInsideTriangle(barycentricCoord)) {
                    Color g {200,0,0};
                    // Interpolate attributes (color, depth, etc.) using barycentric coordinates
                    Color interpolatedColor = interpolateColor(barycentricCoord, g, g, g);

                    // Calculate the depth (Z-coordinate) of the fragment using barycentric coordinates
                    float depth = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;


                    glm::vec3 normal = a.normal * barycentricCoord.x + b.normal * barycentricCoord.y+ c.normal * barycentricCoord.z;

                    float fragmentIntensity = glm::dot(normal, glm::vec3 (0,0,1.0f));
                    // Apply fragment shader to calculate final color with shading
                    Color finalColor = interpolatedColor * fragmentIntensity;

                    // Create a fragment with the position, interpolated attributes, and depth
                    Fragment fragment;
                    fragment.position = glm::ivec2(x, y);
                    fragment.color = finalColor;
                    fragment.z = depth;  // Set the depth of the fragment

                    // Check if the fragment is closer than the stored value in the z-buffer
                    int index = y * WINDOW_WIDTH + x;
                    if (depth < zbuffer[index]) {
                        // Apply fragment shader to calculate final color
                        Color fragmentShaderf = fragmentShader(fragment);

                        // Draw the fragment using SDL_SetRenderDrawColor and SDL_RenderDrawPoint
                        SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                        SDL_RenderDrawPoint(renderer, x, y);

                        // Update the z-buffer value for this pixel
                        zbuffer[index] = depth;
                    }
                }
            }
        }
    }
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 45.0f;
    float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

float a = 3.14f / 3.0f;

glm::mat4 createModelMatrix() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)), glm::vec3(0, 1.0f, 0.0f));
    return transtation * scale * rotation;
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);

    glm::vec3 translation(0.0f, 0.0f, 0.0f); // Definir la posición del objeto en el mundo
    glm::vec3 rotationAngles(0.0f, 0.0f, 0.0f); // Ángulos de rotación en los ejes X, Y y Z (en grados)
    glm::vec3 scale(1.0f, 1.0f, 1.0f);


    glm::vec3 cameraPosition(0.0f, 0.0f, 5.0f); // Mueve la cámara hacia atrás
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
    glm::vec3 upVector(0.0f, 1.0f, 0.0f);

    uniforms.view = glm::lookAt(cameraPosition, targetPosition, upVector);

    srand(time(nullptr));

    SDL_Window* window = SDL_CreateWindow("Pixel Drawer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normal;
    std::vector<Face> faces;

    bool success = loadOBJ("../Nave6.obj", vertices, normal, faces);
    if (!success) {
        // Manejo del error si la carga del archivo falla
        return 1;
    }

// Aplicar una rotación adicional de 180 grados alrededor del eje X para invertir la nave
    glm::mat4 additionalRotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    for (glm::vec3& vertex : vertices) {
        vertex = glm::vec3(additionalRotation * glm::vec4(vertex, 1.0f));
    }

    std::vector<Vertex> vertexArray = setupVertexArray(vertices, normal, faces);

    bool running = true;
    SDL_Event event;
    glm::mat4 rotationMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación
    glm::mat4 traslateMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación
    glm::mat4 scaleMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        uniforms.model = createModelMatrix();    // Asignar la matriz de proyección a uniforms.projection
        uniforms.projection = createProjectionMatrix();
        uniforms.viewport = createViewportMatrix();


        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);

        glm::vec4 transformedLight = glm::inverse(createModelMatrix()) * glm::vec4(L, 0.0f);
        glm::vec3 transformedLightDirection = glm::normalize(glm::vec3(transformedLight));

        // Llamada a la función render con la matriz de vértices transformados
        render(vertexArray, uniforms);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}