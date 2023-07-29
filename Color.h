#include <iostream>

struct Color {

private:
    unsigned char r;
    unsigned char g;
    unsigned char b;

public:
    Color(unsigned char red, unsigned char green, unsigned char blue) {
        // Clamping de los valores de color para asegurarse de que estén en el rango correcto
        r = (red > 255) ? 255 : (red < 0) ? 0 : red;
        g = (green > 255) ? 255 : (green < 0) ? 0 : green;
        b = (blue > 255) ? 255 : (blue < 0) ? 0 : blue;
    }
    Color(): r(0), g(0), b(0) {}

    // Operador de igualdad (override del operador ==)
    bool operator==(const Color& other) const {
        return (r == other.r) && (g == other.g) && (b == other.b);
    }

    // Operador de desigualdad (override del operador !=)
    bool operator!=(const Color& other) const {
        return !(*this == other);
    }

    // Operador de impresión (override del operador <<)
    friend std::ostream& operator<<(std::ostream& os, const Color& color) {
        os << "RGB(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b) << ")";
        return os;
    }

    // Operador de suma (override del operador +)
    Color operator+(const Color& other) const {
        unsigned char newR = (r + other.r > 255) ? 255 : r + other.r;
        unsigned char newG = (g + other.g > 255) ? 255 : g + other.g;
        unsigned char newB = (b + other.b > 255) ? 255 : b + other.b;
        return {newR, newG, newB};
    }

    // Operador de multiplicación por float (override del operador *)
    Color operator*(float scalar) const {
        auto newR = static_cast<unsigned char>(r * scalar);
        auto newG = static_cast<unsigned char>(g * scalar);
        auto newB = static_cast<unsigned char>(b * scalar);
        return {newR, newG, newB};
    }
};