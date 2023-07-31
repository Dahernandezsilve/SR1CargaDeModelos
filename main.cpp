#include <SDL2/SDL.h>
#include <ctime>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" // Necesario para las funciones de transformación de matriz
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

class Color {
public:
    uint8_t r, g, b, a;
};

Color clearColor = {0, 0, 0, 255};
Color currentColor = {255, 255, 255, 255};

SDL_Renderer* renderer;

struct Face {
    std::array<int, 3> vertexIndices;
};

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

void render(const std::vector<glm::vec3>& vertexArray) {
    // Limpiar la pantalla antes de dibujar
    SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    SDL_RenderClear(renderer);

    // Establecer el color actual para dibujar los triángulos
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);

    // Dibujar cada triángulo con las coordenadas proyectadas
    for (size_t i = 0; i < vertexArray.size(); i += 3) {
        const glm::vec3& A = vertexArray[i];
        const glm::vec3& B = vertexArray[i + 1];
        const glm::vec3& C = vertexArray[i + 2];

        // Aplicar el offset para que el modelo se centre en la ventana
        int offsetX = WINDOW_WIDTH / 2;
        int offsetY = WINDOW_HEIGHT / 2;

        // Dibujar los triángulos sumando el offset
        triangle(A + glm::vec3(offsetX, offsetY, 0), B + glm::vec3(offsetX, offsetY, 0), C + glm::vec3(offsetX, offsetY, 0));
    }
}

std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces)
{
    std::vector<glm::vec3> vertexArray;

    // Ajusta esta escala manualmente para obtener el tamaño deseado del modelo en la ventana
    float scale = 1.0f;

    // For each face
    for (const auto& face : faces)
    {
        // Get the vertex positions from the input arrays using the indices from the face
        glm::vec3 vertexPosition1 = vertices[face.vertexIndices[0]];
        glm::vec3 vertexPosition2 = vertices[face.vertexIndices[1]];
        glm::vec3 vertexPosition3 = vertices[face.vertexIndices[2]];

        // Scale the vertex positions
        glm::vec3 vertexScaled1 = vertexPosition1 * scale;
        glm::vec3 vertexScaled2 = vertexPosition2 * scale;
        glm::vec3 vertexScaled3 = vertexPosition3 * scale;

        // Add the vertex positions to the vertex array
        vertexArray.push_back(vertexScaled1);
        vertexArray.push_back(vertexScaled2);
        vertexArray.push_back(vertexScaled3);
    }

    return vertexArray;
}

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open OBJ file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<std::array<int, 3>> temp_faces;

    glm::vec3 centroid(0.0f); // Inicializar el centroide en (0, 0, 0)

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);

            // Actualizar el centroide con el nuevo vértice
            centroid += vertex;
        } else if (type == "f") {
            std::array<int, 3> face_indices{};
            for (int i = 0; i < 3; i++) {
                std::string faceIndexStr;
                iss >> faceIndexStr;

                // Find the position of the first slash to extract the vertex index
                size_t pos = faceIndexStr.find_first_of('/');
                if (pos != std::string::npos) {
                    faceIndexStr = faceIndexStr.substr(0, pos);
                }

                face_indices[i] = std::stoi(faceIndexStr); // No restar 1
            }
            temp_faces.push_back(face_indices);
        }
    }

    // Después de calcular el centroide en la función loadOBJ
// ...

// Calcular el promedio para obtener el centroide final
    centroid /= static_cast<float>(temp_vertices.size());

// Realizar la rotación en el eje Y
    float rotationAngleY = 90.0f; // Ángulo de rotación en grados (ajústalo según necesites)
    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

// Realizar la rotación en el eje X
    float rotationAngleX = 0.0f; // Ángulo de rotación en grados (ajústalo según necesites)
    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleX), glm::vec3(1.0f, 0.0f, 0.0f));

// Realizar la rotación en el eje Z
    float rotationAngleZ = 0.0f; // Ángulo de rotación en grados (ajústalo según necesites)
    glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

// Combinar las transformaciones de rotación en los tres ejes en una sola matriz
    glm::mat4 combinedRotationMatrix = rotationMatrixY * rotationMatrixX * rotationMatrixZ;

// Ahora que tenemos el centroide, podemos calcular las coordenadas finales de los vértices y agregarlos al vector de salida directamente
    out_vertices.reserve(temp_vertices.size());
    for (const auto& vertex : temp_vertices) {
        // Aplicar la traslación para centrar el vértice
        glm::vec3 centeredVertex = (vertex - centroid) * 30.0f; // Escala de 30.0f como en la función original

        // Aplicar la rotación en los tres ejes (X, Y, Z)
        glm::vec4 rotatedVertex = combinedRotationMatrix * glm::vec4(centeredVertex, 1.0f);

        out_vertices.push_back(glm::vec3(rotatedVertex));
    }

    // Convert std::array<int, 3> to std::vector<std::array<int, 3>>
    out_faces.reserve(temp_faces.size() * 2); // Reserve space for triangles
    for (const auto& face : temp_faces) {
        Face f1{}, f2{};

        // Triangulate the face (split the square into two triangles)
        f1.vertexIndices = { face[0] - 1, face[1] - 1, face[2] - 1 };
        f2.vertexIndices = { face[0] - 1, face[2] - 1, face[3] - 1 };

        out_faces.push_back(f1);
        out_faces.push_back(f2);
    }



    return true;
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);

    srand(time(nullptr));

    SDL_Window* window = SDL_CreateWindow("Pixel Drawer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);

    std::vector<glm::vec3> vertices;
    std::vector<Face> faces;

    bool success = loadOBJ("../Nave.obj", vertices, faces);
    if (!success) {
        // Manejo del error si la carga del archivo falla
        return 1;
    }

    std::vector<glm::vec3> vertexArray = setupVertexArray(vertices, faces);

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
        // Llamada a la función render con la matriz de vértices transformados
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        render(vertexArray); // Solo se pasa la matriz de vértices transformados
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
