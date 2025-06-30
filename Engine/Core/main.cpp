#include <iostream>

#include "Engine.h"
int main() {
    try
    {
        Engine engine;
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}