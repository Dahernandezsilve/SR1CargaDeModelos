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

const int WINDOW_WIDTH = 300;
const int WINDOW_HEIGHT = 300;

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
    NAVE
};

struct Model {
    Uniforms uniforms;
    std::vector<Vertex> vertices;
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
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(10.0f, 10.0f, 10.0f));
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
    // Aquí defines la transformación del "modelo 4" en relación con el "modelo 3"
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.3f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((a++) * 4), glm::vec3(0.0f, 1.0f, 0.0f));
    // Aplica la transformación del "modelo 4" en relación con el "modelo 3"
    return modelMatrix3 * translation * scale * rotation;
}

glm::mat4 createModelMatrix6(glm::vec3 cameraPosition3, glm::vec3 targetPosition3,glm::vec3 upVector3, float xRotate, float yRotate) {
    glm::mat4 translation = glm::translate(glm::mat4(1), (targetPosition3 - cameraPosition3) /7.0f + cameraPosition3 +upVector3*0.15f);
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1), glm::radians(xRotate), glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 rotationY = glm::rotate(glm::mat4(1), glm::radians((yRotate)+90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    return translation * scale * rotationX;
}

void updateCameraOrientation() {

}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);
    std::vector<Model> models;
    glm::vec3 cameraPosition(0.0f, 0.0f, 30.0f); // Mueve la cámara hacia atrás
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
    glm::vec3 upVector(0.0f, 1.0f, 0.0f);

    uniforms.view = glm::lookAt(cameraPosition, targetPosition, upVector);

    srand(time(nullptr));

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

    Model model1;
    Model model2;
    Model model3;
    Model model4;
    Model model5;
    Model model6;

    float rotationAngle = 0.0f; // Inicializa la variable de ángulo de rotación
    // Calcula la matriz de transformación para la rotación alrededor del modelo 2
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    frameStart = SDL_GetTicks();

    glm::vec3 cameraPosition2(0.0f, 0.0f, 10.0f); // Mueve la cámara hacia atrás
    glm::vec3 targetPosition2(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
    glm::vec3 upVector2(0.0f, 1.0f, 0.0f);


    glm::vec3 cameraPosition3(0.0f, 0.0f, 10.0f); // Mueve la cámara hacia atrás
    glm::vec3 targetPosition3(0.0f, 0.0f, 0.0f);   // Coloca el centro de la escena en el origen
    glm::vec3 upVector3(0.0f, 1.0f, 0.0f);

    float moveSpeed = 0.1f;
    float moveSpeed2 = 0.05f;
    float xRotate = 1;
    float yRotate = 1;
// Dentro del bucle principal
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                // Maneja las teclas presionadas
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        // Mueve la cámara hacia adelante
                        cameraPosition -= moveSpeed * glm::normalize(targetPosition - cameraPosition);
                        cameraPosition3 -= moveSpeed * glm::normalize(targetPosition - cameraPosition);
                        targetPosition -=  moveSpeed * (targetPosition - cameraPosition);
                        break;
                    case SDLK_s:
                        // Mueve la cámara hacia atrás
                        cameraPosition += moveSpeed * glm::normalize(targetPosition - cameraPosition);
                        cameraPosition3 += moveSpeed * glm::normalize(targetPosition - cameraPosition);
                        targetPosition += moveSpeed * (targetPosition - cameraPosition);
                        targetPosition3 +=  moveSpeed * (targetPosition - cameraPosition);
                        break;
                    case SDLK_a:
                        // Mover hacia la izquierda
                        cameraPosition += moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector3))*5.0f;
                        cameraPosition3 += moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector3))*5.0f;
                        targetPosition += moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector3))*5.0f;
                        targetPosition3 += moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector3))*5.0f;
                        break;
                    case SDLK_d:
                        // Mover hacia la derecha
                        cameraPosition -= moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector3))*5.0f;
                        cameraPosition3 -= moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector3))*5.0f;
                        targetPosition -= moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector3))*5.0f;
                        targetPosition3 -= moveSpeed2 * glm::normalize(glm::cross((targetPosition - cameraPosition),upVector3))*5.0f;
                        break;
                    case SDLK_LEFT:
                        xRotate += 10.0f;
                        break;
                    case SDLK_RIGHT:
                        xRotate -= 10.0f;
                        break;
                    case SDLK_UP:
                        yRotate -= 10.0f;
                        break;
                    case SDLK_DOWN:
                        yRotate += 10.0f;
                        break;

                }
            }

        }
        targetPosition = glm::vec3(5.0f * sin(glm::radians(xRotate)) * cos(glm::radians(yRotate)), 5.0f * sin(glm::radians(yRotate)), -5.0f * cos(glm::radians(xRotate)) * cos(glm::radians(yRotate))) + cameraPosition;
        targetPosition3 = glm::vec3(5.0f * sin(glm::radians(xRotate)) * cos(glm::radians(yRotate)), 5.0f * sin(glm::radians(yRotate)), -5.0f * cos(glm::radians(xRotate)) * cos(glm::radians(yRotate))) + cameraPosition3;


        // Actualizar la matriz de vista con la nueva posición de la cámara
        uniforms.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms2.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms3.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms4.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms5.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniforms6.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);

        models.clear();
        rotationAngle += 0.001f;
        // Define el radio de la traslación circular
        float radius = 3.0f;

        // Calcula la posición en el círculo
        float posX = glm::cos(rotationAngle) * radius;
        float posZ = glm::sin(rotationAngle) * radius;

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1), glm::vec3(posX, 0.0f, posZ));

        float radius2 = 8.0f;

        // Calcula la posición en el círculo
        float posX2 = glm::cos(rotationAngle) * radius2;
        float posZ2 = glm::sin(rotationAngle) * radius2;
        glm::mat4 translationMatrix2 = glm::translate(glm::mat4(1), glm::vec3(posX2, 0.0f, posZ2));

        uniforms.model = createModelMatrixStars();
        uniforms.projection = createProjectionMatrix();
        uniforms.viewport = createViewportMatrix();

        model1.uniforms = uniforms;
        model1.vertices = vertexArray;
        model1.i = ESTRELLA;
        models.push_back(model1);


        uniforms2.model = createModelMatrixEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1);
        uniforms2.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms2.viewport = createViewportMatrix();
        uniforms2.projection = createProjectionMatrix();

        model2.uniforms = uniforms2;
        model2.vertices = vertexArray;
        model2.i = SOL;
        models.push_back(model2);

        uniforms3.model = createModelMatrixEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), 1) * translationMatrix;
        uniforms3.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms3.viewport = createViewportMatrix();
        uniforms3.projection = createProjectionMatrix();

        model3.uniforms = uniforms3;
        model3.vertices = vertexArray;
        model3.i = TIERRA;
        models.push_back(model3);

        uniforms4.model = createModelMatrixEntityWithMoon(uniforms3.model) * translationMatrix;
        uniforms4.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms4.viewport = createViewportMatrix();
        uniforms4.projection = createProjectionMatrix();

        model4.uniforms= uniforms4;
        model4.vertices = vertexArray;
        model4.i = LUNA;
        models.push_back(model4);

        uniforms5.model = createModelMatrixEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.3f),glm::vec3(0.0f, 1.0f, 0.0f), 1.3) * translationMatrix2;
        uniforms5.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms5.viewport = createViewportMatrix();
        uniforms5.projection = createProjectionMatrix();

        model5.uniforms= uniforms5;
        model5.vertices = vertexArray;
        model5.i = PLANETA_GASEOSO;
        models.push_back(model5);

        uniforms6.model = createModelMatrix6(cameraPosition3, targetPosition3, upVector3, xRotate, yRotate);
        uniforms6.view = glm::lookAt(cameraPosition3, targetPosition3, upVector3);
        uniforms6.viewport = createViewportMatrix();
        uniforms6.projection = createProjectionMatrix();

        model6.uniforms= uniforms6;
        model6.vertices = vertexArray2;
        model6.i = NAVE;
        models.push_back(model6);

        glm::vec4 transformedLight = glm::inverse(createModelMatrixStars()) * glm::vec4(L, 0.0f);
        glm::vec3 transformedLightDirection = glm::normalize(glm::vec3(transformedLight));

        SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        SDL_RenderClear(renderer);

        // Clear z-buffer at the beginning of each frame
        std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<double>::max());

        for (const Model& model : models) {
            render(model.vertices, model.uniforms, model.i);
        }

        //render(vertexArray, uniforms);
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