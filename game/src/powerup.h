#pragma once
#include "entity.h"

namespace galaxy {

// ─── PowerUp ──────────────────────────────────────────────────────────────────
struct PowerUp : Entity {
    PowerUpType type = PowerUpType::SPREAD;
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
void renderPowerUps(SDL_Renderer* renderer, const PowerUpPool& pp);

} // namespace galaxy
