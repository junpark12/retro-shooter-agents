#pragma once
#include "types.h"
#include <SDL.h>

namespace galaxy {

struct Player; // defined in player.h

// ─── HUD ──────────────────────────────────────────────────────────────────────
// All rendering is done with SDL2 primitives (SDL_RenderFillRect).
// A simple 5×7 pixel-font is used for all text.

// Draw the in-game HUD:
//   - Score (top-left)
//   - Stage number (top-centre)
//   - Lives remaining (bottom-left, as small ship icons)
//   - Power-up type indicator (bottom-right)
//   - Shield bar when shieldTimer > 0
void renderHUD(SDL_Renderer* renderer, const Player& player, int stageNum);

// Full-screen "STAGE CLEAR!" overlay displayed between stages.
void renderStageClear(SDL_Renderer* renderer, int stageNum, int score);

// Full-screen "GAME OVER" overlay.
void renderGameOver(SDL_Renderer* renderer, int score);

// Full-screen "VICTORY!" overlay shown after clearing all 3 stages.
void renderVictory(SDL_Renderer* renderer, int score);

} // namespace galaxy
