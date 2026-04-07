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
               TTF_Font* font, const Player& player, int stageNum, int hiScore = 0);

// Charge bar drawn near the player when charging (shows charge progress 0–1).
// chargeRatio = player.chargeTimer / CHARGE_TIME, clamped to [0,1].
void renderChargeBar(SDL_Renderer* renderer, int playerX, int playerY, float chargeRatio);

// Power-up timer countdown bar drawn in the HUD.
// powerUpCount = remaining shots; shieldTimer = remaining seconds (>0 for shield).
void renderPowerUpTimer(SDL_Renderer* renderer, TTF_Font* font,
                        int powerUpCount, float shieldTimer, bool hasPowerUp);

// Boss HP bar drawn at the top of the screen during boss fight.
// displayHp is the lerped visual HP value (smooth animation); pass boss.displayHp.
void renderBossHP(SDL_Renderer* renderer, TTF_Font* font,
                  int currentHp, int maxHp, int phase, float displayHp = -1.0f);

// Renders the boss attack warning flash (red tint on boss body area) when attackWarning is true.
void renderBossAttackWarning(SDL_Renderer* renderer, int bossX, int bossY, int bossW, int bossH,
                             float warningRatio);

// Full-screen "STAGE CLEAR!" overlay displayed between stages.
void renderStageClear(SDL_Renderer* renderer, TTF_Font* font,
                      int stageNum, int score);

// Full-screen "GAME OVER" overlay.
void renderGameOver(SDL_Renderer* renderer, TTF_Font* font, int score);

// Full-screen "CONTINUE?" countdown overlay (shown when checkpoint was reached).
// countdown is 0–9; when it reaches 0, game over.
void renderContinue(SDL_Renderer* renderer, TTF_Font* font, int countdown);

// Full-screen "VICTORY!" overlay shown after clearing all 3 stages.
void renderVictory(SDL_Renderer* renderer, TTF_Font* font, int score);

// Small text rendering utility (delegates to SDL2_ttf or pixel font fallback).
void renderText(SDL_Renderer* renderer, TTF_Font* font,
                const char* text, int x, int y, SDL_Color color);

// Flashing "WARNING!!" overlay displayed before boss appearance.
// alpha cycles based on timer for a blinking effect.
void renderWarning(SDL_Renderer* renderer, TTF_Font* font, float timer);

// Combo counter display shown near the top-right during active combos.
// Tier visual: 2–4 (white), 5–9 (yellow), 10–19 (orange), 20+ (red pulse).
void renderCombo(SDL_Renderer* renderer, TTF_Font* font,
                 int comboCount, float comboTimer);

// "PAUSED" overlay displayed when the game is paused.
void renderPaused(SDL_Renderer* renderer, TTF_Font* font);

} // namespace galaxy
