#pragma once
#include "types.h"
#include "asset_manager.h"
#include <SDL.h>

namespace galaxy {

// ─── PowerUp ──────────────────────────────────────────────────────────────────
struct PowerUp : Entity {
    PowerUpType type      = PowerUpType::SPREAD;
    float       bobTimer  = 0.0f;  // for bobbing animation
    int         bobDir    = 1;
};

// Fixed-size pool.
struct PowerUpPool {
    PowerUp pool[MAX_POWERUPS];
};

// Activate the first inactive power-up at the given position.
void spawnPowerUp(PowerUpPool& pp, Vec2 pos, PowerUpType type);

// Move all active power-ups downward and deactivate when off-screen.
void updatePowerUps(PowerUpPool& pp, float dt);

// Render all active power-ups.
void renderPowerUps(SDL_Renderer* renderer, const AssetManager& assets,
                    const PowerUpPool& pp);

} // namespace galaxy
