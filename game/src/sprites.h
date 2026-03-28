#pragma once
#include "types.h"
#include "asset_manager.h"
#include <SDL.h>

namespace galaxy {

struct Player;  // defined in player.h
struct Enemy;   // defined in enemy.h
struct Boss;    // defined in boss.h
struct Bullet;  // defined in bullet.h

// ─── Sprites ──────────────────────────────────────────────────────────────────
// All functions draw to the given renderer at the given (x, y) position.
// x, y refer to the top-left corner of the sprite bounding box.
// If a texture is available via AssetManager, it is used; otherwise falls back
// to SDL2 primitive rendering (neon palette).

// Player ship: size ~28×36 px. Ship type determines sprite.
void renderPlayerSprite(SDL_Renderer* renderer, const AssetManager& assets,
                        int x, int y, ShipType ship, bool invincible = false,
                        int animFrame = 0);

// Enemy sprite; shape and colour vary by type.
void renderEnemySprite(SDL_Renderer* renderer, const AssetManager& assets,
                       int x, int y, EnemyType type, bool lockedOn = false);

// Boss sprite differs by stage number (1–3).
// Optionally draws a lock-on ring overlay.
void renderBossSprite(SDL_Renderer* renderer, const AssetManager& assets,
                      int x, int y, int stageNum, bool lockedOn = false,
                      int phase = 1);

// Bullet sprite. colorIdx selects a color variant (0–3) for Bullet Hell variety.
void renderBulletSprite(SDL_Renderer* renderer, const AssetManager& assets,
                        int x, int y, BulletOwner owner, int colorIdx = 0);

// Power-up sprite; colour and shape by type.
void renderPowerUpSprite(SDL_Renderer* renderer, const AssetManager& assets,
                         int x, int y, PowerUpType type);

// Particle explosion burst; frame 0..7 expands the burst radius.
void renderExplosion(SDL_Renderer* renderer, const AssetManager& assets,
                     int x, int y, int frame, bool big = false);

// Lock-on reticle drawn around a target.
void renderLockOnReticle(SDL_Renderer* renderer, int x, int y,
                         int w, int h, float timer);

// Bomb flash overlay (full-screen white flash when bomb is activated).
void renderBombFlash(SDL_Renderer* renderer, float bombTimer);

// Ship preview for ship select screen.
void renderShipPreview(SDL_Renderer* renderer, const AssetManager& assets,
                       int x, int y, ShipType ship, bool selected);

// ─── Fallback primitive renderers (used when no texture is available) ─────────
void renderPlayerPrimitive(SDL_Renderer* renderer, int x, int y, ShipType ship);
void renderEnemyPrimitive(SDL_Renderer* renderer, int x, int y, EnemyType type);
void renderBossPrimitive(SDL_Renderer* renderer, int x, int y, int stageNum);
void renderBulletPrimitive(SDL_Renderer* renderer, int x, int y,
                           BulletOwner owner, int colorIdx);
void renderPowerUpPrimitive(SDL_Renderer* renderer, int x, int y, PowerUpType type);

} // namespace galaxy
