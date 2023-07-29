//
// Created by daher on 13/07/2023.
//

#include "point.h"
void point(const Vertex2& vertex,  const Color& color) {
    // Obtener las coordenadas enteras del vértice
    int x = static_cast<int>(vertex.x);
    int y = static_cast<int>(vertex.y);

    // Verificar si el punto está dentro de los límites del framebuffer
    if (x >= 0 && x < framebufferWidth && y >= 0 && y < framebufferHeight) {
        // Calcular el índice correspondiente en el framebuffe

        // Establecer el color actual en el punto del framebuffer
        framebuffer[y][x] = color;
    }
}