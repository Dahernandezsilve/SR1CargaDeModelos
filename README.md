# Proyecto de Gráficas por Computadora - Universidad del Valle de Guatemala

## Descripción
Este proyecto es parte del curso de Gráficas por Computadora de la Universidad del Valle de Guatemala (UVG). Es una aplicación de renderizado que utiliza la biblioteca SDL2 para dibujar un modelo 3D en una ventana. El modelo 3D se carga desde un archivo en formato OBJ y se muestra en pantalla utilizando la técnica de triangulación de caras. El objetivo de este proyecto es aprender conceptos fundamentales de gráficas por computadora y aplicarlos para renderizar objetos 3D.

## Dependencias
- SDL2: Biblioteca utilizada para crear ventanas y renderizar gráficos.
- GLM: Biblioteca matemática para operaciones con vectores y matrices.

## Clases y estructuras

### Clase `Color`
- Descripción: Representa un color RGBA (rojo, verde, azul y alfa) de 8 bits.
- Atributos:
  - `r`: Valor del componente rojo (0-255).
  - `g`: Valor del componente verde (0-255).
  - `b`: Valor del componente azul (0-255).
  - `a`: Valor del componente alfa (0-255).

### Estructura `Face`
- Descripción: Representa una cara de un objeto 3D con tres índices de vértices que forman un triángulo.
- Atributos:
  - `vertexIndices`: Arreglo de tres enteros que almacena los índices de los vértices que forman el triángulo.

## Funciones

### `point(int x, int y)`
- Descripción: Dibuja un punto en la posición (x, y) en el renderer actual utilizando SDL_RenderDrawPoint.
- Parámetros:
  - `x`: Coordenada x del punto.
  - `y`: Coordenada y del punto.
- Retorna: Nada.

### `line(const glm::vec3& start, const glm::vec3& end)`
- Descripción: Dibuja una línea desde el punto de inicio hasta el punto final en el renderer actual utilizando SDL_RenderDrawLine.
- Parámetros:
  - `start`: Vector 3D que representa el punto de inicio de la línea.
  - `end`: Vector 3D que representa el punto final de la línea.
- Retorna: Nada.

### `triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)`
- Descripción: Dibuja un triángulo conectando los puntos A, B y C en el renderer actual utilizando la función `line`.
- Parámetros:
  - `A`: Vector 3D que representa el primer vértice del triángulo.
  - `B`: Vector 3D que representa el segundo vértice del triángulo.
  - `C`: Vector 3D que representa el tercer vértice del triángulo.
- Retorna: Nada.

### `render(const std::vector<glm::vec3>& vertexArray)`
- Descripción: Renderiza el modelo 3D en la ventana utilizando la técnica de triangulación de caras.
- Parámetros:
  - `vertexArray`: Vector de vectores 3D que contiene los vértices transformados del modelo.
- Retorna: Nada.

### `std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces)`
- Descripción: Convierte la información de las caras en un solo vector de vértices escalados según la escala especificada.
- Parámetros:
  - `vertices`: Vector de vectores 3D que representa los vértices originales del modelo.
  - `faces`: Vector de estructuras `Face` que contiene la información de las caras del modelo.
- Retorna: Vector de vectores 3D que representa los vértices transformados del modelo con la escala aplicada.

### `bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces)`
- Descripción: Carga un archivo en formato OBJ y extrae los vértices y las caras del modelo 3D.
- Parámetros:
  - `path`: Ruta del archivo OBJ a cargar.
  - `out_vertices`: Vector de vectores 3D donde se almacenarán los vértices del modelo.
  - `out_faces`: Vector de estructuras `Face` donde se almacenarán las caras del modelo.
- Retorna: `true` si la carga del archivo fue exitosa, `false` si hubo un error.

## Uso
1. Compilar el programa utilizando un compilador C++ compatible con C++11 y las bibliotecas SDL2 y GLM.
2. Ejecutar el programa desde la línea de comandos, proporcionando la ruta al archivo OBJ del modelo 3D a cargar. Ejemplo: `./programa ../modelo.obj`.
3. Se abrirá una ventana que mostrará el modelo 3D cargado desde el archivo OBJ. Puedes manipular la vista del modelo utilizando las funciones de la ventana (rotar, acercar, alejar) y cerrar la ventana haciendo clic en la "X" o presionando Alt + F4.

---

Este proyecto es una práctica académica desarrollada por un estudiante de la Universidad del Valle de Guatemala (UVG) en el marco del curso de Gráficas por Computadora. Se ha creado con fines educativos y para aprender conceptos fundamentales de gráficas por computadora y renderizado 3D. Siéntete libre de revisar el código y utilizarlo como referencia para tus propios proyectos. ¡Disfruta explorando el mundo de las gráficas por computadora!
