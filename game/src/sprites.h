#pragma once
#include "types.h"
#include <SDL.h>

namespace galaxy {

// ─── Sprites ──────────────────────────────────────────────────────────────────
// All functions draw to the given renderer at the given (x, y) position.
// x, y refer to the top-left corner of the sprite bounding box.
// No state is maintained — these are pure rendering utilities.

// Player ship: cyan delta-wing triangle, ~28×36 px.
void renderPlayerSprite(SDL_Renderer* renderer, int x, int y);

// Enemy sprite; shape and colour vary by type:
//   SMALL  — small magenta diamond, ~16×16 px
//   MEDIUM — neon-green hexagon,    ~24×24 px
//   LARGE  — red angular wedge,     ~32×28 px
void renderEnemySprite(SDL_Renderer* renderer, int x, int y, EnemyType type);

// Boss sprite differs by stage number (1–3):
//   Stage 1 — wide cyan battleship,   ~64×48 px
//   Stage 2 — yellow rotating cruiser,~72×56 px
//   Stage 3 — magenta fortress,       ~80×64 px
void renderBossSprite(SDL_Renderer* renderer, int x, int y, int stageNum);

// Bullet sprite:
//   PLAYER — bright cyan pill,        ~4×12 px
//   ENEMY  — magenta orb,             ~6×6 px
void renderBulletSprite(SDL_Renderer* renderer, int x, int y, BulletOwner owner);

// Power-up sprite; colour and shape by type:
//   SPREAD  — yellow 5-point star,   ~18×18 px
//   LASER   — cyan vertical bar,     ~8×24 px
//   MISSILE — green chevron,         ~14×18 px
//   SHIELD  — white circle outline,  ~20×20 px
void renderPowerUpSprite(SDL_Renderer* renderer, int x, int y, PowerUpType type);

// Particle explosion burst; frame 0..7 expands the burst radius.
void renderExplosion(SDL_Renderer* renderer, int x, int y, int frame);

} // namespace galaxy
