#include <SDL2/SDL.h>
#include <ctime>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "color.h"
#include <vector>
#include <thread>
#include "loadObj.h"
#include "vertexArray.h"
#include "uniform.h"
#include "shaders.h"
#include <mutex>

std::mutex mutex;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

Uint32 frameStart;      // Tiempo de inicio del cuadro actual
Uint32 frameTime;       // Tiempo transcurrido en el cuadro actual
int frameCount = 0;     // Contador de cuadros renderizados
int fps = 0;            // FPS actual

SDL_Renderer* renderer;
std::array<double, WINDOW_WIDTH * WINDOW_HEIGHT> zbuffer;

enum ShaderId {
    TIERRA,
    SOL,
    ESTRELLA,
    LUNA,
    PLANETA_GASEOSO,
    NAVE,
    PLANETA_RANDOM,
    PLANETA_VERDE
};

struct Model {
    Uniforms uniforms;
    std::vector<Vertex>* vertices;
    ShaderId i;
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
Uniforms uniforms5;
Uniforms uniforms6;
Uniforms uniforms7;
Uniforms uniforms8;
Uniforms unifroms9;

Model model1;
Model model2;
Model model3;
Model model4;
Model model5;
Model model6;
Model model7;
Model model8;
Model model9;


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

void render(const std::vector<Vertex>& vertexArray, const Uniforms& uniform, int id) {
    std::vector<Vertex> transformedVertexArray;
    for (const auto& vertex : vertexArray) {
        auto transformedVertex = vertexShader(vertex, uniform);
        transformedVertexArray.push_back(transformedVertex);
    }

    for (size_t i = 0; i < transformedVertexArray.size(); i += 3) {
        const Vertex& a = transformedVertexArray[i];
        const Vertex& b = transformedVertexArray[i + 1];
        const Vertex& c = transformedVertexArray[i + 2];

        glm::vec3 A = a.position;
        glm::vec3 B = b.position;
        glm::vec3 C = c.position;

        if (a.z<=0 && b.z<=0 && c.z<=0)
            continue;

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

                double zCamera = barycentricCoord.x * a.z + barycentricCoord.y * b.z + barycentricCoord.z * c.z;

                if (isBarycentricCoordInsideTriangle(barycentricCoord) && zCamera>0) {
                    Color g {0,0,0};
                    // Interpolate attributes (color, depth, etc.) using barycentric coordinates
                    Color interpolatedColor = interpolateColor(barycentricCoord, g, g, g);

                    // Calculate the depth (Z-coordinate) of the fragment using barycentric coordinates
                    float depth = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;

                    glm::vec3 normal = a.normal * barycentricCoord.x + b.normal * barycentricCoord.y+ c.normal * barycentricCoord.z;

                    float fragmentIntensity = (abs(glm::dot(normal, L)) > 1 ) ? 1: abs(glm::dot(normal, L));
                    if (id==ESTRELLA){
                        fragmentIntensity = glm::dot(normal, glm::vec3(0.0f,0.0f,1.0f));
                    }
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
                        mutex.lock();
                        // Apply fragment shader to calculate final color
                        Color fragmentS;
                        // Draw the fragment using SDL_SetRenderDrawColor and SDL_RenderDrawPoint
                        //std::cout << "El valor de i es: " << id << std::endl;
                        switch (id) {
                            case ESTRELLA:
                                fragmentS = fragmentShader(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case SOL:
                                fragmentS = sunSolarSystem(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g, fragmentS.b, fragmentS.a);
                                break;
                            case TIERRA:
                                fragmentS = earthSolarSystem(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case LUNA:
                                fragmentS = rockPlanet(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case PLANETA_GASEOSO:
                                fragmentS = gasPlanetV3(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case NAVE:
                                fragmentS = starship(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case PLANETA_RANDOM:
                                fragmentS = randomPlanet(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                            case PLANETA_VERDE:
                                fragmentS = starPlanetGreen(fragment);
                                SDL_SetRenderDrawColor(renderer, fragmentS.r, fragmentS.g,fragmentS.b, fragmentS.a);
                                break;
                        }

                        SDL_RenderDrawPoint(renderer, x, WINDOW_HEIGHT-y);
                        // Update the z-buffer value for this pixel
                        newTime = 0.5f + 1.0f;
                        zbuffer[index] = depth;
                        mutex.unlock();
                    }
                }
            }
        }
    }
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.5f));
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

glm::mat4 createModelMatrixStars() {
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -30.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(20.0f, 20.0f, 20.0f));
    glm::mat4 rotation = glm::mat4(1.0f);
    return translation * scale * rotation;
}

glm::mat4 createModelMatrixEntity(glm::vec3 matrixTranslation, glm::vec3 matrixScale, glm::vec3 matrixRotation, float radianSpeed)  {
    glm::mat4 translation = glm::translate(glm::mat4(1), matrixTranslation);
    glm::mat4 scale = glm::scale(glm::mat4(1), matrixScale);
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)*radianSpeed), matrixRotation);
    return translation * scale * rotation;
}

glm::mat4 createModelMatrixEntityWithMoon(const glm::mat4& modelMatrix3) {
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.3f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++)), glm::vec3(0.0f, 1.0f, 0.0f));
    return modelMatrix3 * translation * scale * rotation;
}

glm::mat4 createModelMatrixShip(glm::vec3 cameraPosition, glm::vec3 targetPosition,glm::vec3 upVector, float xRotate, float yRotate) {
    glm::mat4 translation = glm::translate(glm::mat4(1), (targetPosition - cameraPosition) / 7.0f + cameraPosition - upVector *0.15f);
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.1f, 0.1f, 0.1f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1), glm::radians(-xRotate - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1), glm::radians(-yRotate), glm::vec3(0.0f, 0.0f, 1.0f));
    //std::cout << "Valor de xRotate: " << xRotate << std::endl;
    //std::cout << "Valor de yRotate: " << yRotate << std::endl;
    return translation * scale * rotationX * rotationY;
}

glm::mat4 calculatePositionInCircle(float rotationAngle, float radius){
    float posX = glm::cos(rotationAngle) * radius;
    float posZ = glm::sin(rotationAngle) * radius;
    return glm::translate(glm::mat4(1), glm::vec3(posX, 0.0f, posZ));
}

glm::vec3 calculatePositionInCircleReal(float rotationAngle, float radius){
    float posX = glm::cos(rotationAngle) * radius;
    float posZ = glm::sin(rotationAngle) * radius;
    return glm::vec3(posX, 0.0f, posZ);
}
// Función para imprimir un vec3
void printVec3(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << std::endl;
}

int main(int argc, char* argv[]) {
    //srand(time(nullptr));

    SDL_Init(SDL_INIT_EVERYTHING);
    frameStart = SDL_GetTicks();
    SDL_Window* window = SDL_CreateWindow("Pixel Drawer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    std::vector<Model> models;
    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);

    std::vector<glm::vec3> verticesEntity;
    std::vector<glm::vec3> normalEntity;
    std::vector<Face> facesEntity;
    bool success = loadOBJ("../sphere.obj", verticesEntity, normalEntity, facesEntity);
    if (!success) { return 1; } // Manejo del error si la carga del archivo falla
    std::vector<Vertex> vertexArrayEntity = setupVertexArray(verticesEntity, normalEntity, facesEntity);

    std::vector<glm::vec3> verticesShip;
    std::vector<glm::vec3> normalShip;
    std::vector<Face> facesShip;
    bool success2 = loadOBJ("../Nave6.obj", verticesShip, normalShip, facesShip);
    if (!success2) { return 1; } // Manejo del error si la carga del archivo falla
    std::vector<Vertex> vertexArrayShip = setupVertexArray(verticesShip, normalShip, facesShip);

    float moveSpeedBackForward = 0.1f;
    float moveSpeedLeftRight = 0.05f;
    float xRotate = 0.0f;
    float yRotate = 0.0f;
    float rotationAngle = 0.0f;
    float rotationAngle2 = 0.0f;
    float rotationAngle3 = 0.0f;
    float rotationAngle4 = 0.0f;
    float rotationAngle5 = 0.0f;
    bool moveCameraToFrontOfEarth = false;
    bool moveCameraToFrontOfGasPlanet = false;
    bool moveCameraToFrontOfRandomPlanet = false;
    bool moveCameraToFrontOfGreenPlanet = false;
    bool moveCameraToSolarSystem = false;

    glm::vec3 cameraPosition(0.0f, 0.0f, 5.0f);
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector(0.0f, 1.0f, 0.0f);

    glm::vec3 newCameraPosition;
    glm::vec3 newShipCameraPosition;


    bool running = true;
    SDL_Event event;
    float transitionSpeed = 0.01f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_s:
                        // Mueve la cámara hacia adelante
                        cameraPosition -= moveSpeedBackForward * glm::normalize(targetPosition - cameraPosition);
                        targetPosition -= moveSpeedBackForward * (targetPosition - cameraPosition);
                        break;
                    case SDLK_w:
                        // Mueve la cámara hacia atrás
                        cameraPosition += moveSpeedBackForward * glm::normalize(targetPosition - cameraPosition);
                        targetPosition += moveSpeedBackForward * (targetPosition - cameraPosition);
                        break;
                    case SDLK_d:
                        // Mover hacia la izquierda
                        cameraPosition += moveSpeedLeftRight * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector))*5.0f;
                        targetPosition += moveSpeedLeftRight * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector))*5.0f;
                        break;
                    case SDLK_a:
                        // Mover hacia la derecha
                        cameraPosition -= moveSpeedLeftRight * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector))*5.0f;
                        targetPosition -= moveSpeedLeftRight * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector))*5.0f;
                        break;
                    case SDLK_RIGHT:
                        xRotate += 1.0f;
                        break;
                    case SDLK_LEFT:
                        xRotate -= 1.0f;
                        break;
                    case SDLK_UP:
                        yRotate -= 1.0f;
                        break;
                    case SDLK_DOWN:
                        yRotate += 1.0f;
                        break;
                    case SDLK_t:
                        moveCameraToFrontOfEarth = !moveCameraToFrontOfEarth;
                        break;
                    case SDLK_g:
                        moveCameraToFrontOfGasPlanet = !moveCameraToFrontOfGasPlanet;
                        break;
                    case SDLK_p:
                        moveCameraToSolarSystem = !moveCameraToSolarSystem;
                        break;
                    case SDLK_r:
                        moveCameraToFrontOfRandomPlanet = !moveCameraToFrontOfRandomPlanet;
                        break;
                    case SDLK_v:
                        moveCameraToFrontOfGreenPlanet = !moveCameraToFrontOfGreenPlanet;
                        break;
                }
            }
        }


        models.clear();
        L = cameraPosition - targetPosition;
        rotationAngle += 0.01f;
        rotationAngle2 += 0.0105f;
        rotationAngle3 += 0.01f;
        rotationAngle4 += 0.0115f;
        rotationAngle5 += 0.009f;
        targetPosition = glm::vec3(10.0f * sin(glm::radians(xRotate)) * cos(glm::radians(yRotate)), 10.0f * sin(glm::radians(yRotate)), -10.0f * cos(glm::radians(xRotate)) * cos(glm::radians(yRotate))) + cameraPosition;

        glm::vec3 translationMatrixEarth = calculatePositionInCircleReal(rotationAngle, 2.0f);
        glm::vec3 translationMatrixGasPlanet = calculatePositionInCircleReal(rotationAngle2, 4.0f);
        glm::vec3 translationMatrixMoon = calculatePositionInCircleReal(rotationAngle3, 4.0f);
        glm::vec3 translationMatrixRandomPlanet = calculatePositionInCircleReal(rotationAngle4, 6.0f);
        glm::vec3 translationMatrixGreenPlanet = calculatePositionInCircleReal(rotationAngle5, 7.0f);

        uniforms.model = createModelMatrixStars();
        uniforms.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms.projection = createProjectionMatrix();
        uniforms.viewport = createViewportMatrix();

        model1.uniforms = uniforms;
        model1.vertices = &vertexArrayEntity;
        model1.i = ESTRELLA;

        uniforms2.model = createModelMatrixEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniforms2.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms2.viewport = createViewportMatrix();
        uniforms2.projection = createProjectionMatrix();

        model2.uniforms = uniforms2;
        model2.vertices = &vertexArrayEntity;
        model2.i = SOL;

        uniforms3.model = createModelMatrixEntity(translationMatrixEarth, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniforms3.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms3.viewport = createViewportMatrix();
        uniforms3.projection = createProjectionMatrix();

        model3.uniforms = uniforms3;
        model3.vertices = &vertexArrayEntity;
        model3.i = TIERRA;

        uniforms4.model = createModelMatrixEntityWithMoon(uniforms3.model) * glm::translate(glm::mat4(1), translationMatrixMoon);
        uniforms4.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms4.viewport = createViewportMatrix();
        uniforms4.projection = createProjectionMatrix();

        model4.uniforms= uniforms4;
        model4.vertices = &vertexArrayEntity;
        model4.i = LUNA;

        if (moveCameraToFrontOfGasPlanet) {
            glm::vec3 orientationCamera = targetPosition - cameraPosition;
            targetPosition = translationMatrixGasPlanet + orientationCamera * 0.75f;
            cameraPosition = targetPosition - orientationCamera;
        }

        if (moveCameraToFrontOfEarth) {
            glm::vec3 orientationCamera = targetPosition - cameraPosition;
            targetPosition = translationMatrixEarth + orientationCamera * 0.75f;
            cameraPosition = targetPosition - orientationCamera;
        }

        if (moveCameraToSolarSystem) {
            upVector = glm::vec3 (1.0f, 0.0f, 0.0f);
            cameraPosition = glm::vec3 (0,10,0);
            targetPosition = glm::vec3 (0, 0, 0);
        }

        if (moveCameraToFrontOfGreenPlanet) {
            glm::vec3 orientationCamera = targetPosition - cameraPosition;
            targetPosition = translationMatrixGreenPlanet + orientationCamera * 0.75f;
            cameraPosition = targetPosition - orientationCamera;
        }

        if (moveCameraToFrontOfRandomPlanet) {
            glm::vec3 orientationCamera = targetPosition - cameraPosition;
            targetPosition = translationMatrixRandomPlanet + orientationCamera * 0.75f;
            cameraPosition = targetPosition - orientationCamera;
        }

        uniforms5.model = createModelMatrixEntity(translationMatrixGasPlanet, glm::vec3(0.3f, 0.3f, 0.3f),glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniforms5.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms5.viewport = createViewportMatrix();
        uniforms5.projection = createProjectionMatrix();

        model5.uniforms= uniforms5;
        model5.vertices = &vertexArrayEntity;
        model5.i = PLANETA_GASEOSO;

        uniforms6.model = createModelMatrixShip(cameraPosition, targetPosition, upVector, xRotate, yRotate);
        uniforms6.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms6.viewport = createViewportMatrix();
        uniforms6.projection = createProjectionMatrix();

        model6.uniforms= uniforms6;
        model6.vertices = &vertexArrayShip;
        model6.i = NAVE;

        uniforms7.model = createModelMatrixEntity(translationMatrixRandomPlanet, glm::vec3(0.7f, 0.7f, 0.7f),glm::vec3(0.0f, 1.0f, 0.0f), 0.5f);
        uniforms7.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms7.viewport = createViewportMatrix();
        uniforms7.projection = createProjectionMatrix();

        model7.uniforms= uniforms7;
        model7.vertices = &vertexArrayEntity;
        model7.i = PLANETA_RANDOM;

        uniforms8.model = createModelMatrixEntity(translationMatrixGreenPlanet, glm::vec3(0.3f, 0.3f, 0.3f),glm::vec3(0.0f, 1.0f, 0.0f), 8.0f);
        uniforms8.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms8.viewport = createViewportMatrix();
        uniforms8.projection = createProjectionMatrix();

        model8.uniforms= uniforms8;
        model8.vertices = &vertexArrayEntity;
        model8.i = PLANETA_VERDE;

        //models.push_back(model1);
        models.push_back(model2);
        models.push_back(model3);
        models.push_back(model4);
        models.push_back(model5);
        models.push_back(model6);
        models.push_back(model7);
        models.push_back(model8);

        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);
        std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<double>::max());

        std::vector<std::thread> sunThreads;

        for (const Model& model : models) {
            sunThreads.emplace_back(render, *model.vertices, model.uniforms, model.i);
        }
        for (std::thread& thread : sunThreads) {
            thread.join();
        }

        SDL_RenderPresent(renderer);
        frameTime = SDL_GetTicks() - frameStart;
        frameCount++;
        if (frameTime >= 1000) {
            fps = frameCount;
            frameCount = 0;
            frameStart = SDL_GetTicks(); // Reinicia el tiempo de inicio para el siguiente segundo
        }
        std::string fpsText = "FPS: " + std::to_string(fps);
        SDL_SetWindowTitle(window, fpsText.c_str());
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}