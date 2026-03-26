#pragma once
#include "types.h"
#include <SDL.h>

namespace galaxy {

// ─── Menu ─────────────────────────────────────────────────────────────────────

enum class MenuOption {
    START = 0,
    QUIT  = 1,
    COUNT = 2
};

struct Menu {
    int   selectedOption = 0;     // currently highlighted option index
    float blinkTimer     = 0.0f;  // used to blink the selection cursor
    bool  blinkVisible   = true;
};

// Update the menu: advance blink timer.
void updateMenu(Menu& m, float dt);

// Handle a single SDL event for menu navigation.
// Returns the GameState to transition to:
//   - GameState::PLAYING if the player selected START
//   - GameState::TITLE   otherwise (no transition)
GameState handleMenuEvent(Menu& m, const SDL_Event& e);

// Render the title screen:
//   - "GALAXY STORM" logo in neon letters (top third)
//   - Decorative star background (static, a few bright stars)
//   - Option list: "START", "QUIT" with blinking cursor on selected option
//   - "© 1994 GALAXY SOFT" footer
void renderMenu(SDL_Renderer* renderer, const Menu& m);

} // namespace galaxy
