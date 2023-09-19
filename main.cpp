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

Uint32 frameStart;      // Tiempo de inicio del cuadro actual
Uint32 frameTime;       // Tiempo transcurrido en el cuadro actual
int frameCount = 0;     // Contador de cuadros renderizados
int fps = 0;            // FPS actual

SDL_Renderer* renderer;
std::array<double, WINDOW_WIDTH * WINDOW_HEIGHT> zbuffer;

struct Model {
    Uniforms uniforms;
    std::vector<Vertex> vertices;
    int i;
};

Color clearColor = {0, 0, 0, 255};
Color currentColor = {255, 255, 255, 255};
Color colorA(255, 0, 0, 255); // Red color
Color colorB(0, 255, 255, 255); // Green color
Color colorC(0, 24, 255, 255); // Blue color

glm::vec3 L = glm::vec3(0, 0, 200.0f); // Dirección de la luz en el espacio del ojo

Uniforms uniforms;
Uniforms uniforms2;
Uniforms uniforms3;
Uniforms uniforms4;

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

void render(const std::vector<Vertex>& vertexArray,  const Uniforms& uniforms, int id) {
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
                if (y<0 || x<0 || x>WINDOW_WIDTH || y>WINDOW_HEIGHT)
                    continue;

                glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f); // Central point of the pixel
                glm::vec3 barycentricCoord = calculateBarycentricCoord(A, B, C, pixelPosition);

                if (isBarycentricCoordInsideTriangle(barycentricCoord)) {
                    Color g {0,0,0};
                    // Interpolate attributes (color, depth, etc.) using barycentric coordinates
                    Color interpolatedColor = interpolateColor(barycentricCoord, g, g, g);

                    // Calculate the depth (Z-coordinate) of the fragment using barycentric coordinates
                    float depth = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;


                    glm::vec3 normal = a.normal * barycentricCoord.x + b.normal * barycentricCoord.y+ c.normal * barycentricCoord.z;

                    float fragmentIntensity = glm::dot(normal, glm::vec3 (0,0.0f,1.0f));
                    if (fragmentIntensity<=0 ){
                        continue;
                    }
                    // Apply fragment shader to calculate final color with shading
                    Color finalColor = interpolatedColor * fragmentIntensity;

                    glm::vec3 original = a.original * barycentricCoord.x + b.original * barycentricCoord.y + c.original * barycentricCoord.z;
                    // Create a fragment with the position, interpolated attributes, and depth
                    Fragment fragment;
                    fragment.position = glm::ivec2(x, y);
                    fragment.color = finalColor;
                    fragment.z = depth;  // Set the depth of the fragment
                    fragment.original = original;


                    int index = y * WINDOW_WIDTH + x;
                    if (depth < zbuffer[index]) {
                        // Apply fragment shader to calculate final color


                        // Draw the fragment using SDL_SetRenderDrawColor and SDL_RenderDrawPoint
                        //std::cout << "El valor de i es: " << id << std::endl;
                        if (id==0){
                            Color fragmentShaderf = fragmentShader(fragment);
                            SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                        }
                        if (id==1){
                            Color fragmentShaderf2 =  earthSolarSystem( fragment);
                            SDL_SetRenderDrawColor(renderer, fragmentShaderf2.r, fragmentShaderf2.g, fragmentShaderf2.b, fragmentShaderf2.a);
                        }
                        if (id==2){
                            Color fragmentShaderf3 = sunSolarSystem(fragment);
                            SDL_SetRenderDrawColor(renderer, fragmentShaderf3.r, fragmentShaderf3.g, fragmentShaderf3.b, fragmentShaderf3.a);
                        }
                        if (id==3){
                            Color fragmentShaderf4 = gasPlanet(fragment);
                            SDL_SetRenderDrawColor(renderer, fragmentShaderf4.r, fragmentShaderf4.g, fragmentShaderf4.b, fragmentShaderf4.a);
                        }

                        SDL_RenderDrawPoint(renderer, x, WINDOW_HEIGHT-y);
                        // Update the z-buffer value for this pixel
                        newTime = 0.5f + 1.0f;
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
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(5.0f, 5.0f, 5.0f));
    glm::mat4 rotation = glm::mat4(1.0f);;
    return transtation * scale * rotation;
}

glm::mat4 createModelMatrix2() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)), glm::vec3(0.0f, 1.0f, 0.0f));
    return transtation * scale * rotation;
}

glm::mat4 createModelMatrix3() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.2f, 0.4f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.5f, 0.5f, 0.5f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)*5), glm::vec3(0.0f, 1.0f, 0.0f));
    return transtation * scale * rotation;
}

glm::mat4 createModelMatrix4() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.2f, 0.3f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)*500), glm::vec3(0.0f, 1.0f, 0.0f));
    return transtation * scale * rotation;
}



int main(int argc, char* argv[]) {

    std::vector<Model> models;

    glm::vec3 cameraPosition(0.0f, 0.0f, 3.0f); // Mueve la cámara hacia atrás
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
    glm::vec3 upVector(0.0f, 1.0f, 0.0f);

    uniforms.view = glm::lookAt(cameraPosition, targetPosition, upVector);

    srand(time(nullptr));

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Pixel Drawer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Resto del código...
    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normal;
    std::vector<Face> faces;
    bool success = loadOBJ("../sphere.obj", vertices, normal, faces);
    if (!success) {
        // Manejo del error si la carga del archivo falla
        return 1;
    }

    std::vector<Vertex> vertexArray = setupVertexArray(vertices, normal, faces);

    std::vector<glm::vec3> vertices2;
    std::vector<glm::vec3> normal2;
    std::vector<Face> faces2;
    bool success2 = loadOBJ("../Nave6.obj", vertices2, normal2, faces2);
    if (!success2) {
        // Manejo del error si la carga del archivo falla
        return 1;
    }

    std::vector<Vertex> vertexArray2 = setupVertexArray(vertices2, normal2, faces2);

    bool running = true;
    SDL_Event event;
    //glm::mat4 rotationMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación
    //glm::mat4 traslateMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación
    //glm::mat4 scaleMatrix = glm::mat4(1.0f); // Inicializa la matriz de rotación

    Model model1;
    Model model2;
    Model model3;
    Model model4;

    float rotationAngle = 0.0f; // Inicializa la variable de ángulo de rotación
    // Calcula la matriz de transformación para la rotación alrededor del modelo 2
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    frameStart = SDL_GetTicks();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        rotationAngle += 0.01f;
        // Define el radio de la traslación circular
        float radius = 1.0f;

// Calcula la posición en el círculo
        float posX = glm::cos(rotationAngle) * radius;
        float posZ = glm::sin(rotationAngle) * radius;

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1), glm::vec3(posX, 0.0f, posZ));

        uniforms.model = createModelMatrix();    // Asignar la matriz de proyección a uniforms.projection
        uniforms.projection = createProjectionMatrix();
        uniforms.viewport = createViewportMatrix();


        model1.uniforms = uniforms;
        model1.vertices = vertexArray;
        model1.i = 0;
        //models.push_back(model1);

        glm::vec3 cameraPosition2(0.0f, 0.0f, 5.0f); // Mueve la cámara hacia atrás
        glm::vec3 targetPosition2(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
        glm::vec3 upVector2(0.0f, 1.0f, 0.0f);


        uniforms2.model = createModelMatrix2();
        uniforms2.view = glm::lookAt(cameraPosition2, targetPosition2, upVector2);
        uniforms2.viewport = createViewportMatrix();
        uniforms2.projection = createProjectionMatrix();

        model2.uniforms = uniforms2;
        model2.vertices = vertexArray;
        model2.i = 1;
        models.push_back(model2);

        glm::vec3 cameraPosition3(0.0f, 0.0f, 5.0f); // Mueve la cámara hacia atrás
        glm::vec3 targetPosition3(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
        glm::vec3 upVector3(0.0f, 1.0f, 0.0f);

        uniforms3.model = createModelMatrix3() * translationMatrix;
        uniforms3.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms3.viewport = createViewportMatrix();
        uniforms3.projection = createProjectionMatrix();

        model3.uniforms = uniforms3;
        model3.vertices = vertexArray;
        model3.i = 2;
        //models.push_back(model3);


        uniforms4.model = createModelMatrix4() * translationMatrix;
        uniforms4.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms4.viewport = createViewportMatrix();
        uniforms4.projection = createProjectionMatrix();
        model4.uniforms= uniforms4;
        model4.vertices = vertexArray;
        model4.i = 3;
        //models.push_back(model4);

        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);

        glm::vec4 transformedLight = glm::inverse(createModelMatrix()) * glm::vec4(L, 0.0f);
        glm::vec3 transformedLightDirection = glm::normalize(glm::vec3(transformedLight));

        for (const Model& model : models) {;
            render(model.vertices, model.uniforms, model.i);
        }
        //render(vertexArray, uniforms);
        SDL_RenderPresent(renderer);
        frameTime = SDL_GetTicks() - frameStart;
        frameCount++;

// Si ha transcurrido un segundo, actualiza los FPS y reinicia el contador de cuadros
        if (frameTime >= 1000) {
            fps = frameCount;
            frameCount = 0;
            frameStart = SDL_GetTicks(); // Reinicia el tiempo de inicio para el siguiente segundo
        }

// Luego, muestra los FPS en la ventana de salida (puedes elegir la posición y el formato)
        std::string fpsText = "FPS: " + std::to_string(fps);
        SDL_SetWindowTitle(window, fpsText.c_str());

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}