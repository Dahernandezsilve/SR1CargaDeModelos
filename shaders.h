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
//void printVec3(const glm::vec3& vector) {
    //std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
//}


Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    //uniforms.viewport *
    glm::vec4 transformedVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);
    double z = transformedVertex.z;
    transformedVertex = uniforms.viewport * transformedVertex;
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
    return Vertex {vertexRedux, normal, vertex.position, z};
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
    float oy = 3000.0f;
    float zoom = 5000.0f;

    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    Color tmpColor = (noiseValue < 0.1f) ? lavaColor1 : lavaColor2;

    fragment.color = tmpColor * fragment.z;
    return fragment.color;
}

Color gasPlanetV1(Fragment& fragment) {
    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Define los colores para el planeta gaseoso
    Color cloudColor1 = Color(100, 100, 200); // Color de las nubes
    Color cloudColor2 = Color(200, 255, 255); // Color más claro para las nubes

    // Configuración del ruido
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(1337);
    noise.SetFrequency(0.05f); // Ajusta la frecuencia para controlar la apariencia de las nubes
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(6);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);

    float ox = 3000.0f;
    float oy = 3000.0f;
    float zoom = 200.0f;

    // Generar valor de ruido
    float noiseValue = noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom);

    // Mapear el valor de ruido al rango de colores
    float t = (noiseValue + 1.0f) / 2.0f; // Asegura que esté en el rango [0, 1]

    // Mezcla los colores de las nubes basados en el valor de ruido
    Color cloudColor = cloudColor1 * (1.0f - t) + cloudColor2 * t;

    fragment.color = cloudColor;
    return fragment.color;
}

Color gasPlanetV2(Fragment& fragment) {
    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Define los colores para el planeta gaseoso
    Color vortexColor1 = Color(0, 0, 255); // Azul para el vórtice
    Color vortexColor2 = Color(255, 255, 255); // Blanco para el centro del vórtice

    // Configuración del ruido
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(1337);
    noise.SetFrequency(0.02f); // Ajusta la frecuencia para controlar la apariencia de los vórtices
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(5);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);

    float ox = 3000.0f;
    float oy = 3000.0f;
    float zoom = 500.0f;

    // Generar valor de ruido
    float noiseValue = noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom);

    // Mapear el valor de ruido al rango de colores
    float t = (noiseValue + 1.0f) / 2.0f; // Asegura que esté en el rango [0, 1]

    // Mezcla los colores del vórtice basados en el valor de ruido
    Color vortexColor = vortexColor1 * (1.0f - t) + vortexColor2 * t;

    fragment.color = vortexColor;
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

    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, (fragment.original.z * zoom)));

    // Definir las coordenadas de los polos norte y sur
    glm::vec2 northPoleCoords(0.0f, 1.0f); // Polo norte en la parte superior
    glm::vec2 southPoleCoords(0.0f, -1.0f); // Polo sur en la parte inferior

    // Calcular la distancia desde el fragmento a los polos norte y sur
    float distanceToNorthPole = glm::distance(fragmentCoords, northPoleCoords);
    float distanceToSouthPole = glm::distance(fragmentCoords, southPoleCoords);

    // Definir un radio para las manchas polares
    float polarRadius = 0.1f;

    // Verificar si el fragmento está dentro del radio de alguno de los polos
    if (distanceToNorthPole < polarRadius || distanceToSouthPole < polarRadius) {
        // Está en uno de los polos, por lo que se muestra el color polar
        fragment.color = polar;
    } else {
        // No está en un polo, mezcla el color de la tierra y el océano
        float threshold = 0.1f;
        if (noiseValue < threshold) {
            Color tmpColor = ocean;
            fragment.color = tmpColor * fragment.z;
        } else {
            float gradient = (noiseValue - threshold) / (1.0f - threshold);
            fragment.color = earth * (1.0f - gradient) + ocean * gradient;
        }
    }
    return fragment.color;
}

Color gasPlanetV3(Fragment& fragment) {
    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Define los colores para el planeta gaseoso
    Color bandColor1 = Color(0, 200, 50); // Color de las bandas
    Color bandColor2 = Color(255, 255, 255); // Color más claro para las bandas

    // Configuración del ruido
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(1337);
    noise.SetFrequency(0.01f); // Ajusta la frecuencia para controlar la apariencia de las bandas
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(5);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);

    float ox = 3000.0f;
    float oy = 3000.0f;
    float zoom = 500.0f;

    // Generar valor de ruido
    float noiseValue = noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom);

    // Mapear el valor de ruido al rango de colores
    float t = (noiseValue + 1.0f) / 2.0f; // Asegura que esté en el rango [0, 1]

    // Mezcla los colores de las bandas basados en el valor de ruido
    Color bandColor = bandColor1 * (1.0f - t) + bandColor2 * t;

    fragment.color = bandColor;
    return fragment.color;
}

Color rockPlanet(Fragment& fragment) {
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    Color rockColor1 = Color(160, 160, 160); // Color de la roca principal
    Color rockColor2 = Color(100, 100, 100); // Color de la roca secundaria
    Color craterColor = Color(40, 40, 40);   // Color de los cráteres

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(4321); // Cambia la semilla para variaciones
    noise.SetFrequency(0.02f);
    noise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
    noise.SetFractalOctaves(6);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);

    float ox = 3000.0f;
    float oy = 3000.0f;
    float zoom = 200.0f;

    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom));

    // Definir un umbral para la transición entre los colores de la roca y los cráteres
    float threshold = 0.2f;

    if (noiseValue < threshold) {
        // Usar el color de la roca principal para la mayor parte del planeta
        fragment.color = rockColor1;
    } else {
        // Agregar detalles de cráteres a medida que nos alejamos del color de la roca principal
        float gradient = (noiseValue - threshold) / (1.0f - threshold);
        fragment.color = rockColor1 * (1.0f - gradient) + craterColor * gradient;
    }
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


Color starPlanetGreen(Fragment& fragment) {
    FastNoiseLite noise;
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);
    float scale = 100000.0f;

    if (true) {
        // Utiliza FastNoiseLite para generar un valor de ruido aleatorio
        float noiseValue = noise.GetNoise(fragmentCoords.x * scale, fragmentCoords.y * scale);

        // Define un umbral de brillo para las estrellas
        float brightnessThreshold = 0.2f; // Ajusta esto según lo deseado

        // Comprueba si el valor de ruido es lo suficientemente alto para ser una estrella
        if (noiseValue > brightnessThreshold) {
            // Define el color de las estrellas (por ejemplo, blanco)
            Color starColor(0, 100, 0);
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


Color gasPlanet(Fragment& fragment) {

    FastNoiseLite noise;
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);
    float scale = 100000.0f;

    if (true) {
        // Utiliza FastNoiseLite para generar un valor de ruido aleatorio
        float noiseValue = noise.GetNoise(fragmentCoords.x * scale, fragmentCoords.y * scale);

        // Define un umbral de brillo para las estrellas
        float brightnessThreshold = 0.2f; // Ajusta esto según lo deseado

        // Comprueba si el valor de ruido es lo suficientemente alto para ser una estrella
        if (noiseValue > brightnessThreshold) {
            // Define el color de las estrellas (por ejemplo, blanco)
            Color starColor(100, 0, 0);
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

Color starship(Fragment& fragment) {
    Color shipColor(255, 0, 0);

    fragment.color = shipColor;
    return fragment.color;
}