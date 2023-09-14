#pragma once
#include "glm/glm.hpp"
#include "uniform.h"
#include "fragment.h"
#include "color.h"
#include "fastNoise.h"

float newTime = 0.5f;

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
    return Vertex {vertexRedux, normal, vertex.position};
}

// Declara un objeto FastNoiseLite


Color fragmentShader(Fragment& fragment) {

    FastNoiseLite noise;
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);
    float scale = 100000.0f;

    if (true) {
        // Utiliza FastNoiseLite para generar un valor de ruido aleatorio
        float noiseValue = noise.GetNoise(fragmentCoords.x * scale, fragmentCoords.y * scale);

        // Define un umbral de brillo para las estrellas
        float brightnessThreshold = 0.97f; // Ajusta esto según lo deseado

        // Comprueba si el valor de ruido es lo suficientemente alto para ser una estrella
        if (noiseValue > brightnessThreshold) {
            // Define el color de las estrellas (por ejemplo, blanco)
            Color starColor(100, 100, 100);
            int o = rand() % 156;
            starColor.r += o;
            starColor.g += o;
            starColor.b += o;

            // Aplica el color de la estrella al fragmento
            fragment.color = starColor;

            return fragment.color;
        }
    }

    return fragment.color;
}

Color sunSolarSystem(Fragment& fragment) {
    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Define los colores base de los aros de lava
    Color lavaColor1 = Color(255, 255, 0); // Rojo para el primer anillo
    Color lavaColor2 = Color(255, 125, 0); // Rojo oscuro-anaranjado para el segundo anillo

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite:: NoiseType_Cellular);
    noise.SetSeed(1337);
    noise.SetFrequency(0.010f);
    noise.SetFractalType(FastNoiseLite:: FractalType_PingPong);
    noise.SetFractalOctaves(4);
    noise.SetFractalLacunarity(2 + newTime);
    noise.SetFractalGain(0.90f);
    noise.SetFractalWeightedStrength(0.70f);
    noise.SetFractalPingPongStrength(3 );
    noise.SetCellularDistanceFunction(FastNoiseLite:: CellularDistanceFunction_Euclidean);
    noise.SetCellularReturnType(FastNoiseLite:: CellularReturnType_Distance2Add);
    noise.SetCellularJitter(1);

    float ox = 3000.0f;
    float oy =3000.0f;
    float zoom = 5000.0f;

    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom));

    Color tmpColor = (noiseValue < 0.1f) ? lavaColor1 : lavaColor2;


    fragment.color = tmpColor * fragment.z;
    return fragment.color;
}

Color earthSolarSystem(Fragment& fragment) {
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    Color ocean = Color(0, 191, 255);
    Color earth = Color(0, 128, 0);
    Color polar = Color(255, 255, 255); // Blanco para las regiones polares

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noise.SetSeed(1337);
    noise.SetFrequency(0.005f);
    noise.SetFractalType(FastNoiseLite::FractalType_PingPong);
    noise.SetFractalOctaves(6);
    noise.SetFractalLacunarity(2 + newTime);
    noise.SetFractalGain(1.0f);
    noise.SetFractalWeightedStrength(0.90f);
    noise.SetFractalPingPongStrength(3);
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add);
    noise.SetCellularJitter(1);

    float ox = 3000.0f;
    float oy = 3000.0f;
    float zoom = 500.0f;

    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom));

    // Definir un umbral para la transición entre el color de la tierra y el color polar
    float threshold = 0.1f;

    if (noiseValue < threshold) {
        Color tmpColor = ocean;
        fragment.color = tmpColor * fragment.z;
    } else {
        // Calcular el gradiente vertical basado en noiseValue
        float gradient = (noiseValue - threshold) / (1.0f - threshold);

        // Mezclar el color de la tierra y el color polar usando el gradiente
        fragment.color = earth * (1.0f - gradient) + polar * gradient;
    }

    return fragment.color;
}


Color fragmentShader4(Fragment& fragment) {
    Color color;

    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Calcula la distancia horizontal desde el centro del fragmento al origen (0, 0)
    float distanceX = glm::abs(fragmentCoords.y);

    // Define un umbral para determinar si un fragmento será una estrella
    float starThreshold = 0.995f; // Ajusta esto según lo deseado

    // Genera estrellas aleatoriamente en el fondo del cielo
    if (distanceX < starThreshold) {
        // Define un umbral de brillo para las estrellas
        float brightnessThreshold = 0.2f; // Ajusta esto según lo deseado

        // Genera un valor de brillo aleatorio para la estrella
        float starBrightness = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        // Comprueba si la estrella es lo suficientemente brillante
        if (starBrightness > brightnessThreshold) {
            // Define el color de las estrellas (por ejemplo, blanco)
            Color starColor(0, 120, 20);

            // Aplica el color de la estrella al fragmento
            fragment.color = starColor * fragment.z;

            // Escala el brillo de la estrella por su

            return fragment.color;
        }
    }

    // Si no es una estrella, puedes continuar con el procesamiento actual
    // (por ejemplo, puedes mantener el código existente que genera los aros de lava).

    // ...

    return fragment.color;
}

Color randomPlanet(Fragment& fragment) {
    Color color;

    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Calcula la distancia horizontal desde el centro del fragmento al origen (0, 0)
    float distanceX = glm::abs(fragmentCoords.y);

    // Define el ancho horizontal de los aros de lava y los valores de atenuación para cada anillo
    float lavaWidth1 = 0.1f; // Ancho del primer anillo
    float lavaWidth2 = 0.4f; // Ancho del segundo anillo
    float lavaWidth3 = 0.5f; // Ancho del tercer anillo
    float attenuation1 = glm::smoothstep(lavaWidth1 - 0.1f, lavaWidth1, distanceX);
    float attenuation2 = glm::smoothstep(lavaWidth2 - 0.05f, lavaWidth2, distanceX) - attenuation1;
    float attenuation3 = glm::smoothstep(lavaWidth3 - 0.05f, lavaWidth3, distanceX) - attenuation2 - attenuation1;

    // Define los colores base de los aros de lava
    Color lavaColor1 = Color(255, 0, 0); // Rojo para el primer anillo
    Color lavaColor2 = Color(255, 69, 0); // Rojo oscuro-anaranjado para el segundo anillo
    Color lavaColor3 = Color(255, 165, 0); // Naranja para el tercer anillo

    // Configura y genera el valor de ruido utilizando FastNoise
    FastNoiseLite noise;
    noise.SetSeed(12345); // Configura una semilla aleatoria (ajusta según sea necesario)
    float noiseValue = noise.GetNoise(fragment.original.x, fragment.original.y);

    // Escala y ajusta el valor de ruido para que tenga un rango adecuado para la mezcla
    float noiseScale = 2.0f;
    noiseValue = (noiseValue + 1.0f) * noiseScale;

    // Combina los colores de los aros de lava en función de la atenuación y el valor de ruido
    fragment.color = (lavaColor1 * attenuation1 + lavaColor2 * attenuation2 + lavaColor3 * attenuation3) + Color(noiseValue, noiseValue, noiseValue);

    return fragment.color;
}