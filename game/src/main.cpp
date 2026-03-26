// Galaxy Storm — Entry Point
// Initialises SDL2, runs the game loop, then shuts down.

#include "game.h"
#include <SDL.h>

int main(int /*argc*/, char* /*argv*/[]) {
    galaxy::Game game;

    if (!game.init()) {
        return 1;
    }

    game.run();
    game.shutdown();
    return 0;
}
