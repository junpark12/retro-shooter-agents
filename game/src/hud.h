#pragma once
#include "types.h"
#include "asset_manager.h"
#include <SDL.h>
#include <SDL_ttf.h>

namespace galaxy {

struct Player; // defined in player.h

// ─── HUD ──────────────────────────────────────────────────────────────────────
// Rendering uses SDL2_ttf for text and SDL2 primitives / sprites for icons.

// Draw the in-game HUD:
//   - Score (top-left), Hi-Score (top-centre)
//   - Stage number indicator
//   - Lives remaining (life icons, bottom-left)
//   - Bomb stock (bomb icons, bottom-left)
//   - Power level bar (bottom-right)
//   - Power-up type indicator
//   - Lock-on status indicator
//   - Shield/invincibility bar when active
void renderHUD(SDL_Renderer* renderer, const AssetManager& assets,
               TTF_Font* font, const Player& player, int stageNum);

// Boss HP bar drawn at the top of the screen during boss fight.
void renderBossHP(SDL_Renderer* renderer, TTF_Font* font,
                  int currentHp, int maxHp, int phase);

// Full-screen "STAGE CLEAR!" overlay displayed between stages.
void renderStageClear(SDL_Renderer* renderer, TTF_Font* font,
                      int stageNum, int score);

// Full-screen "GAME OVER" overlay.
void renderGameOver(SDL_Renderer* renderer, TTF_Font* font, int score);

// Full-screen "VICTORY!" overlay shown after clearing all 3 stages.
void renderVictory(SDL_Renderer* renderer, TTF_Font* font, int score);

// Small text rendering utility (delegates to SDL2_ttf or pixel font fallback).
void renderText(SDL_Renderer* renderer, TTF_Font* font,
                const char* text, int x, int y, SDL_Color color);

} // namespace galaxy
