#pragma once
#include "types.h"
#include "asset_manager.h"
#include <SDL.h>
#include <SDL_ttf.h>

namespace galaxy {

// ─── Menu ─────────────────────────────────────────────────────────────────────

enum class MenuOption {
    START       = 0,
    HIGH_SCORE  = 1,
    QUIT        = 2,
    COUNT       = 3
};

struct Menu {
    int   selectedOption = 0;     // currently highlighted option index
    float blinkTimer     = 0.0f;  // used to blink the selection cursor
    bool  blinkVisible   = true;
    float animTimer      = 0.0f;  // title animation timer
};

// ─── ShipSelect ───────────────────────────────────────────────────────────────

struct ShipSelect {
    int   selectedShip  = 0;     // 0 = BAGON, 1 = DAMUL, 2 = GUNEX
    float blinkTimer    = 0.0f;
    bool  blinkVisible  = true;
    float previewTimer  = 0.0f;  // for animated previews
};

// Update the menu: advance blink timer.
void updateMenu(Menu& m, float dt);

// Handle a single SDL event for menu navigation.
// Returns the GameState to transition to:
//   - GameState::SHIP_SELECT  if the player selected START
//   - GameState::HIGH_SCORE   if the player selected HIGH SCORE
//   - GameState::QUIT         if QUIT selected
//   - GameState::TITLE        otherwise (no transition)
GameState handleMenuEvent(Menu& m, const SDL_Event& e);

// Render the title screen with SDL2_ttf text and asset sprites.
// hiScore is the current all-time high score to display.
void renderMenu(SDL_Renderer* renderer, const AssetManager& assets,
                TTF_Font* font, const Menu& m, int hiScore = 0);

// Handle a single SDL event on the high score screen.
// Returns GameState::TITLE when the player presses back.
GameState handleHighScoreEvent(const SDL_Event& e);

// Render the high score screen.
void renderHighScore(SDL_Renderer* renderer, TTF_Font* font, int hiScore);

// Update ship select screen.
void updateShipSelect(ShipSelect& ss, float dt);

// Handle input for ship selection.
// Returns GameState::PLAYING (with chosen ship) or GameState::TITLE (back).
// Sets selectedShip via outShip parameter.
GameState handleShipSelectEvent(ShipSelect& ss, const SDL_Event& e,
                                ShipType& outShip);

// Render ship select screen: show 3 ship previews with stats and description.
void renderShipSelect(SDL_Renderer* renderer, const AssetManager& assets,
                      TTF_Font* font, const ShipSelect& ss);

} // namespace galaxy
