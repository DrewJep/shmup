#include "Game.h"
#include <iostream>
#include <exception>

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unhandled unknown exception caught during runtime." << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}

