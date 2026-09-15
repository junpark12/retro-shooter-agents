#pragma once

#include "types.h"
#include <SDL.h>

namespace galaxy {

inline bool isInitialKeyPress(const SDL_Event& event) {
    return event.type == SDL_KEYDOWN && event.key.repeat == 0;
}

inline GameState handlePauseEvent(GameState state, const SDL_Event& event) {
    if (state == GameState::PLAYING && event.type == SDL_WINDOWEVENT &&
        (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
         event.window.event == SDL_WINDOWEVENT_MINIMIZED)) {
        return GameState::PAUSED;
    }
    if (!isInitialKeyPress(event)) return state;
    if (event.key.keysym.scancode != SDL_SCANCODE_P &&
        event.key.keysym.scancode != SDL_SCANCODE_ESCAPE) return state;
    if (state == GameState::PLAYING) return GameState::PAUSED;
    if (state == GameState::PAUSED) return GameState::PLAYING;
    return state;
}

} // namespace galaxy
