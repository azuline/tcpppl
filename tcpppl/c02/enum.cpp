#include <cstdio>
#include <iostream>

enum class Color { red, blue, green };

// using enum Color;

int main() {
    std::printf("Color::green=%d", Color::green);
}
