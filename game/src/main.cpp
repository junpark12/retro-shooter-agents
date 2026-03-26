// Galaxy Storm - Entry Point (skeleton)
// This file will be fully implemented by the Dev agent.

#include <SDL.h>

int main(int argc, char* argv[]) {
    // TODO: Implement SDL2 initialization and game loop
    // This skeleton exists so the project structure is valid.
    // Assign the Phase 2 Dev issues to @copilot to implement.

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Galaxy Storm",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        480, 640,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Placeholder: simple loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
